#include <GLES3/gl3.h>
#include <EGL/egl.h>
#include <emscripten.h>
#include <emscripten/html5.h>
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <emscripten/fetch.h>
#include <string.h>

// Structure to pass data between fetch callbacks
struct FetchData {
    std::string content;
    bool completed = false;
    bool success = false;
};

// Called when fetch succeeds
void downloadSucceeded(emscripten_fetch_t *fetch) {
    FetchData* data = (FetchData*)fetch->userData;
    
    if (fetch->data && fetch->numBytes > 0) {
        data->content.assign(fetch->data, fetch->numBytes);
        data->success = true;
    }
    
    data->completed = true;
    emscripten_fetch_close(fetch); // Free resources
}

// Called when fetch fails
void downloadFailed(emscripten_fetch_t *fetch) {
    FetchData* data = (FetchData*)fetch->userData;
    data->completed = true;
    data->success = false;
    emscripten_fetch_close(fetch);
}

// Main download function (synchronous-style using async fetch)
std::string downloadContent(const std::string& url) {
    FetchData fetchData;
    
    emscripten_fetch_attr_t attr;
    emscripten_fetch_attr_init(&attr);
    strcpy(attr.requestMethod, "GET");
    
    // Configure fetch attributes
    attr.attributes = EMSCRIPTEN_FETCH_LOAD_TO_MEMORY;
    attr.onsuccess = downloadSucceeded;
    attr.onerror = downloadFailed;
    attr.userData = &fetchData;
    
    // Start the fetch
    emscripten_fetch(&attr, url.c_str());
    
    // Wait for completion (in a real app, you'd use async patterns)
    while (!fetchData.completed) {
        emscripten_sleep(100); // Don't block the main thread indefinitely
    }
    
    if (fetchData.success) {
        return fetchData.content;
    }
    return "";
}
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

    Program(uint16_t WindowWidth, uint16_t WindowHeight){
        this->WindowWidth = WindowWidth;
        this->WindowHeight = WindowHeight;
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
        std::cout << "vertex lens: " << sizeof(rec_indices)/sizeof(GLuint) << std::endl;
        this->VertexLens.push_back(sizeof(rec_indices)/sizeof(GLuint));

        EBO_ids.push_back(0);
        CurrentBufferPtr = &EBO_ids.back();
        glGenBuffers(1, &EBO_ids.back());
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, *CurrentBufferPtr);

        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(rec_indices), rec_indices, GL_STATIC_DRAW);

        return true;
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

    program = new Program(800, 600);

    GLuint shader = program->CreateShaderProgram("./shaders/vertex.glsl", "./shaders/fragment.glsl");

    program->AddRectangle(100, 100, 100, 100);

    emscripten_set_main_loop(MainLoop, 0, 1);

    return 0;
}