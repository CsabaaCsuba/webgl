#include <stdint.h>
#include <stdlib.h>
#include "../enums.h"


#define DECLARE_TYPED_POOL(Type)                                                  \
                                                                                  \
    typedef struct {                                                              \
        Type* objects;                                                            \
        uint32_t length;                                                          \
        uint32_t capacity;                                                        \
    } Type##Pool;                                                                 \
                                                                                  \
    static Result Type##Pool_init(Type##Pool* pool, uint32_t elem_count) {        \
        pool->objects = (Type*)malloc(sizeof(Type) * capacity);                   \
        pool->length = 0;                                                         \
        pool->capacity = capacity;                                                \
        if (!pool->objects || !pool->in_use) {                                    \
            free(pool->objects);                                                  \
            return FAIL;                                                          \
        }                                                                         \
        return SUCCESS;                                                           \
    }                                                                             \
                                                                                  \
    static Result Type##Pool_grow(Type##Pool* pool, uint32_t added_elements_count) {\
        pool->capacity += added_elements_count;\
        Type* new_ptr = (Type*)realloc((void*)pool->objects, pool->capacity);      \
        if (new_ptr == NULL){\
            fprintf(stderr, "failed to grow pool\n");\
            pool->capacity -= added_elements_count;\
            return FAIL;\
        }\
        pool->objects = new_ptr;\
        return SUCCESS;                                                           \
    }                                                                             \
    \
    static Result Type##Remove_at_idx(Type##Pool* pool, uint32_t idx) {           \
        if (pool == NULL){\
            fprintf(stderr, "pool is null\n");\
            return FAIL;\
        }\
        if (pool->length-1 < idx){\
            fprintf(stderr, "Provided index is larger than the pool length\n");\
            return FAIL;\
        }\
        pool->length -= 1;\
        pool->objects[idx] = pool->objects[pool->length];\
        return SUCCESS;\
    }\
    static Result Type##Pool_add_item(Type##Pool* pool, Type item){\
        if (pool == NULL){\
            fprintf(stderr, "pool is null\n");\
            return FAIL;\
        }\
        if (pool->objects == NULL){\
            fprintf(stderr, "pool's object array is null\n");\
            return FAIL;\
        }\
        if (pool->length == pool->capacity){\
            if (Type##Pool_grow(pool, pool->capacity/2) != SUCCESS) {\
                fprintf(stderr, "failed to grow pool for new item\n");\
                return FAIL;\
            }\
        }\
        pool->objects[pool->length] = item;\
        pool->length += 1;\
        return SUCCESS;\
    }                                                                             \
                                                                                  \
    static void Type##Pool_destroy(Type##Pool* pool) {                            \
        free(pool->objects);                                                      \
        pool->objects = NULL;                                                     \
        pool->capacity = 0;                                                       \
        pool->length = 0;                                                         \
    }\

