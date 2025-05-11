#include <GLES3/gl3.h>
#include <EGL/egl.h>
#include <emscripten.h>
#include <emscripten/html5.h>
#include <math.h>
#include <stdio.h>
#include "MyMath.h"
#define STB_TRUETYPE_IMPLEMENTATION
#include <stb/stb_truetype.h>
#include "../emscripten_fetch.h"


void setup_emscripten(float* WindowWidth, float* WindowHeight){
    EmscriptenWebGLContextAttributes attrs;
    emscripten_webgl_init_context_attributes(&attrs);
    attrs.majorVersion = 2; // WebGL 2.0 corresponds to OpenGL ES 3.0
    attrs.minorVersion = 0;

    EMSCRIPTEN_WEBGL_CONTEXT_HANDLE context = emscripten_webgl_create_context("#canvas", &attrs);
    emscripten_webgl_make_context_current(context);

    double canvasWidth, canvasHeight;
    EMSCRIPTEN_RESULT res = emscripten_get_element_css_size("#canvas", &canvasWidth, &canvasHeight);

    glViewport(0, 0, (GLsizei)canvasWidth, (GLsizei)canvasHeight);
    *WindowWidth = (float)canvasWidth;
    *WindowHeight = (float)canvasHeight;
}


typedef struct MainLoopArgs {
    float WindowWidth;
    float WindowHeight;
    GLuint VAO;
    GLuint ShaderProgram;
    GLuint Texture;
    GLsizei NumberOfElements;
} MainLoopArgs;

void main_loop(void* Args){
    MainLoopArgs LoopArgs = *(MainLoopArgs*)Args;
    glClearColor(0.1, 0.5, 0.2, 1.0);
    glClear(GL_COLOR_BUFFER_BIT);

    glUseProgram(LoopArgs.ShaderProgram);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, LoopArgs.Texture);
    glUniform1i(glGetUniformLocation(LoopArgs.ShaderProgram, "textTexture"), 0);
    glBindVertexArray(LoopArgs.VAO);

    glDrawElements(GL_TRIANGLES, LoopArgs.NumberOfElements, GL_UNSIGNED_INT, NULL);

}

GLuint InitFont(char* FontName, stbtt_bakedchar* CharData, size_t CharDataLen){
    size_t BufferSize;
    uint8_t* FontData = downloadBinaryContent(FontName, &BufferSize);
    uint8_t TempBitmap[512*512];
    stbtt_BakeFontBitmap(FontData, 0, 64.0, TempBitmap, 512, 512, 32, CharDataLen, CharData);
    
    GLuint Texture;
    glGenTextures(1, &Texture);
    glBindTexture(GL_TEXTURE_2D, Texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, 512, 512, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, TempBitmap);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    return Texture;
}

GLuint InitText(char* Text, Vec2 Position, GLuint Texture, stbtt_bakedchar* CharData, size_t CharDataLen, GLsizei* NumIndices) {
    GLuint VAO, VBO, EBO;

    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    size_t text_length = strlen(Text);
    //because there are 2 coordinates, and each character's "quad" has 4 corners, and each vertex needs 2 coordinates for the texture's alignment
    size_t vertices_len = text_length * 4 * 4 * sizeof(float); // 4 vertices per char, 4 floats per vertex
    float* vertices = (float*)malloc(vertices_len);

    //because 6 triangle indices are needed to store them as quads
    size_t indices_len = text_length * 6 * sizeof(GLuint); // 6 indices per char
    GLuint* indices = (GLuint*)malloc(indices_len);

    size_t valid_chars = 0; // Count of valid characters that will be rendered

    for (size_t i=0, j=0; *Text; ++Text) {
        if (*Text >= 32) { // Only process printable characters
            stbtt_aligned_quad q;
            stbtt_GetBakedQuad(CharData, 512, 512, *Text-32, &Position.data[0], &Position.data[1], &q, 1);

            size_t vertex_offset = valid_chars * 4 * 4; // 4 vertices per char, 4 floats per vertex
            vertices[vertex_offset + 0] = q.s0; vertices[vertex_offset + 1] = q.t1; 
            vertices[vertex_offset + 2] = q.x0; vertices[vertex_offset + 3] = q.y0;
            
            vertices[vertex_offset + 4] = q.s1; vertices[vertex_offset + 5] = q.t1; 
            vertices[vertex_offset + 6] = q.x1; vertices[vertex_offset + 7] = q.y0;
            
            vertices[vertex_offset + 8] = q.s1; vertices[vertex_offset + 9] = q.t0; 
            vertices[vertex_offset + 10] = q.x1; vertices[vertex_offset + 11] = q.y1;
            
            vertices[vertex_offset + 12] = q.s0; vertices[vertex_offset + 13] = q.t0; 
            vertices[vertex_offset + 14] = q.x0; vertices[vertex_offset + 15] = q.y1;

            // Calculate vertex indices for this character quad (4 vertices per char)
            size_t index_offset = valid_chars * 6; // 6 indices per char
            GLuint vertexBase = valid_chars * 4; // Each character starts with vertex index (valid_chars * 4)
            
            indices[index_offset + 0] = vertexBase;     // First triangle, first vertex
            indices[index_offset + 1] = vertexBase + 1; // First triangle, second vertex
            indices[index_offset + 2] = vertexBase + 2; // First triangle, third vertex
            
            indices[index_offset + 3] = vertexBase;     // Second triangle, first vertex
            indices[index_offset + 4] = vertexBase + 2; // Second triangle, second vertex
            indices[index_offset + 5] = vertexBase + 3; // Second triangle, third vertex
            
            valid_chars++; // Increment only for valid characters
        }
    }

    printf("First character location on the screen: %fx %fy\n", vertices[0], vertices[1]);

    // Set the actual number of indices for rendering
    *NumIndices = valid_chars * 6;
    
    // Adjust buffer sizes based on actual character count
    vertices_len = valid_chars * 4 * 4 * sizeof(float);
    indices_len = valid_chars * 6 * sizeof(GLuint);
    
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices_len, vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices_len, indices, GL_STATIC_DRAW);

    // Configure vertex attributes
    // Texture coordinates (s, t)
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // Position coordinates (x, y)
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // Free the temporary memory
    free(vertices);
    free(indices);

    return VAO;
}

GLuint CreateShaderProgram(char* VertexSource, char* FragmentSource){
    const GLchar* VertexCode = downloadContent(VertexSource);
    const GLchar* FragmentCode = downloadContent(FragmentSource);

    printf("Vertex shader code: %s\n", VertexCode);
    printf("Fragment shader code: %s\n", FragmentCode);

    GLint success;
    char infoLog[512];

    GLuint VertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(VertexShader, 1, &VertexCode, NULL);
    glCompileShader(VertexShader);

    // Check vertex shader compilation
    glGetShaderiv(VertexShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(VertexShader, 512, NULL, infoLog);
        fprintf(stderr, "ERROR::VERTEX_SHADER_COMPILATION_FAILED\n%s\n", (char*)infoLog);
    }

    // 2. Fragment Shader
    GLuint FragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(FragmentShader, 1, &FragmentCode, NULL);
    glCompileShader(FragmentShader);

    // Check fragment shader compilation
    glGetShaderiv(FragmentShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(FragmentShader, 512, NULL, infoLog);
        fprintf(stderr, "ERROR::FRAGMENT_SHADER_COMPILATION_FAILED\n%s\n", (char*)infoLog);
    }

    // 3. Shader Program
    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, VertexShader);
    glAttachShader(shaderProgram, FragmentShader);
    glLinkProgram(shaderProgram);

    // Check linking
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        fprintf(stderr, "ERROR::SHADER_PROGRAM_LINKING_FAILED\n%s\n", (char*)infoLog);
    }

    // 4. Cleanup
    glDeleteShader(VertexShader);
    glDeleteShader(FragmentShader);

    return shaderProgram;

}

int main(){
    float WindowWidth, WindowHeight;
    setup_emscripten(&WindowWidth, &WindowHeight);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    size_t CharDataLen = 96;
    stbtt_bakedchar* CharData = (stbtt_bakedchar*)malloc(sizeof(stbtt_bakedchar)*CharDataLen);
    GLuint FontTexture = InitFont("/fonts/open-sans/OpenSans-Regular.ttf", CharData, CharDataLen);

    GLsizei IndicesCount;

    GLuint text = InitText("Halo halo", (Vec2){.data={400.0f, 400.0f}}, FontTexture, CharData, CharDataLen, &IndicesCount);


    
    MainLoopArgs LoopArgs = {
        .WindowWidth = WindowWidth,
        .WindowHeight = WindowHeight,
        .VAO = text,
        .NumberOfElements = IndicesCount,
        .ShaderProgram = CreateShaderProgram("/shaders/text.vert", "/shaders/text.frag"),
    };

    emscripten_set_main_loop_arg(main_loop, (void*)&LoopArgs, 0, EM_TRUE);
    return 0;
}