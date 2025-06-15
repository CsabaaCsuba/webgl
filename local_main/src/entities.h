#pragma once
#include <flecs.h>
#include <GLES3/gl3.h>
#include "types.h"
#ifndef MY_MATH
    #include "MyMath.h"
#endif

typedef struct {
    float x, y, z;
} Position, Velocity, Scale;

typedef struct {
    Quat rotation;
} Rotation;

typedef struct {
    uint32_t render_object_id;
    uint32_t vertex_buffer_id;
    uint32_t element_buffer_id;
    uint32_t shader_program_id;
    uint32_t element_count;
} RenderGroup;

//Camera Projection (+ Position, but that's already defined)
typedef struct {
    Vec3 up_vector;
} UpVector;

typedef struct {
    Vec3 target_position;
} Target;

typedef struct {
    float fov_rad;
    float aspect_ratio;
} FovRad_AR;

typedef struct {
    float near, far;
} Near_Far;


// Function declarations
void Render(ecs_iter_t *it);
void setup_world_components(ecs_world_t* world);
void InstantiateCube(ecs_world_t* world, float x, float y, float z, GLuint shader_program_id);
