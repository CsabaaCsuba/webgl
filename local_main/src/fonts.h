#pragma once
#include <GLES3/gl3.h>
#include <ft2build.h>
#include FT_FREETYPE_H

#include <hb.h>
#include <hb-ft.h>
#include "types.h"
#include "enums.h"
#include "MyMath.h"


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

void render_text_hb(Font* font, Text text, float x, float y, float scale, float color[3]) {
    // Create HarfBuzz buffer
    hb_buffer_t* buf = hb_buffer_create();
    hb_buffer_add_utf8(buf, text.text, -1, 0, -1);
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
    glUseProgram(text.shader_program);
    glUniform3f(glGetUniformLocation(text.shader_program, "textColor"), color[0], color[1], color[2]);
    glActiveTexture(GL_TEXTURE0);
    glBindVertexArray(text.VAO);
    
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
        glBindBuffer(GL_ARRAY_BUFFER, text.VBO);
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

Result init_text(Text* text_ptr, Font font, char* raw_text, uint32_t shader_program){
    // Set up VAO and VBO
    glGenVertexArrays(1, &text_ptr->VAO);
    glGenBuffers(1, &text_ptr->VBO);
    glBindVertexArray(text_ptr->VAO);
    glBindBuffer(GL_ARRAY_BUFFER, text_ptr->VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    uint64_t text_len = strlen(raw_text)+1;
    text_ptr->text = (char*)malloc(text_len);
    if (text_ptr->text == 0){
        glDeleteBuffers(1, &text_ptr->VBO);
        glDeleteVertexArrays(1, &text_ptr->VBO);

        printf("failed to allocate for text copying..\n");
        return FAIL;
    }

    strncpy(text_ptr->text, raw_text, text_len-1);
    text_ptr->text[text_len-1] = '\0';
    text_ptr->shader_program = shader_program;

    return SUCCESS;
    
}