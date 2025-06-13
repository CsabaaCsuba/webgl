#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <GLES3/gl3.h>
#include <GLFW/glfw3.h>
#include "enums.h"
#include "shader_compiler.h"
#include "types.h"
#include "fonts.h"
#include "MyMath.h"
#include "mem/pool.h"
#include "mem/arena.h"

// #define ARENA_SIZE 1024 * 1024 //1 MB

void set_text_projection(Mat4 matrix, char* location_name, uint32_t shader_program){
    glUseProgram(shader_program);
    glUniformMatrix4fv(glGetUniformLocation(shader_program, location_name), 1, GL_FALSE, matrix.data);
}

void error_callback(int error, const char* description) {
    fprintf(stderr, "GLFW Error: %s\n", description);
}

int main() {
    //log setup
    freopen("error.log", "w", stderr);

    // Initialize GLFW
    glfwSetErrorCallback(error_callback);
    if (!glfwInit()) {
        fprintf(stderr, "Failed to initialize GLFW\n");
        return -1;
    }
    
    // Configure GLFW for OpenGL ES 3.0
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
    
    // Create window
    GLFWwindow* window = glfwCreateWindow(800, 600, "Rotating Cube", NULL, NULL);
    if (!window) {
        fprintf(stderr, "Failed to create GLFW window\n");
        glfwTerminate();
        return -1;
    }
    
    glfwMakeContextCurrent(window);
    
    // Create shader program
    uint32_t shader_program, text_shader; 
    ShaderCompilerResult result = create_shader_program("./shaders/vertex.glsl", "./shaders/fragment.glsl", &shader_program);
    ShaderCompilerResult text_result = create_shader_program("./shaders/text/text_vert.glsl", "./shaders/text/text_frag.glsl", &text_shader);

    if (result == COMPILE_FAIL || text_result == COMPILE_FAIL){
        printf("shader compilation failed.\n");
        return -1;
    }

    // Enable depth testing
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    Font font1;

    // Initialize font (Ubuntu Linux x64 font paths)
    const char* font_paths[] = {
        // "/usr/share/fonts/truetype/noto/NotoSansCJK-Regular.ttc",
        "/usr/share/fonts/opentype/noto/NotoSansCJK-Regular.ttc", 
        "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",
        "/usr/share/fonts/TTF/DejaVuSans.ttf",
        "/usr/share/fonts/truetype/liberation/LiberationSans-Regular.ttf",
        "/usr/share/fonts/truetype/ubuntu/Ubuntu-R.ttf",
        "/usr/share/fonts/truetype/noto/NotoSans-Regular.ttf"
    };
    
    int font_loaded = 0;
    for (int i = 0; i < sizeof(font_paths)/sizeof(font_paths[0]); i++) {
        printf("Trying font: %s\n", font_paths[i]);
        if (init_font(&font1, font_paths[i], 48)) {
            printf("Successfully loaded font: %s\n", font_paths[i]);
            font_loaded = 1;
            break;
        }
    }
    
    if (!font_loaded) {
        fprintf(stderr, "Failed to load any font. Please install fonts with Unicode support.\n");
        // fprintf(stderr, "  sudo apt install fonts-noto-cjk fonts-dejavu fonts-liberation fonts-ubuntu\n");
        return -1;
    }
    
    // Text samples
    char* raw_english_text = "Hello, World!";

    Text text1;
    Result text_init_res = init_text(&text1, font1, raw_english_text, text_shader);

    if (text_init_res != SUCCESS){
        printf("Text initializiation unsuccesfull...\n");
        return -1;
    }

    float red[3] = {1.0f, 0.0f, 0.0f};

    Mat4 ortho = Mat4_create_orthographic(0.0f, 800.0f, 0.0f, 600.0f, -1.0f, 1.0f);

    // Main loop
    while (!glfwWindowShouldClose(window)) {
        // Clear screen
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        set_text_projection(ortho, "projection", text_shader);
        render_text_hb(&font1, text1, 10.0f, 10.0f, 0.5f, red);

    
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    
    glDeleteProgram(shader_program);
    
    glfwTerminate();
    return 0;
}

