use std::cell::RefCell;
use std::rc::Rc;
use wasm_bindgen::prelude::*;
use wasm_bindgen_futures::spawn_local;
use web_sys::{
    console, window, WebGl2RenderingContext, WebGlBuffer, WebGlProgram,
    WebGlShader, WebGlVertexArrayObject, Window,
};
use wasm_bindgen::JsCast;
use glam::{Mat4, Vec3, Vec4};

use crate::array::*;
use crate::shader_utils::*;
use crate::utils::*;

pub struct Shape {
    pub vao: WebGlVertexArrayObject,
    pub vbo: WebGlBuffer,
    pub ebo: WebGlBuffer,
    pub transform: Mat4,
    pub color: Vec4,
    pub shader_program: WebGlProgram,
    pub indices_len: i32,
}

impl Shape {
    pub fn sphere(gl: &WebGl2RenderingContext, center: Vec3, shader_program: WebGlProgram, radius: f32) -> Self {
        let sector_count = 36;
        let stack_count = 18;
        let mut vertices = vec![];
        let mut indices = vec![];

        for i in 0..=stack_count {
            let stack_angle = std::f32::consts::PI / 2.0 - i as f32 * std::f32::consts::PI / stack_count as f32;
            let xy = radius * stack_angle.cos();
            let z = radius * stack_angle.sin();

            for j in 0..=sector_count {
                let sector_angle = j as f32 * 2.0 * std::f32::consts::PI / sector_count as f32;
                let x = xy * sector_angle.cos();
                let y = xy * sector_angle.sin();

                vertices.extend_from_slice(&[center.x + x, center.y + y, center.z + z]);
            }
        }

        for i in 0..stack_count {
            let k1 = i * (sector_count + 1);
            let k2 = k1 + sector_count + 1;

            for j in 0..sector_count {
                if i != 0 {
                    indices.extend_from_slice(&[k1 + j, k2 + j, k1 + j + 1]);
                }
                if i != (stack_count - 1) {
                    indices.extend_from_slice(&[k1 + j + 1, k2 + j, k2 + j + 1]);
                }
            }
        }

        let vao = gl.create_vertex_array().expect("Failed to create VAO");
        let vbo = gl.create_buffer().expect("Failed to create VBO");
        let ebo = gl.create_buffer().expect("Failed to create EBO");

        gl.bind_vertex_array(Some(&vao));
        gl.bind_buffer(WebGl2RenderingContext::ARRAY_BUFFER, Some(&vbo));
        let vert_array = unsafe { js_sys::Float32Array::view(&vertices) };
        gl.buffer_data_with_array_buffer_view(
            WebGl2RenderingContext::ARRAY_BUFFER,
            &vert_array,
            WebGl2RenderingContext::STATIC_DRAW,
        );

        gl.bind_buffer(WebGl2RenderingContext::ELEMENT_ARRAY_BUFFER, Some(&ebo));
        let index_array = unsafe { js_sys::Uint32Array::view(&indices.iter().map(|&i| i as u32).collect::<Vec<_>>()) };
        gl.buffer_data_with_array_buffer_view(
            WebGl2RenderingContext::ELEMENT_ARRAY_BUFFER,
            &index_array,
            WebGl2RenderingContext::STATIC_DRAW,
        );

        gl.vertex_attrib_pointer_with_i32(0, 3, WebGl2RenderingContext::FLOAT, false, 12, 0);
        gl.enable_vertex_attrib_array(0);
        gl.bind_vertex_array(None);

        Shape {
            vao,
            vbo,
            ebo,
            transform: Mat4::from_translation(center),
            color: Vec4::new(0.5, 0.9, 0.5, 1.0),
            shader_program,
            indices_len: indices.len() as i32,
        }
    }

    pub fn cube(gl: &WebGl2RenderingContext, shader_program: WebGlProgram, width: f32) -> Self {
        let a = width / 2.0;
        let vertices = [
            a, 0.0, a,  -a, 0.0, a,  -a, 0.0, -a,   a, 0.0, -a,
            a, width, a,  -a, width, a,  -a, width, -a,   a, width, -a,
        ];

        let indices: [u32; 36] = [
            0, 1, 2,  0, 2, 3,
            0, 1, 5,  0, 5, 4,
            1, 2, 6,  1, 6, 5,
            2, 3, 7,  2, 7, 6,
            3, 0, 4,  3, 4, 7,
            4, 5, 6,  4, 6, 7,
        ];

        let vao = gl.create_vertex_array().expect("Failed to create VAO");
        let vbo = gl.create_buffer().expect("Failed to create VBO");
        let ebo = gl.create_buffer().expect("Failed to create EBO");

        gl.bind_vertex_array(Some(&vao));
        gl.bind_buffer(WebGl2RenderingContext::ARRAY_BUFFER, Some(&vbo));
        let vert_array = unsafe { js_sys::Float32Array::view(&vertices) };
        gl.buffer_data_with_array_buffer_view(
            WebGl2RenderingContext::ARRAY_BUFFER,
            &vert_array,
            WebGl2RenderingContext::STATIC_DRAW,
        );

        gl.bind_buffer(WebGl2RenderingContext::ELEMENT_ARRAY_BUFFER, Some(&ebo));
        let index_array = unsafe { js_sys::Uint32Array::view(&indices) };
        gl.buffer_data_with_array_buffer_view(
            WebGl2RenderingContext::ELEMENT_ARRAY_BUFFER,
            &index_array,
            WebGl2RenderingContext::STATIC_DRAW,
        );

        gl.vertex_attrib_pointer_with_i32(0, 3, WebGl2RenderingContext::FLOAT, false, 12, 0);
        gl.enable_vertex_attrib_array(0);
        gl.bind_vertex_array(None);

        Shape {
            vao,
            vbo,
            ebo,
            transform: Mat4::from_translation(Vec3::ZERO),
            color: Vec4::new(0.5, 0.9, 0.5, 1.0),
            shader_program,
            indices_len: indices.len() as i32,
        }
    }
}

pub struct Camera {
    context: Rc<RefCell<WebGl2RenderingContext>>,
    window_width: f32,
    window_height: f32,
    position: Vec3,
    orientation: Vec3,
    up: Vec3,
    view_matrix: Mat4,
    projection_matrix: Mat4,
    combined_matrix: Mat4,
}

impl Camera {
    pub fn new(
        context: Rc<RefCell<WebGl2RenderingContext>>,
        window_width: f32,
        window_height: f32,
        position: Vec3,
        fov_deg: f32,
        near: f32,
        far: f32,
    ) -> Self {
        let orientation = Vec3::new(0.0, 0.0, -1.0);
        let up = Vec3::Y;

        let mut view = Mat4::look_at_rh(position, position + orientation, up);
        let projection = Mat4::perspective_rh_gl(fov_deg.to_radians(), window_width / window_height, near, far);
        let combined_matrix = projection * view;

        Camera {
            context,
            window_width,
            window_height,
            position,
            orientation,
            up,
            view_matrix: view,
            projection_matrix: projection,
            combined_matrix,
        }
    }

    pub fn use_projection(&self, shader_program: &WebGlProgram, uniform_name: &str) {
        let gl = self.context.borrow();
        if let Some(location) = gl.get_uniform_location(shader_program, uniform_name) {
            gl.uniform_matrix4fv_with_f32_array(Some(&location), false, &self.combined_matrix.to_cols_array());
        } else {
            console::log_1(&format!("Uniform location \"{}\" not found!", uniform_name).into());
        }
    }

    pub fn debug_info(&self) -> String {
        let target = self.position + self.orientation;
        format!(
            "Camera Debug Info:\nPosition: {:?}\nTarget: {:?}\nUp: {:?}\nView Matrix:\n{:?}",
            self.position, target, self.up, self.view_matrix
        )
    }

    pub fn set_position(&mut self, position: Vec3) {
        self.position = position;
        self.update_matrices();
    }

    pub fn set_orientation(&mut self, orientation: Vec3) {
        self.orientation = orientation;
        self.update_matrices();
    }

    fn update_matrices(&mut self) {
        self.view_matrix = Mat4::look_at_rh(self.position, self.position + self.orientation, self.up);
        self.combined_matrix = self.projection_matrix * self.view_matrix;
    }
}

pub struct Manager {
    pub shapes: Vec<Shape>,
    pub camera: Camera,
}

impl Manager {
    pub fn new(camera: Camera, shapes: Vec<Shape>) -> Self {
        Manager { camera, shapes }
    }
}
