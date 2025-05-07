
#include "emscripten_fetch.hpp"
#include <GLES3/gl3.h>
#include <EGL/egl.h>
#include <emscripten.h>
#include <emscripten/html5.h>
#include <math.h>
#include <iostream>
#include <stdio.h>
#define STB_TRUETYPE_IMPLEMENTATION
#include <stb/stb_truetype.h>

typedef struct {
    GLuint texture_id;       // OpenGL texture ID
    int texture_width;       // Width of the texture atlas
    int texture_height;      // Height of the texture atlas
    stbtt_bakedchar* chars;  // Character data
    int first_char;          // First character in the range
    int num_chars;           // Number of characters in the range
} FontAtlas;

typedef struct Params {
    float window_width;
    float windot_height;
    GLuint shader;
    GLuint VAO;
    FontAtlas font;
} Params;

void render_text(FontAtlas* font, GLuint text_shader, GLuint VAO,
    GLuint uniform_text_color, GLuint uniform_text_sampler, 
    float x, float y, const char* text, float scale, float color[4]) {
   glUseProgram(text_shader);
   glUniform4fv(uniform_text_color, 1, color);
   glUniform1i(uniform_text_sampler, 0);
   
   // Bind texture
   glActiveTexture(GL_TEXTURE0);
   glBindTexture(GL_TEXTURE_2D, font->texture_id);
   
   // Bind VAO
   glBindVertexArray(VAO);
   
   float startX = x;
   
   // For each character in string
   for (int i = 0; text[i] != '\0'; i++) {
       int char_index = text[i] - font->first_char;
       if (char_index < 0 || char_index >= font->num_chars) 
           continue;
           
       stbtt_aligned_quad q;
       stbtt_GetBakedQuad(font->chars, font->texture_width, font->texture_height,
                          char_index, &x, &y, &q, 1);
       
       // Calculate model matrix for this character (position and scale)
       // This would be implemented with a proper matrix math library in real code
       
       // Draw the character quad using the VAO and EBO
       glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
   }
   
   // Unbind VAO
   glBindVertexArray(0);
}

GLuint CreateRectangle() {
   // Define vertices for a quad (2 triangles)
   float vertices[] = {
       // positions      // texture coords
       0.0f, 1.0f,       0.0f, 0.0f,   // top left
       1.0f, 1.0f,       1.0f, 0.0f,   // top right
       1.0f, 0.0f,       1.0f, 1.0f,   // bottom right
       0.0f, 0.0f,       0.0f, 1.0f    // bottom left
   };
   
   unsigned int indices[] = {
       0, 1, 2,  // first triangle
       0, 2, 3   // second triangle
   };
   
   // Generate buffers
   GLuint VAO, VBO, EBO;
   glGenVertexArrays(1, &VAO);
   glGenBuffers(1, &VBO);
   glGenBuffers(1, &EBO);
   
   // Bind buffers
   glBindVertexArray(VAO);
   
   glBindBuffer(GL_ARRAY_BUFFER, VBO);
   glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
   
   glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
   glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
   
   // Position attribute
   glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
   glEnableVertexAttribArray(0);
   
   // Texture coordinate attribute
   glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
   glEnableVertexAttribArray(1);
   
   // Unbind
   glBindBuffer(GL_ARRAY_BUFFER, 0);
   glBindVertexArray(0);

   return VAO;
}

void setup_emscripten(){
   EmscriptenWebGLContextAttributes attrs;
   emscripten_webgl_init_context_attributes(&attrs);
   attrs.majorVersion = 2; // WebGL 2.0 corresponds to OpenGL ES 3.0
   attrs.minorVersion = 0;

   EMSCRIPTEN_WEBGL_CONTEXT_HANDLE context = emscripten_webgl_create_context("#canvas", &attrs);
   emscripten_webgl_make_context_current(context);

}

//for example, OpenSans-Bold.ttf, 16.0f, 
FontAtlas CreateFont(std::string font_name, float font_size, int first_char, int num_chars){
   std::vector<uint8_t> binary_data = downloadBinaryContent(font_name);
   FontAtlas atlas = {0};
   atlas.first_char = first_char,
   atlas.num_chars = num_chars,
   atlas.texture_width = 512;
   atlas.texture_height = 512;

   uint8_t temp_bitmap[512*512];
   atlas.chars = (stbtt_bakedchar*)malloc(sizeof(stbtt_bakedchar)*num_chars);

   uint8_t* font_buffer = (uint8_t*)malloc(binary_data.size());

   stbtt_BakeFontBitmap(font_buffer, 0, font_size, temp_bitmap, atlas.texture_width, atlas.texture_height, first_char, num_chars, atlas.chars);

   glGenTextures(1, &atlas.texture_id);
   glBindTexture(GL_TEXTURE_2D, atlas.texture_id);
   glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA, atlas.texture_width, atlas.texture_height, 0, GL_ALPHA, GL_UNSIGNED_BYTE, temp_bitmap);

   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

   return atlas;
}

GLuint CompileShader(GLenum type, const char *source_file){
   std::string fileContents = downloadContent(source_file);
   std::cout << "string contents: " << fileContents << std::endl;
   GLuint shader = glCreateShader(type);
   const GLchar* shaderSource = fileContents.data();
   glShaderSource(shader, 1, &shaderSource, NULL);
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

GLuint CreateShaderProgram(const char *vertex_source, const char *fragment_source){
   GLuint vertexShader_id = CompileShader(GL_VERTEX_SHADER, vertex_source);
   if (vertexShader_id == 0){
       std::cout << "vertex shader compilation failed..." << std::endl;
   }

   GLuint fragmentShader_id = CompileShader(GL_FRAGMENT_SHADER, fragment_source);
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
           return 0;
       } else {
           printf("Program linking failed with no log.\n");
           return 0;
       }

       glDeleteShader(vertexShader_id);
       glDeleteShader(fragmentShader_id);
       glDeleteProgram(shaderProgram_id);
   }

   glDeleteShader(vertexShader_id);
   glDeleteShader(fragmentShader_id);

   return shaderProgram_id;
}

void main_loop(void* arg){
    Params window_params = *(Params*)arg;




}

int main(){
    // emscripten_set_main_loop(main_loop, 0, 1);
    std::cout << "halo" << std::endl;

    double canvasWidth, canvasHeight;
    EMSCRIPTEN_RESULT res = emscripten_get_element_css_size("#canvas", &canvasWidth, &canvasHeight);

    glViewport(0, 0, (GLsizei)canvasWidth, (GLsizei)canvasHeight);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    GLuint shader = CreateShaderProgram("shaders/text.vert", "shaders/text.frag");

    GLuint text_rec = CreateRectangle();
    FontAtlas font = CreateFont("include/open-sans/OpenSans-Regular.ttf", 16.0f, 32, 96);

    Params loop_args = {
        .window_width = (float)canvasWidth,
        .window_width = (float)canvasHeight,
        .shader = shader,
        .VAO = text_rec,
        .font = font,
    };

    emscripten_set_main_loop_arg(main_loop, (void*)&loop_args, 0, 1);
    return 0;
}

