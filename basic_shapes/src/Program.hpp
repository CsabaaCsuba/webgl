#pragma once
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

class Program{
public:
    float WindowWidth;
    float WindowHeight;
    Camera cam;
    std::vector<Shape> shapes;

    Program(double WindowWidth, double WindowHeight, glm::vec3 position){
        this->WindowWidth = (float)WindowWidth;
        this->WindowHeight = (float)WindowHeight;
        Camera new_cam((int)WindowWidth, (int) WindowHeight, position);
        this->cam = new_cam;
    }

    void AddShape(Shape shape){
        shapes.push_back(shape);
    }


    void DrawShape(Shape shape){
        GLuint current_vao = shape.VAO;
        glUseProgram(shape.shaderprogram_id.value());
        glBindVertexArray(current_vao);

        if (shape.shape_type == ShapeType::MESH){
            for (int i = 0; i < shape.vertex_len; i += 4) {
                glDrawArrays(GL_LINE_LOOP, i, 4);
            }
            
        }
        else {
            glDrawElements(GL_TRIANGLES, shape.vertex_len, GL_UNSIGNED_INT, 0);
        }
        
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

    static GLuint CreateShaderProgram(const char *vertex_source, const char *fragment_source){
        GLuint vertexShader_id = Program::CompileShader(GL_VERTEX_SHADER, vertex_source);
        if (vertexShader_id == 0){
            std::cout << "vertex shader compilation failed..." << std::endl;
        }
    
        GLuint fragmentShader_id = Program::CompileShader(GL_FRAGMENT_SHADER, fragment_source);
        if (fragmentShader_id == 0) {
            glDeleteShader(vertexShader_id);
            std::cout << "fragment shader compilation failed..." << std::endl;
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
        }
    
        glDeleteShader(vertexShader_id);
        glDeleteShader(fragmentShader_id);

        return shaderProgram_id;
    }

    void UseShader(size_t idx){
        if (idx > shapes.size()-1){
            std::cout << "IDX is larger than the available amount of shapes" << std::endl;
            return;
        }

        if (!shapes[idx].shaderprogram_id){
            std::cout << "ShaderProgram ID is not set yet..." << std::endl;
            return;
        }
        glUseProgram(shapes[idx].shaderprogram_id.value());
    }
        
};