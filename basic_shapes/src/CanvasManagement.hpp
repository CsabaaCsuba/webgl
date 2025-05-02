#pragma once
#include <emscripten/html5.h>
#include <iostream>

EM_BOOL immerse(int eventType, const EmscriptenMouseEvent *e, void *userData) {
    // Request pointer lock on the canvas
    emscripten_request_pointerlock("#canvas", EM_TRUE);
    return EM_TRUE;
}

EM_BOOL mouse_move(int eventType, const EmscriptenMouseEvent *e, void *userData) {
    double dx = e->movementX;
    double dy = e->movementY;

    std::cout << "mouse moved to: " << dx << "x, " << dy << "y" << std::endl;
    // Use dx, dy to control camera, etc.
    return EM_TRUE;
}