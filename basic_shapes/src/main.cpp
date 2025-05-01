#include <GLES3/gl3.h>
#include <EGL/egl.h>
#include <emscripten.h>
#include <emscripten/html5.h>
#include <math.h>
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <emscripten/fetch.h>
#include <string.h>
#include "Shape.hpp"
#include "../include/emscripten_fetch.hpp"

class Program{
private:
    float WindowWidth;
    float WindowHeight;
public:
    std::vector<GLuint> VAO_ids; // vertex array object -> the id of a configuration
    std::vector<GLuint> VBO_ids; // vertex buffer object -> stores the vertices that will be drawn
    std::vector<GLuint> EBO_ids; // element buffer object -> stores the element indices of the drawn things
    std::vector<GLuint> ShaderProgram_ids;
    std::vector<size_t> VertexLens;

    Program(double WindowWidth, double WindowHeight){
        this->WindowWidth = (float)WindowWidth;
        this->WindowHeight = (float)WindowHeight;
    }

    bool AddRectangle(uint16_t x, uint16_t y, uint16_t width, uint16_t height){
        float left_up_point_x = (x / this->WindowWidth) * 2 - 1; 
        float left_up_point_y = 1 - (y / this->WindowHeight) * 2; 
    
        float right_up_point_x = ((x + (float)width) / this->WindowWidth) * 2 - 1;
        float right_up_point_y = left_up_point_y; //they share y coordinates
    
    
        float left_down_point_x = left_up_point_x; //they share x coordinates
        float left_down_point_y = 1 - ((y + (float)height) / this->WindowHeight) * 2;
    
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

        VAO_ids.push_back(0);
        glGenVertexArrays(1, &VAO_ids.back());
        glBindVertexArray(VAO_ids.back());

        VBO_ids.push_back(0);
        GLuint* CurrentBufferPtr = &VBO_ids.back();
        glGenBuffers(1, CurrentBufferPtr);
        glBindBuffer(GL_ARRAY_BUFFER, *CurrentBufferPtr);

        glBufferData(GL_ARRAY_BUFFER, sizeof(rec_vertices), rec_vertices, GL_STATIC_DRAW); //GL_STATIC_DRAW means it won't be changed
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), NULL);
        glEnableVertexAttribArray(0);
        // std::cout << "vertex lens: " << sizeof(rec_indices)/sizeof(GLuint) << std::endl;
        this->VertexLens.push_back(sizeof(rec_indices)/sizeof(GLuint));

        EBO_ids.push_back(0);
        CurrentBufferPtr = &EBO_ids.back();
        glGenBuffers(1, &EBO_ids.back());
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, *CurrentBufferPtr);

        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(rec_indices), rec_indices, GL_STATIC_DRAW);

        return true;
    }

    bool AddCircle(uint16_t x, uint16_t y, uint16_t r, uint16_t precision) {
        const int segments = precision; // Number of segments to approximate the circle
        std::vector<float> vertices;
        std::vector<GLuint> indices;

        float angle = 2.0f * M_PI / float(segments);
        
        // Center point
        float center_x = (x / this->WindowWidth) * 2 - 1;
        float center_y = 1 - (y / this->WindowHeight) * 2;
        float center_z = 0.0f;
        
        // Radius in normalized coordinates
        float radius_x = (r / this->WindowWidth) * 2;
        float radius_y = (r / this->WindowHeight) * 2;
        
        // Add center point
        vertices.push_back(center_x);
        vertices.push_back(center_y);
        vertices.push_back(center_z);
        
        // Add points around the circle
        for (int i = 0; i <= segments; i++) {
            
            float x_pos = center_x + radius_x * cos(angle*i);
            float y_pos = center_y + radius_y * sin(angle*i);
            // std::cout << i << ". angle: " << angle << "position: " << x_pos << "x " << y_pos << "y" << std::endl;
            
            vertices.push_back(x_pos);
            vertices.push_back(y_pos);
            vertices.push_back(center_z);
        }
        
        // Create indices for triangle fan
        for (int i = 1; i <= segments; i++) {
            indices.push_back(0);       // Center point
            indices.push_back(i);
            indices.push_back(i + 1);
        }
        // Close the circle
        indices.push_back(0);
        indices.push_back(segments);
        indices.push_back(1);
        
        // Create VAO, VBO, and EBO
        VAO_ids.push_back(0);
        glGenVertexArrays(1, &VAO_ids.back());
        glBindVertexArray(VAO_ids.back());
        
        VBO_ids.push_back(0);
        GLuint* CurrentBufferPtr = &VBO_ids.back();
        glGenBuffers(1, CurrentBufferPtr);
        glBindBuffer(GL_ARRAY_BUFFER, *CurrentBufferPtr);
        
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), NULL);
        glEnableVertexAttribArray(0);
        
        this->VertexLens.push_back(indices.size());
        
        EBO_ids.push_back(0);
        CurrentBufferPtr = &EBO_ids.back();
        glGenBuffers(1, CurrentBufferPtr);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, *CurrentBufferPtr);
        
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), indices.data(), GL_STATIC_DRAW);
        
        return true;
    }
    void DrawShape(size_t VAO_idx, size_t Shader_idx){
        GLuint current_vao = this->VAO_ids[VAO_idx];
        glUseProgram(this->ShaderProgram_ids[Shader_idx]);
        glBindVertexArray(current_vao);
        glDrawElements(GL_TRIANGLES, VertexLens[VAO_idx], GL_UNSIGNED_INT, 0);

        glBindVertexArray(0);
    }

    static void DrawShape(GLuint VAO_id, GLuint ShaderProgram_id, GLuint VertexLength){
        glUseProgram(ShaderProgram_id);
        glBindVertexArray(VAO_id);
        glDrawElements(GL_TRIANGLES, VertexLength, GL_UNSIGNED_INT, 0);

        glBindVertexArray(0);
    }


    static GLuint CompileShader(GLenum type, const char *source_file){
        std::string fileContents = downloadContent(source_file);
        std::cout << "string contents: " << fileContents << std::endl;
        GLuint shader = glCreateShader(type);
        const GLchar* whatever = fileContents.data();
        glShaderSource(shader, 1, &whatever, NULL);
        glCompileShader(shader);

        GLint status;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
        if (status != GL_TRUE){
            char buffer[512];
            glGetShaderInfoLog(shader, 512, NULL, buffer);
            printf("Shader (type %u) compilation failed: %s\n", type, buffer);
            return 0;
        }

        return shader;
    }

    bool CreateShaderProgram(const char *vertex_source, const char *fragment_source){
        GLuint vertexShader_id;
        GLuint fragmentShader_id;

        vertexShader_id = Program::CompileShader(GL_VERTEX_SHADER, vertex_source);
        fragmentShader_id = Program::CompileShader(GL_FRAGMENT_SHADER, fragment_source);

        GLuint shaderProgram_id = glCreateProgram();
        glAttachShader(shaderProgram_id, vertexShader_id);
        glAttachShader(shaderProgram_id, fragmentShader_id);
        glLinkProgram(shaderProgram_id);

        GLint status;
        glGetProgramiv(shaderProgram_id, GL_LINK_STATUS, &status);
        if (status != GL_TRUE){
            char buffer[512];
            glGetProgramInfoLog(shaderProgram_id, 512, NULL, buffer);
            printf("Program linking failed: %s\n", buffer);
            glDeleteShader(vertexShader_id);
            glDeleteShader(fragmentShader_id);
            return false;
        }

        ShaderProgram_ids.push_back(shaderProgram_id);

        glDeleteShader(vertexShader_id);
        glDeleteShader(fragmentShader_id);

        return true;
    }
};

Program *program;

void MainLoop(){
    glClearColor(0.0, 0.0, 0.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT);

    Program::DrawShape(program->VAO_ids[0], program->ShaderProgram_ids[0], program->VertexLens[0]);

    program->DrawShape(1, 0);

    //this might not work
    eglSwapBuffers(eglGetCurrentDisplay(), eglGetCurrentSurface(EGL_DRAW));
}

int main(){
    std::cout << "halo" << std::endl;
    EmscriptenWebGLContextAttributes attrs;
    emscripten_webgl_init_context_attributes(&attrs);
    attrs.majorVersion = 2; // WebGL 2.0 corresponds to OpenGL ES 3.0
    attrs.minorVersion = 0;

    EMSCRIPTEN_WEBGL_CONTEXT_HANDLE context = emscripten_webgl_create_context("#canvas", &attrs);
    emscripten_webgl_make_context_current(context);

    double canvasWidth, canvasHeight;
    EMSCRIPTEN_RESULT res = emscripten_get_element_css_size("#canvas", &canvasWidth, &canvasHeight);

    program = new Program(canvasWidth, canvasHeight);

    GLuint shader = program->CreateShaderProgram("./shaders/vertex.glsl", "./shaders/fragment.glsl");

    program->AddRectangle(100, 100, 100, 100);

    program->AddCircle(300, 300, 75, 180);

    emscripten_set_main_loop(MainLoop, 0, 1);

    return 0;
}