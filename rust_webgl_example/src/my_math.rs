use std::f32::consts::PI;
use std::ops::{Add, Mul, Sub};
use std::ptr::copy_nonoverlapping;

#[derive(Debug, Copy, Clone, PartialEq)]
pub struct Vec3 {
    x: f32,
    y: f32,
    z: f32,
}

impl Vec3 {
    pub fn new(x: f32, y: f32, z: f32) -> Self {
        return Vec3 { x, y, z };
    }
    pub fn normalize(&mut self) {
        let len = (self.x * self.x + self.y * self.y + self.z * self.z).sqrt();
        self.x /= len;
        self.y /= len;
        self.z /= len;
    }
    pub fn cross(v1: &Vec3, v2: &Vec3) -> Self {
        return Vec3::new(
            v1.y * v2.z - v1.z * v2.y,
            v1.z * v2.x - v1.x * v2.z,
            v1.x * v2.y - v1.y * v2.x,
        );
    }
}

impl Sub for Vec3 {
    type Output = Vec3;

    fn sub(self, other: Vec3) -> Vec3 {
        Vec3 {
            x: self.x - other.x,
            y: self.y - other.y,
            z: self.z - other.z,
        }
    }
}
impl Add for Vec3 {
    type Output = Vec3;

    fn add(self, other: Self) -> Vec3 {
        Vec3 {
            x: self.x + other.x,
            y: self.y + other.y,
            z: self.z + other.z,
        }
    }
}

#[derive(Debug, Copy, Clone, PartialEq)]
pub struct Vec4 {
    x: f32,
    y: f32,
    z: f32,
    w: f32,
}

#[derive(Debug, Copy, Clone, PartialEq)]
pub struct Mat4 {
    x: [f32; 4],
    y: [f32; 4],
    z: [f32; 4],
    w: [f32; 4],
}

impl Mat4 {
    pub fn default(default_num: f32) -> Self {
        Mat4 {
            x: [default_num, default_num, default_num, default_num],
            y: [default_num, default_num, default_num, default_num],
            z: [default_num, default_num, default_num, default_num],
            w: [default_num, default_num, default_num, default_num],
        }
    }
    #[inline]
    pub fn flatten(&self) -> [[f32; 4]; 4] {
        unsafe {
            let mut output: [[f32; 4]; 4] = [[0.0; 4]; 4];

            // Copy each row directly
            std::ptr::copy_nonoverlapping(self.x.as_ptr(), output[0].as_mut_ptr(), 4);
            std::ptr::copy_nonoverlapping(self.y.as_ptr(), output[1].as_mut_ptr(), 4);
            std::ptr::copy_nonoverlapping(self.z.as_ptr(), output[2].as_mut_ptr(), 4);
            std::ptr::copy_nonoverlapping(self.w.as_ptr(), output[3].as_mut_ptr(), 4);

            output
        }
    }

    pub fn as_array(&self) -> [f32; 16] {
        unsafe {
            let mut output = [0.0; 16];

            std::ptr::copy_nonoverlapping(self.x.as_ptr(), output.as_mut_ptr(), 4);
            std::ptr::copy_nonoverlapping(self.y.as_ptr(), output.as_mut_ptr().offset(4), 4);
            std::ptr::copy_nonoverlapping(self.z.as_ptr(), output.as_mut_ptr().offset(8), 4);
            std::ptr::copy_nonoverlapping(self.w.as_ptr(), output.as_mut_ptr().offset(16), 4);

            output
        }
    }

    #[inline]
    pub fn from_raw(raw: [[f32; 4]; 4]) -> Self {
        unsafe {
            let mut mat = Mat4 {
                x: [0.0; 4],
                y: [0.0; 4],
                z: [0.0; 4],
                w: [0.0; 4],
            };

            // Copy each row directly using highly optimized memory operations
            std::ptr::copy_nonoverlapping(raw[0].as_ptr(), mat.x.as_mut_ptr(), 4);
            std::ptr::copy_nonoverlapping(raw[1].as_ptr(), mat.y.as_mut_ptr(), 4);
            std::ptr::copy_nonoverlapping(raw[2].as_ptr(), mat.z.as_mut_ptr(), 4);
            std::ptr::copy_nonoverlapping(raw[3].as_ptr(), mat.w.as_mut_ptr(), 4);

            mat
        }
    }

    pub fn look_at(&mut self, from: &Vec3, to: &Vec3, up: &Vec3) {
        let mut forward = *from - *to;
        forward.normalize();
        let mut right = Vec3::cross(&up, &forward);
        right.normalize();
        let up = Vec3::cross(&forward, &right);

        self.x = [right.x, right.y, right.z, 0.0];
        self.y = [up.x, up.y, up.z, 0.0];
        self.z = [forward.x, forward.y, forward.z, 0.0];
        self.w = [from.x, from.y, from.z, 0.0];
    }

    pub fn perspective(&mut self, fov: f32, near: f32, far: f32) {
        let scale = 1.0 / (fov * 0.5 * PI / 180.0).tan() as f32;
        self.x[0] = scale;
        self.y[1] = scale;
        self.z[2] = -far / (far - near);
        self.z[3] = -1.0;
        self.w[2] = -far * near / (far - near);
        self.w[3] = 0.0;
    }
}

impl Mul for Mat4 {
    type Output = Mat4;
    fn mul(self, other: Self) -> Mat4 {
        let mut output: [[f32; 4]; 4] = [[0.0; 4]; 4];

        for i in 0..4 {
            for j in 0..4 {
                for k in 0..4 {
                    output[i][j] = self.flatten()[i][k] * other.flatten()[k][j];
                }
            }
        }

        return Mat4::from_raw(output);
    }
}
