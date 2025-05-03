#define GLM_ENABLE_EXPERIMENTAL 
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "glm/gtx/rotate_vector.hpp"
#include "glm/gtx/vector_angle.hpp"
#include "glm/ext/matrix_clip_space.hpp"
#include "glm/ext/matrix_transform.hpp"
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
#include "Camera.hpp"
#include "CanvasManagement.hpp"
#include "../include/emscripten_fetch.hpp"

class Program{
public:
    float WindowWidth;
    float WindowHeight;
    Camera cam;
    std::vector<Shape> shapes;

    Program(double WindowWidth, double WindowHeight, glm::vec3 position){
        this->WindowWidth = (float)WindowWidth;
        this->WindowHeight = (float)WindowHeight;
        Camera new_cam((int)WindowWidth, (int) WindowHeight, position);
        this->cam = new_cam;
    }

    void AddShape(Shape shape){
        shapes.push_back(shape);
    }


    void DrawShape(Shape shape){
        GLuint current_vao = shape.VAO;
        glUseProgram(shape.shaderprogram_id.value());
        glBindVertexArray(current_vao);

        if (shape.shape_type == ShapeType::MESH){
            for (int i = 0; i < shape.vertex_len; i += 4) {
                glDrawArrays(GL_LINE_LOOP, i, 4);
            }
            
        }
        else {
            glDrawElements(GL_TRIANGLES, shape.vertex_len, GL_UNSIGNED_INT, 0);
        }
        

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
            GLint logLength = 0;
            glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logLength);

            if (logLength > 1) {
                std::vector<char> log(logLength);
                glGetShaderInfoLog(shader, logLength, NULL, log.data());
                printf("Shader (type %u) compilation failed: %s\n", type, log.data());
            }
            return 0;
        }

        return shader;
    }

    static GLuint CreateShaderProgram(const char *vertex_source, const char *fragment_source){
        GLuint vertexShader_id = Program::CompileShader(GL_VERTEX_SHADER, vertex_source);
        if (vertexShader_id == 0){
            std::cout << "vertex shader compilation failed..." << std::endl;
        }
    
        GLuint fragmentShader_id = Program::CompileShader(GL_FRAGMENT_SHADER, fragment_source);
        if (fragmentShader_id == 0) {
            glDeleteShader(vertexShader_id);
            std::cout << "fragment shader compilation failed..." << std::endl;
        }
    
        GLuint shaderProgram_id = glCreateProgram();
        glAttachShader(shaderProgram_id, vertexShader_id);
        glAttachShader(shaderProgram_id, fragmentShader_id);
        glLinkProgram(shaderProgram_id);
    
        GLint status;
        glGetProgramiv(shaderProgram_id, GL_LINK_STATUS, &status);
        if (status != GL_TRUE){
            GLint logLength = 0;
            glGetProgramiv(shaderProgram_id, GL_INFO_LOG_LENGTH, &logLength);
            if (logLength > 1) {
                std::vector<char> log(logLength);
                glGetProgramInfoLog(shaderProgram_id, logLength, NULL, log.data());
                printf("Program linking failed: %s\n", log.data());
            } else {
                printf("Program linking failed with no log.\n");
            }
    
            glDeleteShader(vertexShader_id);
            glDeleteShader(fragmentShader_id);
            glDeleteProgram(shaderProgram_id);
        }
    
        glDeleteShader(vertexShader_id);
        glDeleteShader(fragmentShader_id);

        return shaderProgram_id;
    }

    void UseShader(size_t idx){
        if (idx > shapes.size()-1){
            std::cout << "IDX is larger than the available amount of shapes" << std::endl;
            return;
        }

        if (!shapes[idx].shaderprogram_id){
            std::cout << "ShaderProgram ID is not set yet..." << std::endl;
            return;
        }
        glUseProgram(shapes[idx].shaderprogram_id.value());
    }
    
};

Program *program;
EGLDisplay display;
EGLSurface surface;

double lastTime = 0;
double currentTime = 0;
double frameTime = 0;

void MainLoop(){
    currentTime = emscripten_get_now();
    frameTime = currentTime - lastTime;
    lastTime = currentTime;

    frameTime /= 1000;

    std::cout << "current frametime: " << frameTime << "s" << std::endl;

    glClearColor(0.2, 0.2, 0.2, 1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    program->UseShader(0);
    // program->cam.
    program->cam.Matrix(90.0f, 0.02, 100.0f, program->shapes[0].shaderprogram_id.value(), "CamMatrix");

    program->DrawShape(program->shapes[0]);


    //this might not work
    eglSwapBuffers(display, surface);
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

    glViewport(0, 0, (GLsizei)canvasWidth, (GLsizei)canvasHeight);

    program = new Program(canvasWidth, canvasHeight, glm::vec3(0.0f, 1.1f, 3.0f));

    // todo: write shaders with projection matrices
    GLuint shader = program->CreateShaderProgram("./shaders/3d/3d.vert", "./shaders/3d/3d_white.frag");

    Shape mesh = Shape::InitMesh(5);

    mesh.AssignShaderProgram(shader);


    GLuint simple_shader = program->CreateShaderProgram("./shaders/2d/vertex.glsl", "./shaders/2d/fragment.glsl");

    float rectWidth = 20.0f;
    float rectHeight = 20.0f;
    float rectX = canvasWidth / 2 - rectWidth / 2;
    float rectY = canvasHeight / 2 - rectHeight / 2;
    Shape lilrec = Shape::InitRectangle(rectX, rectY, rectWidth, rectHeight);

    lilrec.AssignShaderProgram(simple_shader);

    program->AddShape(mesh);
    program->AddShape(lilrec);

    mouseX = rectX;
    mouseY = rectY;

    glEnable(GL_DEPTH_TEST);

    //todo: add a spinning cube

    //this must be defined for buffer swapping (VSYNC)
    display = eglGetCurrentDisplay();
    surface = eglGetCurrentSurface(EGL_DRAW);

    emscripten_set_click_callback("#canvas", (void*)&(program->cam), EM_TRUE, immerse);
    //lilrec only lives in the main function, however it gets added as the second shape.
    emscripten_set_mousemove_callback("#canvas", (void*)&(program->cam), EM_TRUE, mouse_move);
    emscripten_set_keydown_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, (void*)&(program->cam), EM_TRUE, key_callback);
    emscripten_set_main_loop(MainLoop, 0, 1);

    return 0;
}