#pragma once
#include "enums.h"
#include "fonts.h"
#include "MyMath.h"
#include "types.h"
#include "shader_compiler.h"

typedef struct RenderItem {
    uint32_t EntityID;
    //flags:
    //  bit 0    |   bit 1   |   bit 2   |   bit 3   |   bit 4  |   bit 5   |   bit 6   |   bit 7   |
    //  active
    uint8_t flags;

    uint32_t VAO;
    uint32_t VBO;
    uint32_t EBO;
    uint32_t shader_program;
    
    enum RenderShape shape;
} RenderItem;

void render_all_items(RenderItem* items){

}