#include <GLES3/gl3.h>
#include <EGL/egl.h>
#include <emscripten.h>
#include <emscripten/html5.h>
#include <stdio.h>

GLuint program;
GLuint vertexBuffer;

const char* vertexShaderSource =
    "#version 300 es\n"
    "in vec2 position;\n"
    "void main() {\n"
    "  gl_Position = vec4(position, 0.0, 1.0);\n"
    "}\n";

const char* fragmentShaderSource =
    "#version 300 es\n"
    "precision mediump float;\n"
    "out vec4 outColor;\n"
    "void main() {\n"
    "  outColor = vec4(1.0, 0.0, 0.0, 1.0);\n"
    "}\n";

GLuint compileShader(GLenum type, const char* source) {
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, NULL);
    glCompileShader(shader);
    
    GLint status;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
    if (status != GL_TRUE) {
        char buffer[512];
        glGetShaderInfoLog(shader, 512, NULL, buffer);
        printf("Shader compilation failed: %s\n", buffer);
        return 0;
    }
    
    return shader;
}

void initGL() {
    // Compile shaders
    GLuint vertexShader = compileShader(GL_VERTEX_SHADER, vertexShaderSource);
    GLuint fragmentShader = compileShader(GL_FRAGMENT_SHADER, fragmentShaderSource);
    
    // Create program
    program = glCreateProgram();
    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);
    glLinkProgram(program);
    
    GLint status;
    glGetProgramiv(program, GL_LINK_STATUS, &status);
    if (status != GL_TRUE) {
        char buffer[512];
        glGetProgramInfoLog(program, 512, NULL, buffer);
        printf("Program linking failed: %s\n", buffer);
        return;
    }
    
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    
    // Create buffer
    float vertices[] = {
        0.0f, 0.5f,    // Top
        -0.5f, -0.5f,  // Bottom left
        0.5f, -0.5f    // Bottom right
    };

    GLuint indices[] = {
        0, 1, 2
    };

    GLuint EBO_id;
    
    glGenBuffers(1, &vertexBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    
    glGenBuffers(1, &EBO_id);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO_id);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
    
    // Set up vertex attribute
    GLint posAttrib = glGetAttribLocation(program, "position");
    glEnableVertexAttribArray(posAttrib);
    glVertexAttribPointer(posAttrib, 2, GL_FLOAT, GL_FALSE, 0, 0);
}

void renderFrame() {
    // Clear the canvas
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    
    // Draw the triangle
    glUseProgram(program);
    glDrawArrays(GL_TRIANGLES, 0, 3);
}

void mainLoop() {
    renderFrame();
}

int main() {
    EmscriptenWebGLContextAttributes attrs;
    emscripten_webgl_init_context_attributes(&attrs);
    attrs.majorVersion = 2; // WebGL 2.0 corresponds to OpenGL ES 3.0
    attrs.minorVersion = 0;
    
    EMSCRIPTEN_WEBGL_CONTEXT_HANDLE context = emscripten_webgl_create_context("#canvas", &attrs);
    emscripten_webgl_make_context_current(context);
    
    initGL();
    emscripten_set_main_loop(mainLoop, 0, 1);
    
    return 0;
}