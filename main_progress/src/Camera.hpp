#pragma once
#include <GLES3/gl3.h>
#include <emscripten/html5.h>
#include <stdio.h>
#define GLM_ENABLE_EXPERIMENTAL 
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "glm/gtx/rotate_vector.hpp"
#include "glm/gtx/vector_angle.hpp"

#include "utils/MyMath.hpp"

constexpr float PI_180 = (float)M_PI / 180.0f;

class Camera {
public:
    glm::vec3 Position_g;
    Vec3 Position;
    glm::vec3 Orientation_g = glm::vec3(0.0f, 0.0f, -1.0f);
    Vec3 Orientation = Vec3::create(0.0f, 0.0f, -1.0f);
    glm::vec3 Up_g = glm::vec3(0.0f, 1.0f, 0.0f);
    Vec3 Up = Vec3::create(0.0f, 1.0f, 0.0f);
    glm::mat4 ViewMatrix_g = glm::mat4x4(1.0f);
    Mat4 ViewMatrix = Mat4::create(1.0f);

    float width;
    float height;

    float speed = 0.02f;
    float sensitivity = 10.0f;

    Camera(){

    }

    Camera(int width, int height, Vec3 position){
        this->width = (float)width;
        this->height = (float)height;
        this->Position = position;
        this->Position_g = glm::vec3(position.data[0], position.data[1], position.data[2]);
    }

    void SetProjection(float FOVdeg, float nearPlane, float farPlane){
        glm::mat4 view_g = glm::mat4(1.0f);
        Mat4 view(1.0f);
        glm::mat4 projection_g = glm::mat4(1.0f);
        Mat4 projection(1.0f);

        view_g = glm::lookAt(Position_g, Position_g + Orientation_g, Up_g);
        view = Mat4::create_view(Position, Position + Orientation, Up);
        projection_g = glm::perspective(glm::radians(FOVdeg), (float)(width) / (float)(height), nearPlane, farPlane);
        projection = Mat4::create_perspective(PI_180 * FOVdeg, width/height, nearPlane, farPlane);
        ViewMatrix = projection * view;
        ViewMatrix_g = projection_g * view_g;

        // Print GLM matrices for debugging
        printf("=== GLM View Matrix ===\n");
        for (int i = 0; i < 4; i++) {
            printf("%8.3f %8.3f %8.3f %8.3f\n", 
                view_g[0][i], view_g[1][i], view_g[2][i], view_g[3][i]);
        }
        
        printf("\n=== GLM Projection Matrix ===\n");
        for (int i = 0; i < 4; i++) {
            printf("%8.3f %8.3f %8.3f %8.3f\n", 
                projection_g[0][i], projection_g[1][i], projection_g[2][i], projection_g[3][i]);
        }
        
        printf("\n=== GLM Final ViewMatrix_g ===\n");
        for (int i = 0; i < 4; i++) {
            printf("%8.3f %8.3f %8.3f %8.3f\n", 
                ViewMatrix_g[0][i], ViewMatrix_g[1][i], ViewMatrix_g[2][i], ViewMatrix_g[3][i]);
        }

        // Print your custom matrices for comparison
        printf("\n=== Custom View Matrix ===\n");
        for (int i = 0; i < 4; i++) {
            printf("%8.3f %8.3f %8.3f %8.3f\n", 
                view.data[i*4], view.data[i*4+1], view.data[i*4+2], view.data[i*4+3]);
        }
        
        printf("\n=== Custom Projection Matrix ===\n");
        for (int i = 0; i < 4; i++) {
            printf("%8.3f %8.3f %8.3f %8.3f\n", 
                projection.data[i*4], projection.data[i*4+1], projection.data[i*4+2], projection.data[i*4+3]);
        }
        
        printf("\n=== Custom Final ViewMatrix ===\n");
        for (int i = 0; i < 4; i++) {
            printf("%8.3f %8.3f %8.3f %8.3f\n", 
                ViewMatrix.data[i*4], ViewMatrix.data[i*4+1], ViewMatrix.data[i*4+2], ViewMatrix.data[i*4+3]);
        }
        printf("\n");
    }

    void UpdateMatrix(GLuint shaderID, const char* uniform){
        glUniformMatrix4fv(glGetUniformLocation(shaderID, uniform), 1, GL_TRUE, ViewMatrix.data);
        // glUniformMatrix4fv(glGetUniformLocation(shaderID, uniform), 1, GL_FALSE, glm::value_ptr(ViewMatrix_g));
    }

};