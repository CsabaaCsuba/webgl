#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <GLES3/gl3.h>
#include <GLFW/glfw3.h>
#include "enums.h"
#include "shader_compiler.h"

#define ARENA_SIZE 1024 * 1024 //1 MB

void error_callback(int error, const char* description) {
    fprintf(stderr, "GLFW Error: %s\n", description);
}


int main() {
    //log setup
    freopen("error.log", "w", stderr);

    // Initialize GLFW
    glfwSetErrorCallback(error_callback);
    if (!glfwInit()) {
        fprintf(stderr, "Failed to initialize GLFW\n");
        return -1;
    }
    
    // Configure GLFW for OpenGL ES 3.0
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
    
    // Create window
    GLFWwindow* window = glfwCreateWindow(800, 600, "Rotating Cube", NULL, NULL);
    if (!window) {
        fprintf(stderr, "Failed to create GLFW window\n");
        glfwTerminate();
        return -1;
    }
    
    glfwMakeContextCurrent(window);
    
    // Create shader program
    uint32_t shader_program; 
    ShaderCompilerResult result = create_shader_program("./shaders/vertex.glsl", "./shaders/fragment.glsl", &shader_program);
    
    if (result == COMPILE_FAIL){
        printf("shader compilation failed.\n");
        return -1;
    }

    
    // Enable depth testing
    glEnable(GL_DEPTH_TEST);
    
    // Main loop
    while (!glfwWindowShouldClose(window)) {
        // Clear screen
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
    
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    
    glDeleteProgram(shader_program);
    
    glfwTerminate();
    return 0;
}
