#pragma once
#include <math.h>
#include <stddef.h>
#ifndef M_PI
    #define M_PI            3.14159265358979323846
#endif

#ifndef MY_MATH
#define MY_MATH

#define PI_180 ((float)M_PI / 180.0f)

typedef struct Vec3 {
    union {
        struct { float x, y, z; };
        float data[3];
    };
} Vec3;

typedef struct Mat4 {
    float data[16];
} Mat4;

typedef struct Quat {
    union {
        struct { float w, x, y, z; };
        float data[4];
    };
} Quat;

typedef struct Vec4 {
    union {
        struct { float x, y, z, w; };
        float data[4];
    };
} Vec4;

typedef struct Vec2 {
    union {
        struct { float x, y; };
        float data[2];
    };
} Vec2;

float Q_rsqrt(float number);
float fast_sqrt(float number);

Vec3 Vec3_create(float x, float y, float z);
Vec3 Vec3_add(Vec3 a, Vec3 b);
void Vec3_add_assign(Vec3* a, Vec3 b);
Vec3 Vec3_sub(Vec3 a, Vec3 b);
Vec3 Vec3_neg(Vec3 v);
void Vec3_sub_assign(Vec3* a, Vec3 b);
Vec3 Vec3_mul_scalar(Vec3 v, float s);
Vec3 Vec3_mul_mat4(Vec3 v, Mat4 m);
Vec3 Vec3_cross(Vec3 v, Vec3 u);
float Vec3_dot(Vec3 v, Vec3 u);
float Vec3_angle(Vec3 v1, Vec3 v2);
float Vec3_radians(float degrees);
Vec3 Vec3_rotate_vector(Vec3 vector, float angle_rad, Vec3 axis);
float Vec3_length(Vec3 v);
Vec3 Vec3_normalize(Vec3 v);

Mat4 Mat4_create(float num);
Mat4 Mat4_create_from_array(float* numbers);
Mat4 Mat4_mul(Mat4 a, Mat4 b);
void Mat4_add_assign(Mat4* a, Mat4 b);
Mat4 Mat4_create_rotation(float angle_rad, Vec3 axis);
Mat4 Mat4_rotate(Mat4 input_matrix, float angle_rad, Vec3 axis);
Mat4 Mat4_rotate_degrees(Mat4 input_matrix, float angle_deg, Vec3 axis);
Mat4 Mat4_create_perspective(float FOVRad, float AspectRatio, float near, float far);
Mat4 Mat4_create_orthographic(float left, float right, float bottom, float top, float near, float far);
Mat4 Mat4_create_view(Vec3 CameraPos, Vec3 TargetPos, Vec3 UpVector);
Mat4 Mat4_identity(void);
Mat4 Mat4_from_position(Vec3 position);
Mat4 Mat4_create_model(Vec3 translation, Quat rotation, Vec3 scale);

Quat Quat_create(float w, float x, float y, float z);
Quat Quat_from_rotation_vector(float rot_deg, Vec3 rot_vector);
Mat4 Quat_to_matrix(Quat q);
Quat Quat_mul(Quat a, Quat b);
Quat Quat_conjugate(Quat q);
Quat Quat_neg(Quat q);
Quat Quat_identity();

Vec4 Vec4_create(float x, float y, float z, float w);
Vec4 Vec4_from_vec3(Vec3 vec3, float w);
Vec4 Vec4_add(Vec4 a, Vec4 b);
void Vec4_add_assign(Vec4* a, Vec4 b);
Vec4 Vec4_sub(Vec4 a, Vec4 b);
Vec4 Vec4_neg(Vec4 v);
void Vec4_sub_assign(Vec4* a, Vec4 b);
Vec4 Vec4_mul_scalar(Vec4 v, float s);
Vec4 Vec4_mul_mat4(Vec4 v, Mat4 m);
float Vec4_dot(Vec4 v, Vec4 u);
float Vec4_angle(Vec4 v1, Vec4 v2);
float Vec4_length(Vec4 v);
Vec4 Vec4_normalize(Vec4 v);
Vec3 Vec4_to_vec3(Vec4 v);
Vec3 Vec4_to_vec3_perspective(Vec4 v);

Vec2 Vec2_create(float x, float y);
Vec2 Vec2_add(Vec2 a, Vec2 b);
void Vec2_add_assign(Vec2* a, Vec2 b);
Vec2 Vec2_sub(Vec2 a, Vec2 b);
Vec2 Vec2_neg(Vec2 v);
void Vec2_sub_assign(Vec2* a, Vec2 b);
Vec2 Vec2_mul_scalar(Vec2 v, float s);
float Vec2_dot(Vec2 v, Vec2 u);
float Vec2_angle(Vec2 v1, Vec2 v2);
float Vec2_cross(Vec2 v, Vec2 u);
Vec2 Vec2_rotate_vector(Vec2 vector, float angle_rad);
float Vec2_radians(float degrees);
float Vec2_length(Vec2 v);
Vec2 Vec2_normalize(Vec2 v);
Vec3 Vec2_to_vec3(Vec2 v, float z);

#endif