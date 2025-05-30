#pragma once
#include <iostream>
#include <vector>
#include <array>
#include <optional>
#include <GLES3/gl3.h>
// #define GLM_ENABLE_EXPERIMENTAL 
// #include "glm/glm.hpp"
// #include "glm/gtc/matrix_transform.hpp"
// #include "glm/gtc/type_ptr.hpp"
// #include "glm/gtx/rotate_vector.hpp"
// #include "glm/gtx/vector_angle.hpp"
// #include "glm/ext/matrix_clip_space.hpp"
// #include "glm/ext/matrix_transform.hpp"
#include "emscripten_fetch.hpp"
#include "utils/MyMath.hpp"

#define MESH_COUNT 4

typedef enum ShapeType {
    TRIANGLE,
    MESH,
    PLAIN,
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
    Vec3 position;
    Mat4 scale = Mat4::identity();  //this gives us 1x scaling at every axis
    Quat rotation;
    Mat4 translation;

    Shape(){

    }

    void AssignShaderProgram(GLuint programID){
        shaderprogram_id = programID;
    }

    static void TexturedOpenglConfig(Shape* shape, float* vertices, uint32_t vertices_len, GLuint* indices, uint32_t indices_len, std::string imagePath){
        // Shape shape = *shape;

        // Generate and bind VAO
        glGenVertexArrays(1, &shape->VAO);
        glBindVertexArray(shape->VAO);
    
        // Generate and bind VBO
        glGenBuffers(1, &shape->VBO);
        glBindBuffer(GL_ARRAY_BUFFER, shape->VBO);
        glBufferData(GL_ARRAY_BUFFER, vertices_len*sizeof(float), vertices, GL_STATIC_DRAW);
    
        // Position attribute
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        
        // Texture coordinate attribute
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);
    
        // Store the number of indices
        shape->vertex_len = indices_len;
    
        // Generate and bind EBO
        GLuint NewEBO;
        glGenBuffers(1, &NewEBO);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, NewEBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices_len*sizeof(GLuint), indices, GL_STATIC_DRAW);
        shape->EBO = NewEBO;
    
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
    
        shape->texture_id = texture;
    
        // Unbind VAO, VBO, and texture to prevent accidental modification
        glBindVertexArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindTexture(GL_TEXTURE_2D, 0);
    }

    static Shape InitCube(float width, std::string imagePath){
        float a = width/2.0f; // Half width for vertex positions
        
        Shape shape;
        shape.shape_type = ShapeType::CUBE;
        shape.translation = Mat4::identity();
        shape.translation.data[3] = 0.0f;
        shape.translation.data[7] = 0.0f;
        shape.translation.data[11] = 0.0f;
        shape.position = Vec3::create(0.0f, 0.0f, 0.0f);
    
        
        
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

        Shape::TexturedOpenglConfig(&shape, vertices, sizeof(vertices)/sizeof(float), indices, sizeof(indices)/sizeof(GLuint), imagePath);
    
    
        return shape;
    }

    static Shape InitPlain(float x, float z, std::string imagePath){
        Shape shape;
        shape.shape_type = ShapeType::PLAIN;

        shape.translation = Mat4::identity();
        shape.translation.data[12] = 0.0f;
        shape.translation.data[13] = 0.0f;
        shape.translation.data[14] = 0.0f;
        shape.position = Vec3::create(0.0f, 0.0f, 0.0f);

        const float x_half = x / 2.0f;
        const float z_half = z / 2.0f;

        float vertices[] = {
            x_half, 0.0f, z_half, 1.0f, 1.0f,
            x_half, 0.0f, -z_half, 1.0f, 0.0f,
            -x_half, 0.0f, -z_half, 0.0f, 0.0f,
            -x_half, 0.0f, z_half, 0.0f, 1.0f,
        };

        GLuint indices[] = {
            0, 1, 2,
            2, 3, 0
        };

        Shape::TexturedOpenglConfig(&shape, vertices, sizeof(vertices)/sizeof(float), indices, sizeof(indices)/sizeof(GLuint), imagePath);

        return shape;
    }

    static Shape InitSphere(float d, Vec3 pos, std::string imagePath){
        Shape shape;
        shape.shape_type = ShapeType::SPHERE;
        shape.translation = Mat4::identity();
        shape.translation.data[12] = 0.0f;
        shape.translation.data[13] = 0.0f;
        shape.translation.data[14] = 0.0f;
        shape.position = pos;

        // Sphere parameters
        const int longitudeSegments = 32; // Number of vertical segments
        const int latitudeSegments = 16;  // Number of horizontal segments
        
        // Calculate total vertices and indices
        const int totalVertices = (latitudeSegments + 1) * (longitudeSegments + 1);
        const int totalIndices = latitudeSegments * longitudeSegments * 6;
        
        // Allocate arrays
        std::vector<float> vertices;
        std::vector<GLuint> indices;
        vertices.reserve(totalVertices * 5); // 5 floats per vertex (x,y,z,u,v)
        indices.reserve(totalIndices);
        
        // Generate vertices
        for (int lat = 0; lat <= latitudeSegments; ++lat) {
            float theta = lat * M_PI / latitudeSegments; // 0 to PI
            float sinTheta = sin(theta);
            float cosTheta = cos(theta);
            
            for (int lon = 0; lon <= longitudeSegments; ++lon) {
                float phi = lon * 2.0f * M_PI / longitudeSegments; // 0 to 2*PI
                float sinPhi = sin(phi);
                float cosPhi = cos(phi);
                
                // Calculate vertex position
                float x = d * sinTheta * cosPhi;
                float y = d * cosTheta;
                float z = d * sinTheta * sinPhi;
                
                // Calculate texture coordinates
                float u = (float)lon / longitudeSegments;
                float v = (float)lat / latitudeSegments;
                
                // Add vertex data (position + texture coords)
                vertices.push_back(x);
                vertices.push_back(y);
                vertices.push_back(z);
                vertices.push_back(u);
                vertices.push_back(v);
            }
        }
        
        // Generate indices
        for (int lat = 0; lat < latitudeSegments; ++lat) {
            for (int lon = 0; lon < longitudeSegments; ++lon) {
                // Calculate vertex indices for current quad
                int first = lat * (longitudeSegments + 1) + lon;
                int second = first + longitudeSegments + 1;
                
                // First triangle
                indices.push_back(first);
                indices.push_back(second);
                indices.push_back(first + 1);
                
                // Second triangle
                indices.push_back(second);
                indices.push_back(second + 1);
                indices.push_back(first + 1);
            }
        }
        
        // Configure OpenGL objects
        Shape::TexturedOpenglConfig(&shape, vertices.data(), vertices.size(), 
                                indices.data(), indices.size(), imagePath);
        
        return shape;
    }

    void SupplyTransformationValues(Mat4 VP_matrix){
        if (!shaderprogram_id.has_value()){
            std::cout << "There is no shaderprogram assigned to this shape object..." << std::endl;
            return;
        }
        
        GLint mvp_matrix_uniform = glGetUniformLocation(shaderprogram_id.value(), "mvp_matrix");

        // Check if uniforms were found
        if (mvp_matrix_uniform == -1)
            std::cout << "Warning: 'rotation' uniform not found in shader" << std::endl;


        //calculating the MVP matrix on the CPU
        Mat4 model = translation * rotation.to_matrix() * scale;
        Mat4 MVP = VP_matrix * model;
        glUniformMatrix4fv(mvp_matrix_uniform, 1, false, MVP.data);
    }

    void save_position(){
        translation = Mat4::identity();
        translation.data[12] = position.data[0]; // Assuming column-major
        translation.data[13] = position.data[1];
        translation.data[14] = position.data[2];
    }
};