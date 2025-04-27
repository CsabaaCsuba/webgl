#pragma once
#include <GLES3/gl3.h>

class VBO {
public:
    GLuint ID;
    
    VBO(GLfloat* vertices, GLsizeiptr size){
        glGenBuffers(1, &ID);

        glBindBuffer(GL_ARRAY_BUFFER, ID);
        // Introduce the vertices into the VBO
        glBufferData(GL_ARRAY_BUFFER, size, vertices, GL_STATIC_DRAW);
    }

    void Bind(){
        glBindBuffer(GL_ARRAY_BUFFER, ID);
    }

    void Unbind(){
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }

    void Delete(){
        glDeleteBuffers(1, &ID);
    }
};