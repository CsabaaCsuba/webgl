#pragma once
#include <wasm_simd128.h>
#include <math.h>

float Q_rsqrt(float number) {
    long i;
    float x2, y;
    const float threehalfs = 1.5F;

    x2 = number * 0.5F;
    y  = number;
    i  = *(long*)&y;                      // evil floating point bit level hacking
    i  = 0x5f3759df - (i >> 1);           // what the?
    y  = *(float*)&i;
    y  = y * (threehalfs - (x2 * y * y)); // 1st iteration

    return y;
}

float fast_sqrt(float number) {
    return number * Q_rsqrt(number); // sqrt(x) = x * 1/sqrt(x)
}

struct Vec3 {
    float data[3];

    Vec3(){
        data[0] = 0.0f;
        data[1] = 0.0f;
        data[2] = 0.0f;
    }

    Vec3(float x, float y, float z){
        data[0] = x;
        data[1] = y;
        data[2] = z;
    }

    Vec3(float num){
        data[0] = num;
        data[1] = num;
        data[2] = num;
    }

    static Vec3 create(float x, float y, float z){
        Vec3 new_vec(x, y, z);
        return new_vec;
    }

    Vec3 operator+(Vec3 other){
        Vec3 result(0.0f);

        result.data[0] = data[0] + other.data[0];
        result.data[1] = data[1] + other.data[1];
        result.data[2] = data[2] + other.data[2];

        return result;
    }

    void operator+=(Vec3 other){
        data[0] += other.data[0];
        data[1] += other.data[1];
        data[2] += other.data[2];
    }

    Vec3 operator-(Vec3 other){ //vector subtraction is just addition with the values of the latter vector multiplied by -1
        Vec3 auxilarry = other;
        auxilarry.data[0] = -auxilarry.data[0];
        auxilarry.data[1] = -auxilarry.data[1];
        auxilarry.data[2] = -auxilarry.data[2];

        auxilarry.data[0] += data[0];
        auxilarry.data[1] += data[1];
        auxilarry.data[2] += data[2];

        return auxilarry;
    }

    Vec3 operator-() const{
        return Vec3(-data[0], -data[1], -data[2]);
    }

    void operator-=(Vec3 other){
        data[0] -= other.data[0];
        data[1] -= other.data[1];
        data[2] -= other.data[2];
    }

    Vec3 operator*(float other){
        Vec3 result_vec(
            data[0]*other,
            data[1]*other,
            data[2]*other
        );
        return result_vec;
    }

    static Vec3 cross(Vec3 v, Vec3 u){
        Vec3 new_vec(0.0f);

        new_vec.data[0] = v.data[1] * u.data[2] - v.data[2] * u.data[1];
        new_vec.data[1] = v.data[2] * u.data[0] - v.data[0] * u.data[2];
        new_vec.data[2] = v.data[0] * u.data[1] - v.data[1] * u.data[0];

        return new_vec;
    }

    static float dot(Vec3 v, Vec3 u){
        float result = v.data[0] * u.data[0] + v.data[1] * u.data[1] + v.data[2] * u.data[2];
        return result;
    }

    float length(){
        // return fast_sqrt(
        //     powf(data[0], 2.0f)+powf(data[1], 2.0f)+powf(data[2], 2.0f)
        // );
        return sqrtf(
            powf(data[0], 2.0f)+powf(data[1], 2.0f)+powf(data[2], 2.0f)
        );
    }

    Vec3 normalize(){
        float len = length();
        Vec3 new_vec(
            data[0] / len,
            data[1] / len,
            data[2] / len
        );

        return new_vec;
    }

};

struct Mat4 {
    float data[16];

    Mat4(){
        for (int i=0; i<16; i++){
            data[i] = 0.0f;
        }
    }

    Mat4(float num){
        for (int i=0; i<16; i++){
            data[i] = num;
        }
    }

    Mat4(float* numbers){
        for (int i=0; i<16; i++){
            data[i] = numbers[i];
        }
    }

    static Mat4 create(float num){
        Mat4 new_mat(num);
        return new_mat;
    }


    Mat4 operator*(Mat4 other){
        float result[16];
        
        for(int row = 0; row < 4; row++) {
            for(int col = 0; col < 4; col++) {
                result[row * 4 + col] = 0;
                for(int k = 0; k < 4; k++) {
                    result[row * 4 + col] += data[row * 4 + k] * other.data[k * 4 + col];
                }
            }
        }
        
        return Mat4(result);
    }

    


    static Mat4 create_perspective(float FOVRad, float AspectRatio, float near, float far){
        float f = 1 / tanf(FOVRad/2);
        float n = 1 / (near - far);
        float data[16] = {
            f/AspectRatio, 0.0f, 0.0f, 0.0f,
            0.0f, f, 0.0f, 0.0f,
            0.0f, 0.0f, (far+near)*n, (2*far*near)*n,
            0.0f, 0.0f, -1.0f, 0.0f
        };
        return Mat4(data);
    }

    static Mat4 create_view(Vec3 CameraPos, Vec3 TargetPos, Vec3 UpVector){
        
        Vec3 forward = (TargetPos-CameraPos).normalize();
        Vec3 right = Vec3::cross(forward, UpVector).normalize();
        Vec3 actual_up = Vec3::cross(right, forward);

        float matrix[] = {
            right.data[0], right.data[1], right.data[2], -Vec3::dot(right, CameraPos),
            actual_up.data[0], actual_up.data[1], actual_up.data[2], -Vec3::dot(actual_up, CameraPos),
            -forward.data[0], -forward.data[1], -forward.data[2], Vec3::dot(forward, CameraPos),
            0.0f, 0.0f, 0.0f, 1.0f
        };

        return Mat4(matrix);
    }

    

    static Mat4 identity(){
        Mat4 mat(0.0f);
        mat.data[0] = 1.0f;
        mat.data[5] = 1.0f;
        mat.data[10] = 1.0f;
        mat.data[15] = 1.0f;
        return mat;
    }
};




