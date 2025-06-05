#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include <GLES3/gl3.h>
#include <EGL/egl.h>
#include <GLFW/glfw3.h>

#include <ft2build.h>
#include FT_FREETYPE_H

#include <hb.h>
#include <hb-ft.h>

// OpenGL ES 3.0 compatible shader sources
const char* vertex_shader_source = "#version 300 es\n"
    "precision mediump float;\n"
    "layout (location = 0) in vec4 vertex;\n"
    "out vec2 TexCoords;\n"
    "uniform mat4 projection;\n"
    "void main() {\n"
    "    gl_Position = projection * vec4(vertex.xy, 0.0, 1.0);\n"
    "    TexCoords = vertex.zw;\n"
    "}\0";

const char* fragment_shader_source = "#version 300 es\n"
    "precision mediump float;\n"
    "in vec2 TexCoords;\n"
    "out vec4 color;\n"
    "uniform sampler2D text;\n"
    "uniform vec3 textColor;\n"
    "void main() {\n"
    "    vec4 sampled = vec4(1.0, 1.0, 1.0, texture(text, TexCoords).r);\n"
    "    color = vec4(textColor, 1.0) * sampled;\n"
    "}\0";

typedef struct {
    unsigned int texture_id;
    int size_x, size_y;
    int bearing_x, bearing_y;
    unsigned int advance;
} Character;

typedef struct {
    Character characters[65536]; // Support for Unicode characters
    int loaded[65536];
} FontAtlas;

typedef struct {
    FT_Library ft;
    FT_Face face;
    hb_font_t* hb_font;
    FontAtlas atlas;
} Font;

unsigned int shader_program;
unsigned int VAO, VBO;
Font font;

void error_callback(int error, const char* description) {
    fprintf(stderr, "GLFW Error: %s\n", description);
}

unsigned int compile_shader(unsigned int type, const char* source) {
    unsigned int shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, NULL);
    glCompileShader(shader);
    
    int success;
    char info_log[512];
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(shader, 512, NULL, info_log);
        fprintf(stderr, "Shader compilation failed: %s\n", info_log);
    }
    
    return shader;
}

unsigned int create_shader_program() {
    unsigned int vertex_shader = compile_shader(GL_VERTEX_SHADER, vertex_shader_source);
    unsigned int fragment_shader = compile_shader(GL_FRAGMENT_SHADER, fragment_shader_source);
    
    unsigned int program = glCreateProgram();
    glAttachShader(program, vertex_shader);
    glAttachShader(program, fragment_shader);
    glLinkProgram(program);
    
    int success;
    char info_log[512];
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(program, 512, NULL, info_log);
        fprintf(stderr, "Shader program linking failed: %s\n", info_log);
    }
    
    glDeleteShader(vertex_shader);
    glDeleteShader(fragment_shader);
    
    return program;
}

int init_font(Font* font, const char* font_path, int font_size) {
    // Initialize FreeType
    if (FT_Init_FreeType(&font->ft)) {
        fprintf(stderr, "Could not init FreeType Library\n");
        return 0;
    }
    
    // Load font face
    if (FT_New_Face(font->ft, font_path, 0, &font->face)) {
        fprintf(stderr, "Failed to load font: %s\n", font_path);
        FT_Done_FreeType(font->ft);
        return 0;
    }
    
    // Print font information
    printf("Font family: %s\n", font->face->family_name);
    printf("Font style: %s\n", font->face->style_name);
    printf("Number of faces: %ld\n", font->face->num_faces);
    printf("Number of glyphs: %ld\n", font->face->num_glyphs);
    
    // Set font size
    FT_Set_Pixel_Sizes(font->face, 0, font_size);
    
    // Create HarfBuzz font
    font->hb_font = hb_ft_font_create(font->face, NULL);
    
    // Initialize atlas
    memset(font->atlas.loaded, 0, sizeof(font->atlas.loaded));
    
    return 1;
}

void load_character(Font* font, unsigned int glyph_index) {
    if (glyph_index >= 65536 || font->atlas.loaded[glyph_index]) return;
    
    // Load glyph by index (not codepoint)
    if (FT_Load_Glyph(font->face, glyph_index, FT_LOAD_RENDER)) {
        fprintf(stderr, "Failed to load Glyph for glyph index %u\n", glyph_index);
        return;
    }
    
    // Generate texture
    unsigned int texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    
    // OpenGL ES 3.0 texture format
    glTexImage2D(
        GL_TEXTURE_2D,
        0,
        GL_R8,  // Use GL_R8 instead of GL_RED for internal format
        font->face->glyph->bitmap.width,
        font->face->glyph->bitmap.rows,
        0,
        GL_RED,
        GL_UNSIGNED_BYTE,
        font->face->glyph->bitmap.buffer
    );
    
    // Set texture options
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    
    // Store character
    Character character = {
        texture,
        font->face->glyph->bitmap.width,
        font->face->glyph->bitmap.rows,
        font->face->glyph->bitmap_left,
        font->face->glyph->bitmap_top,
        font->face->glyph->advance.x
    };
    
    font->atlas.characters[glyph_index] = character;
    font->atlas.loaded[glyph_index] = 1;
}

void render_text_hb(Font* font, const char* text, float x, float y, float scale, float color[3]) {
    // Create HarfBuzz buffer
    hb_buffer_t* buf = hb_buffer_create();
    hb_buffer_add_utf8(buf, text, -1, 0, -1);
    hb_buffer_set_direction(buf, HB_DIRECTION_LTR);
    hb_buffer_set_script(buf, HB_SCRIPT_COMMON);
    hb_buffer_set_language(buf, hb_language_from_string("en", -1));
    
    // Auto-detect script for better shaping
    hb_buffer_guess_segment_properties(buf);
    
    // Shape the text
    hb_shape(font->hb_font, buf, NULL, 0);
    
    // Get glyph information
    unsigned int glyph_count;
    hb_glyph_info_t* glyph_info = hb_buffer_get_glyph_infos(buf, &glyph_count);
    hb_glyph_position_t* glyph_pos = hb_buffer_get_glyph_positions(buf, &glyph_count);
    
    // printf("Rendering text: '%s' with %u glyphs\n", text, glyph_count);
    
    // Set up OpenGL state
    glUseProgram(shader_program);
    glUniform3f(glGetUniformLocation(shader_program, "textColor"), color[0], color[1], color[2]);
    glActiveTexture(GL_TEXTURE0);
    glBindVertexArray(VAO);
    
    float current_x = x;
    float current_y = y;
    
    // Render each glyph
    for (unsigned int i = 0; i < glyph_count; i++) {
        unsigned int glyph_index = glyph_info[i].codepoint; // This is actually glyph index from HarfBuzz
        
        // Load character if not already loaded
        if (glyph_index < 65536 && !font->atlas.loaded[glyph_index]) {
            load_character(font, glyph_index);
        }
        
        if (glyph_index >= 65536 || !font->atlas.loaded[glyph_index]) {
            continue; // Skip if glyph couldn't be loaded
        }
        
        Character ch = font->atlas.characters[glyph_index];
        
        float x_offset = glyph_pos[i].x_offset / 64.0f;
        float y_offset = glyph_pos[i].y_offset / 64.0f;
        
        float xpos = current_x + ch.bearing_x * scale + x_offset * scale;
        float ypos = current_y - (ch.size_y - ch.bearing_y) * scale + y_offset * scale;
        
        float w = ch.size_x * scale;
        float h = ch.size_y * scale;
        
        // Update VBO for each character
        float vertices[6][4] = {
            { xpos,     ypos + h,   0.0f, 0.0f },
            { xpos,     ypos,       0.0f, 1.0f },
            { xpos + w, ypos,       1.0f, 1.0f },
            
            { xpos,     ypos + h,   0.0f, 0.0f },
            { xpos + w, ypos,       1.0f, 1.0f },
            { xpos + w, ypos + h,   1.0f, 0.0f }
        };
        
        // Render glyph texture over quad
        glBindTexture(GL_TEXTURE_2D, ch.texture_id);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        
        // Advance cursor
        current_x += (glyph_pos[i].x_advance / 64.0f) * scale;
        current_y += (glyph_pos[i].y_advance / 64.0f) * scale;
    }
    
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
    
    // Clean up HarfBuzz buffer
    hb_buffer_destroy(buf);
}

int main() {
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
    GLFWwindow* window = glfwCreateWindow(800, 600, "OpenGL ES 3.0 HarfBuzz Text Rendering", NULL, NULL);
    if (!window) {
        fprintf(stderr, "Failed to create GLFW window\n");
        glfwTerminate();
        return -1;
    }
    
    glfwMakeContextCurrent(window);
    
    // Check OpenGL ES version
    printf("OpenGL ES Version: %s\n", glGetString(GL_VERSION));
    printf("Renderer: %s\n", glGetString(GL_RENDERER));
    printf("Vendor: %s\n", glGetString(GL_VENDOR));
    
    // Configure OpenGL ES
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    // OpenGL ES 3.0 doesn't support GL_UNPACK_ALIGNMENT by default, but it should work
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    
    // Create shader program
    shader_program = create_shader_program();
    
    // Set up projection matrix (orthographic)
    float projection[16] = {0};
    projection[0] = 2.0f / 800.0f;   // 2/width
    projection[5] = 2.0f / 600.0f;   // 2/height
    projection[10] = -1.0f;
    projection[12] = -1.0f;          // -1
    projection[13] = -1.0f;          // -1
    projection[15] = 1.0f;
    
    glUseProgram(shader_program);
    glUniformMatrix4fv(glGetUniformLocation(shader_program, "projection"), 1, GL_FALSE, projection);
    
    // Set up VAO and VBO
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    
    // Initialize font (Ubuntu Linux x64 font paths)
    const char* font_paths[] = {
        "/usr/share/fonts/truetype/noto/NotoSansCJK-Regular.ttc",
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
        if (init_font(&font, font_paths[i], 48)) {
            printf("Successfully loaded font: %s\n", font_paths[i]);
            font_loaded = 1;
            break;
        }
    }
    
    if (!font_loaded) {
        fprintf(stderr, "Failed to load any font. Please install fonts with Unicode support:\n");
        fprintf(stderr, "  sudo apt install fonts-noto-cjk fonts-dejavu fonts-liberation fonts-ubuntu\n");
        return -1;
    }
    
    // Text samples
    const char* english_text = "Hello, World!";
    const char* japanese_text = "こんにちは世界"; // "Hello World" in Japanese
    const char* mixed_text = "Hello 世界 World!";
    
    // Colors
    float red[3] = {1.0f, 0.0f, 0.0f};
    float green[3] = {0.0f, 1.0f, 0.0f};
    float blue[3] = {0.0f, 0.0f, 1.0f};
    
    // Main loop
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        
        // Clear screen
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        
        // Render text samples
        render_text_hb(&font, english_text, 50.0f, 500.0f, 1.0f, red);
        render_text_hb(&font, japanese_text, 50.0f, 400.0f, 1.0f, green);
        render_text_hb(&font, mixed_text, 50.0f, 300.0f, 1.0f, blue);
        
        glfwSwapBuffers(window);
    }
    
    // Cleanup
    hb_font_destroy(font.hb_font);
    FT_Done_Face(font.face);
    FT_Done_FreeType(font.ft);
    
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteProgram(shader_program);
    
    glfwTerminate();
    return 0;
}