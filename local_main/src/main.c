#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <GLES3/gl3.h>
#include <GLFW/glfw3.h>
#include "enums.h"
#include "shader_compiler.h"

#define ARENA_SIZE 1024 * 1024 * 1024


// Cube vertices and colors
float vertices[] = {
    // Positions         // Colors
    -0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 0.0f,
     0.5f, -0.5f, -0.5f, 0.0f, 1.0f, 0.0f,
     0.5f,  0.5f, -0.5f, 0.0f, 0.0f, 1.0f,
    -0.5f,  0.5f, -0.5f, 1.0f, 1.0f, 0.0f,
    -0.5f, -0.5f,  0.5f, 0.0f, 1.0f, 1.0f,
     0.5f, -0.5f,  0.5f, 1.0f, 0.0f, 1.0f,
     0.5f,  0.5f,  0.5f, 0.5f, 0.5f, 0.5f,
    -0.5f,  0.5f,  0.5f, 1.0f, 1.0f, 1.0f
};

uint32_t indices[] = {
    0, 1, 2, 2, 3, 0, // front
    1, 5, 6, 6, 2, 1, // right
    5, 4, 7, 7, 6, 5, // back
    4, 0, 3, 3, 7, 4, // left
    3, 2, 6, 6, 7, 3, // top
    4, 5, 1, 1, 0, 4  // bottom
};

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
    
    // Create VAO, VBO, and EBO
    uint32_t VAO, VBO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);
    
    glBindVertexArray(VAO);
    
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
    
    // Position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    
    // Color attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    
    // Enable depth testing
    glEnable(GL_DEPTH_TEST);
    
    // Main loop
    while (!glfwWindowShouldClose(window)) {
        // Clear screen
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        // Use shader
        glUseProgram(shader_program);
        
        // Create transformations
        float time = glfwGetTime();
        
        // Model matrix (rotation around Y axis)
        float model[16] = {0};
        model[0] = cos(time);  model[2] = sin(time);
        model[5] = 1.0f;
        model[8] = -sin(time); model[10] = cos(time);
        model[15] = 1.0f;
        
        // View matrix (camera slightly moved back)
        float view[16] = {0};
        view[0] = 1.0f; view[5] = 1.0f; view[10] = 1.0f;
        view[14] = -2.0f; view[15] = 1.0f;
        
        // Projection matrix (perspective)
        float projection[16] = {0};
        float fov = 45.0f * (3.14159f / 180.0f);
        float aspect = 800.0f / 600.0f;
        float near = 0.1f;
        float far = 100.0f;
        
        projection[0] = 1.0f / (aspect * tan(fov/2));
        projection[5] = 1.0f / tan(fov/2);
        projection[10] = -(far + near) / (far - near);
        projection[11] = -1.0f;
        projection[14] = -(2.0f * far * near) / (far - near);
        
        // Pass matrices to shader
        glUniformMatrix4fv(glGetUniformLocation(shader_program, "model"), 1, GL_FALSE, model);
        glUniformMatrix4fv(glGetUniformLocation(shader_program, "view"), 1, GL_FALSE, view);
        glUniformMatrix4fv(glGetUniformLocation(shader_program, "projection"), 1, GL_FALSE, projection);
        
        // Render cube
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
        
        // Swap buffers and poll events
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    
    // Cleanup
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
    glDeleteProgram(shader_program);
    
    glfwTerminate();
    return 0;
}
