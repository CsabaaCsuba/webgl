#define GLM_ENABLE_EXPERIMENTAL 
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "glm/gtx/rotate_vector.hpp"
#include "glm/gtx/vector_angle.hpp"
#include "glm/ext/matrix_clip_space.hpp"
#include "glm/ext/matrix_transform.hpp"
#include <GLES3/gl3.h>
#include <EGL/egl.h>
#include <emscripten.h>
#include <emscripten/html5.h>
#include <math.h>
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <emscripten/fetch.h>
#include <string.h>
#include "Shape.hpp"
#include "Camera.hpp"
#include "CanvasManagement.hpp"
#include "../include/emscripten_fetch.hpp"
#include "Program.hpp"
#include <optional>


Program *program;
EGLDisplay display;
EGLSurface surface;

double lastTime = 0;
double currentTime = 0;
double frameTime = 0;

void update_camera(Camera* camera) {
    if (keyStates["KeyW"])
        camera->Position += camera->speed * camera->Orientation;
    if (keyStates["KeyS"])
        camera->Position -= camera->speed * camera->Orientation;
    if (keyStates["KeyA"])
        camera->Position -= camera->speed * glm::normalize(glm::cross(camera->Orientation, camera->Up));
    if (keyStates["KeyD"])
        camera->Position += camera->speed * glm::normalize(glm::cross(camera->Orientation, camera->Up));
    if (keyStates["ControlLeft"])
        camera->Position += camera->speed * -camera->Up;
    if (keyStates["Space"])
        camera->Position += camera->speed * camera->Up;
}


void MainLoop(){
    currentTime = emscripten_get_now();
    frameTime = currentTime - lastTime;
    lastTime = currentTime;

    std::cout << "current frametime: " << frameTime << "ms" << std::endl;

    glClearColor(0.2, 0.2, 0.2, 1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    update_camera(&program->cam);

    program->cam.Matrix(90.0f, 0.02, 100.0f);

    program->UseShader(0);

    program->cam.UpdateMatrix(program->shapes[0].shaderprogram_id.value(), "CamMatrix");
    
    program->DrawShape(program->shapes[0]);


    //this might not work
    eglSwapBuffers(display, surface);
}

int main(){
    std::cout << "halo" << std::endl;
    EmscriptenWebGLContextAttributes attrs;
    emscripten_webgl_init_context_attributes(&attrs);
    attrs.majorVersion = 2; // WebGL 2.0 corresponds to OpenGL ES 3.0
    attrs.minorVersion = 0;

    EMSCRIPTEN_WEBGL_CONTEXT_HANDLE context = emscripten_webgl_create_context("#canvas", &attrs);
    emscripten_webgl_make_context_current(context);

    double canvasWidth, canvasHeight;
    EMSCRIPTEN_RESULT res = emscripten_get_element_css_size("#canvas", &canvasWidth, &canvasHeight);

    glViewport(0, 0, (GLsizei)canvasWidth, (GLsizei)canvasHeight);

    program = new Program(canvasWidth, canvasHeight, glm::vec3(0.0f, 1.1f, 3.0f));

    Shape myCube = Shape::InitCube(1.5f);
    GLuint cubeShader = program->CreateShaderProgram("./shaders/3d/3d_xyz.vert", "./shaders/3d/3d_white.frag");

    myCube.AssignShaderProgram(cubeShader);

    program->AddShape(myCube);

    glEnable(GL_DEPTH_TEST);

    //todo: add a spinning cube

    //this must be defined for buffer swapping (VSYNC)
    display = eglGetCurrentDisplay();
    surface = eglGetCurrentSurface(EGL_DRAW);

    emscripten_set_click_callback("#canvas", (void*)&(program->cam), EM_TRUE, immerse);
    //lilrec only lives in the main function, however it gets added as the second shape.
    emscripten_set_mousemove_callback("#canvas", (void*)&(program->cam), EM_TRUE, mouse_move);
    emscripten_set_keydown_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, (void*)&(program->cam), EM_TRUE, key_callback);
    emscripten_set_keyup_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, (void*)&(program->cam), EM_TRUE, key_callback);

    emscripten_set_main_loop(MainLoop, 0, 1);

    return 0;
}