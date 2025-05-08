#pragma once
#include <iostream>
#include <vector>
#include <array>
#include <optional>
#include <GLES3/gl3.h>
#define GLM_ENABLE_EXPERIMENTAL 
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "glm/gtx/rotate_vector.hpp"
#include "glm/gtx/vector_angle.hpp"
#include "glm/ext/matrix_clip_space.hpp"
#include "glm/ext/matrix_transform.hpp"

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
    ShapeType shape_type = ShapeType::TRIANGLE;
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

    //a cube should have uniforms supplied to it
    static Shape InitCube(float width, std::string image_path){
        //todo: fetch image path
        Shape shape;
        shape.shape_type = ShapeType::CUBE;

        float a = width/2.0f;

        float vertices[] = {
            a, 0.0f,  a,
            -a, 0.0f,  a,
            -a, 0.0f, -a,
            a, 0.0f, -a,

            a, 2*a,  a,  
            -a, 2*a,  a,
            -a, 2*a, -a, 
            a, 2*a, -a,
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
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), NULL);
        glEnableVertexAttribArray(0);


        // std::cout << "vertex lens: " << sizeof(rec_indices)/sizeof(GLuint) << std::endl;
        // this->VertexLens.push_back(sizeof(rec_indices)/sizeof(GLuint));
        shape.vertex_len = sizeof(indices) / sizeof(GLuint); //because there are 3 data points for each vertex


        GLuint NewEBO;
        glGenBuffers(1, &NewEBO);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, NewEBO);

        shape.EBO = NewEBO;

        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

        shape.x_y_coords = std::nullopt;

        return shape;
    }

    void SupplyTransformationValues(
        glm::vec3 rotation_angles,
        glm::vec3 translation_positions,
        glm::vec3 scaling_xyz
    ){
        if (!shaderprogram_id.has_value()){
            std::cout << "There is no shaderprogram assigned to this shape object..." << std::endl;
        }
        GLint rotation_uniform = glGetUniformLocation(shaderprogram_id.value(), "rotation");
        GLint translation_uniform = glGetUniformLocation(shaderprogram_id.value(), "translation");
        GLint scaling_uniform = glGetUniformLocation(shaderprogram_id.value(), "scaling");

        glUniform3fv(rotation_uniform, 1, glm::value_ptr(rotation_angles));
        glUniform3fv(translation_uniform, 1, glm::value_ptr(translation_positions));
        glUniform3fv(scaling_uniform, 1, glm::value_ptr(scaling_xyz));
    }

};