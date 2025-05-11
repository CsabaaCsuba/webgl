#pragma once
#include <math.h>
#include <stdio.h>

struct Vector {
    void* data;
    size_t type_size; //size of the type
    size_t size;
    size_t capacity;
};

size_t len_bytes(Vector* vec){
    if (vec == NULL){
        printf("null pointer encountered when calculating length in bytes\n");
        return 0;
    }
    else {
        return vec->size * vec->type_size; //size of the type
    }
}

void push_back(Vector* vec, void* data, size_t data_len){
    //resize vector if it's size is larger than it's capacity
    if (vec->size == vec->capacity){

    }
    else {

    }
}