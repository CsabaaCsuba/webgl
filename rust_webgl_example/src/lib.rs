// #![feature(f16_and_f128)] //this requires nightly
use std::cell::RefCell;
use std::rc::Rc;
use std::thread::spawn;
use wasm_bindgen::prelude::*;
use wasm_bindgen_futures::spawn_local;
use web_sys::{WebGl2RenderingContext, WebGlProgram, WebGlShader, console};

mod array;
use array::*;

mod manager;
use manager::*;

mod my_math;
use my_math::*;

mod shader_utils;
use shader_utils::*;

mod utils;
use utils::*;

#[wasm_bindgen(start)]
pub fn start() {
    spawn_local(async {
        if let Err(e) = async_main().await {
            console::log_1(&format!("Caught error: {:?}", e).into());
        }
    });
}

async fn async_main() -> Result<(), JsValue> {
    // Access the document and canvas
    let window = web_sys::window().ok_or("No global window found")?;
    let document = window.document().ok_or("No document found")?;
    let canvas = document
        .get_element_by_id("canvas")
        .ok_or("No canvas found")?
        .dyn_into::<web_sys::HtmlCanvasElement>()?;
    // Get the WebGL rendering context
    let gl = canvas
        .get_context("webgl2")?
        .ok_or("Failed to get WebGL2 context")?
        .dyn_into::<WebGl2RenderingContext>()?;

    let vert1_code = fetch_shader_code("shaders/base2d.vert").await?;
    let frag1_code = fetch_shader_code("shaders/base2d.frag").await?;

    // Initialize shaders
    let vertex_shader = compile_shader(&gl, WebGl2RenderingContext::VERTEX_SHADER, &vert1_code)?;
    let fragment_shader =
        compile_shader(&gl, WebGl2RenderingContext::FRAGMENT_SHADER, &frag1_code)?;
    let program = link_program(&gl, &vertex_shader, &fragment_shader)?;
    gl.use_program(Some(&program));
    // Set up the vertices
    let vertices: [f32; 6] = [
        0.0, 0.5, // Top vertex
        -0.5, -0.5, // Bottom left vertex
        0.5, -0.5, // Bottom right vertex
    ];
    let buffer = gl.create_buffer().ok_or("Failed to create buffer")?;
    gl.bind_buffer(WebGl2RenderingContext::ARRAY_BUFFER, Some(&buffer));

    let indices = [0, 1, 2];

    let vertices_array = js_float32_arr(&vertices);
    gl.buffer_data_with_array_buffer_view(
        WebGl2RenderingContext::ARRAY_BUFFER,
        &vertices_array,
        WebGl2RenderingContext::STATIC_DRAW,
    );

    let ebo = gl.create_buffer().ok_or("failed to create ebo ")?;
    gl.bind_buffer(WebGl2RenderingContext::ELEMENT_ARRAY_BUFFER, Some(&ebo));

    let indices_array = js_i32_arr(&indices);
    gl.buffer_data_with_array_buffer_view(
        WebGl2RenderingContext::ELEMENT_ARRAY_BUFFER,
        &indices_array,
        WebGl2RenderingContext::STATIC_DRAW,
    );

    // Link the position attribute in the vertex shader
    let position = gl.get_attrib_location(&program, "position") as u32;
    gl.vertex_attrib_pointer_with_i32(position, 2, WebGl2RenderingContext::FLOAT, false, 0, 0);
    gl.enable_vertex_attrib_array(position);
    // Clear the canvas and draw the triangle
    gl.clear_color(0.0, 0.0, 0.0, 1.0);
    gl.clear(WebGl2RenderingContext::COLOR_BUFFER_BIT);
    gl.draw_elements_with_f64(
        WebGl2RenderingContext::TRIANGLES,
        indices.len() as i32,
        WebGl2RenderingContext::UNSIGNED_INT,
        0.0,
    );
    Ok(())
}
