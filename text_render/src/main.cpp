#include "emscripten_fetch.hpp"
#include <GLES3/gl3.h>
#include <emscripten.h>
#include <emscripten/html5.h>
#include <iostream>
#include <stdio.h>
#include <vector>
#include <string>

// Include STB TrueType for font rendering
#define STB_TRUETYPE_IMPLEMENTATION
#include <stb/stb_truetype.h>

// Font Atlas structure - simplified
typedef struct {
    GLuint texture_id;
    int texture_width;
    int texture_height;
    stbtt_bakedchar* chars;
    int first_char;
    int num_chars;
} FontAtlas;

// App parameters - simplified
typedef struct Params {
    float window_width;
    float window_height;
    GLuint shader;
    GLuint VAO;
    GLuint VBO;
    FontAtlas font;
    GLuint uniform_projection;
    GLuint uniform_text_color;
    GLuint uniform_text_sampler;
    float projection[4]; // Simple 2D transformation: [scale_x, scale_y, offset_x, offset_y]
} Params;

// Create a simple 2D projection - just scales and offsets
void simple2DProjection(float* proj, float width, float height) {
    // Scale: 2/width and -2/height (flip Y)
    proj[0] = 2.0f / width;
    proj[1] = -2.0f / height;
    // Offset: -1 for both axes
    proj[2] = -1.0f;
    proj[3] = 1.0f;
}

// Apply the simplified projection to a vertex
void applyProjection(float* result, float x, float y, float* proj) {
    result[0] = x * proj[0] + proj[2];
    result[1] = y * proj[1] + proj[3];
}

// Render text with simplified projection
void render_text(FontAtlas* font, GLuint shader, GLuint VAO, GLuint VBO,
    GLuint uniform_projection, GLuint uniform_text_color, GLuint uniform_text_sampler, 
    float* projection, float x, float y, const char* text, float color[4]) {
    
    glUseProgram(shader);
    
    // Pass the simplified projection to the shader
    glUniform4fv(uniform_projection, 1, projection);
    glUniform4fv(uniform_text_color, 1, color);
    glUniform1i(uniform_text_sampler, 0);
    
    // Bind texture and VAO
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, font->texture_id);
    glBindVertexArray(VAO);
    
    // For each character in string
    float current_x = x;
    for (int i = 0; text[i] != '\0'; i++) {
        int char_index = text[i] - font->first_char;
        if (char_index < 0 || char_index >= font->num_chars) 
            continue;
        
        stbtt_aligned_quad q;
        float xpos = current_x;
        float ypos = y;
        
        stbtt_GetBakedQuad(font->chars, font->texture_width, font->texture_height,
                          char_index, &xpos, &ypos, &q, 1);
        
        // Update position for the next character
        current_x = xpos;
        
        // Create a quad for this character
        float vertices[6][4] = {
            { q.x0, q.y0, q.s0, q.t0 },
            { q.x1, q.y0, q.s1, q.t0 },
            { q.x1, q.y1, q.s1, q.t1 },
            
            { q.x0, q.y0, q.s0, q.t0 },
            { q.x1, q.y1, q.s1, q.t1 },
            { q.x0, q.y1, q.s0, q.t1 }
        };
        
        // Update VBO data
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        
        // Draw the character
        glDrawArrays(GL_TRIANGLES, 0, 6);
    }
    
    // Unbind VAO and texture
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
}

// Setup vertex array for text rendering - simplified
GLuint CreateTextVAO(GLuint* VBO) {
    GLuint VAO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, VBO);
    
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, *VBO);
    // Reserve space for 6 vertices with 4 components each (x, y, s, t)
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
    
    // Position attribute (x, y)
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    
    // Texture coordinates (s, t)
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);
    
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    
    return VAO;
}

// Setup WebGL context
void setup_emscripten() {
    EmscriptenWebGLContextAttributes attrs;
    emscripten_webgl_init_context_attributes(&attrs);
    attrs.majorVersion = 2; // WebGL 2.0
    attrs.minorVersion = 0;
    
    EMSCRIPTEN_WEBGL_CONTEXT_HANDLE context = emscripten_webgl_create_context("#canvas", &attrs);
    emscripten_webgl_make_context_current(context);
}

// Create font from TTF file
FontAtlas CreateFont(std::string font_name, float font_size, int first_char, int num_chars) {
    std::vector<uint8_t> binary_data = downloadBinaryContent(font_name);
    
    FontAtlas atlas = {0};
    
    if (binary_data.empty()) {
        std::cerr << "Failed to download font: " << font_name << std::endl;
        return atlas;
    }
    
    atlas.first_char = first_char;
    atlas.num_chars = num_chars;
    atlas.texture_width = 1024;
    atlas.texture_height = 1024;
    
    uint8_t* temp_bitmap = new uint8_t[atlas.texture_width * atlas.texture_height];
    memset(temp_bitmap, 0, atlas.texture_width * atlas.texture_height);
    
    // Allocate memory for character data
    atlas.chars = new stbtt_bakedchar[num_chars];
    
    int result = stbtt_BakeFontBitmap(
        binary_data.data(), 
        0, 
        font_size, 
        temp_bitmap, 
        atlas.texture_width, 
        atlas.texture_height, 
        first_char, 
        num_chars, 
        atlas.chars
    );
    
    if (result <= 0) {
        std::cerr << "Font baking failed with result: " << result << std::endl;
        delete[] atlas.chars;
        delete[] temp_bitmap;
        atlas = {0};
        return atlas;
    }
    
    glGenTextures(1, &atlas.texture_id);
    glBindTexture(GL_TEXTURE_2D, atlas.texture_id);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, atlas.texture_width, atlas.texture_height, 0, GL_RED, GL_UNSIGNED_BYTE, temp_bitmap);
    
    // Set texture parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    
    delete[] temp_bitmap;
    
    std::cout << "Font atlas created successfully: " << atlas.texture_id << std::endl;
    return atlas;
}

// Compile shader from source
GLuint CompileShader(GLenum type, const char *source_file) {
    std::string fileContents = downloadContent(source_file);
    std::cout << "Shader source: " << source_file << std::endl;
    
    GLuint shader = glCreateShader(type);
    const GLchar* shaderSource = fileContents.c_str();
    glShaderSource(shader, 1, &shaderSource, NULL);
    glCompileShader(shader);
    
    GLint status;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
    if (status != GL_TRUE) {
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

// Create shader program from vertex and fragment sources
GLuint CreateShaderProgram(const char *vertex_source, const char *fragment_source) {
    GLuint vertexShader_id = CompileShader(GL_VERTEX_SHADER, vertex_source);
    if (vertexShader_id == 0) {
        std::cout << "vertex shader compilation failed..." << std::endl;
        return 0;
    }
    
    GLuint fragmentShader_id = CompileShader(GL_FRAGMENT_SHADER, fragment_source);
    if (fragmentShader_id == 0) {
        glDeleteShader(vertexShader_id);
        std::cout << "fragment shader compilation failed..." << std::endl;
        return 0;
    }
    
    GLuint shaderProgram_id = glCreateProgram();
    glAttachShader(shaderProgram_id, vertexShader_id);
    glAttachShader(shaderProgram_id, fragmentShader_id);
    glLinkProgram(shaderProgram_id);
    
    GLint status;
    glGetProgramiv(shaderProgram_id, GL_LINK_STATUS, &status);
    if (status != GL_TRUE) {
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
        return 0;
    }
    
    glDeleteShader(vertexShader_id);
    glDeleteShader(fragmentShader_id);
    
    return shaderProgram_id;
}

// Main rendering loop
void main_loop(void* arg) {
    Params* params = (Params*)arg;
    
    // Clear the screen
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    
    // Text properties
    float textColor[4] = {1.0f, 0.6f, 1.0f, 1.0f}; // White text
    const char* text = "Hello World";
    
    // Center the text
    float textWidth = strlen(text) * 16.0f; // Approximate width
    float textX = (params->window_width - textWidth) / 2.0f;
    float textY = params->window_height / 2.0f - 12.0f;
    
    // Render the text
    render_text(&params->font, params->shader, params->VAO, params->VBO,
                params->uniform_projection, params->uniform_text_color, params->uniform_text_sampler,
                params->projection, textX, textY, text, textColor);
}

// Clean up resources
void cleanup(Params* params) {
    if (params->font.chars) {
        delete[] params->font.chars;
        params->font.chars = nullptr;
    }
    
    if (params->font.texture_id) {
        glDeleteTextures(1, &params->font.texture_id);
    }
    
    if (params->shader) {
        glDeleteProgram(params->shader);
    }
    
    if (params->VAO) {
        glDeleteVertexArrays(1, &params->VAO);
    }
    
    if (params->VBO) {
        glDeleteBuffers(1, &params->VBO);
    }
}

int main() {
    std::cout << "Starting text renderer application..." << std::endl;
    
    setup_emscripten();
    
    // Get canvas size
    double canvasWidth, canvasHeight;
    EMSCRIPTEN_RESULT res = emscripten_get_element_css_size("#canvas", &canvasWidth, &canvasHeight);
    if (res != EMSCRIPTEN_RESULT_SUCCESS) {
        std::cerr << "Failed to get canvas size." << std::endl;
        return 1;
    }
    
    std::cout << "Canvas size: " << canvasWidth << "x" << canvasHeight << std::endl;
    
    // Set up viewport
    glViewport(0, 0, (GLsizei)canvasWidth, (GLsizei)canvasHeight);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    // Create shader program
    GLuint shader = CreateShaderProgram("shaders/text.vert", "shaders/text.frag");
    if (!shader) {
        std::cerr << "Failed to create shader program." << std::endl;
        return 1;
    }
    
    // Get shader uniforms
    GLuint uniform_projection = glGetUniformLocation(shader, "projection");
    GLuint uniform_text_color = glGetUniformLocation(shader, "textColor");
    GLuint uniform_text_sampler = glGetUniformLocation(shader, "text");
    
    // Create simplified 2D projection
    float projection[4];
    simple2DProjection(projection, (float)canvasWidth, (float)canvasHeight);
    
    // Create VAO for text rendering
    GLuint VBO;
    GLuint VAO = CreateTextVAO(&VBO);
    
    // Create font atlas
    FontAtlas font = CreateFont("include/open-sans/OpenSans-Regular.ttf", 48.0f, 32, 96);
    if (font.texture_id == 0 || font.chars == nullptr) {
        std::cerr << "Failed to create font." << std::endl;
        return 1;
    }
    
    // Initialize parameters
    Params* params = new Params{
        .window_width = (float)canvasWidth,
        .window_height = (float)canvasHeight,
        .shader = shader,
        .VAO = VAO,
        .VBO = VBO,
        .font = font,
        .uniform_projection = uniform_projection,
        .uniform_text_color = uniform_text_color,
        .uniform_text_sampler = uniform_text_sampler
    };
    
    // Copy projection values
    for (int i = 0; i < 4; i++) {
        params->projection[i] = projection[i];
    }
    
    // Start main loop
    emscripten_set_main_loop_arg(main_loop, (void*)params, 0, 1);
    
    // This code will never be reached in a browser environment
    cleanup(params);
    delete params;
    
    return 0;
}