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
#include "emscripten_fetch.hpp"

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
    std::optional<GLuint> texture_id;

    Shape(){

    }

    void AssignShaderProgram(GLuint programID){
        shaderprogram_id = programID;
    }

    static Shape InitCube(float width, std::string imagePath){
        Shape shape;
        shape.shape_type = ShapeType::CUBE;
    
        float a = width/2.0f; // Half width for vertex positions
        
        // Vertices with positions and texture coordinates for each face
        // Format: x, y, z, tex_u, tex_v
        float vertices[] = {
            // Front face (positive Z)
            -a, -a,  a,   0.0f, 1.0f,  // bottom-left
             a, -a,  a,   1.0f, 1.0f,  // bottom-right
             a,  a,  a,   1.0f, 0.0f,  // top-right
            -a,  a,  a,   0.0f, 0.0f,  // top-left
            
            // Back face (negative Z)
            -a, -a, -a,   1.0f, 1.0f,  // bottom-left
            -a,  a, -a,   1.0f, 0.0f,  // top-left
             a,  a, -a,   0.0f, 0.0f,  // top-right
             a, -a, -a,   0.0f, 1.0f,  // bottom-right
            
            // Top face (positive Y)
            -a,  a,  a,   0.0f, 1.0f,  // front-left
             a,  a,  a,   1.0f, 1.0f,  // front-right
             a,  a, -a,   1.0f, 0.0f,  // back-right
            -a,  a, -a,   0.0f, 0.0f,  // back-left
            
            // Bottom face (negative Y)
            -a, -a,  a,   0.0f, 0.0f,  // front-left
            -a, -a, -a,   0.0f, 1.0f,  // back-left
             a, -a, -a,   1.0f, 1.0f,  // back-right
             a, -a,  a,   1.0f, 0.0f,  // front-right
            
            // Right face (positive X)
             a, -a,  a,   0.0f, 1.0f,  // bottom-front
             a, -a, -a,   1.0f, 1.0f,  // bottom-back
             a,  a, -a,   1.0f, 0.0f,  // top-back
             a,  a,  a,   0.0f, 0.0f,  // top-front
            
            // Left face (negative X)
            -a, -a,  a,   1.0f, 1.0f,  // bottom-front
            -a,  a,  a,   1.0f, 0.0f,  // top-front
            -a,  a, -a,   0.0f, 0.0f,  // top-back
            -a, -a, -a,   0.0f, 1.0f   // bottom-back
        };
    
        // Indices for drawing the triangles (6 faces, 2 triangles per face)
        GLuint indices[] = {
            0,  1,  2,    2,  3,  0,  // Front face
            4,  5,  6,    6,  7,  4,  // Back face
            8,  9,  10,   10, 11, 8,  // Top face
            12, 13, 14,   14, 15, 12, // Bottom face
            16, 17, 18,   18, 19, 16, // Right face
            20, 21, 22,   22, 23, 20  // Left face
        };
    
        // Generate and bind VAO
        glGenVertexArrays(1, &shape.VAO);
        glBindVertexArray(shape.VAO);
    
        // Generate and bind VBO
        glGenBuffers(1, &shape.VBO);
        glBindBuffer(GL_ARRAY_BUFFER, shape.VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    
        // Position attribute
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        
        // Texture coordinate attribute
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);
    
        // Store the number of indices
        shape.vertex_len = sizeof(indices) / sizeof(GLuint);
    
        // Generate and bind EBO
        GLuint NewEBO;
        glGenBuffers(1, &NewEBO);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, NewEBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
        shape.EBO = NewEBO;
    
        // Create and setup texture
        GLuint texture;
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);
    
        // Set texture parameters - these can significantly affect texture appearance
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        
        // Use GL_LINEAR for a smoother appearance, or GL_NEAREST for pixel-perfect textures
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    
        // Load image using the provided path parameter
        std::cout << "Loading texture from: " << imagePath << std::endl;
        Image texture_image = LoadImageFromURL(imagePath);
    
        if (texture_image.data) {
            std::cout << "Texture loaded successfully: " << texture_image.width << "x" << texture_image.height 
                      << " with " << texture_image.channels << " channels" << std::endl;
            
            // Always use RGBA format since we forced 4 channels in stbi_load_from_memory
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texture_image.width, texture_image.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, texture_image.data);
            glGenerateMipmap(GL_TEXTURE_2D);
        } else {
            std::cout << "Failed to load texture!" << std::endl;
        }
    
        shape.texture_id = texture;
    
        // Unbind VAO, VBO, and texture to prevent accidental modification
        glBindVertexArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindTexture(GL_TEXTURE_2D, 0);
    
        return shape;
    }

    void SupplyTransformationValues(
        glm::vec3 rotation_angles,
        glm::vec3 translation_positions,
        glm::vec3 scaling_xyz
    ){
        if (!shaderprogram_id.has_value()){
            std::cout << "There is no shaderprogram assigned to this shape object..." << std::endl;
            return;
        }
        
        GLint rotation_uniform = glGetUniformLocation(shaderprogram_id.value(), "rotation");
        GLint translation_uniform = glGetUniformLocation(shaderprogram_id.value(), "translation");
        GLint scaling_uniform = glGetUniformLocation(shaderprogram_id.value(), "scaling");

        // Check if uniforms were found
        if (rotation_uniform == -1)
            std::cout << "Warning: 'rotation' uniform not found in shader" << std::endl;
        if (translation_uniform == -1)
            std::cout << "Warning: 'translation' uniform not found in shader" << std::endl;
        if (scaling_uniform == -1)
            std::cout << "Warning: 'scaling' uniform not found in shader" << std::endl;

        glUniform3fv(rotation_uniform, 1, glm::value_ptr(rotation_angles));
        glUniform3fv(translation_uniform, 1, glm::value_ptr(translation_positions));
        glUniform3fv(scaling_uniform, 1, glm::value_ptr(scaling_xyz));
    }
};