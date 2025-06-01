#pragma once

#include <hb.h>

struct TextManager;
struct Font;

struct TextManager {
    hb_buffer_t* TextBuffer;
    uint32_t glyph_count;
    hb_glyph_info_t* glyph_info;
    hb_glyph_position_t* glyph_pos;
    Font font;

    TextManager(char* text, Font InputFont){
        font = InputFont;
        TextBuffer = hb_buffer_create();

        hb_buffer_add_utf8(TextBuffer, text, -1, 0, -1);

        hb_buffer_set_direction(TextBuffer, HB_DIRECTION_LTR);
        hb_buffer_set_script(TextBuffer, HB_SCRIPT_LATIN);
        hb_buffer_set_language(TextBuffer, hb_language_from_string("hu", -1));

        //this has to be used when I don't know the language's properties?
        //hb_buffer_guess_segment_properties(TextBuffer);

        hb_shape(font.font, TextBuffer, nullptr, 0);
        glyph_info = hb_buffer_get_glyph_infos(TextBuffer, &glyph_count);
        glyph_pos = hb_buffer_get_glyph_positions(TextBuffer, &glyph_count);
    }

    void DrawEveryGlyph(){
        hb_position_t cursor_x = 0;
        hb_position_t cursor_y = 0;
        for (uint32_t i = 0; i < glyph_count; i++) {
            hb_codepoint_t glyphid  = glyph_info[i].codepoint;
            hb_position_t x_offset  = glyph_pos[i].x_offset;
            hb_position_t y_offset  = glyph_pos[i].y_offset;
            hb_position_t x_advance = glyph_pos[i].x_advance;
            hb_position_t y_advance = glyph_pos[i].y_advance;
            //I don't know what this line is supposed to do
            //draw_glyph(glyphid, cursor_x + x_offset, cursor_y + y_offset);
            cursor_x += x_advance;
            cursor_y += y_advance;
        }
    }


    ~TextManager(){
        hb_buffer_destroy(TextBuffer);
    }

    
};

struct Font {
    hb_blob_t* blob;
    hb_face_t *face;
    hb_font_t *font;

    Font(){
        blob = nullptr;
        face = nullptr;
        font = nullptr;
    }

    Font(char* filename){
        blob = hb_blob_create_from_file(filename);
        face = hb_face_create(blob, 0);
        font = hb_font_create(face);
    }

    ~Font(){
        hb_font_destroy(font);
        hb_face_destroy(face);
        hb_blob_destroy(blob);
    }

};