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
#include "../include/emscripten_fetch.hpp"

class Program{
private:
    float WindowWidth;
    float WindowHeight;
    
public:
    Camera cam;
    std::vector<Shape> shapes;
    std::vector<GLuint> ShaderProgram_ids;

    Program(double WindowWidth, double WindowHeight, glm::vec3 position){
        this->WindowWidth = (float)WindowWidth;
        this->WindowHeight = (float)WindowHeight;
        Camera new_cam((int)WindowWidth, (int) WindowHeight, position);
        this->cam = new_cam;
    }

    void AddShape(Shape shape){
        shapes.push_back(shape);
    }


    void DrawShape(size_t idx, size_t program_idx){
        GLuint current_vao = this->shapes[idx].VAO;
        glUseProgram(this->ShaderProgram_ids[program_idx]);
        glBindVertexArray(current_vao);
        GLshort OpenglShape = (this->shapes[idx].shape_type == ShapeType::MESH) ? GL_LINE_LOOP : GL_TRIANGLES;
        glDrawElements(OpenglShape, this->shapes[idx].vertex_len, GL_UNSIGNED_INT, 0);

        glBindVertexArray(0);
    }

    static GLuint CompileShader(GLenum type, const char *source_file){
        std::string fileContents = downloadContent(source_file);
        std::cout << "string contents: " << fileContents << std::endl;
        GLuint shader = glCreateShader(type);
        const GLchar* whatever = fileContents.data();
        glShaderSource(shader, 1, &whatever, NULL);
        glCompileShader(shader);

        GLint status;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
        if (status != GL_TRUE){
            GLint logLength = 0;
            glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logLength);

            if (logLength > 1) {
                std::vector<char> log(logLength);
                glGetShaderInfoLog(shader, logLength, NULL, log.data());
                printf("Shader (type %u) compilation failed: %s\n", type, log.data());
            }
            return 0;
        }

        return shader;
    }

    bool CreateShaderProgram(const char *vertex_source, const char *fragment_source){
        GLuint vertexShader_id = Program::CompileShader(GL_VERTEX_SHADER, vertex_source);
        if (vertexShader_id == 0) return false;
    
        GLuint fragmentShader_id = Program::CompileShader(GL_FRAGMENT_SHADER, fragment_source);
        if (fragmentShader_id == 0) {
            glDeleteShader(vertexShader_id);
            return false;
        }
    
        GLuint shaderProgram_id = glCreateProgram();
        glAttachShader(shaderProgram_id, vertexShader_id);
        glAttachShader(shaderProgram_id, fragmentShader_id);
        glLinkProgram(shaderProgram_id);
    
        GLint status;
        glGetProgramiv(shaderProgram_id, GL_LINK_STATUS, &status);
        if (status != GL_TRUE){
            GLint logLength = 0;
            glGetProgramiv(shaderProgram_id, GL_INFO_LOG_LENGTH, &logLength);
            if (logLength > 1) {
                std::vector<char> log(logLength);
                glGetProgramInfoLog(shaderProgram_id, logLength, NULL, log.data());
                printf("Program linking failed: %s\n", log.data());
            } else {
                printf("Program linking failed with no log.\n");
            }
    
            glDeleteShader(vertexShader_id);
            glDeleteShader(fragmentShader_id);
            glDeleteProgram(shaderProgram_id);
            return false;
        }
    
        ShaderProgram_ids.push_back(shaderProgram_id);
    
        glDeleteShader(vertexShader_id);
        glDeleteShader(fragmentShader_id);
    
        return true;
    }
    
};

Program *program;
EGLDisplay display;
EGLSurface surface;

void MainLoop(){
    glClearColor(0.2, 0.2, 0.2, 1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    //activate the first compiled shader
    glUseProgram(program->ShaderProgram_ids[0]);

    //supply the shader the matrix value
    program->cam.Matrix(90.0f, 0.02, 100.0f, program->ShaderProgram_ids[0], "CamMatrix");

    program->DrawShape(0, 0);

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

    program = new Program(canvasWidth, canvasHeight, glm::vec3(0.0f, 2.0f, 1.0f));

    //todo: write shaders with projection matrices
    GLuint shader = program->CreateShaderProgram("./shaders/3d.vert", "./shaders/3d_white.frag");

    Shape mesh = Shape::InitMesh();

    program->AddShape(mesh);

    glEnable(GL_DEPTH_TEST);

    //todo: add a spinning cube

    //this must be defined for buffer swapping (VSYNC)
    display = eglGetCurrentDisplay();
    surface = eglGetCurrentSurface(EGL_DRAW);
    emscripten_set_main_loop(MainLoop, 0, 1);

    return 0;
}