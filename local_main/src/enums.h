#pragma once

typedef enum Result {
    FAIL = 0,
    SUCCESS = 1,
} Result;

typedef enum FileReadResult {
    FILE_READ_SUCCESS,
    FILE_READ_FAILURE_OPEN,
    FILE_READ_FAILURE_ALLOC,
    FILE_READ_FAILURE_READ
} FileReadResult;


typedef enum ShaderCompilerResult {
    COMPILE_FAIL = 0,
    COMPILE_SUCCESS = 1,
} ShaderCompilerResult;

typedef enum RenderShape {
    CUBE = 0,
    TEXT = 1,
    SPHERE = 2,
    CUSTOM = 3,
} RenderShape;