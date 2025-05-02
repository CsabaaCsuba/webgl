#pragma once
#include <iostream>
#include <vector>
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

    Shape(){

    }

    void AssignShaderProgram(GLuint programID){
        shaderprogram_id = programID;
    }

    static Shape InitRectangle(uint16_t x, uint16_t y, uint16_t width, uint16_t height, float windowWidth, float windowHeight){
        Shape shape;

        float left_up_point_x = (x / windowWidth) * 2 - 1; 
        float left_up_point_y = 1 - (y / windowHeight) * 2; 
    
        float right_up_point_x = ((x + (float)width) / windowWidth) * 2 - 1;
        float right_up_point_y = left_up_point_y; //they share y coordinates
    
    
        float left_down_point_x = left_up_point_x; //they share x coordinates
        float left_down_point_y = 1 - ((y + (float)height) / windowHeight) * 2;
    
        float right_down_point_x = right_up_point_x; //same coordinate
        float right_down_point_y = left_down_point_y; //nothing has to be calculated for the right down point

        float rec_vertices[] = {
            left_up_point_x, left_up_point_y, 0.0,
            right_up_point_x, right_up_point_y, 0.0,
            left_down_point_x, left_down_point_y, 0.0,
            right_down_point_x, right_down_point_y, 0.0,
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
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), NULL);
        glEnableVertexAttribArray(0);
        // std::cout << "vertex lens: " << sizeof(rec_indices)/sizeof(GLuint) << std::endl;
        // this->VertexLens.push_back(sizeof(rec_indices)/sizeof(GLuint));
        shape.vertex_len = sizeof(rec_indices)/sizeof(GLuint);


        GLuint NewEBO;
        glGenBuffers(1, &NewEBO);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, NewEBO);

        shape.EBO = NewEBO;

        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(rec_indices), rec_indices, GL_STATIC_DRAW);


        return shape;
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

    return shape;
}



};