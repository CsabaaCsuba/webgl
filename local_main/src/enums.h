#pragma once

typedef enum Result {
    FAIL = 0,
    SUCCESS = 1,
} Result;

typedef enum {
    FILE_READ_SUCCESS,
    FILE_READ_FAILURE_OPEN,
    FILE_READ_FAILURE_ALLOC,
    FILE_READ_FAILURE_READ
} FileReadResult;


typedef enum {
    COMPILE_FAIL = 0,
    COMPILE_SUCCESS = 1,
} ShaderCompilerResult;