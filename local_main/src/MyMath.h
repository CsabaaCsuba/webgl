#pragma once
#include <math.h>
#include <stddef.h>
#ifndef M_PI
    #define M_PI            3.14159265358979323846
#endif

#define PI_180 ((float)M_PI / 180.0f)

typedef struct Vec3 {
    float data[3];
} Vec3;

typedef struct Mat4 {
    float data[16];
} Mat4;

typedef struct Quat {
    float data[4];
} Quat;

typedef struct Vec4 {
    float data[4];
} Vec4;

typedef struct Vec2 {
    float data[2];
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

Quat Quat_create(float w, float x, float y, float z);
Quat Quat_from_rotation_vector(float rot_deg, Vec3 rot_vector);
Mat4 Quat_to_matrix(Quat q);
Quat Quat_mul(Quat a, Quat b);
Quat Quat_conjugate(Quat q);
Quat Quat_neg(Quat q);

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

float Q_rsqrt(float number) {
    long i;
    float x2, y;
    const float threehalfs = 1.5F;

    x2 = number * 0.5F;
    y = number;
    i = *(long*)&y;
    i = 0x5f3759df - (i >> 1);
    y = *(float*)&i;
    y = y * (threehalfs - (x2 * y * y));

    return y;
}

float fast_sqrt(float number) {
    return number * Q_rsqrt(number);
}

Vec3 Vec3_create(float x, float y, float z) {
    Vec3 v;
    v.data[0] = x;
    v.data[1] = y;
    v.data[2] = z;
    return v;
}

Vec3 Vec3_add(Vec3 a, Vec3 b) {
    return Vec3_create(
        a.data[0] + b.data[0],
        a.data[1] + b.data[1],
        a.data[2] + b.data[2]
    );
}

void Vec3_add_assign(Vec3* a, Vec3 b) {
    a->data[0] += b.data[0];
    a->data[1] += b.data[1];
    a->data[2] += b.data[2];
}

Vec3 Vec3_sub(Vec3 a, Vec3 b) {
    return Vec3_create(
        a.data[0] - b.data[0],
        a.data[1] - b.data[1],
        a.data[2] - b.data[2]
    );
}

Vec3 Vec3_neg(Vec3 v) {
    return Vec3_create(-v.data[0], -v.data[1], -v.data[2]);
}

void Vec3_sub_assign(Vec3* a, Vec3 b) {
    a->data[0] -= b.data[0];
    a->data[1] -= b.data[1];
    a->data[2] -= b.data[2];
}

Vec3 Vec3_mul_scalar(Vec3 v, float s) {
    return Vec3_create(
        v.data[0] * s,
        v.data[1] * s,
        v.data[2] * s
    );
}

Vec3 Vec3_cross(Vec3 v, Vec3 u) {
    return Vec3_create(
        v.data[1] * u.data[2] - v.data[2] * u.data[1],
        v.data[2] * u.data[0] - v.data[0] * u.data[2],
        v.data[0] * u.data[1] - v.data[1] * u.data[0]
    );
}

float Vec3_dot(Vec3 v, Vec3 u) {
    return v.data[0] * u.data[0] + v.data[1] * u.data[1] + v.data[2] * u.data[2];
}

float Vec3_angle(Vec3 v1, Vec3 v2) {
    float dot_product = Vec3_dot(Vec3_normalize(v1), Vec3_normalize(v2));
    if (dot_product > 1.0f) dot_product = 1.0f;
    if (dot_product < -1.0f) dot_product = -1.0f;
    return acosf(dot_product);
}

float Vec3_radians(float degrees) {
    return degrees * PI_180;
}

Vec3 Vec3_rotate_vector(Vec3 vector, float angle_rad, Vec3 axis) {
    Mat4 rotation = Mat4_create_rotation(angle_rad, axis);
    return Vec3_mul_mat4(vector, rotation);
}

float Vec3_length(Vec3 v) {
    return sqrtf(v.data[0] * v.data[0] + v.data[1] * v.data[1] + v.data[2] * v.data[2]);
}

Vec3 Vec3_normalize(Vec3 v) {
    float len = Vec3_length(v);
    return Vec3_create(
        v.data[0] / len,
        v.data[1] / len,
        v.data[2] / len
    );
}

Mat4 Mat4_create(float num) {
    Mat4 m;
    for (int i = 0; i < 16; i++) {
        m.data[i] = num;
    }
    return m;
}

Mat4 Mat4_create_from_array(float* numbers) {
    Mat4 m;
    for (int i = 0; i < 16; i++) {
        m.data[i] = numbers[i];
    }
    return m;
}

Mat4 Mat4_mul(Mat4 a, Mat4 b) {
    Mat4 result;
    for(int col = 0; col < 4; col++) {
        for(int row = 0; row < 4; row++) {
            result.data[col * 4 + row] = 0;
            for(int k = 0; k < 4; k++) {
                result.data[col * 4 + row] += a.data[k * 4 + row] * b.data[col * 4 + k];
            }
        }
    }
    return result;
}

void Mat4_add_assign(Mat4* a, Mat4 b) {
    for (int i = 0; i < 16; i++) {
        a->data[i] += b.data[i];
    }
}

Mat4 Mat4_create_rotation(float angle_rad, Vec3 axis) {
    Vec3 normalized_axis = Vec3_normalize(axis);

    float c = cosf(angle_rad);
    float s = sinf(angle_rad);
    float one_minus_c = 1.0f - c;

    float x = normalized_axis.data[0];
    float y = normalized_axis.data[1];
    float z = normalized_axis.data[2];

    float matrix[16] = {
        c + x*x*one_minus_c,    x*y*one_minus_c - z*s,  x*z*one_minus_c + y*s,  0.0f,
        y*x*one_minus_c + z*s,  c + y*y*one_minus_c,    y*z*one_minus_c - x*s,  0.0f,
        z*x*one_minus_c - y*s,  z*y*one_minus_c + x*s,  c + z*z*one_minus_c,    0.0f,
        0.0f,                   0.0f,                   0.0f,                   1.0f
    };

    return Mat4_create_from_array(matrix);
}

Mat4 Mat4_rotate(Mat4 input_matrix, float angle_rad, Vec3 axis) {
    Mat4 rotation_matrix = Mat4_create_rotation(angle_rad, axis);
    return Mat4_mul(input_matrix, rotation_matrix);
}

Mat4 Mat4_rotate_degrees(Mat4 input_matrix, float angle_deg, Vec3 axis) {
    return Mat4_rotate(input_matrix, angle_deg * PI_180, axis);
}

Mat4 Mat4_create_perspective(float FOVRad, float AspectRatio, float near, float far) {
    float f = 1 / tanf(FOVRad/2);
    float n = 1 / (near - far);
    float data[16] = {
        f/AspectRatio, 0.0f, 0.0f, 0.0f,
        0.0f, f, 0.0f, 0.0f,
        0.0f, 0.0f, (far+near)*n, -1.0f,
        0.0f, 0.0f, (2*far*near)*n, 0.0f
    };
    return Mat4_create_from_array(data);
}

Mat4 Mat4_create_orthographic(float left, float right, float bottom, float top, float near, float far) {
    float data[16] = {
        2.0f / (right - left),  0.0f,                   0.0f,                   0.0f,
        0.0f,                   2.0f / (top - bottom),  0.0f,                   0.0f,
        0.0f,                   0.0f,                   -2.0f / (far - near),   0.0f,
        -(right + left) / (right - left), -(top + bottom) / (top - bottom), -(far + near) / (far - near), 1.0f
    };
    return Mat4_create_from_array(data);
}

Mat4 Mat4_create_view(Vec3 CameraPos, Vec3 TargetPos, Vec3 UpVector) {
    Vec3 forward = Vec3_normalize(Vec3_sub(TargetPos, CameraPos));
    Vec3 right = Vec3_normalize(Vec3_cross(forward, UpVector));
    Vec3 actual_up = Vec3_cross(right, forward);

    float matrix[] = {
        right.data[0], actual_up.data[0], -forward.data[0], 0.0f,
        right.data[1], actual_up.data[1], -forward.data[1], 0.0f,
        right.data[2], actual_up.data[2], -forward.data[2], 0.0f,
        -Vec3_dot(right, CameraPos), -Vec3_dot(actual_up, CameraPos), Vec3_dot(forward, CameraPos), 1.0f
    };

    return Mat4_create_from_array(matrix);
}

Mat4 Mat4_identity(void) {
    Mat4 mat = Mat4_create(0.0f);
    mat.data[0] = 1.0f;
    mat.data[5] = 1.0f;
    mat.data[10] = 1.0f;
    mat.data[15] = 1.0f;
    return mat;
}

Mat4 Mat4_from_position(Vec3 position) {
    Mat4 mat = Mat4_identity();
    mat.data[3] = position.data[0];
    mat.data[7] = position.data[1];
    mat.data[11] = position.data[2];
    return mat;
}

Vec3 Vec3_mul_mat4(Vec3 v, Mat4 m) {
    float x = v.data[0] * m.data[0] + v.data[1] * m.data[4] + v.data[2] * m.data[8];
    float y = v.data[0] * m.data[1] + v.data[1] * m.data[5] + v.data[2] * m.data[9];
    float z = v.data[0] * m.data[2] + v.data[1] * m.data[6] + v.data[2] * m.data[10];
    return Vec3_create(x, y, z);
}

Quat Quat_create(float w, float x, float y, float z) {
    Quat q;
    q.data[0] = w;
    q.data[1] = x;
    q.data[2] = y;
    q.data[3] = z;
    return q;
}

Quat Quat_from_rotation_vector(float rot_deg, Vec3 rot_vector) {
    float rad = PI_180 * rot_deg;
    return Quat_create(
        cosf(rad / 2.0f),
        rot_vector.data[0] * sinf(rad / 2.0f),
        rot_vector.data[1] * sinf(rad / 2.0f),
        rot_vector.data[2] * sinf(rad / 2.0f)
    );
}

Mat4 Quat_to_matrix(Quat q) {
    float w = q.data[0], x = q.data[1], y = q.data[2], z = q.data[3];

    float matrix[] = {
        1.0f-2.0f*(y*y + z*z),  2.0f*(x*y - w*z),       2.0f*(x*z + w*y),       0.0f,
        2.0f*(x*y + w*z),       1.0f-2.0f*(x*x + z*z),  2.0f*(y*z - w*x),       0.0f,
        2.0f*(x*z - w*y),       2.0f*(y*z + w*x),       1.0f-2.0f*(x*x + y*y),  0.0f,
        0.0f,                   0.0f,                   0.0f,                   1.0f
    };

    return Mat4_create_from_array(matrix);
}

Quat Quat_mul(Quat a, Quat b) {
    return Quat_create(
        a.data[0] * b.data[0] - a.data[1] * b.data[1] - a.data[2] * b.data[2] - a.data[3] * b.data[3],
        a.data[0] * b.data[1] + a.data[1] * b.data[0] + a.data[2] * b.data[3] - a.data[3] * b.data[2],
        a.data[0] * b.data[2] - a.data[1] * b.data[3] + a.data[2] * b.data[0] + a.data[3] * b.data[1],
        a.data[0] * b.data[3] + a.data[1] * b.data[2] - a.data[2] * b.data[1] + a.data[3] * b.data[0]
    );
}

Quat Quat_conjugate(Quat q) {
    return Quat_create(q.data[0], -q.data[1], -q.data[2], -q.data[3]);
}

Quat Quat_neg(Quat q) {
    return Quat_create(-q.data[0], -q.data[1], -q.data[2], -q.data[3]);
}

Vec4 Vec4_create(float x, float y, float z, float w) {
    Vec4 v;
    v.data[0] = x;
    v.data[1] = y;
    v.data[2] = z;
    v.data[3] = w;
    return v;
}

Vec4 Vec4_from_vec3(Vec3 vec3, float w) {
    return Vec4_create(vec3.data[0], vec3.data[1], vec3.data[2], w);
}

Vec4 Vec4_add(Vec4 a, Vec4 b) {
    return Vec4_create(
        a.data[0] + b.data[0],
        a.data[1] + b.data[1],
        a.data[2] + b.data[2],
        a.data[3] + b.data[3]
    );
}

void Vec4_add_assign(Vec4* a, Vec4 b) {
    a->data[0] += b.data[0];
    a->data[1] += b.data[1];
    a->data[2] += b.data[2];
    a->data[3] += b.data[3];
}

Vec4 Vec4_sub(Vec4 a, Vec4 b) {
    return Vec4_create(
        a.data[0] - b.data[0],
        a.data[1] - b.data[1],
        a.data[2] - b.data[2],
        a.data[3] - b.data[3]
    );
}

Vec4 Vec4_neg(Vec4 v) {
    return Vec4_create(-v.data[0], -v.data[1], -v.data[2], -v.data[3]);
}

void Vec4_sub_assign(Vec4* a, Vec4 b) {
    a->data[0] -= b.data[0];
    a->data[1] -= b.data[1];
    a->data[2] -= b.data[2];
    a->data[3] -= b.data[3];
}

Vec4 Vec4_mul_scalar(Vec4 v, float s) {
    return Vec4_create(
        v.data[0] * s,
        v.data[1] * s,
        v.data[2] * s,
        v.data[3] * s
    );
}

Vec4 Vec4_mul_mat4(Vec4 v, Mat4 m) {
    float x = v.data[0] * m.data[0] + v.data[1] * m.data[4] + v.data[2] * m.data[8]  + v.data[3] * m.data[12];
    float y = v.data[0] * m.data[1] + v.data[1] * m.data[5] + v.data[2] * m.data[9]  + v.data[3] * m.data[13];
    float z = v.data[0] * m.data[2] + v.data[1] * m.data[6] + v.data[2] * m.data[10] + v.data[3] * m.data[14];
    float w = v.data[0] * m.data[3] + v.data[1] * m.data[7] + v.data[2] * m.data[11] + v.data[3] * m.data[15];
    return Vec4_create(x, y, z, w);
}

float Vec4_dot(Vec4 v, Vec4 u) {
    return v.data[0] * u.data[0] + v.data[1] * u.data[1] + v.data[2] * u.data[2] + v.data[3] * u.data[3];
}

float Vec4_angle(Vec4 v1, Vec4 v2) {
    float dot_product = Vec4_dot(Vec4_normalize(v1), Vec4_normalize(v2));
    if (dot_product > 1.0f) dot_product = 1.0f;
    if (dot_product < -1.0f) dot_product = -1.0f;
    return acosf(dot_product);
}

float Vec4_length(Vec4 v) {
    return sqrtf(v.data[0] * v.data[0] + v.data[1] * v.data[1] + v.data[2] * v.data[2] + v.data[3] * v.data[3]);
}

Vec4 Vec4_normalize(Vec4 v) {
    float len = Vec4_length(v);
    if (len == 0.0f) return Vec4_create(0.0f, 0.0f, 0.0f, 0.0f);
    return Vec4_create(
        v.data[0] / len,
        v.data[1] / len,
        v.data[2] / len,
        v.data[3] / len
    );
}

Vec3 Vec4_to_vec3(Vec4 v) {
    return Vec3_create(v.data[0], v.data[1], v.data[2]);
}

Vec3 Vec4_to_vec3_perspective(Vec4 v) {
    if (v.data[3] == 0.0f) return Vec3_create(v.data[0], v.data[1], v.data[2]);
    return Vec3_create(v.data[0] / v.data[3], v.data[1] / v.data[3], v.data[2] / v.data[3]);
}

Vec2 Vec2_create(float x, float y) {
    Vec2 v;
    v.data[0] = x;
    v.data[1] = y;
    return v;
}

Vec2 Vec2_add(Vec2 a, Vec2 b) {
    return Vec2_create(
        a.data[0] + b.data[0],
        a.data[1] + b.data[1]
    );
}

void Vec2_add_assign(Vec2* a, Vec2 b) {
    a->data[0] += b.data[0];
    a->data[1] += b.data[1];
}

Vec2 Vec2_sub(Vec2 a, Vec2 b) {
    return Vec2_create(
        a.data[0] - b.data[0],
        a.data[1] - b.data[1]
    );
}

Vec2 Vec2_neg(Vec2 v) {
    return Vec2_create(-v.data[0], -v.data[1]);
}

void Vec2_sub_assign(Vec2* a, Vec2 b) {
    a->data[0] -= b.data[0];
    a->data[1] -= b.data[1];
}

Vec2 Vec2_mul_scalar(Vec2 v, float s) {
    return Vec2_create(
        v.data[0] * s,
        v.data[1] * s
    );
}

float Vec2_dot(Vec2 v, Vec2 u) {
    return v.data[0] * u.data[0] + v.data[1] * u.data[1];
}

float Vec2_angle(Vec2 v1, Vec2 v2) {
    float dot_product = Vec2_dot(Vec2_normalize(v1), Vec2_normalize(v2));
    if (dot_product > 1.0f) dot_product = 1.0f;
    if (dot_product < -1.0f) dot_product = -1.0f;
    return acosf(dot_product);
}

float Vec2_cross(Vec2 v, Vec2 u) {
    return v.data[0] * u.data[1] - v.data[1] * u.data[0];
}

Vec2 Vec2_rotate_vector(Vec2 vector, float angle_rad) {
    float cos_a = cosf(angle_rad);
    float sin_a = sinf(angle_rad);
    return Vec2_create(
        vector.data[0] * cos_a - vector.data[1] * sin_a,
        vector.data[0] * sin_a + vector.data[1] * cos_a
    );
}

float Vec2_radians(float degrees) {
    return degrees * PI_180;
}

float Vec2_length(Vec2 v) {
    return sqrtf(v.data[0] * v.data[0] + v.data[1] * v.data[1]);
}

Vec2 Vec2_normalize(Vec2 v) {
    float len = Vec2_length(v);
    if (len == 0.0f) return Vec2_create(0.0f, 0.0f);
    return Vec2_create(v.data[0] / len, v.data[1] / len);
}

Vec3 Vec2_to_vec3(Vec2 v, float z) {
    return Vec3_create(v.data[0], v.data[1], z);
}
