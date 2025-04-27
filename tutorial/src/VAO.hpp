#pragma once
#include <GLES3/gl3.h>
#include "VBO.hpp"
#include "EBO.hpp"

class VAO {
public:
    GLuint ID;
    // VBO vbo;

    VAO(){
        glGenVertexArrays(1, &ID);
    }

    void LinkVBO(VBO vbo, GLuint layout){
        vbo.Bind();

        glVertexAttribPointer(layout, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
        // Enable the Vertex Attribute so that OpenGL knows to use it
        glEnableVertexAttribArray(layout);

        vbo.Unbind();
    }

    void LinkAttrib(VBO vbo, GLuint layout, GLuint numComponents, GLenum type, GLsizeiptr stride, void* offset){
        vbo.Bind();
        glVertexAttribPointer(layout, numComponents, type, GL_FALSE, stride, offset);
        glEnableVertexAttribArray(layout);


        vbo.Unbind();
    }

    void Bind(){
        glBindVertexArray(ID);
    }

    void Unbind(){
        glBindVertexArray(0);
    }

    void Delete(){
        glDeleteVertexArrays(1, &ID);
    }
};