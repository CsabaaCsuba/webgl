#pragma once
#include <GLES3/gl3.h>
#include <emscripten/html5.h>
#define GLM_ENABLE_EXPERIMENTAL 
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "glm/gtx/rotate_vector.hpp"
#include "glm/gtx/vector_angle.hpp"

class Camera {
public:
    glm::vec3 Position;
    glm::vec3 Orientation = glm::vec3(0.0f, 0.0f, -1.0f);
    glm::vec3 Up = glm::vec3(0.0f, 1.0f, 0.0f);

    int width;
    int height;

    float speed = 0.02f;
    float sensitivity = 100.0f;

    Camera(){

    }

    Camera(int width, int height, glm::vec3 position){
        this->width = width;
        this->height = height;
        this->Position = position;
    }

    void Matrix(float FOVdeg, float nearPlane, float farPlane, GLuint shaderID, const char* uniform){
        glm::mat4 view = glm::mat4(1.0f);
        glm::mat4 projection = glm::mat4(1.0f);

        view = glm::lookAt(Position, Position + Orientation, Up);
        projection = glm::perspective(glm::radians(FOVdeg), (float)(width) / (float)(height), nearPlane, farPlane);

        glUniformMatrix4fv(glGetUniformLocation(shaderID, uniform), 1, GL_FALSE, glm::value_ptr(projection * view));

    }

};