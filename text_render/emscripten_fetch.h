#pragma once
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <emscripten/html5.h>
#include <emscripten/fetch.h>
#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

// Image structure
typedef struct {
    unsigned char* data;
    int width;
    int height;
    int channels;
} Image;

// Create and initialize image
Image* Image_create() {
    Image* img = (Image*)malloc(sizeof(Image));
    if (img) {
        img->data = NULL;
        img->width = 0;
        img->height = 0;
        img->channels = 0;
    }
    return img;
}

// Free image resources
void Image_destroy(Image* img) {
    if (img) {
        if (img->data) {
            stbi_image_free(img->data);
        }
        free(img);
    }
}

// Structure to pass data between fetch callbacks for text content
typedef struct {
    char* content;
    size_t content_size;
    size_t content_capacity;
    int completed;
    int success;
} FetchData;

// Initialize fetch data structure
FetchData* FetchData_create() {
    FetchData* data = (FetchData*)malloc(sizeof(FetchData));
    if (data) {
        data->content = NULL;
        data->content_size = 0;
        data->content_capacity = 0;
        data->completed = 0;
        data->success = 0;
    }
    return data;
}

// Free fetch data resources
void FetchData_destroy(FetchData* data) {
    if (data) {
        if (data->content) {
            free(data->content);
        }
        free(data);
    }
}

// Called when fetch succeeds
void downloadSucceeded(emscripten_fetch_t *fetch) {
    FetchData* data = (FetchData*)fetch->userData;
    
    if (fetch->data && fetch->numBytes > 0) {
        // Allocate memory and copy data
        data->content = (char*)malloc(fetch->numBytes + 1); // +1 for null terminator
        if (data->content) {
            memcpy(data->content, fetch->data, fetch->numBytes);
            data->content[fetch->numBytes] = '\0'; // Null-terminate the string
            data->content_size = fetch->numBytes;
            data->content_capacity = fetch->numBytes + 1;
            data->success = 1;
        }
    }
    
    data->completed = 1;
    emscripten_fetch_close(fetch); // Free resources
}

// Called when fetch fails
void downloadFailed(emscripten_fetch_t *fetch) {
    FetchData* data = (FetchData*)fetch->userData;
    data->completed = 1;
    data->success = 0;
    emscripten_fetch_close(fetch);
}

// Main download function (synchronous-style using async fetch)
char* downloadContent(const char* url) {
    FetchData* fetchData = FetchData_create();
    if (!fetchData) {
        return NULL;
    }
    
    emscripten_fetch_attr_t attr;
    emscripten_fetch_attr_init(&attr);
    strcpy(attr.requestMethod, "GET");
    
    // Configure fetch attributes
    attr.attributes = EMSCRIPTEN_FETCH_LOAD_TO_MEMORY;
    attr.onsuccess = downloadSucceeded;
    attr.onerror = downloadFailed;
    attr.userData = fetchData;
    
    // Start the fetch
    emscripten_fetch(&attr, url);
    
    // Wait for completion (in a real app, you'd use async patterns)
    while (!fetchData->completed) {
        emscripten_sleep(100); // Don't block the main thread indefinitely
    }
    
    char* result = NULL;
    
    if (fetchData->success && fetchData->content) {
        // Create a copy of the content to return
        result = strdup(fetchData->content);
    }
    
    // Clean up fetch data
    FetchData_destroy(fetchData);
    
    return result;
}

// Structure to pass data between fetch callbacks for binary data
typedef struct {
    unsigned char* content;
    size_t content_size;
    size_t content_capacity;
    int completed;
    int success;
} BinaryFetchData;

// Initialize binary fetch data structure
BinaryFetchData* BinaryFetchData_create() {
    BinaryFetchData* data = (BinaryFetchData*)malloc(sizeof(BinaryFetchData));
    if (data) {
        data->content = NULL;
        data->content_size = 0;
        data->content_capacity = 0;
        data->completed = 0;
        data->success = 0;
    }
    return data;
}

// Free binary fetch data resources
void BinaryFetchData_destroy(BinaryFetchData* data) {
    if (data) {
        if (data->content) {
            free(data->content);
        }
        free(data);
    }
}

// Called when binary fetch succeeds
void downloadBinarySucceeded(emscripten_fetch_t *fetch) {
    BinaryFetchData* data = (BinaryFetchData*)fetch->userData;
    
    if (fetch->data && fetch->numBytes > 0) {
        // Allocate memory and copy binary data
        data->content = (unsigned char*)malloc(fetch->numBytes);
        if (data->content) {
            memcpy(data->content, fetch->data, fetch->numBytes);
            data->content_size = fetch->numBytes;
            data->content_capacity = fetch->numBytes;
            data->success = 1;
        }
    }
    
    data->completed = 1;
    emscripten_fetch_close(fetch); // Free resources
}

// Called when binary fetch fails
void downloadBinaryFailed(emscripten_fetch_t *fetch) {
    BinaryFetchData* data = (BinaryFetchData*)fetch->userData;
    data->completed = 1;
    data->success = 0;
    emscripten_fetch_close(fetch);
}

// Download binary content (synchronous-style using async fetch)
// Returns dynamically allocated buffer that must be freed by caller
// size_out will be set to the size of the returned buffer
unsigned char* downloadBinaryContent(const char* url, size_t* size_out) {
    BinaryFetchData* fetchData = BinaryFetchData_create();
    if (!fetchData) {
        if (size_out) *size_out = 0;
        return NULL;
    }
    
    emscripten_fetch_attr_t attr;
    emscripten_fetch_attr_init(&attr);
    strcpy(attr.requestMethod, "GET");
    
    // Configure fetch attributes for binary data
    attr.attributes = EMSCRIPTEN_FETCH_LOAD_TO_MEMORY;
    attr.onsuccess = downloadBinarySucceeded;
    attr.onerror = downloadBinaryFailed;
    attr.userData = fetchData;
    
    // Start the fetch
    emscripten_fetch(&attr, url);
    
    // Wait for completion (in a real app, you'd use async patterns)
    while (!fetchData->completed) {
        emscripten_sleep(100); // Sleep to avoid blocking indefinitely
    }
    
    unsigned char* result = NULL;
    size_t result_size = 0;
    
    if (fetchData->success && fetchData->content) {
        // Create a copy of the content to return
        result = (unsigned char*)malloc(fetchData->content_size);
        if (result) {
            memcpy(result, fetchData->content, fetchData->content_size);
            result_size = fetchData->content_size;
        }
    }
    
    // Set output size if pointer was provided
    if (size_out) {
        *size_out = result_size;
    }
    
    // Clean up fetch data
    BinaryFetchData_destroy(fetchData);
    
    return result;
}

// Helper function to save binary data to an in-memory file system
int saveBinaryToFS(const char* filename, const unsigned char* data, size_t data_size) {
    FILE* file = fopen(filename, "wb");
    if (!file) {
        return 0;
    }
    
    size_t written = fwrite(data, 1, data_size, file);
    fclose(file);
    
    return written == data_size;
}

// Callback function type for async binary download
typedef void (*BinaryDownloadCallback)(const unsigned char* data, size_t data_size, int success, void* user_data);

// Structure to hold data for async callback
typedef struct {
    BinaryDownloadCallback callback;
    void* user_data;
} AsyncCallbackData;

// Called when async binary fetch succeeds
void downloadBinaryAsyncSucceeded(emscripten_fetch_t *fetch) {
    AsyncCallbackData* callback_data = (AsyncCallbackData*)fetch->userData;
    
    if (fetch->data && fetch->numBytes > 0 && callback_data->callback) {
        // Call the user's callback with the data
        callback_data->callback(
            (const unsigned char*)fetch->data, 
            fetch->numBytes, 
            1, // success
            callback_data->user_data
        );
    } else if (callback_data->callback) {
        // Call with NULL if no data
        callback_data->callback(NULL, 0, 0, callback_data->user_data);
    }
    
    // Clean up
    emscripten_fetch_close(fetch);
    free(callback_data);
}

// Called when async binary fetch fails
void downloadBinaryAsyncFailed(emscripten_fetch_t *fetch) {
    AsyncCallbackData* callback_data = (AsyncCallbackData*)fetch->userData;
    
    if (callback_data->callback) {
        // Call the user's callback with failure
        callback_data->callback(NULL, 0, 0, callback_data->user_data);
    }
    
    // Clean up
    emscripten_fetch_close(fetch);
    free(callback_data);
}

// Async version using callbacks
void downloadBinaryContentAsync(const char* url, BinaryDownloadCallback callback, void* user_data) {
    // Allocate callback data structure
    AsyncCallbackData* callback_data = (AsyncCallbackData*)malloc(sizeof(AsyncCallbackData));
    if (!callback_data) {
        if (callback) {
            callback(NULL, 0, 0, user_data);
        }
        return;
    }
    
    callback_data->callback = callback;
    callback_data->user_data = user_data;
    
    emscripten_fetch_attr_t attr;
    emscripten_fetch_attr_init(&attr);
    strcpy(attr.requestMethod, "GET");
    
    // Configure fetch attributes
    attr.attributes = EMSCRIPTEN_FETCH_LOAD_TO_MEMORY;
    attr.onsuccess = downloadBinaryAsyncSucceeded;
    attr.onerror = downloadBinaryAsyncFailed;
    attr.userData = callback_data;
    
    // Start the fetch
    emscripten_fetch(&attr, url);
}

// Improved image loading function for better JPEG handling
Image* LoadImageFromURL(const char* url) {
    size_t imageData_size = 0;
    unsigned char* imageData = downloadBinaryContent(url, &imageData_size);
    Image* img = Image_create();
    
    if (imageData && imageData_size > 0 && img) {
        // Force 4 channels (RGBA) for consistent texture format
        img->data = stbi_load_from_memory(
            imageData, 
            (int)imageData_size, 
            &img->width, 
            &img->height, 
            &img->channels, 
            4  // Force RGBA format (4 channels)
        );
        
        if (img->data) {
            // Always set channels to 4 since we forced RGBA
            img->channels = 4;
            printf("Successfully loaded image: %dx%d with %d channels\n", 
                  img->width, img->height, img->channels);
        } else {
            printf("Failed to decode image: %s\n", stbi_failure_reason());
        }
    } else {
        printf("Failed to fetch image data from URL: %s\n", url);
    }
    
    // Free the downloaded binary data
    if (imageData) {
        free(imageData);
    }
    
    return img;
}