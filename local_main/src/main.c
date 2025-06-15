// main.c
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <GLES3/gl3.h>
#include <GLFW/glfw3.h>
#include "flecs.h"

// Optional: your math and graphics helpers (assumed header-only or inline .c)
#ifndef MY_MATH
    #include "MyMath.h"
#endif
#include "shader_compiler.h"  // if you want to include implementation directly
#include "fonts.h"
#include "entities.h" // include ECS component/system setup

ecs_world_t* world;


void error_callback(int error, const char* description) {
    fprintf(stderr, "GLFW Error: %s\n", description);
}

int main(void) {
    freopen("error.log", "w", stderr);
    glfwSetErrorCallback(error_callback);

    if (!glfwInit()) {
        fprintf(stderr, "GLFW init failed\n");
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);

    GLFWwindow* window = glfwCreateWindow(800, 600, "Engine", NULL, NULL);
    if (!window) {
        fprintf(stderr, "Window creation failed\n");
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);

    // --- ECS Setup ---
    world = ecs_init();
    setup_world_components(world);

    InstantiateCube(world, 0.0f, 0.0f, 0.0f, 0);

    // OpenGL setup
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    float deltaTime = 0.0f;
    float lastFrame = 0.0f;

    while (!glfwWindowShouldClose(window)) {
        float currentFrame = glfwGetTime();

        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        glClearColor(0.0f, 0.4f, 0.5f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Run ECS systems
        ecs_progress(world, deltaTime);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    ecs_fini(world);
    glfwTerminate();
    return 0;
}
