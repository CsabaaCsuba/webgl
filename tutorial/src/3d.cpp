#include "glm/glm.hpp"
#include "glm/ext/matrix_clip_space.hpp"
#include "glm/ext/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"
#include <iostream>
#include <GLES3/gl3.h>
#include <EGL/egl.h>
#include <emscripten.h>
#include <emscripten/html5.h>
#include <cmath>
#include <chrono>
#include "ShaderClass.hpp"
#include "VAO.hpp"
#include "EBO.hpp"
#include "Texture.hpp"
#include "Camera.hpp"
#define STB_IMAGE_IMPLEMENTATION
#include <../include/stb_image.h>

VAO *VAO1;
Shader *shaderProgram;
Texture *popCat;
Camera *camera;
float width, height;
size_t indicesSize;

bool isButtonHidden = false;

// userData can be ignored for now...
EM_BOOL key_callback(int eventType, const EmscriptenKeyboardEvent *e, void *userData)
{
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
            ;
        }
        if (strcmp(e->code, "ShiftLeft") == 0)
        {
            camera->speed = 0.4f;
        }
    }
    else if (eventType == EMSCRIPTEN_EVENT_KEYUP)
    {
        printf("Key up: %s\n", e->key);
        if (strcmp(e->code, "ShiftLeft") == 0)
        {
            camera->speed = 0.1f;
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

EM_BOOL mouse_click_callback(int eventType, const EmscriptenMouseEvent *e, void *userData)
{
    if (eventType == EMSCRIPTEN_EVENT_MOUSEDOWN)
    {
        printf("Mouse clicked at coordinates: (%d, %d)\n", e->clientX, e->clientY);

        // Check which button was pressed
        if (e->button == 0)
        { // Left mouse button
            printf("Left button clicked\n");

            float rotX = camera->sensitivity * (float)(e->clientY - (height / 2)) / height;
            float rotY = camera->sensitivity * (float)(e->clientX - (width / 2)) / width;

            glm::vec3 right = glm::normalize(glm::cross(camera->Orientation, camera->Up));
            glm::vec3 newOrientation = glm::rotate(camera->Orientation, glm::radians(-rotX), right);

            if (!(glm::angle(newOrientation, camera->Up) <= glm::radians(5.0f)) &&
                !(glm::angle(newOrientation, -camera->Up) <= glm::radians(5.0f)))
            {
                camera->Orientation = newOrientation;
            }

            camera->Orientation = glm::rotate(camera->Orientation, glm::radians(-rotY), camera->Up);

            // Set cursor position to center of the screen
            // emscripten_set_mousemove_callback(EMSCRIPTEN_EVENT_TARGET_DOCUMENT, 0, 1, [](int, const EmscriptenMouseEvent* e, void*){
            //     emscripten_request_pointerlock("#canvas", 1);
            //     return EM_TRUE;
            // });
        }
        else if (e->button == 1)
        {
            printf("Middle button clicked\n");
        }
        else if (e->button == 2)
        {
            printf("Right button clicked\n");
        }

        // Check modifier keys
        if (e->ctrlKey)
            printf("Ctrl key was pressed during click\n");
        if (e->shiftKey)
            printf("Shift key was pressed during click\n");
        if (e->altKey)
            printf("Alt key was pressed during click\n");
    }

    return EM_TRUE; // Return true to prevent default browser action
}

void MainLoop()
{
    glClearColor(0.07f, 0.13f, 0.17f, 1.0f);
    // Clean the back buffer and assign the new color to it
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    // Tell OpenGL which Shader Program we want to use
    shaderProgram->Activate();

    camera->Matrix(45.0f, 0.1f, 100.0f, shaderProgram, "camMatrix");

    popCat->Bind();
    // Bind the VAO so OpenGL knows to use it
    VAO1->Bind();
    // Draw primitives, number of indices, datatype of indices, index of indices
    glDrawElements(GL_TRIANGLES, indicesSize / sizeof(int), GL_UNSIGNED_INT, 0);

    // this might not work
    eglSwapBuffers(eglGetCurrentDisplay(), eglGetCurrentSurface(EGL_DRAW));
}

int main()
{
    EmscriptenWebGLContextAttributes attrs;
    emscripten_webgl_init_context_attributes(&attrs);
    attrs.majorVersion = 2; // WebGL 2.0 corresponds to OpenGL ES 3.0
    attrs.minorVersion = 0;

    EMSCRIPTEN_WEBGL_CONTEXT_HANDLE context = emscripten_webgl_create_context("#canvas", &attrs);
    emscripten_webgl_make_context_current(context);

    double canvasWidth, canvasHeight;
    EMSCRIPTEN_RESULT res = emscripten_get_element_css_size("#canvas", &canvasWidth, &canvasHeight);

    glViewport(0, 0, (GLsizei)canvasWidth, (GLsizei)canvasHeight);

    width = (float)canvasWidth;
    height = (float)canvasHeight;

    // Vertices coordinates
    GLfloat vertices[] =
        {
            // coordinates       //colors            //texture coordinate description
            -0.5f, 0.0f, 0.5f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f,  // Lower left corner
            -0.5f, 0.0f, -0.5f, 0.0f, 1.0f, 0.0f, 5.0f, 0.0f, // Upper left corner
            0.5f, 0.0f, -0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,  // Upper right corner
            0.5f, 0.0f, 0.5f, 1.0f, 1.0f, 1.0f, 5.0f, 0.0f,   // Lower right corner
            0.0f, 0.8f, 0.0f, 1.0f, 1.0f, 1.0f, 2.5f, 5.0f    // Lower right corner
        };

    // Indices for vertices order
    GLuint indices[] =
        {
            0, 1, 2,
            0, 2, 3,
            0, 1, 4,
            1, 2, 4,
            2, 3, 4,
            3, 0, 4};

    indicesSize = sizeof(indices);

    shaderProgram = new Shader("./shaders/3d/3d.vert", "./shaders/3d/3d.frag");

    VAO1 = new VAO();
    VAO1->Bind();

    VBO VBO1(vertices, sizeof(vertices));

    EBO EBO1(indices, sizeof(indices));

    // VAO1->LinkVBO(VBO1, 0);
    VAO1->LinkAttrib(VBO1, 0, 3, GL_FLOAT, sizeof(float) * 8, (void *)0);
    VAO1->LinkAttrib(VBO1, 1, 3, GL_FLOAT, sizeof(float) * 8, (void *)(sizeof(float) * 3));
    VAO1->LinkAttrib(VBO1, 2, 2, GL_FLOAT, sizeof(float) * 8, (void *)(sizeof(float) * 6));

    VAO1->Unbind();
    VBO1.Unbind();
    EBO1.Unbind();

    popCat = new Texture("./textures/pop_cat.png", GL_TEXTURE_2D, GL_TEXTURE0, GL_RGBA, GL_UNSIGNED_BYTE);
    popCat->texUnit(shaderProgram, "tex0", 0);

    // depth testing is necessary in 3d, so opengl knows which pixel to put on the screen
    glEnable(GL_DEPTH_TEST);

    camera = new Camera((int)width, (int)height, glm::vec3(0.0f, 0.0f, 2.0f));

    emscripten_set_keydown_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, 0, 1, key_callback);
    emscripten_set_mousedown_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, NULL, 1, mouse_click_callback);
    emscripten_set_main_loop(MainLoop, 0, 1);

    VAO1->Delete();
    VBO1.Delete();
    EBO1.Delete();
    popCat->Delete();
    shaderProgram->Delete();

    return 0;
}