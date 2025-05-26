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

// Debug utility function to check for OpenGL errors
void checkGLError(const char* operation) {
    GLenum error;
    while ((error = glGetError()) != GL_NO_ERROR) {
        std::cout << "OpenGL error after " << operation << ": 0x" << std::hex << error << std::dec << std::endl;
    }
}

class Program{
public:
    float WindowWidth;
    float WindowHeight;
    Camera cam;
    std::vector<Shape> shapes;

    Program(double WindowWidth, double WindowHeight, glm::vec3 CameraPosition){
        this->WindowWidth = (float)WindowWidth;
        this->WindowHeight = (float)WindowHeight;
        Camera new_cam((int)WindowWidth, (int) WindowHeight, CameraPosition);
        this->cam = new_cam;
    }

    void AddShape(Shape shape){
        shapes.push_back(shape);
    }

    void DrawShape(Shape shape){
        if (!shape.shaderprogram_id.has_value()) {
            std::cout << "Error: No shader program assigned to shape!" << std::endl;
            return;
        }
    
        // Use the shader program
        glUseProgram(shape.shaderprogram_id.value());
        checkGLError("glUseProgram");
    
        // Bind the VAO
        glBindVertexArray(shape.VAO);
        checkGLError("glBindVertexArray");
    
        // Handle texture if available
        if (shape.texture_id.has_value()) {
            
            glActiveTexture(GL_TEXTURE0);
            checkGLError("glActiveTexture");
            
            glBindTexture(GL_TEXTURE_2D, shape.texture_id.value());
            checkGLError("glBindTexture");
            
        } else {
            std::cout << "Warning: No texture assigned to this shape!" << std::endl;
        }
        
        // Draw the shape based on its type
        if (shape.shape_type == ShapeType::MESH) {
            for (int i = 0; i < shape.vertex_len; i += 4) {
                glDrawArrays(GL_LINE_LOOP, i, 4);
                checkGLError("glDrawArrays");
            }
        }
        else {
            // Check if indices are properly set up
            if (shape.EBO.has_value()) {
                glDrawElements(GL_TRIANGLES, shape.vertex_len, GL_UNSIGNED_INT, 0);
                checkGLError("glDrawElements");
            } else {
                std::cout << "Warning: No EBO assigned to this shape!" << std::endl;
            }
        }
        
        // Unbind VAO and texture to prevent unintended modifications
        glBindVertexArray(0);
        glBindTexture(GL_TEXTURE_2D, 0);
    }

    static GLuint CompileShader(GLenum type, const char *source_file){
        std::string fileContents = downloadContent(source_file);
        std::cout << "Compiling shader from: " << source_file << std::endl;
        
        if (fileContents.empty()) {
            std::cout << "Error: Could not load shader file: " << source_file << std::endl;
            return 0;
        }
        
        GLuint shader = glCreateShader(type);
        const GLchar* shaderSource = fileContents.data();
        glShaderSource(shader, 1, &shaderSource, NULL);
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
        
        std::cout << "Shader compiled successfully!" << std::endl;
        return shader;
    }

    static GLuint CreateShaderProgram(const char *vertex_source, const char *fragment_source){
        GLuint vertexShader_id = Program::CompileShader(GL_VERTEX_SHADER, vertex_source);
        if (vertexShader_id == 0){
            std::cout << "Vertex shader compilation failed..." << std::endl;
            return 0;
        }
    
        GLuint fragmentShader_id = Program::CompileShader(GL_FRAGMENT_SHADER, fragment_source);
        if (fragmentShader_id == 0) {
            glDeleteShader(vertexShader_id);
            std::cout << "Fragment shader compilation failed..." << std::endl;
            return 0;
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
            return 0;
        }
    
        glDeleteShader(vertexShader_id);
        glDeleteShader(fragmentShader_id);
        
        std::cout << "Shader program created successfully: " << shaderProgram_id << std::endl;
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
        checkGLError("UseShader.glUseProgram");
    }
};