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
    ShapeType shape_type;
    GLuint VAO;
    GLuint VBO;
    std::optional<GLuint> EBO;
    GLuint vertex_len;
    std::optional<GLuint> shaderprogram_id;
    std::optional<std::array<float, 2>> x_y_coords;

    Shape(){

    }

    void AssignShaderProgram(GLuint programID){
        shaderprogram_id = programID;
    }

    static Shape InitRectangle(float x, float y, float width, float height){
        Shape shape;

        float rec_vertices[] = {
            0, 0,                 // bottom-left
            0 + width, 0,         // bottom-right
            0, 0 + height,        // top-left
            0 + width, 0 + height // top-right
        };
    
        GLuint rec_indices[] = {
            0, 1, 2,
            1, 2, 3
        };

        glGenVertexArrays(1, &shape.VAO);
        glBindVertexArray(shape.VAO);

        glGenBuffers(1, &shape.VBO);
        glBindBuffer(GL_ARRAY_BUFFER, shape.VBO);

        glBufferData(GL_ARRAY_BUFFER, sizeof(rec_vertices), rec_vertices, GL_STATIC_DRAW); //GL_STATIC_DRAW means it won't be changed
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), NULL);
        glEnableVertexAttribArray(0);
        // std::cout << "vertex lens: " << sizeof(rec_indices)/sizeof(GLuint) << std::endl;
        // this->VertexLens.push_back(sizeof(rec_indices)/sizeof(GLuint));
        shape.vertex_len = sizeof(rec_indices)/sizeof(GLuint);


        GLuint NewEBO;
        glGenBuffers(1, &NewEBO);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, NewEBO);

        shape.EBO = NewEBO;

        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(rec_indices), rec_indices, GL_STATIC_DRAW);

        shape.x_y_coords = std::array<float, 2>{x, y};
        // shape.x_y_coords = std::nullopt;

        return shape;
    }

    void SetCoordinates(float x, float y){
        x_y_coords = std::array<float, 2>{x, y};
        std::cout << "coordinates set to: " << x_y_coords.value()[0] << "x, " << x_y_coords.value()[1] << "y" << std::endl;
    }

    void SupplyCoordinates(float windowWidth, float windowHeight){
        if (!shaderprogram_id.has_value()){
            std::cout << "please assign a shaderprogram ID first!" << std::endl;
            return;
        }

        GLint offsetLocation = glGetUniformLocation(shaderprogram_id.value(), "CurrentPositions");
        glUniform2f(offsetLocation, x_y_coords.value()[0], x_y_coords.value()[1]);

        offsetLocation = glGetUniformLocation(shaderprogram_id.value(), "ScreenSize");
        glUniform2f(offsetLocation, windowWidth, windowHeight);
    }

    void SupplyCoordinates(float x, float y, float windowWidth, float windowHeight){
        if (!shaderprogram_id.has_value()){
            std::cout << "please assign a shaderprogram ID first!" << std::endl;
            return;
        }
        glUseProgram(shaderprogram_id.value());
        GLint offsetLocation = glGetUniformLocation(shaderprogram_id.value(), "CurrentPositions");
        glUniform2f(offsetLocation, x, y);

        offsetLocation = glGetUniformLocation(shaderprogram_id.value(), "ScreenSize");
        glUniform2f(offsetLocation, windowWidth, windowHeight);
    }


    static Shape InitMesh(int MeshCount) {
    Shape shape;
    shape.shape_type = ShapeType::MESH;

    std::vector<float> line_coords;

    // For each square cell in the grid
    for (int x = -MeshCount; x < MeshCount; ++x) {
        for (int z = -MeshCount; z < MeshCount; ++z) {
            // Bottom-left
            line_coords.push_back((float)x);
            line_coords.push_back((float)z);
            // Bottom-right
            line_coords.push_back((float)(x + 1));
            line_coords.push_back((float)z);
            // Top-right
            line_coords.push_back((float)(x + 1));
            line_coords.push_back((float)(z + 1));
            // Top-left
            line_coords.push_back((float)x);
            line_coords.push_back((float)(z + 1));
        }
    }

    glGenVertexArrays(1, &shape.VAO);
    glBindVertexArray(shape.VAO);

    glGenBuffers(1, &shape.VBO);
    glBindBuffer(GL_ARRAY_BUFFER, shape.VBO);
    glBufferData(GL_ARRAY_BUFFER, line_coords.size() * sizeof(float), line_coords.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    shape.vertex_len = line_coords.size() / 2;

    shape.x_y_coords = std::nullopt;

    return shape;
}



};