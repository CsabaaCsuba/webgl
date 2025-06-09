#pragma once
#include "enums.h"
#include "fonts.h"
#include "MyMath.h"
#include "types.h"
#include "shader_compiler.h"

typedef struct RenderItems {
    uint32_t item_count;
    uint32_t item_cap;
    uint32_t* entity_ids;

    uint32_t* VAOs;
    uint32_t* VBOs;
    uint32_t* EBOs;
    uint32_t* shader_programs;
    uint32_t* vertex_counts;
    uint32_t* index_counts;
    
    enum RenderShape* shapes;
} RenderItem;

void init_render_items(RenderItem* items_ptr, uint32_t item_cap){
    
}

void render_all_items(RenderItems* items){

}