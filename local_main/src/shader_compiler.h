#pragma once
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include <GLES3/gl3.h>
#include "enums.h"

// Function prototype
FileReadResult read_file(const char *filename, char **file_contents);


ShaderCompilerResult compile_shader(uint32_t type, const char* source, uint32_t* shader_ptr) {
    uint32_t shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, NULL);
    glCompileShader(shader);
    
    int success;
    char info_log[512];
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(shader, 512, NULL, info_log);
        fprintf(stderr, "Shader compilation failed: %s\n", info_log);
        return COMPILE_FAIL;
    }
    
    *shader_ptr = shader;
    return COMPILE_SUCCESS;
}


ShaderCompilerResult create_shader_program(char* vertex_name, char* fragment_name, uint32_t* program_ptr) {
    char* vertex_shader_source;
    
    char* fragment_shader_source;
    

    if (read_file(vertex_name, &vertex_shader_source) != FILE_READ_SUCCESS){
        fprintf(stderr, "Couldn't find vertex shader at the given location: %s\n", vertex_name);
        return COMPILE_FAIL;
    }
    if (read_file(fragment_name, &fragment_shader_source) != FILE_READ_SUCCESS){
        fprintf(stderr, "Couldn't find fragment shader at the given location: %s\n", fragment_name);
        free(vertex_shader_source);
        return COMPILE_FAIL;
    }


    uint32_t vertex_shader;
    compile_shader(GL_VERTEX_SHADER, vertex_shader_source, &vertex_shader);
    uint32_t fragment_shader;
    compile_shader(GL_FRAGMENT_SHADER, fragment_shader_source, &fragment_shader);
    
    uint32_t program = glCreateProgram();
    glAttachShader(program, vertex_shader);
    glAttachShader(program, fragment_shader);
    glLinkProgram(program);
    
    int success;
    char info_log[512];
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(program, 512, NULL, info_log);
        fprintf(stderr, "Shader program linking failed: %s\n", info_log);
        free(vertex_shader_source);
        free(fragment_shader_source);
        return COMPILE_FAIL;
    }
    
    glDeleteShader(vertex_shader);
    glDeleteShader(fragment_shader);
    
    *program_ptr = program;
    return COMPILE_SUCCESS;
}


FileReadResult read_file(const char *filename, char **file_contents) {
    if (filename == NULL || file_contents == NULL) {
        return FILE_READ_FAILURE_OPEN;
    }

    FILE *file = fopen(filename, "rb");
    if (!file) {
        return FILE_READ_FAILURE_OPEN;
    }

    if (fseek(file, 0, SEEK_END) != 0) {
        fclose(file);
        return FILE_READ_FAILURE_READ;
    }

    long size = ftell(file);
    if (size < 0) {
        fclose(file);
        return FILE_READ_FAILURE_READ;
    }

    rewind(file);

    char *buffer = (char*)malloc((size_t)size + 1);
    if (!buffer) {
        fclose(file);
        return FILE_READ_FAILURE_ALLOC;
    }

    size_t read = fread(buffer, 1, (size_t)size, file);
    if (read != (size_t)size) {
        free(buffer);
        fclose(file);
        return FILE_READ_FAILURE_READ;
    }

    buffer[size] = '\0'; // Null-terminate for string use
    *file_contents = buffer;

    fclose(file);
    return FILE_READ_SUCCESS;
}
