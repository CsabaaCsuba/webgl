use crate::my_math::*;
use wasm_bindgen::prelude::*;
use web_sys::{WebGl2RenderingContext, WebGlProgram, WebGlShader};
use std::cell::RefCell;
use std::rc::Rc;

pub struct Shape {
    vao: u32,
    vbo: u32,
    ebo: u32,
    transform: Mat4, //transform matrix
}

pub struct Camera {
    context: Rc<RefCell<WebGl2RenderingContext>>,
    window_width: f32,
    window_height: f32,
    position: Vec3,
    orientation: Vec3,
    up: Vec3,
    view_matrix: Mat4,
}

impl  Camera {
    pub fn new(
        context: Rc<RefCell<WebGl2RenderingContext>>,
        window_width: f32,
        window_height: f32,
        position: Vec3,
        fov_deg: f32,
        near: f32,
        far: f32,
    ) -> Self {
        let mut view = Mat4::default(1.0);
        let mut projection = Mat4::default(1.0);

        let orientation = Vec3::new(0.0, 0.0, -1.0);
        let up = Vec3::new(0.0, 1.0, 0.0);

        view.look_at(&position, &(position + orientation), &up);
        projection.perspective(fov_deg, near, far);

        let view_matrix = projection * view;

        return Camera {
            context,
            window_width,
            window_height,
            position,
            orientation,
            up,
            view_matrix,
        };
    }

    pub fn use_projection(&self, shader_program: &WebGlProgram, uniform_name: &str) {
        let gl = self.context.borrow();
        // Get the uniform location
        let uniform_location = gl.get_uniform_location(shader_program, uniform_name);

        // Convert the Mat4 to a slice of f32 values
        let matrix_data = self.view_matrix.as_array();

        if let Some(location) = uniform_location {
            gl.uniform_matrix4fv_with_f32_array(Some(&location), false, &matrix_data);
        } else {
            println!("Uniform location with name \"{uniform_name}\" not found!");
        }
    }
}

pub struct Manager {
    shapes: Vec<Shape>,
    camera: Camera,
}
