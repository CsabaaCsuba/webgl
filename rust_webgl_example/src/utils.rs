use web_sys::{console, window, WebGl2RenderingContext, WebGlProgram, WebGlShader, Window};

pub fn check_gl_error(gl: &WebGl2RenderingContext, label: &str) {
    let err = gl.get_error();
    if err != WebGl2RenderingContext::NO_ERROR {
        console::error_1(&format!("GL Error at {}: {:?}", label, err).into());
    }
}

pub trait MySliceExt {
    fn leni(&self) -> i32;
}

impl<T> MySliceExt for [T] {
    #[inline]
    fn leni(&self) -> i32 {
        return self.len() as i32;
    }
}