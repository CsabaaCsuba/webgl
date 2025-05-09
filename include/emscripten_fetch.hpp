#pragma once
#include <string>
#include <string.h>
#include <emscripten/html5.h>
#include <emscripten/fetch.h>
#include <vector>
#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

// Image structure
struct Image {
    unsigned char* data = nullptr;
    int width = 0;
    int height = 0;
    int channels = 0;

    ~Image() {
        if (data) {
            stbi_image_free(data);
        }
    }
};


// Structure to pass data between fetch callbacks
struct FetchData {
    std::string content;
    bool completed = false;
    bool success = false;
};

// Called when fetch succeeds
void downloadSucceeded(emscripten_fetch_t *fetch) {
    FetchData* data = (FetchData*)fetch->userData;
    
    if (fetch->data && fetch->numBytes > 0) {
        data->content.assign(fetch->data, fetch->numBytes);
        data->success = true;
    }
    
    data->completed = true;
    emscripten_fetch_close(fetch); // Free resources
}

// Called when fetch fails
void downloadFailed(emscripten_fetch_t *fetch) {
    FetchData* data = (FetchData*)fetch->userData;
    data->completed = true;
    data->success = false;
    emscripten_fetch_close(fetch);
}

// Main download function (synchronous-style using async fetch)
std::string downloadContent(const std::string& url) {
    FetchData fetchData;
    
    emscripten_fetch_attr_t attr;
    emscripten_fetch_attr_init(&attr);
    strcpy(attr.requestMethod, "GET");
    
    // Configure fetch attributes
    attr.attributes = EMSCRIPTEN_FETCH_LOAD_TO_MEMORY;
    attr.onsuccess = downloadSucceeded;
    attr.onerror = downloadFailed;
    attr.userData = &fetchData;
    
    // Start the fetch
    emscripten_fetch(&attr, url.c_str());
    
    // Wait for completion (in a real app, you'd use async patterns)
    while (!fetchData.completed) {
        emscripten_sleep(100); // Don't block the main thread indefinitely
    }
    
    if (fetchData.success) {
        return fetchData.content;
    }
    return "";
}

// Structure to pass data between fetch callbacks for binary data
struct BinaryFetchData {
    std::vector<uint8_t> content;  // Using vector<uint8_t> for binary data
    bool completed = false;
    bool success = false;
};

// Called when binary fetch succeeds
void downloadBinarySucceeded(emscripten_fetch_t *fetch) {
    BinaryFetchData* data = (BinaryFetchData*)fetch->userData;
    
    if (fetch->data && fetch->numBytes > 0) {
        // Copy raw bytes to our vector
        data->content.resize(fetch->numBytes);
        memcpy(data->content.data(), fetch->data, fetch->numBytes);
        data->success = true;
    }
    
    data->completed = true;
    emscripten_fetch_close(fetch); // Free resources
}

// Called when binary fetch fails
void downloadBinaryFailed(emscripten_fetch_t *fetch) {
    BinaryFetchData* data = (BinaryFetchData*)fetch->userData;
    data->completed = true;
    data->success = false;
    emscripten_fetch_close(fetch);
}

// Download binary content (synchronous-style using async fetch)
std::vector<uint8_t> downloadBinaryContent(const std::string& url) {
    BinaryFetchData fetchData;
    
    emscripten_fetch_attr_t attr;
    emscripten_fetch_attr_init(&attr);
    strcpy(attr.requestMethod, "GET");
    
    // Configure fetch attributes for binary data
    attr.attributes = EMSCRIPTEN_FETCH_LOAD_TO_MEMORY;
    attr.onsuccess = downloadBinarySucceeded;
    attr.onerror = downloadBinaryFailed;
    attr.userData = &fetchData;
    
    // Start the fetch
    emscripten_fetch(&attr, url.c_str());
    
    // Wait for completion (in a real app, you'd use async patterns)
    while (!fetchData.completed) {
        emscripten_sleep(100); // Sleep to avoid blocking indefinitely
    }
    
    if (fetchData.success) {
        return fetchData.content;
    }
    return std::vector<uint8_t>(); // Return empty vector on failure
}

// Helper function to save binary data to an in-memory file system (if needed)
bool saveBinaryToFS(const std::string& filename, const std::vector<uint8_t>& data) {
    FILE* file = fopen(filename.c_str(), "wb");
    if (!file) {
        return false;
    }
    
    size_t written = fwrite(data.data(), 1, data.size(), file);
    fclose(file);
    
    return written == data.size();
}

// Optional: Async version using callbacks
void downloadBinaryContentAsync(const std::string& url, 
                               std::function<void(const std::vector<uint8_t>&, bool)> callback) {
    BinaryFetchData* fetchData = new BinaryFetchData(); // Allocate dynamically
    
    emscripten_fetch_attr_t attr;
    emscripten_fetch_attr_init(&attr);
    strcpy(attr.requestMethod, "GET");
    
    // Configure fetch attributes
    attr.attributes = EMSCRIPTEN_FETCH_LOAD_TO_MEMORY;
    attr.userData = fetchData;
    
    // Custom callbacks to handle the async result
    attr.onsuccess = [](emscripten_fetch_t *fetch) {
        BinaryFetchData* data = (BinaryFetchData*)fetch->userData;
        
        if (fetch->data && fetch->numBytes > 0) {
            data->content.resize(fetch->numBytes);
            memcpy(data->content.data(), fetch->data, fetch->numBytes);
            data->success = true;
        }
        
        // Extract data before callback
        std::vector<uint8_t> content = data->content;
        bool success = data->success;
        
        // Get callback function pointer which was attached to the fetch data
        auto callback = *reinterpret_cast<std::function<void(const std::vector<uint8_t>&, bool)>*>(fetch->userData);
        
        // Clean up
        emscripten_fetch_close(fetch);
        delete data;
        
        // Execute callback with result
        callback(content, success);
    };
    
    attr.onerror = [](emscripten_fetch_t *fetch) {
        BinaryFetchData* data = (BinaryFetchData*)fetch->userData;
        
        // Get callback function which was attached to the fetch data
        auto callback = *reinterpret_cast<std::function<void(const std::vector<uint8_t>&, bool)>*>(fetch->userData);
        
        // Clean up
        emscripten_fetch_close(fetch);
        delete data;
        
        // Execute callback with empty result
        callback(std::vector<uint8_t>(), false);
    };
    
    // Start the fetch
    emscripten_fetch(&attr, url.c_str());
}



// Improved image loading function for better JPEG handling
Image LoadImageFromURL(const std::string& url) {
    std::vector<uint8_t> imageData = downloadBinaryContent(url);
    Image img;
    
    if (!imageData.empty()) {
        // Force 4 channels (RGBA) for consistent texture format
        img.data = stbi_load_from_memory(
            imageData.data(), 
            imageData.size(), 
            &img.width, 
            &img.height, 
            &img.channels, 
            4  // Force RGBA format (4 channels)
        );
        
        if (img.data) {
            // Always set channels to 4 since we forced RGBA
            img.channels = 4;
            printf("Successfully loaded image: %dx%d with %d channels\n", 
                  img.width, img.height, img.channels);
        } else {
            printf("Failed to decode image: %s\n", stbi_failure_reason());
        }
    } else {
        printf("Failed to fetch image data from URL: %s\n", url.c_str());
    }

    return img;
}