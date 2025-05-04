#pragma once
#include <emscripten/html5.h>
#include <iostream>


float WindowWidth, WindowHeight;
float mouseX, mouseY;
float boostSpeed = 0.04;
bool isBoosting = false;

EM_BOOL immerse(int eventType, const EmscriptenMouseEvent *e, void *userData) {
    Camera* camera = (Camera*)userData;
    WindowWidth = (float)camera->width;
    WindowHeight = (float)camera->height;
    mouseX = WindowWidth/2;
    mouseY = WindowHeight/2;
    // Request pointer lock on the canvas
    emscripten_request_pointerlock("#canvas", EM_TRUE);
    return EM_TRUE;
}

EM_BOOL mouse_move(int eventType, const EmscriptenMouseEvent *e, void *userData) {
    Camera* camera = (Camera*)userData;
    
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

        float width = (float)canvasWidth;
        float height = (float)canvasHeight;
        
        if (mouseX < 0) mouseX = 0;
        if (mouseX > WindowWidth) mouseX = WindowWidth;
        if (mouseY < 0) mouseY = 0;
        if (mouseY > WindowHeight) mouseY = WindowHeight;

        float rotX = camera->sensitivity * (float)(mouseY - (height / 2)) / height;
		float rotY = camera->sensitivity * (float)(mouseX - (width / 2)) / width;

		// Calculates upcoming vertical change in the Orientation
		glm::vec3 newOrientation = glm::rotate(camera->Orientation, glm::radians(-rotX), glm::normalize(glm::cross(camera->Orientation, camera->Up)));

		// Decides whether or not the next vertical Orientation is legal or not
		if (abs(glm::angle(newOrientation, camera->Up) - glm::radians(90.0f)) <= glm::radians(85.0f))
		{
			camera->Orientation = newOrientation;
		}

		// Rotates the Orientation left and right
		camera->Orientation = glm::rotate(camera->Orientation, glm::radians(-rotY), camera->Up);
        
        mouseX = WindowWidth/2;
        mouseY = WindowHeight/2;

    }

    // std::cout << "Mouse position: " << mouseX << "x, " << mouseY << "y" << std::endl;
    
    return EM_TRUE;
}

EM_BOOL key_callback(int eventType, const EmscriptenKeyboardEvent *e, void *userData){
    Camera* camera = (Camera*)userData;
    

    if (eventType == EMSCRIPTEN_EVENT_KEYDOWN)
    {
        printf("Key down: %s, code: %s\n", e->key, e->code);

        // Check for specific keys
        if (strcmp(e->code, "KeyW") == 0)
        {
            camera->Position += camera->speed * camera->Orientation;
        }
        if (strcmp(e->code, "KeyA") == 0)
        {
            camera->Position += camera->speed * -glm::normalize(glm::cross(camera->Orientation, camera->Up));
        }
        if (strcmp(e->code, "KeyS") == 0)
        {
            camera->Position += camera->speed * -camera->Orientation;
        }
        if (strcmp(e->code, "KeyD") == 0)
        {
            camera->Position += camera->speed * glm::normalize(glm::cross(camera->Orientation, camera->Up));
        }
        if (strcmp(e->code, "Space") == 0)
        {
            camera->Position += camera->speed * -camera->Up;
        }
        if (strcmp(e->code, "ControlLeft") == 0)
        {
            camera->Position += camera->speed * camera->Up;
        }
        if (strcmp(e->code, "ShiftLeft") == 0)
        {
            isBoosting = !isBoosting;
            camera->speed = 0.02 + boostSpeed * isBoosting;
        }
    }

    if (strlen(e->key) > 1)
    {
        printf("Special or non-ASCII key detected\n");
    }

    // You can also check properties like e->locale to get the user's language
    printf("User locale: %s\n", e->locale);

    // Return true to prevent the default browser action
    return EM_TRUE;
}