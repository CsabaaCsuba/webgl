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
#include "MyCanvas.hpp"

VAO *VAO1;
Shader *shaderProgram;
Texture *popCat;
Camera *camera;
float width, height;
size_t indicesSize;
EGLDisplay display;
EGLSurface surface;

double lastTime = 0;
double currentTime = 0;
double frameTime = 0;

void MainLoop()
{
    currentTime = emscripten_get_now();
    frameTime = currentTime - lastTime;
    lastTime = currentTime;

    frameTime /= 1000;
    glClearColor(0.07f, 0.13f, 0.17f, 1.0f);
    // Clean the back buffer and assign the new color to it
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    // Tell OpenGL which Shader Program we want to use
    shaderProgram->Activate();

    camera->UpdateMatrix(45.0f, 0.1f, 100.0f);

    camera->Matrix(shaderProgram->ID, "camMatrix");

    popCat->Bind();
    // Bind the VAO so OpenGL knows to use it
    VAO1->Bind();
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
    GLuint indices[] ={
            0, 1, 2,
            0, 2, 3,
            0, 1, 4,
            1, 2, 4,
            2, 3, 4,
            3, 0, 4
    };

    GLfloat lightVertices[] = {
        -0.1f, -0.1f,  0.1f,
        -0.1f, -0.1f, -0.1f,
        0.1f, -0.1f, -0.1f,
        0.1f, -0.1f,  0.1f,
        -0.1f,  0.1f,  0.1f,
        -0.1f,  0.1f, -0.1f,
        0.1f,  0.1f, -0.1f,
        0.1f,  0.1f,  0.1f
    };

    GLuint lightIndices[] = {
        0, 1, 2,
        0, 2, 3,
        0, 4, 7,
        0, 7, 3,
        3, 7, 6,
        3, 6, 2,
        2, 6, 5,
        2, 5, 1,
        1, 5, 4,
        1, 4, 0,
        4, 5, 6,
        4, 6, 7
    };




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

    display = eglGetCurrentDisplay();
    surface = eglGetCurrentSurface(EGL_DRAW);

    emscripten_set_click_callback("#canvas", (void*)camera, EM_TRUE, immerse);
    //lilrec only lives in the main function, however it gets added as the second shape.
    emscripten_set_mousemove_callback("#canvas", (void*)camera, EM_TRUE, mouse_move);
    emscripten_set_keydown_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, (void*)camera, EM_TRUE, key_callback);
    emscripten_set_main_loop(MainLoop, 0, 1);

    VAO1->Delete();
    VBO1.Delete();
    EBO1.Delete();
    popCat->Delete();
    shaderProgram->Delete();

    return 0;
}