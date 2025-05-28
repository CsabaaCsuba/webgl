#pragma once
#include <wasm_simd128.h>
#include <math.h>

// Forward declarations
struct Vec3;
struct Mat4;
struct Quat;

constexpr float PI_180 = (float)M_PI / 180.0f;

float Q_rsqrt(float number);
float fast_sqrt(float number);

// Vec3 declaration (implementation later)
struct Vec3 {
    float data[3];
    
    Vec3();
    Vec3(float x, float y, float z);
    Vec3(float num);
    
    static Vec3 create(float x, float y, float z);
    Vec3 operator+(Vec3 other);
    void operator+=(Vec3 other);
    Vec3 operator-(Vec3 other);
    Vec3 operator-() const;
    void operator-=(Vec3 other);
    Vec3 operator*(float other);
    Vec3 operator*(Mat4 matrix);
    
    static Vec3 cross(Vec3 v, Vec3 u);
    static float dot(Vec3 v, Vec3 u);
    static float angle(Vec3 v1, Vec3 v2);
    static float radians(float degrees);
    static Vec3 rotate_vector(Vec3 vector, float angle_rad, Vec3 axis);
    
    float length();
    Vec3 normalize();
};

// Mat4 declaration (implementation later)
struct Mat4 {
    float data[16];
    
    Mat4();
    Mat4(float num);
    Mat4(float* numbers);
    
    static Mat4 create(float num);
    Mat4 operator*(Mat4 other);
    void operator+=(Mat4 other);
    
    static Mat4 create_rotation(float angle_rad, Vec3 axis);
    static Mat4 rotate(Mat4 input_matrix, float angle_rad, Vec3 axis);
    static Mat4 rotate_degrees(Mat4 input_matrix, float angle_deg, Vec3 axis);
    static Mat4 create_perspective(float FOVRad, float AspectRatio, float near, float far);
    static Mat4 create_view(Vec3 CameraPos, Vec3 TargetPos, Vec3 UpVector);
    static Mat4 identity();
    static Mat4 from_position(Vec3 position);
};

// Quat declaration (implementation later)
struct Quat {
    float data[4];
    
    Quat();
    Quat(float w, float x, float y, float z);
    
    static Quat from_rotation_vector(float rot_deg, Vec3 rot_vector);
    Mat4 to_matrix();
    Quat operator*(Quat other);
    Quat conjugate();
    Quat operator-() const;
};

// Now implement all the functions

float Q_rsqrt(float number) {
    long i;
    float x2, y;
    const float threehalfs = 1.5F;

    x2 = number * 0.5F;
    y  = number;
    i  = *(long*)&y;
    i  = 0x5f3759df - (i >> 1);
    y  = *(float*)&i;
    y  = y * (threehalfs - (x2 * y * y));

    return y;
}

float fast_sqrt(float number) {
    return number * Q_rsqrt(number);
}

// Vec3 implementations
Vec3::Vec3() {
    data[0] = 0.0f;
    data[1] = 0.0f;
    data[2] = 0.0f;
}

Vec3::Vec3(float x, float y, float z) {
    data[0] = x;
    data[1] = y;
    data[2] = z;
}

Vec3::Vec3(float num) {
    data[0] = num;
    data[1] = num;
    data[2] = num;
}

Vec3 Vec3::create(float x, float y, float z) {
    return Vec3(x, y, z);
}

Vec3 Vec3::operator+(Vec3 other) {
    return Vec3(
        data[0] + other.data[0],
        data[1] + other.data[1],
        data[2] + other.data[2]
    );
}

void Vec3::operator+=(Vec3 other) {
    data[0] += other.data[0];
    data[1] += other.data[1];
    data[2] += other.data[2];
}

Vec3 Vec3::operator-(Vec3 other) {
    return Vec3(
        data[0] - other.data[0],
        data[1] - other.data[1],
        data[2] - other.data[2]
    );
}

Vec3 Vec3::operator-() const {
    return Vec3(-data[0], -data[1], -data[2]);
}

void Vec3::operator-=(Vec3 other) {
    data[0] -= other.data[0];
    data[1] -= other.data[1];
    data[2] -= other.data[2];
}

Vec3 Vec3::operator*(float other) {
    return Vec3(
        data[0] * other,
        data[1] * other,
        data[2] * other
    );
}

Vec3 Vec3::cross(Vec3 v, Vec3 u) {
    return Vec3(
        v.data[1] * u.data[2] - v.data[2] * u.data[1],
        v.data[2] * u.data[0] - v.data[0] * u.data[2],
        v.data[0] * u.data[1] - v.data[1] * u.data[0]
    );
}

float Vec3::dot(Vec3 v, Vec3 u) {
    return v.data[0] * u.data[0] + v.data[1] * u.data[1] + v.data[2] * u.data[2];
}

float Vec3::angle(Vec3 v1, Vec3 v2) {
    float dot_product = Vec3::dot(v1.normalize(), v2.normalize());
    if (dot_product > 1.0f) dot_product = 1.0f;
    if (dot_product < -1.0f) dot_product = -1.0f;
    return acosf(dot_product);
}

float Vec3::radians(float degrees) {
    return degrees * PI_180;
}

Vec3 Vec3::rotate_vector(Vec3 vector, float angle_rad, Vec3 axis) {
    Mat4 rotation = Mat4::create_rotation(angle_rad, axis);
    return vector * rotation;
}

float Vec3::length() {
    return sqrtf(
        data[0] * data[0] + 
        data[1] * data[1] + 
        data[2] * data[2]
    );
}

Vec3 Vec3::normalize() {
    float len = length();
    return Vec3(
        data[0] / len,
        data[1] / len,
        data[2] / len
    );
}

// Mat4 implementations
Mat4::Mat4() {
    for (int i = 0; i < 16; i++) {
        data[i] = 0.0f;
    }
}

Mat4::Mat4(float num) {
    for (int i = 0; i < 16; i++) {
        data[i] = num;
    }
}

Mat4::Mat4(float* numbers) {
    for (int i = 0; i < 16; i++) {
        data[i] = numbers[i];
    }
}

Mat4 Mat4::create(float num) {
    return Mat4(num);
}

Mat4 Mat4::operator*(Mat4 other) {
    float result[16];
    
    for(int col = 0; col < 4; col++) {
        for(int row = 0; row < 4; row++) {
            result[col * 4 + row] = 0;
            for(int k = 0; k < 4; k++) {
                result[col * 4 + row] += data[k * 4 + row] * other.data[col * 4 + k];
            }
        }
    }
    
    return Mat4(result);
}

void Mat4::operator+=(Mat4 other){
    for (int col = 0; col<4; col++){
        for (int row=0; row<4; row++){
            data[col * 4 + row] += other.data[col * 4 + row];
        }
    }
}

Mat4 Mat4::create_rotation(float angle_rad, Vec3 axis) {
    Vec3 normalized_axis = axis.normalize();

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

    return Mat4(matrix);
}

Mat4 Mat4::rotate(Mat4 input_matrix, float angle_rad, Vec3 axis) {
    Mat4 rotation_matrix = create_rotation(angle_rad, axis);
    return input_matrix * rotation_matrix;
}

Mat4 Mat4::rotate_degrees(Mat4 input_matrix, float angle_deg, Vec3 axis) {
    return rotate(input_matrix, angle_deg * PI_180, axis);
}

Mat4 Mat4::create_perspective(float FOVRad, float AspectRatio, float near, float far) {
    float f = 1 / tanf(FOVRad/2);
    float n = 1 / (near - far);
    float data[16] = {
        f/AspectRatio, 0.0f, 0.0f, 0.0f,
        0.0f, f, 0.0f, 0.0f,
        0.0f, 0.0f, (far+near)*n, -1.0f,
        0.0f, 0.0f, (2*far*near)*n, 0.0f
    };
    return Mat4(data);
}

Mat4 Mat4::create_view(Vec3 CameraPos, Vec3 TargetPos, Vec3 UpVector) {
    Vec3 forward = (TargetPos - CameraPos).normalize();
    Vec3 right = Vec3::cross(forward, UpVector).normalize();
    Vec3 actual_up = Vec3::cross(right, forward);

    float matrix[] = {
        right.data[0], actual_up.data[0], -forward.data[0], 0.0f,
        right.data[1], actual_up.data[1], -forward.data[1], 0.0f,
        right.data[2], actual_up.data[2], -forward.data[2], 0.0f,
        -Vec3::dot(right, CameraPos), -Vec3::dot(actual_up, CameraPos), Vec3::dot(forward, CameraPos), 1.0f
    };

    return Mat4(matrix);
}

Mat4 Mat4::identity() {
    Mat4 mat(0.0f);
    mat.data[0] = 1.0f;
    mat.data[5] = 1.0f;
    mat.data[10] = 1.0f;
    mat.data[15] = 1.0f;
    return mat;
}

Mat4 Mat4::from_position(Vec3 position) {
    Mat4 mat = Mat4::identity();
    mat.data[3] = position.data[0];
    mat.data[7] = position.data[1];
    mat.data[11] = position.data[2];
    return mat;
}

// Vec3's Mat4 multiplication (needs full Mat4 definition)
Vec3 Vec3::operator*(Mat4 matrix) {
    float x = data[0] * matrix.data[0] + data[1] * matrix.data[4] + data[2] * matrix.data[8];
    float y = data[0] * matrix.data[1] + data[1] * matrix.data[5] + data[2] * matrix.data[9];
    float z = data[0] * matrix.data[2] + data[1] * matrix.data[6] + data[2] * matrix.data[10];
    return Vec3(x, y, z);
}

// Quat implementations
Quat::Quat() {
    data[0] = 1.0f;
    data[1] = 0.0f;
    data[2] = 0.0f;
    data[3] = 0.0f;
}

Quat::Quat(float w, float x, float y, float z) {
    data[0] = w;
    data[1] = x;
    data[2] = y;
    data[3] = z;
}

Quat Quat::from_rotation_vector(float rot_deg, Vec3 rot_vector) {
    float rad = PI_180 * rot_deg;
    return Quat(
        cosf(rad / 2.0f),
        rot_vector.data[0] * sinf(rad / 2.0f),
        rot_vector.data[1] * sinf(rad / 2.0f),
        rot_vector.data[2] * sinf(rad / 2.0f)
    );
}

Mat4 Quat::to_matrix() {
    float w = data[0], x = data[1], y = data[2], z = data[3];

    float matrix[] = {
        1.0f-2.0f*(y*y + z*z),  2.0f*(x*y - w*z),       2.0f*(x*z + w*y),       0.0f,
        2.0f*(x*y + w*z),       1.0f-2.0f*(x*x + z*z),  2.0f*(y*z - w*x),       0.0f,
        2.0f*(x*z - w*y),       2.0f*(y*z + w*x),       1.0f-2.0f*(x*x + y*y),  0.0f,
        0.0f,                   0.0f,                   0.0f,                   1.0f
    };

    return Mat4(matrix);
}

Quat Quat::operator*(Quat other) {
    return Quat(
        data[0] * other.data[0] - data[1] * other.data[1] - data[2] * other.data[2] - data[3] * other.data[3],
        data[0] * other.data[1] + data[1] * other.data[0] + data[2] * other.data[3] - data[3] * other.data[2],
        data[0] * other.data[2] - data[1] * other.data[3] + data[2] * other.data[0] + data[3] * other.data[1],
        data[0] * other.data[3] + data[1] * other.data[2] - data[2] * other.data[1] + data[3] * other.data[0]
    );
}

Quat Quat::conjugate() {
    return Quat(data[0], -data[1], -data[2], -data[3]);
}

Quat Quat::operator-() const {
    return Quat(-data[0], -data[1], -data[2], -data[3]);
}