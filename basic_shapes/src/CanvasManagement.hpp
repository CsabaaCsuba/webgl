#pragma once
#include <emscripten/html5.h>
#include <iostream>

float mouseX = 0.0f;
float mouseY = 0.0f;

EM_BOOL immerse(int eventType, const EmscriptenMouseEvent *e, void *userData) {
    // Request pointer lock on the canvas
    emscripten_request_pointerlock("#canvas", EM_TRUE);
    return EM_TRUE;
}

EM_BOOL mouse_move(int eventType, const EmscriptenMouseEvent *e, void *userData) {
    Shape* cursorShape = (Shape*)userData;
    
    // Check if we have pointer lock
    EmscriptenPointerlockChangeEvent pointerLockStatus;
    emscripten_get_pointerlock_status(&pointerLockStatus);
    
    if (pointerLockStatus.isActive) {
        // In pointer lock mode, use movement deltas
        float dx = (float)e->movementX;
        float dy = (float)e->movementY;
        
        mouseX += dx;
        mouseY += dy;
        
        double canvasWidth, canvasHeight;
        emscripten_get_element_css_size("#canvas", &canvasWidth, &canvasHeight);
        
        // Constrain to canvas
        mouseX = fmax(0, fmin(mouseX, (float)canvasWidth));
        mouseY = fmax(0, fmin(mouseY, (float)canvasHeight));
    } else {
        // Without pointer lock, use absolute position
        mouseX = (float)e->clientX;
        mouseY = (float)e->clientY;
    }

    // std::cout << "Mouse position: " << mouseX << "x, " << mouseY << "y" << std::endl;
    
    if (cursorShape) {
        cursorShape->SetCoordinates(mouseX, mouseY);
    } else {
        std::cout << "Error: userData is null" << std::endl;
    }
    
    return EM_TRUE;
}