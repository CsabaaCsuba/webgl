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
#include "../include/emscripten_fetch.hpp"

class Program{
private:
    float WindowWidth;
    float WindowHeight;
public:
    std::vector<GLuint> VAO_ids; // vertex array object -> the id of a configuration
    std::vector<GLuint> VBO_ids; // vertex buffer object -> stores the vertices that will be drawn
    std::vector<GLuint> EBO_ids; // element buffer object -> stores the element indices of the drawn things
    std::vector<GLuint> ShaderProgram_ids;
    std::vector<size_t> VertexLens;

    Program(double WindowWidth, double WindowHeight){
        this->WindowWidth = (float)WindowWidth;
        this->WindowHeight = (float)WindowHeight;
    }

    void DrawShape(size_t VAO_idx, size_t Shader_idx){
        GLuint current_vao = this->VAO_ids[VAO_idx];
        glUseProgram(this->ShaderProgram_ids[Shader_idx]);
        glBindVertexArray(current_vao);
        glDrawElements(GL_TRIANGLES, VertexLens[VAO_idx], GL_UNSIGNED_INT, 0);

        glBindVertexArray(0);
    }

    static void DrawShape(GLuint VAO_id, GLuint ShaderProgram_id, GLuint VertexLength){
        glUseProgram(ShaderProgram_id);
        glBindVertexArray(VAO_id);
        glDrawElements(GL_TRIANGLES, VertexLength, GL_UNSIGNED_INT, 0);

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
            char buffer[512];
            glGetShaderInfoLog(shader, 512, NULL, buffer);
            printf("Shader (type %u) compilation failed: %s\n", type, buffer);
            return 0;
        }

        return shader;
    }

    bool CreateShaderProgram(const char *vertex_source, const char *fragment_source){
        GLuint vertexShader_id;
        GLuint fragmentShader_id;

        vertexShader_id = Program::CompileShader(GL_VERTEX_SHADER, vertex_source);
        fragmentShader_id = Program::CompileShader(GL_FRAGMENT_SHADER, fragment_source);

        GLuint shaderProgram_id = glCreateProgram();
        glAttachShader(shaderProgram_id, vertexShader_id);
        glAttachShader(shaderProgram_id, fragmentShader_id);
        glLinkProgram(shaderProgram_id);

        GLint status;
        glGetProgramiv(shaderProgram_id, GL_LINK_STATUS, &status);
        if (status != GL_TRUE){
            char buffer[512];
            glGetProgramInfoLog(shaderProgram_id, 512, NULL, buffer);
            printf("Program linking failed: %s\n", buffer);
            glDeleteShader(vertexShader_id);
            glDeleteShader(fragmentShader_id);
            return false;
        }

        ShaderProgram_ids.push_back(shaderProgram_id);

        glDeleteShader(vertexShader_id);
        glDeleteShader(fragmentShader_id);

        return true;
    }
};

Program *program;

void MainLoop(){
    glClearColor(0.0, 0.0, 0.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT);

    Program::DrawShape(program->VAO_ids[0], program->ShaderProgram_ids[0], program->VertexLens[0]);

    program->DrawShape(1, 0);

    //this might not work
    eglSwapBuffers(eglGetCurrentDisplay(), eglGetCurrentSurface(EGL_DRAW));
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

    program = new Program(canvasWidth, canvasHeight);

    //todo: write shaders with projection matrices
    GLuint shader = program->CreateShaderProgram("./shaders/vertex.glsl", "./shaders/fragment.glsl");

    //todo: add cube drawing

    emscripten_set_main_loop(MainLoop, 0, 1);

    return 0;
}