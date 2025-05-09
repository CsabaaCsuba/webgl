#pragma once
#include "emscripten_fetch.hpp"
#include <GLES3/gl3.h>
#include <iostream>
#include <stdio.h>
#include <vector>
#include <string>

#define STB_TRUETYPE_IMPLEMENTATION
#include <stb/stb_truetype.h>

typedef struct {
    GLuint texture_id;
    int texture_width;
    int texture_height;
    stbtt_bakedchar* chars;
    int first_char;
    int num_chars;
} FontAtlas;

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