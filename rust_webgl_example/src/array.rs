#![allow(dead_code)]
use js_sys::{Float32Array, Int32Array, Uint32Array};

pub fn js_float32_arr(arr_data: &[f32]) -> Float32Array {
    //intense face slapping
    unsafe {
        return js_sys::Float32Array::view(&arr_data);
    }
}

pub fn js_u32_arr(arr_data: &[u32]) -> Uint32Array {
    //intense face slapping
    unsafe {
        return js_sys::Uint32Array::view(&arr_data);
    }
}

pub fn js_i32_arr(arr_data: &[i32]) -> Int32Array {
    //intense face slapping
    unsafe {
        return js_sys::Int32Array::view(&arr_data);
    }
}
