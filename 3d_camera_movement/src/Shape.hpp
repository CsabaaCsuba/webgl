#pragma once
#include <iostream>
#include <vector>
#include <array>
#include <optional>
#include <GLES3/gl3.h>

#define MESH_COUNT 4

typedef enum ShapeType {
    TRIANGLE,
    MESH,
    SQUARE,
    CIRCLE,
    CUBE,
    SPHERE
} ShapeType;

class Shape {
public:
    ShapeType shape_type = ShapeType::CUBE;
    GLuint VAO;
    GLuint VBO;
    std::optional<GLuint> EBO;
    GLuint vertex_len;
    std::optional<GLuint> shaderprogram_id;

    Shape(){

    }

    void AssignShaderProgram(GLuint programID){
        shaderprogram_id = programID;
    }

    //a cube should have uniforms supplied to it
    static Shape InitCube(float width){
        Shape shape;
        shape.shape_type = ShapeType::CUBE;

        float a = width/2.0f;

        float vertices[] = {
            a, 0.0f, a,
            -a, 0.0f, a,
            -a, 0.0f, -a,
            a, 0.0f, -a,

            a, a*2.0f, a,
            -a, a*2.0f, a,
            -a, a*2.0f, -a,
            a, a*2.0f, -a,
        };

        GLuint indices[] = {
            0, 1, 2,
            0, 2, 3, //bottom face
            0, 1, 5, 
            0, 5, 4, //"close side"
            1, 2, 6,
            1, 5, 6, //"left side"
            2, 6, 7,
            2, 3, 7, //"far side"
            0, 3, 4,
            3, 4, 7, //"right side"
            4, 5, 6,
            4, 6, 7 //upper face
        };

        glGenVertexArrays(1, &shape.VAO);
        glBindVertexArray(shape.VAO);

        glGenBuffers(1, &shape.VBO);
        glBindBuffer(GL_ARRAY_BUFFER, shape.VBO);

        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW); //GL_STATIC_DRAW means it won't be changed
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), NULL);
        glEnableVertexAttribArray(0);
        // std::cout << "vertex lens: " << sizeof(rec_indices)/sizeof(GLuint) << std::endl;
        // this->VertexLens.push_back(sizeof(rec_indices)/sizeof(GLuint));
        shape.vertex_len = sizeof(vertices)/3; //because there are 3 data points for each vertex


        GLuint NewEBO;
        glGenBuffers(1, &NewEBO);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, NewEBO);

        shape.EBO = NewEBO;

        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

        return shape;
    }

};