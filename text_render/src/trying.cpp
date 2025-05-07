#include <GLES3/gl3.h>
#include <emscripten/emscripten.h>
#include <emscripten/html5.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vector>
#include <functional>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <map>

#include "emscripten_fetch.hpp" 

// STB TrueType implementation
#define STB_TRUETYPE_IMPLEMENTATION
#include <stb/stb_truetype.h>

// Character data structure
struct Character {
    GLuint textureID;  // Texture ID for this glyph
    glm::ivec2 size;   // Size of glyph
    glm::ivec2 bearing;// Offset from baseline to left/top of glyph
    GLuint advance;    // Offset to advance to next glyph
};

// Font atlas structure
struct FontAtlas {
    std::map<GLchar, Character> characters;
    int fontSize;
    bool isLoaded;
};

// Shader source code
const char* vertexShaderSource = R"(#version 300 es
layout (location = 0) in vec4 vertex; // <vec2 pos, vec2 tex>
out vec2 TexCoords;

uniform mat4 projection;

void main()
{
    gl_Position = projection * vec4(vertex.xy, 0.0, 1.0);
    TexCoords = vertex.zw;
}
)";

const char* fragmentShaderSource = R"(#version 300 es
precision mediump float;
in vec2 TexCoords;
out vec4 color;

uniform sampler2D text;
uniform vec3 textColor;

void main()
{
    float alpha = texture(text, TexCoords).r;
    color = vec4(textColor, alpha);
}
)";

// Global variables
GLuint shaderProgram;
GLuint VAO, VBO;
glm::mat4 projection;
EMSCRIPTEN_WEBGL_CONTEXT_HANDLE context;
FontAtlas mainFont;
int screenWidth = 800;
int screenHeight = 600;

// Function prototypes
GLuint compileShader(GLenum type, const char* source);
GLuint createShaderProgram(const char* vertexSource, const char* fragmentSource);
void initTextRendering();
void loadFontAsync(const char* fontUrl, int fontSize);
FontAtlas createFontAtlas(const std::vector<uint8_t>& fontData, int fontSize);
void renderText(const std::string& text, float x, float y, float scale, glm::vec3 color);
void mainLoop();

// Initialize WebGL context and set up rendering
int main() {
    // Set up HTML5 canvas
    EmscriptenWebGLContextAttributes attrs;
    emscripten_webgl_init_context_attributes(&attrs);
    attrs.majorVersion = 3;
    attrs.minorVersion = 0;
    
    // Create WebGL context
    context = emscripten_webgl_create_context("#canvas", &attrs);
    if (context <= 0) {
        printf("Failed to create WebGL context!\n");
        return -1;
    }
    
    // Make the context current
    emscripten_webgl_make_context_current(context);
    
    // Get canvas size
    double width, height;
    emscripten_get_element_css_size("#canvas", &width, &height);
    screenWidth = static_cast<int>(width);
    screenHeight = static_cast<int>(height);
    
    // Set up OpenGL
    glViewport(0, 0, screenWidth, screenHeight);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    // Initialize text rendering
    initTextRendering();
    
    // Load font asynchronously
    loadFontAsync("fonts/open-sans/OpenSans-Regular.ttf", 24);
    
    // Start main loop
    emscripten_set_main_loop(mainLoop, 0, 1);
    
    return 0;
}

// Set up shader program and buffers for text rendering
void initTextRendering() {
    // Create shader program
    shaderProgram = createShaderProgram(vertexShaderSource, fragmentShaderSource);
    
    // Set up projection matrix
    projection = glm::ortho(0.0f, static_cast<float>(screenWidth), 
                           0.0f, static_cast<float>(screenHeight));
    
    // Set up VAO and VBO for text quads
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    
    // We'll allocate enough memory for 6 vertices (2 triangles)
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
    
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
    
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    
    // Initialize font atlas
    mainFont.isLoaded = false;
}

// Load font from URL using provided emscripten_fetch utility
void loadFontAsync(const char* fontUrl, int fontSize) {
    printf("Loading font from: %s\n", fontUrl);
    
    // Use the async binary download function
    downloadBinaryContentAsync(fontUrl, 
        [fontSize](const std::vector<uint8_t>& data, bool success) {
            if (success && !data.empty()) {
                // Create font atlas from downloaded data
                mainFont = createFontAtlas(data, fontSize);
                printf("Font loaded successfully\n");
            } else {
                printf("Failed to load font\n");
            }
        }
    );
}

// Create font atlas from downloaded font data
FontAtlas createFontAtlas(const std::vector<uint8_t>& fontData, int fontSize) {
    FontAtlas atlas;
    atlas.fontSize = fontSize;
    
    // Check if we have valid font data
    if (fontData.empty()) {
        printf("Empty font data\n");
        atlas.isLoaded = false;
        return atlas;
    }
    
    // Initialize FreeType and create font face
    stbtt_fontinfo font;
    if (!stbtt_InitFont(&font, fontData.data(), 0)) {
        printf("Failed to initialize font\n");
        atlas.isLoaded = false;
        return atlas;
    }
    
    // Calculate scale for font size
    float scale = stbtt_ScaleForPixelHeight(&font, static_cast<float>(fontSize));
    
    // Get font metrics
    int ascent, descent, lineGap;
    stbtt_GetFontVMetrics(&font, &ascent, &descent, &lineGap);
    
    // Scaled metrics
    ascent = static_cast<int>(ascent * scale);
    descent = static_cast<int>(descent * scale);
    
    // Generate a texture for each character
    for (unsigned char c = 32; c < 128; c++) {
        // Get character metrics
        int advance, lsb, x0, y0, x1, y1;
        stbtt_GetCodepointHMetrics(&font, c, &advance, &lsb);
        stbtt_GetCodepointBitmapBox(&font, c, scale, scale, &x0, &y0, &x1, &y1);
        
        // Character width and height
        int w = x1 - x0;
        int h = y1 - y0;
        
        // Skip characters with no width
        if (w <= 0 || h <= 0) {
            Character ch = {
                0,
                glm::ivec2(0, 0),
                glm::ivec2(0, 0),
                static_cast<GLuint>(advance * scale)
            };
            atlas.characters.insert(std::pair<GLchar, Character>(c, ch));
            continue;
        }
        
        // Create bitmap for glyph
        unsigned char* bitmap = new unsigned char[w * h];
        stbtt_MakeCodepointBitmap(&font, bitmap, w, h, w, scale, scale, c);
        
        // Generate texture
        GLuint texture;
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, w, h, 0, GL_RED, GL_UNSIGNED_BYTE, bitmap);
        
        // Set texture options
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        
        // Store character data
        Character character = {
            texture,
            glm::ivec2(w, h),
            glm::ivec2(x0, -y0),  // Note: y0 is negative
            static_cast<GLuint>(advance * scale)
        };
        
        atlas.characters.insert(std::pair<GLchar, Character>(c, character));
        delete[] bitmap;
    }
    
    atlas.isLoaded = true;
    return atlas;
}

// Render text with the loaded font
void renderText(const std::string& text, float x, float y, float scale, glm::vec3 color) {
    // Check if font is loaded
    if (!mainFont.isLoaded) {
        return;
    }
    
    // Activate shader
    glUseProgram(shaderProgram);
    
    // Set projection matrix
    GLint projLoc = glGetUniformLocation(shaderProgram, "projection");
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));
    
    // Set text color
    GLint colorLoc = glGetUniformLocation(shaderProgram, "textColor");
    glUniform3fv(colorLoc, 1, glm::value_ptr(color));
    
    // Activate texture unit
    glActiveTexture(GL_TEXTURE0);
    
    // Set texture sampler
    GLint samplerLoc = glGetUniformLocation(shaderProgram, "text");
    glUniform1i(samplerLoc, 0);
    
    // Bind VAO
    glBindVertexArray(VAO);
    
    // Iterate through characters
    std::string::const_iterator c;
    for (c = text.begin(); c != text.end(); c++) {
        // Skip if character is not in our map
        auto it = mainFont.characters.find(*c);
        if (it == mainFont.characters.end()) {
            continue;
        }
        
        Character ch = it->second;
        
        // Skip characters with no texture
        if (ch.textureID == 0) {
            x += ch.advance >> 6;
            continue;
        }
        
        // Calculate position
        float xpos = x + ch.bearing.x * scale;
        float ypos = y - (ch.size.y - ch.bearing.y) * scale;
        
        // Calculate size
        float w = ch.size.x * scale;
        float h = ch.size.y * scale;
        
        // Update VBO for each character
        float vertices[6][4] = {
            { xpos,     ypos + h,   0.0f, 0.0f },            
            { xpos,     ypos,       0.0f, 1.0f },
            { xpos + w, ypos,       1.0f, 1.0f },
            
            { xpos,     ypos + h,   0.0f, 0.0f },
            { xpos + w, ypos,       1.0f, 1.0f },
            { xpos + w, ypos + h,   1.0f, 0.0f }           
        };
        
        // Bind texture
        glBindTexture(GL_TEXTURE_2D, ch.textureID);
        
        // Update VBO
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        
        // Render quad
        glDrawArrays(GL_TRIANGLES, 0, 6);
        
        // Advance cursor for next glyph
        x += (ch.advance >> 6) * scale; // Bitshift by 6 to get value in pixels (2^6 = 64)
    }
    
    // Unbind VAO and texture
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
}

// Main rendering loop
void mainLoop() {
    // Clear the screen
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    
    // Update canvas size if needed
    double width, height;
    emscripten_get_element_css_size("#canvas", &width, &height);
    if (width != screenWidth || height != screenHeight) {
        screenWidth = static_cast<int>(width);
        screenHeight = static_cast<int>(height);
        glViewport(0, 0, screenWidth, screenHeight);
        
        // Update projection matrix
        projection = glm::ortho(0.0f, static_cast<float>(screenWidth), 
                               0.0f, static_cast<float>(screenHeight));
    }
    
    // Render sample text if font is loaded
    if (mainFont.isLoaded) {
        renderText("Hello, OpenGL ES 3.0!", 25.0f, 100.0f, 1.0f, glm::vec3(1.0f, 1.0f, 1.0f));
        renderText("Text rendering with stb_truetype", 25.0f, 200.0f, 1.5f, glm::vec3(1.0f, 1.0f, 0.0f));
        renderText("Using emscripten fetch API", 25.0f, 300.0f, 0.8f, glm::vec3(0.5f, 0.8f, 0.2f));
    }
}

// Compile a shader from source
GLuint compileShader(GLenum type, const char* source) {
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, NULL);
    glCompileShader(shader);
    
    // Check for compilation errors
    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        GLchar infoLog[512];
        glGetShaderInfoLog(shader, 512, NULL, infoLog);
        printf("Shader compilation error: %s\n", infoLog);
        return 0;
    }
    
    return shader;
}

// Create a shader program from vertex and fragment shaders
GLuint createShaderProgram(const char* vertexSource, const char* fragmentSource) {
    // Compile shaders
    GLuint vertexShader = compileShader(GL_VERTEX_SHADER, vertexSource);
    GLuint fragmentShader = compileShader(GL_FRAGMENT_SHADER, fragmentSource);
    
    if (!vertexShader || !fragmentShader) {
        return 0;
    }
    
    // Create and link program
    GLuint program = glCreateProgram();
    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);
    glLinkProgram(program);
    
    // Check for linking errors
    GLint success;
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        GLchar infoLog[512];
        glGetProgramInfoLog(program, 512, NULL, infoLog);
        printf("Shader program linking error: %s\n", infoLog);
        return 0;
    }
    
    // Clean up shaders
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    
    return program;
}

