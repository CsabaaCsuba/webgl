#pragma once
#include <string>
#include <string.h>
#include <emscripten/html5.h>
#include <emscripten/fetch.h>

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
