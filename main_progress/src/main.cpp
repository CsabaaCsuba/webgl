// #define GLM_ENABLE_EXPERIMENTAL 
// #include "glm/glm.hpp"
// #include "glm/gtc/matrix_transform.hpp"
// #include "glm/gtc/type_ptr.hpp"
// #include "glm/gtx/rotate_vector.hpp"
// #include "glm/gtx/vector_angle.hpp"
// #include "glm/ext/matrix_clip_space.hpp"
// #include "glm/ext/matrix_transform.hpp"
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
#include "utils/MyMath.hpp"


Program *program;
EGLDisplay display;
EGLSurface surface;

double lastTime = 0;
double currentTime = 0;
double frameTime = 0;

void   update_camera(Camera* camera) {
    if (keyStates["KeyW"])
        camera->Position += (camera->Orientation * camera->speed);
    if (keyStates["KeyS"])
        camera->Position -= camera->Orientation * camera->speed;
    if (keyStates["KeyA"])
        camera->Position -= Vec3::cross(camera->Orientation, camera->Up).normalize() * camera->speed;
    if (keyStates["KeyD"])
        camera->Position += Vec3::cross(camera->Orientation, camera->Up).normalize() * camera->speed;
    if (keyStates["ControlLeft"])
        camera->Position +=  -camera->Up * camera->speed;
    if (keyStates["Space"])
        camera->Position += camera->Up * camera->speed;
}

Vec3 movement = Vec3(0.0f, 1.0f, 0.0f);
float rotation_speed = 10.0f;

bool is_upwards = true;


float velocity_x = 10.0f;
float velocity_y = 10.0f;

void MainLoop(){
    currentTime = emscripten_get_now();
    frameTime = currentTime - lastTime;
    lastTime = currentTime;

    //this is just some movement-related example
    frameTime /= 10000;

    float rotation_angle = frameTime * rotation_speed * 180.0f / M_PI; // Convert to degrees
    Vec3 rotation_axis(0, 1, 0); // Y-axis rotation

    Quat frame_rotation = Quat::from_rotation_vector(rotation_angle, rotation_axis);
    program->shapes[0].rotation = program->shapes[0].rotation * frame_rotation;

    Mat4 incremental_translation = Mat4::identity();
    program->shapes[0].position.data[0] += frameTime * velocity_x;
    program->shapes[0].position.data[1] += frameTime * velocity_y;

    program->shapes[0].save_position();


    // std::cout << "current frametime: " << frameTime*1000 << "ms" << std::endl;

    glClearColor(0.2, 0.2, 0.2, 1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    update_camera(&program->cam);

    program->cam.SetProjection(100.0f, 0.02f, 100.0f);

    program->UseShader(0);

    program->shapes[0].SupplyTransformationValues(program->cam.VP_MATRIX);
    
    program->DrawShape(program->shapes[0]);

    // eglSwapBuffers(display, surface);
}

int main(){
    std::cout << "Starting application..." << std::endl;
    EmscriptenWebGLContextAttributes attrs;
    emscripten_webgl_init_context_attributes(&attrs);
    attrs.majorVersion = 2; // WebGL 2.0 corresponds to OpenGL ES 3.0
    attrs.minorVersion = 0;

    attrs.enableExtensionsByDefault = 1;
    attrs.powerPreference = EM_WEBGL_POWER_PREFERENCE_HIGH_PERFORMANCE;
    

    EMSCRIPTEN_WEBGL_CONTEXT_HANDLE context = emscripten_webgl_create_context("#canvas", &attrs);
    emscripten_webgl_make_context_current(context);

    double canvasWidth, canvasHeight;
    EMSCRIPTEN_RESULT res = emscripten_get_element_css_size("#canvas", &canvasWidth, &canvasHeight);

    glViewport(0, 0, (GLsizei)canvasWidth, (GLsizei)canvasHeight);

    program = new Program(canvasWidth, canvasHeight, Vec3::create(0.0f, 2.0f, 4.0f));

    // Create textured cube with explicit path
    const std::string texturePath = "textures/cobblestone.jpeg";
    std::cout << "Creating cube with texture: " << texturePath << std::endl;
    Shape myCube = Shape::InitCube(1.5f, texturePath);
    
    GLuint cubeShader = program->CreateShaderProgram("./shaders/3d.vert", "./shaders/3d.frag");
    std::cout << "Shader program created: " << cubeShader << std::endl;

    myCube.AssignShaderProgram(cubeShader);

    program->AddShape(myCube);

    glEnable(GL_DEPTH_TEST);

    display = eglGetCurrentDisplay();
    surface = eglGetCurrentSurface(EGL_DRAW);

    emscripten_set_click_callback("#canvas", (void*)&(program->cam), EM_TRUE, immerse);
    emscripten_set_mousemove_callback("#canvas", (void*)&(program->cam), EM_TRUE, mouse_move);
    emscripten_set_keydown_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, (void*)&(program->cam), EM_TRUE, key_callback);
    emscripten_set_keyup_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, (void*)&(program->cam), EM_TRUE, key_callback);

    emscripten_set_main_loop(MainLoop, 0, 1);
    // emscripten_set_main_loop_timing(EM_TIMING_SETTIMEOUT, 0);


    return 0;
}