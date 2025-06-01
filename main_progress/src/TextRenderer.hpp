#pragma once

#include <hb-ft.h>
#include <ft2build.h>
#include FT_FREETYPE_H
#include <GLES3/gl3.h>
#include <emscripten.h>
#include <vector>
#include <map>
#include <string>
#include <iostream>
#include "utils/MyMath.hpp"

struct GlyphTexture {
    GLuint texture_id;
    int width, height;
    int bearing_x, bearing_y;
    int advance;
};

class TextFont {
public:
    hb_blob_t* blob;
    hb_face_t* face;
    hb_font_t* font;
    FT_Library ft_library;
    FT_Face ft_face;
    std::map<hb_codepoint_t, GlyphTexture> glyph_cache;
    
    TextFont() : blob(nullptr), face(nullptr), font(nullptr) {
        FT_Init_FreeType(&ft_library);
    }

    TextFont(const char* font_data, size_t font_size, int pixel_size = 48) {
        FT_Init_FreeType(&ft_library);
        
        // Create HarfBuzz objects
        blob = hb_blob_create(font_data, font_size, HB_MEMORY_MODE_READONLY, nullptr, nullptr);
        face = hb_face_create(blob, 0);
        font = hb_font_create(face);
        
        // Create FreeType face for rendering
        FT_New_Memory_Face(ft_library, (const FT_Byte*)font_data, font_size, 0, &ft_face);
        FT_Set_Pixel_Sizes(ft_face, 0, pixel_size);
        
        // Configure HarfBuzz to use FreeType
        hb_ft_font_set_funcs(font);
    }

    GlyphTexture& GetGlyphTexture(hb_codepoint_t glyph_id) {
        auto it = glyph_cache.find(glyph_id);
        if (it != glyph_cache.end()) {
            return it->second;
        }
        
        // Load glyph with FreeType
        FT_Load_Glyph(ft_face, glyph_id, FT_LOAD_RENDER);
        FT_GlyphSlot glyph = ft_face->glyph;
        
        // Create OpenGL texture
        GLuint texture;
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);
        
        // Upload glyph bitmap to texture
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, 
                     glyph->bitmap.width, glyph->bitmap.rows, 
                     0, GL_RED, GL_UNSIGNED_BYTE, 
                     glyph->bitmap.buffer);
        
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        
        // Cache the glyph texture
        GlyphTexture glyph_tex = {
            texture,
            (int)glyph->bitmap.width,
            (int)glyph->bitmap.rows,
            glyph->bitmap_left,
            glyph->bitmap_top,
            (int)(glyph->advance.x >> 6)
        };
        
        glyph_cache[glyph_id] = glyph_tex;
        return glyph_cache[glyph_id];
    }

    ~TextFont() {
        for (auto& pair : glyph_cache) {
            glDeleteTextures(1, &pair.second.texture_id);
        }
        
        if (font) hb_font_destroy(font);
        if (face) hb_face_destroy(face);
        if (blob) hb_blob_destroy(blob);
        if (ft_face) FT_Done_Face(ft_face);
        FT_Done_FreeType(ft_library);
    }
};

class TextRenderer {
private:
    GLuint shader_program;
    GLuint vbo, vao;
    GLint uniform_projection;
    GLint uniform_texture;
    GLint uniform_color;
    
public:
    TextRenderer() {
        InitializeShaders();
        InitializeBuffers();
    }
    
    void InitializeShaders() {
        const char* vertex_shader_source = R"(
            #version 300 es
            in vec4 vertex; // <vec2 pos, vec2 tex>
            uniform mat4 projection;
            out vec2 TexCoords;

            void main() {
                gl_Position = projection * vec4(vertex.xy, 0.0, 1.0);
                TexCoords = vertex.zw;
            }
        )";
        
        const char* fragment_shader_source = R"(
            #version 300 es
            precision mediump float;
            in vec2 TexCoords;
            uniform sampler2D text;
            uniform vec3 textColor;
            out vec4 fragColor;

            void main() {
                vec4 sampled = vec4(1.0, 1.0, 1.0, texture(text, TexCoords).r);
                fragColor = vec4(textColor, 1.0) * sampled;
            }
        )";
        
        // Compile shaders
        GLuint vertex_shader = CompileShader(GL_VERTEX_SHADER, vertex_shader_source);
        GLuint fragment_shader = CompileShader(GL_FRAGMENT_SHADER, fragment_shader_source);
        
        // Create program
        shader_program = glCreateProgram();
        glAttachShader(shader_program, vertex_shader);
        glAttachShader(shader_program, fragment_shader);
        glLinkProgram(shader_program);
        
        // Check linking status
        GLint status;
        glGetProgramiv(shader_program, GL_LINK_STATUS, &status);
        if (status != GL_TRUE) {
            GLint logLength = 0;
            glGetProgramiv(shader_program, GL_INFO_LOG_LENGTH, &logLength);
            if (logLength > 1) {
                std::vector<char> log(logLength);
                glGetProgramInfoLog(shader_program, logLength, nullptr, log.data());
                std::cout << "Text shader linking failed: " << log.data() << std::endl;
            }
        }
        
        // Get uniform locations
        uniform_projection = glGetUniformLocation(shader_program, "projection");
        uniform_texture = glGetUniformLocation(shader_program, "text");
        uniform_color = glGetUniformLocation(shader_program, "textColor");
        
        glDeleteShader(vertex_shader);
        glDeleteShader(fragment_shader);
    }
    
    GLuint CompileShader(GLenum type, const char* source) {
        GLuint shader = glCreateShader(type);
        glShaderSource(shader, 1, &source, nullptr);
        glCompileShader(shader);
        
        GLint status;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
        if (status != GL_TRUE) {
            GLint logLength = 0;
            glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logLength);
            if (logLength > 1) {
                std::vector<char> log(logLength);
                glGetShaderInfoLog(shader, logLength, nullptr, log.data());
                std::cout << "Text shader compilation failed: " << log.data() << std::endl;
            }
        }
        
        return shader;
    }
    
    void InitializeBuffers() {
        glGenVertexArrays(1, &vao);
        glGenBuffers(1, &vbo);
    }
    
    void RenderText(const char* text, TextFont& font, float x, float y, float scale, 
                   Vec3 color, float screen_width, float screen_height) {
        
        // Save current OpenGL state
        GLint current_program;
        glGetIntegerv(GL_CURRENT_PROGRAM, &current_program);
        
        GLboolean blend_enabled = glIsEnabled(GL_BLEND);
        GLint blend_src, blend_dst;
        glGetIntegerv(GL_BLEND_SRC_ALPHA, &blend_src);
        glGetIntegerv(GL_BLEND_DST_ALPHA, &blend_dst);
        
        // Set up text rendering state
        glUseProgram(shader_program);
        glUniform3f(uniform_color, color.data[0], color.data[1], color.data[2]);
        glActiveTexture(GL_TEXTURE0);
        glUniform1i(uniform_texture, 0);
        
        // Create orthographic projection matrix for screen space
        Mat4 projection = Mat4::create_orthographic(0.0f, screen_width, 0.0f, screen_height, -1.0f, 1.0f);
        glUniformMatrix4fv(uniform_projection, 1, GL_FALSE, projection.data);
        
        // Shape text with HarfBuzz
        hb_buffer_t* buffer = hb_buffer_create();
        hb_buffer_add_utf8(buffer, text, -1, 0, -1);
        hb_buffer_set_direction(buffer, HB_DIRECTION_LTR);
        hb_buffer_set_script(buffer, HB_SCRIPT_LATIN);
        hb_buffer_set_language(buffer, hb_language_from_string("hu", -1));
        
        hb_shape(font.font, buffer, nullptr, 0);
        
        unsigned int glyph_count;
        hb_glyph_info_t* glyph_info = hb_buffer_get_glyph_infos(buffer, &glyph_count);
        hb_glyph_position_t* glyph_pos = hb_buffer_get_glyph_positions(buffer, &glyph_count);
        
        // Enable blending for transparency
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        
        glBindVertexArray(vao);
        
        float cursor_x = x;
        float cursor_y = y;
        
        for (unsigned int i = 0; i < glyph_count; i++) {
            GlyphTexture& glyph_tex = font.GetGlyphTexture(glyph_info[i].codepoint);
            
            float xpos = cursor_x + (glyph_pos[i].x_offset / 64.0f + glyph_tex.bearing_x) * scale;
            float ypos = cursor_y - (glyph_pos[i].y_offset / 64.0f - glyph_tex.bearing_y) * scale;
            
            float w = glyph_tex.width * scale;
            float h = glyph_tex.height * scale;
            
            // Create quad vertices
            float vertices[6][4] = {
                { xpos,     ypos + h,   0.0f, 0.0f },
                { xpos,     ypos,       0.0f, 1.0f },
                { xpos + w, ypos,       1.0f, 1.0f },
                
                { xpos,     ypos + h,   0.0f, 0.0f },
                { xpos + w, ypos,       1.0f, 1.0f },
                { xpos + w, ypos + h,   1.0f, 0.0f }
            };
            
            // Bind glyph texture
            glBindTexture(GL_TEXTURE_2D, glyph_tex.texture_id);
            
            // Update VBO
            glBindBuffer(GL_ARRAY_BUFFER, vbo);
            glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_DYNAMIC_DRAW);
            
            // Set vertex attributes
            glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
            glEnableVertexAttribArray(0);
            
            // Render quad
            glDrawArrays(GL_TRIANGLES, 0, 6);
            
            // Advance cursor
            cursor_x += (glyph_pos[i].x_advance / 64.0f) * scale;
            cursor_y += (glyph_pos[i].y_advance / 64.0f) * scale;
        }
        
        // Restore OpenGL state
        glBindVertexArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindTexture(GL_TEXTURE_2D, 0);
        glUseProgram(current_program);
        
        if (!blend_enabled) {
            glDisable(GL_BLEND);
        } else {
            glBlendFunc(blend_src, blend_dst);
        }
        
        hb_buffer_destroy(buffer);
    }
    
    ~TextRenderer() {
        glDeleteProgram(shader_program);
        glDeleteBuffers(1, &vbo);
        glDeleteVertexArrays(1, &vao);
    }
};

// Updated TextManager to work with your system
class TextManager {
public:
    TextFont font;
    TextRenderer* renderer;
    
    TextManager(const char* font_data, size_t font_size, TextRenderer* text_renderer, int pixel_size = 48) 
        : font(font_data, font_size, pixel_size), renderer(text_renderer) {
    }
    
    void RenderText(const char* text, float x, float y, float scale, Vec3 color, float screen_width, float screen_height) {
        if (renderer) {
            renderer->RenderText(text, font, x, y, scale, color, screen_width, screen_height);
        }
    }
    
    // Helper function to get text dimensions
    Vec2 GetTextSize(const char* text, float scale) {
        hb_buffer_t* buffer = hb_buffer_create();
        hb_buffer_add_utf8(buffer, text, -1, 0, -1);
        hb_buffer_set_direction(buffer, HB_DIRECTION_LTR);
        hb_buffer_set_script(buffer, HB_SCRIPT_LATIN);
        hb_buffer_set_language(buffer, hb_language_from_string("hu", -1));
        
        hb_shape(font.font, buffer, nullptr, 0);
        
        unsigned int glyph_count;
        hb_glyph_info_t* glyph_info = hb_buffer_get_glyph_infos(buffer, &glyph_count);
        hb_glyph_position_t* glyph_pos = hb_buffer_get_glyph_positions(buffer, &glyph_count);
        
        float total_width = 0;
        float max_height = 0;
        
        for (unsigned int i = 0; i < glyph_count; i++) {
            GlyphTexture& glyph_tex = font.GetGlyphTexture(glyph_info[i].codepoint);
            total_width += (glyph_pos[i].x_advance / 64.0f) * scale;
            float height = glyph_tex.height * scale;
            if (height > max_height) max_height = height;
        }
        
        hb_buffer_destroy(buffer);
        return Vec2::create(total_width, max_height);
    }
};