#!/usr/bin/env python3
import os
import sys
from pathlib import Path
import argparse

BUILD_PY_TEMPLATE = '''# build.py
import subprocess
from pathlib import Path

SRC_DIR = Path("src")
BUILD_DIR = Path("build")
BUILD_DIR.mkdir(exist_ok=True)

CLANG = "clang"
OUTPUT = BUILD_DIR / "main"

CFLAGS = [
    "-std=c23",
    "-O2",
    "-Wall",
    "-I/usr/include",
    "-I/usr/include/freetype2",
    "-I/usr/include/harfbuzz",
    "-L/usr/lib/x86_64-linux-gnu",
    "-lglfw",
    "-lGLESv2",
    "-lfreetype",
    "-lharfbuzz",
    "-lm",
    "-fshort-enums",
]

def build():
    src_files = list(SRC_DIR.glob("*.c"))
    if not src_files:
        print("Error: No source files found in", SRC_DIR)
        return

    cmd = [CLANG] + CFLAGS + [str(src) for src in src_files] + ["-o", str(OUTPUT)]
    print("Compiling and linking:", " ".join(cmd))
    
    try:
        subprocess.run(cmd, check=True)
        print(f"✅ Build complete: {OUTPUT}")
        return True
    except subprocess.CalledProcessError as e:
        print(f"❌ Build failed with error code {e.returncode}")
        return False

if __name__ == "__main__":
    if build():
        subprocess.run([str(OUTPUT)])
'''

MAIN_C_TEMPLATE = '''#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <GLES3/gl3.h>
#include <GLFW/glfw3.h>

// Shader sources
const char* vertex_shader_source =
    "#version 300 es\\n"
    "in vec3 position;\\n"
    "in vec3 color;\\n"
    "out vec3 Color;\\n"
    "uniform mat4 model;\\n"
    "uniform mat4 view;\\n"
    "uniform mat4 projection;\\n"
    "void main() {\\n"
    "    Color = color;\\n"
    "    gl_Position = projection * view * model * vec4(position, 1.0);\\n"
    "}\\n";

const char* fragment_shader_source =
    "#version 300 es\\n"
    "precision mediump float;\\n"
    "in vec3 Color;\\n"
    "out vec4 fragColor;\\n"
    "void main() {\\n"
    "    fragColor = vec4(Color, 1.0);\\n"
    "}\\n";

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

unsigned int indices[] = {
    0, 1, 2, 2, 3, 0, // front
    1, 5, 6, 6, 2, 1, // right
    5, 4, 7, 7, 6, 5, // back
    4, 0, 3, 3, 7, 4, // left
    3, 2, 6, 6, 7, 3, // top
    4, 5, 1, 1, 0, 4  // bottom
};

void error_callback(int error, const char* description) {
    fprintf(stderr, "GLFW Error: %s\\n", description);
}

unsigned int compile_shader(unsigned int type, const char* source) {
    unsigned int shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, NULL);
    glCompileShader(shader);
    
    int success;
    char info_log[512];
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(shader, 512, NULL, info_log);
        fprintf(stderr, "Shader compilation failed: %s\\n", info_log);
    }
    
    return shader;
}

unsigned int create_shader_program() {
    unsigned int vertex_shader = compile_shader(GL_VERTEX_SHADER, vertex_shader_source);
    unsigned int fragment_shader = compile_shader(GL_FRAGMENT_SHADER, fragment_shader_source);
    
    unsigned int program = glCreateProgram();
    glAttachShader(program, vertex_shader);
    glAttachShader(program, fragment_shader);
    glLinkProgram(program);
    
    int success;
    char info_log[512];
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(program, 512, NULL, info_log);
        fprintf(stderr, "Shader program linking failed: %s\\n", info_log);
    }
    
    glDeleteShader(vertex_shader);
    glDeleteShader(fragment_shader);
    
    return program;
}

int main() {
    // Initialize GLFW
    glfwSetErrorCallback(error_callback);
    if (!glfwInit()) {
        fprintf(stderr, "Failed to initialize GLFW\\n");
        return -1;
    }
    
    // Configure GLFW for OpenGL ES 3.0
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
    
    // Create window
    GLFWwindow* window = glfwCreateWindow(800, 600, "Rotating Cube", NULL, NULL);
    if (!window) {
        fprintf(stderr, "Failed to create GLFW window\\n");
        glfwTerminate();
        return -1;
    }
    
    glfwMakeContextCurrent(window);
    
    // Create shader program
    unsigned int shader_program = create_shader_program();
    
    // Create VAO, VBO, and EBO
    unsigned int VAO, VBO, EBO;
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
'''
def create_project(project_name):
    # Create project directory
    project_dir = Path(project_name)
    try:
        project_dir.mkdir(exist_ok=False)
    except FileExistsError:
        print(f"Error: Project directory '{project_name}' already exists")
        return False

    # Create src directory
    src_dir = project_dir / "src"
    src_dir.mkdir()

    # Create build.py
    build_py = project_dir / "build.py"
    with build_py.open('w') as f:
        f.write(BUILD_PY_TEMPLATE)
    build_py.chmod(0o755)  # Make it executable

    # Create main.c with rotating cube example
    main_c = src_dir / "main.c"
    with main_c.open('w') as f:
        f.write(MAIN_C_TEMPLATE)

    print(f"✅ Created new project '{project_name}' with:")
    print(f"  - {build_py}")
    print(f"  - {main_c}")
    print("\nTo build and run:")
    print(f"  cd {project_name} && ./build.py")
    return True

def main():
    parser = argparse.ArgumentParser(description='Create a new OpenGL ES project with a rotating cube example')
    parser.add_argument('project_name', help='Name of the project to create')
    args = parser.parse_args()

    if not create_project(args.project_name):
        sys.exit(1)

if __name__ == "__main__":
    main()