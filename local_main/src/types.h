#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <ft2build.h>
#include FT_FREETYPE_H

#include <hb.h>
#include <hb-ft.h>

typedef uint32_t u32;
typedef uint64_t u64;

//text-rendering related
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

typedef struct Text {
    uint32_t VAO;
    uint32_t VBO;
    uint32_t shader_program;
    char* text;
} Text;

