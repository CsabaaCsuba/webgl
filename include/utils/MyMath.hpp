#include <wasm_simd128.h>
#include <math.h>

struct Mat4 {
    float data[16];

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

    }

    

    static Mat4 identity(){
        Mat4 mat(0.0f);
        mat.data[0] = 1.0f;
        mat.data[5] = 1.0f;
        mat.data[10] = 1.0f;
        mat.data[15] = 1.0f;
    }
};

struct Vec3 {
    float data[3];

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

    static Vec3 normalize(Vec3 v1){

    }

    //todo: implement adding and subtracting values

};