#pragma once
#include <GLES3/gl3.h>
#include "../emscripten_fetch/fetch.hpp"

class Shader {
public:
    GLuint ID;
    Shader(const std::string& vertexFile, const std::string& fragmentFile){
        std::string vertexCode = downloadContent(vertexFile);
        std::string fragmentCode = downloadContent(fragmentFile);

        const char* vertexShaderSource = vertexCode.c_str();
        const char* fragmentShaderSource = fragmentCode.c_str();

        // Create Vertex Shader Object and get its reference
        GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
        // Attach Vertex Shader source to the Vertex Shader Object
        glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
        // Compile the Vertex Shader into machine code
        glCompileShader(vertexShader);

        // Create Fragment Shader Object and get its reference
        GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
        // Attach Fragment Shader source to the Fragment Shader Object
        glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
        // Compile the Vertex Shader into machine code
        glCompileShader(fragmentShader);

        // Create Shader Program Object and get its reference
        ID = glCreateProgram();
        // Attach the Vertex and Fragment Shaders to the Shader Program
        glAttachShader(ID, vertexShader);
        glAttachShader(ID, fragmentShader);
        // Wrap-up/Link all the shaders together into the Shader Program
        glLinkProgram(ID);

        // Delete the now useless Vertex and Fragment Shader objects
        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);
    }

    ~Shader(){
        this->Delete();
    }

    void Activate(){
        glUseProgram(ID);
    }

    void Delete(){
        glDeleteProgram(ID);
    }
};