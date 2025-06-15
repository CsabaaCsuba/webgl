#include "entities.h"
#include <GLES3/gl3.h>

Mat4 perspective;
Mat4 view;

ECS_COMPONENT_DECLARE(Position);
ECS_COMPONENT_DECLARE(Velocity);
ECS_COMPONENT_DECLARE(Scale);
ECS_COMPONENT_DECLARE(Rotation);
ECS_COMPONENT_DECLARE(RenderGroup);
//Camera
ECS_COMPONENT_DECLARE(UpVector);
ECS_COMPONENT_DECLARE(Target);
ECS_COMPONENT_DECLARE(FovRad_AR);
ECS_COMPONENT_DECLARE(Near_Far);

Mat4 transform_matrix;

void Render(ecs_iter_t *it){
    Position* p = ecs_field(it, Position, 1);
    RenderGroup* render_group = ecs_field(it, RenderGroup, 2);

    for (int i = 0; i < it->count; i++) {
        glUseProgram(render_group[i].shader_program_id);
        glBindVertexArray(render_group[i].render_object_id);
        glDrawElements(GL_TRIANGLES, render_group[i].element_count, GL_UNSIGNED_INT, NULL);
    }
}

void setup_world_components(ecs_world_t* world){
    ECS_COMPONENT_DEFINE(world, Position);
    ECS_COMPONENT_DEFINE(world, Velocity);
    ECS_COMPONENT_DEFINE(world, Scale);
    ECS_COMPONENT_DEFINE(world, Rotation);
    ECS_COMPONENT_DEFINE(world, RenderGroup);
    //Camera
    ECS_COMPONENT_DECLARE(UpVector);
    ECS_COMPONENT_DECLARE(Target);
    ECS_COMPONENT_DECLARE(FovRad_AR);
    ECS_COMPONENT_DECLARE(Near_Far);
}

void UseCamera(ecs_world_t* world, uint64_t camera_id) {
    // Check if entity exists
    if (!ecs_is_valid(world, camera_id)) {
        printf("Camera entity with ID %lu does not exist.\n", camera_id);
        return;
    }

    // Get components - all must exist for this to be a valid camera
    Position* pos = ecs_get(world, camera_id, Position);
    UpVector* up = ecs_get(world, camera_id, UpVector);
    Target* target = ecs_get(world, camera_id, Target);
    FovRad_AR* fov_ar = ecs_get(world, camera_id, FovRad_AR);
    Near_Far* near_far = ecs_get(world, camera_id, Near_Far);

    if (!pos || !up || !target || !fov_ar || !near_far) {
        fprintf(stderr, "Entity %lu is missing one or more camera components.\n", camera_id);
        //create a fallback camera
        return;
    }

    perspective = Mat4_create_perspective(fov_ar->fov_rad, fov_ar->aspect_ratio, near_far->near, near_far->far);

    view = Mat4_create_view(pos->pos, target->target_position, up->up_vector);

    glUniformMatrix4fv(,1, GL_FALSE, perspective.data);

    // Now you can use these components, for example:
    // printf("Using Camera %lu:\n", camera_id);
    // printf(" Position: (%f, %f, %f)\n", pos->x, pos->y, pos->z);
    // printf(" Target: (%f, %f, %f)\n", target->target_position.data[0], target->target_position.data[1], target->target_position.data[2]);
    // printf(" Up Vector: (%f, %f, %f)\n", up->up_vector.data[0], up->up_vector.data[1], up->up_vector.data[2]);
    // printf(" FOV (rad): %f, Aspect Ratio: %f\n", fov_ar->fov_rad, fov_ar->aspect_ratio);
    // printf(" Near: %f, Far: %f\n", near_far->near, near_far->far);

    
}

void AddCamera(ecs_world_t* world, Vec3 pos, Vec3 target_pos, Vec3 up, float fov_rad, float aspect_ratio, float near, float far){
    ecs_entity_t camera = ecs_new(world);

    ecs_set(world, camera, Position, { pos.data[0], pos.data[1], pos.data[2] });
    ecs_set(world, camera, UpVector, { up });
    ecs_set(world, camera, Target, { target_pos });
    ecs_set(world, camera, FovRad_AR, { fov_rad, aspect_ratio });
    ecs_set(world, camera, Near_Far, { near, far });

    // Optionally: assign a tag or name
    ecs_set_name(world, camera, "MainCamera");
}

void InstantiateCube(ecs_world_t* world, float x, float y, float z, GLuint shader_program_id) {
    // Define cube vertices (position only, you can expand this with normals/UVs)
    float cube_vertices[] = {
        -0.5f, -0.5f, -0.5f,  // 0
         0.5f, -0.5f, -0.5f,  // 1
         0.5f,  0.5f, -0.5f,  // 2
        -0.5f,  0.5f, -0.5f,  // 3
        -0.5f, -0.5f,  0.5f,  // 4
         0.5f, -0.5f,  0.5f,  // 5
         0.5f,  0.5f,  0.5f,  // 6
        -0.5f,  0.5f,  0.5f   // 7
    };

    uint32_t cube_indices[] = {
        // back face
        0, 1, 2, 2, 3, 0,
        // front face
        4, 5, 6, 6, 7, 4,
        // left face
        4, 0, 3, 3, 7, 4,
        // right face
        1, 5, 6, 6, 2, 1,
        // bottom face
        4, 5, 1, 1, 0, 4,
        // top face
        3, 2, 6, 6, 7, 3
    };

    GLuint VAO, VBO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cube_vertices), cube_vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(cube_indices), cube_indices, GL_STATIC_DRAW);

    // Assume position-only vertices
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindVertexArray(0); // unbind VAO

    // Create ECS entity
    ecs_entity_t cube = ecs_new(world);

    // Attach components
    ecs_set(world, cube, Position, {x, y, z});
    ecs_set(world, cube, Scale, {1.0f, 1.0f, 1.0f});
    ecs_set(world, cube, Rotation, {Quat_identity()}); // assuming Quat_Identity is defined

    ecs_set(world, cube, RenderGroup, {
        .render_object_id = VAO,
        .vertex_buffer_id = VBO,
        .element_buffer_id = EBO,
        .shader_program_id = shader_program_id,
        .element_count = sizeof(cube_indices) / sizeof(uint32_t)
    });
}
