#pragma once
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "../enums.h"

#define ARENA_SIZE (1024 * 1024 * 10)

/*
WHAT AN ARENA SHOULD DO?
-add different data types in an array
-deallocate all of them at once
-ensure that the deallocated data won't be used again
-for now, "holes" in the arena won't be used again

How should it achieve these?
-There should be a System which manages the Arenas that are already allocated.
-If there isn't enough data in any of the arenas, a new arena will get allocated
-Upon freeing every data in an arena, the arena gets deallocated as well

*/

typedef unsigned char byte;

typedef struct Arena {
    byte* start;
    uint32_t free_start_offset;
    uint32_t used_bytes;
    uint32_t capacity_bytes;
    uint32_t remaining_bytes_at_end;
} Arena;

typedef struct ArenaSystem{
    Arena* arenas;
    uint32_t arena_count;
} ArenaSystem;

Result InitArena(Arena* arena, uint32_t size){
    if (arena == NULL){
        fprintf(stderr, "lin gan guliguli\n");
        return FAIL;
    }
    uint32_t arena_size = (size == 0) ? ARENA_SIZE : size;

    arena->start = malloc(arena_size);
    if (arena->start == NULL){
        fprintf(stderr, "initial arena allocation failed\n");
        return FAIL;
    }

    arena->free_start_offset = 0;
    arena->capacity_bytes = arena_size;
    arena->remaining_bytes_at_end = arena_size;
    arena->used_bytes = 0;

    return SUCCESS;
}

void DestroyArena(Arena* arena){
    if (arena == NULL){
        fprintf(stderr, "passed in arena was null. Freeing won't happen\n");
        return;
    }
    if (arena->start == NULL){
        fprintf(stderr, "start of arena was null. Freeing won't happen\n");
        return;
    }
    free(arena->start);
    arena->start = NULL;
    arena->free_start_offset = 0;
}


Result InitArenaSystem(ArenaSystem* arena_system) {
    if (arena_system == NULL){
        fprintf(stderr, "what the hell\n");
        return FAIL;
    }

    arena_system->arenas = (Arena*)malloc(sizeof(Arena)*10);
    if (arena_system->arenas == NULL){
        fprintf(stderr, "initial arena-system allocation failed\n");
        return FAIL;
    }
    arena_system->arena_count = 10;

    for (int i=0; i<arena_system->arena_count; i++){
        Arena new_arena;
        if (InitArena(&new_arena, 0) == FAIL){
            //iterate until i, and free every arena that already exists
            for (int j=0; j<i; j++){
                DestroyArena(arena_system->arenas+j); //+j for lighter syntax
            }
            // + also free the arenas container
            free(arena_system->arenas);
            arena_system->arenas = NULL;
            fprintf(stderr, "Arena #%d initialization failed\n", i);
            return FAIL;
        }
        arena_system->arenas[i] = new_arena;
    }

    return SUCCESS;
}

Result AddArenas(ArenaSystem* arena_system, uint32_t amount){
    if (arena_system == NULL){
        fprintf(stderr, "omajgad bro\n");
        return FAIL;
    }
    if (arena_system->arenas == NULL){
        fprintf(stderr, "arenas' array is null\n");
        return FAIL;
    }

    Arena* new_arena_ptr = (Arena*)realloc(arena_system->arenas, (arena_system->arena_count+amount)*sizeof(Arena));

    if (new_arena_ptr == NULL){
        fprintf(stderr, "failed to reallocate new arena pointer\n");
        return FAIL;
    }

    for (int i=arena_system->arena_count; i<arena_system->arena_count+amount; i++){
        Arena new_arena;
        if (InitArena(&new_arena, 0) == FAIL){
            fprintf(stderr, "Arena #%d initialization failed. Gotta make the arena array smaller now.\n", i);
            //cutting down on it's size to be as long as the current iteration count.
            arena_system->arenas = (Arena*)realloc(new_arena_ptr, i*sizeof(Arena));
            arena_system->arena_count = i;
            return SUCCESS;
        }
        new_arena_ptr[i] = new_arena;
    }

    arena_system->arenas = new_arena_ptr;
    arena_system->arena_count += amount;


    return SUCCESS;
}

Result ArenaMalloc(ArenaSystem* arena_system, byte** ptr_address, uint32_t size_bytes){
    if (arena_system == NULL || ptr_address == NULL){
        fprintf(stderr, "oi oi oi: %p %p\n", arena_system, ptr_address);
        return FAIL;
    }
    if (size_bytes >= ARENA_SIZE){
        printf("memory needed is larger than the average arena. Creating custom-sized arena...\n");
        
        // First, try to expand the arena system to hold one more arena
        Arena* new_arena_ptr = (Arena*)realloc(arena_system->arenas, (arena_system->arena_count + 1) * sizeof(Arena));
        if (new_arena_ptr == NULL) {
            fprintf(stderr, "Failed to expand arena system for custom arena\n");
            return FAIL;
        }
        arena_system->arenas = new_arena_ptr;
        
        Arena custom_arena;
        if (InitArena(&custom_arena, size_bytes) == FAIL) {
            fprintf(stderr, "Failed to create custom arena of size %u\n", size_bytes);
            return FAIL;
        }
        
        // Add the custom arena to the system
        arena_system->arenas[arena_system->arena_count] = custom_arena;
        arena_system->arena_count++;
        
        // Allocate from the new custom arena
        int custom_index = arena_system->arena_count - 1;
        *ptr_address = arena_system->arenas[custom_index].start;
        arena_system->arenas[custom_index].free_start_offset += size_bytes;
        arena_system->arenas[custom_index].used_bytes += size_bytes;
        arena_system->arenas[custom_index].remaining_bytes_at_end = 0;
        
        return SUCCESS;
    }
    for (int i=0; i<arena_system->arena_count; i++){
        uint32_t free = arena_system->arenas[i].capacity_bytes-arena_system->arenas[i].used_bytes;
        if (free >= size_bytes){ //if there's enough space
            //this might be bullshit idk tho
            *ptr_address = arena_system->arenas[i].start + arena_system->arenas[i].free_start_offset;
            arena_system->arenas[i].free_start_offset += size_bytes;
            arena_system->arenas[i].used_bytes += size_bytes;
            arena_system->arenas[i].remaining_bytes_at_end -= size_bytes;

            return SUCCESS; //we found where the data will fit. No need to go forward.
        }
    }
    //if the loop ended, that means there isn't enough space for any of the arenas -> time to allocate more
    if (AddArenas(arena_system, 1) == SUCCESS) {
        // Try the last arena
        int last = arena_system->arena_count - 1;
        if (arena_system->arenas[last].capacity_bytes >= size_bytes) {
            *ptr_address = arena_system->arenas[last].start;
            arena_system->arenas[last].free_start_offset += size_bytes;
            arena_system->arenas[last].used_bytes += size_bytes;
            arena_system->arenas[last].remaining_bytes_at_end -= size_bytes;
            return SUCCESS;
        }
    }
    return FAIL; // No space available
}

Result ArenaFree(ArenaSystem* arena_system, byte** ptr_address, uint32_t size_bytes){
    //todo: go through each arena, and pick the one which contains the address range for the given pointer
    for (int i=0; i<arena_system->arena_count; i++){
        if (arena_system->arenas[i].start <= *ptr_address && arena_system->arenas[i].start+arena_system->arenas[i].capacity_bytes > *ptr_address){
            arena_system->arenas[i].used_bytes -= size_bytes; //subtract the size of the bytes;
            *ptr_address = NULL;
            //the arena is "free" again if there are no used things now.
            if (arena_system->arenas[i].used_bytes == 0){
                arena_system->arenas[i].free_start_offset = 0;
                arena_system->arenas[i].remaining_bytes_at_end = arena_system->arenas[i].capacity_bytes;
            }
            return SUCCESS;
        }
    }
    fprintf(stderr, "pointer %p not found among arenas\n", *ptr_address);
    return FAIL;
}

//two pointers incoming
Result ArenaCleanup(ArenaSystem* arena_system) {
    if (arena_system == NULL) {
        fprintf(stderr, "Arena system is null\n");
        return FAIL;
    }
    
    if (arena_system->arenas == NULL || arena_system->arena_count == 0) {
        return SUCCESS; // Nothing to clean
    }
    
    uint32_t write_index = 0;
    
    // Compact the array by moving non-empty arenas to the front
    for (uint32_t read_index = 0; read_index < arena_system->arena_count; read_index++) {
        if (arena_system->arenas[read_index].used_bytes == 0) {
            // This arena is empty, destroy it
            DestroyArena(&arena_system->arenas[read_index]);
        } else {
            // This arena has data, keep it
            if (write_index != read_index) {
                arena_system->arenas[write_index] = arena_system->arenas[read_index];
            }
            write_index++;
        }
    }
    
    // Update the count to reflect removed arenas
    uint32_t removed_count = arena_system->arena_count - write_index;
    arena_system->arena_count = write_index;
    
    // Optionally shrink the array to save memory
    if (removed_count > 0 && arena_system->arena_count > 0) {
        Arena* new_arenas = (Arena*)realloc(arena_system->arenas, 
                                           arena_system->arena_count * sizeof(Arena));
        if (new_arenas != NULL) {
            arena_system->arenas = new_arenas;
        }
        // If realloc fails, we keep the larger array (no data loss)
    }
    
    // Handle edge case: all arenas were removed
    if (arena_system->arena_count == 0) {
        free(arena_system->arenas);
        arena_system->arenas = NULL;
    }
    
    printf("Cleaned up %u empty arenas\n", removed_count);
    return SUCCESS;
}

void DestroyArenaSystem(ArenaSystem* arena_system){
    if (arena_system == NULL){
        printf("arena system is null\n");
        return;
    }
    if (arena_system->arenas == NULL){
        printf("arenas array is null\n");
        return;
    }
    for (int i=0; i<arena_system->arena_count; i++){
        DestroyArena(arena_system->arenas+i);
    }

    free(arena_system->arenas);
    arena_system->arenas = NULL;
    arena_system->arena_count = 0;
}

void PrintArenaSystemStats(ArenaSystem* arena_system) {
    if (arena_system == NULL) return;
    
    printf("Arena System Stats:\n");
    printf("  Total arenas: %u\n", arena_system->arena_count);
    
    uint32_t total_capacity = 0;
    uint32_t total_used = 0;
    uint32_t empty_arenas = 0;
    
    for (uint32_t i = 0; i < arena_system->arena_count; i++) {
        total_capacity += arena_system->arenas[i].capacity_bytes;
        total_used += arena_system->arenas[i].used_bytes;
        if (arena_system->arenas[i].used_bytes == 0) {
            empty_arenas++;
        }
    }
    
    printf("  Total capacity: %u bytes (%.2f MB)\n", 
           total_capacity, total_capacity / (1024.0 * 1024.0));
    printf("  Total used: %u bytes (%.2f MB)\n", 
           total_used, total_used / (1024.0 * 1024.0));
    printf("  Utilization: %.1f%%\n", 
           total_capacity > 0 ? (total_used * 100.0) / total_capacity : 0);
    printf("  Empty arenas: %u\n", empty_arenas);
}
