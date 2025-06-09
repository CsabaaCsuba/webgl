#pragma once
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include "enums.h"
#define ARENA_SIZE 1024 * 1024 * 10

//where each allocation begins and ends
typedef struct StartEndPair {
    uint32_t start_offset;
    uint32_t end_offset;
} StartEndPair;

typedef struct Arena {
    void* data_start;
    StartEndPair* offsetpairs;
    uint32_t size;
    uint32_t free;
    uint16_t offset_pairs_length;
} Arena;

typedef struct ArenaSystem {
    Arena* arenas;
    uint16_t arena_count;
} ArenaSystem;

Result init_arena_system(ArenaSystem* arena_system){
    if (init_arena(arena_system->arenas, ARENA_SIZE) == FAIL){
        fprintf(stderr, "Allocating arena system failed...\n");
        return FAIL;
    }

    arena_system->arena_count = 1;

    return SUCCESS;
}

Result init_arena(Arena* arena, uint32_t arena_size){
    int offset_pairs_len = 100;

    arena->data_start = malloc(arena_size);
    if (arena->data_start == NULL){
        fprintf(stderr, "Allocating memory for a new arena failed...\n");
        return FAIL;
    }
    
    arena->size = arena_size;
    arena->free = arena_size;
    arena->offsetpairs = (StartEndPair*)malloc(offset_pairs_len*sizeof(StartEndPair));
    arena->offset_pairs_length = offset_pairs_len;

    if (arena->offsetpairs == NULL){
        fprintf(stderr, "Allocating memory for offset pair failed...\n");
        free(arena->data_start);
        return FAIL;
    }

    return SUCCESS;
}

Result arena_system_alloc(ArenaSystem arena_system, uint32_t memory_size){
    
}