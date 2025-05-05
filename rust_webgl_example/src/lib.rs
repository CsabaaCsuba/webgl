// #![feature(f16_and_f128)] //this requires nightly
use std::cell::RefCell;
use std::rc::Rc;
use std::thread::spawn;
use wasm_bindgen::prelude::*;
use wasm_bindgen_futures::spawn_local;
use web_sys::{console, window, WebGl2RenderingContext, WebGlProgram, WebGlShader, Window};
use glam::{Vec3, Vec4, Mat4};

mod array;
use array::*;

mod manager;
use manager::*;


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


    let gl_ref: Rc<RefCell<WebGl2RenderingContext>> = Rc::new(RefCell::new(gl.clone()));

    let cam = Camera::new(
        gl_ref.clone(),
        window.inner_width()?.as_f64().ok_or("Failed to convert width")? as f32,
        window.inner_height()?.as_f64().ok_or("Failed to convert height")? as f32,
        Vec3::new(0.0, 0.0, 2.0),
        90.0,
        0.01,
        100.0
    );

    let basic_3d_shader = make_shader(&gl, "shaders/3d/base3d.vert", "shaders/3d/color.frag").await?;

    let sphere = Shape::sphere(&gl, Vec3::new(0.0, 0.5, 0.0), basic_3d_shader, 1.0);
    // let cube = Shape::cube(&gl, basic_3d_shader, 1.0);

    let manager = Manager::new(cam, vec![sphere]);

    
    gl.enable(WebGl2RenderingContext::DEPTH_TEST);
    start_render_loop(gl_ref, manager);

    Ok(())
}


fn start_render_loop(ref_gl: Rc<RefCell<WebGl2RenderingContext>>, manager: Manager) {
    let f: Rc<RefCell<Option<Closure<dyn FnMut()>>>> = Rc::new(RefCell::new(None));
    let g: Rc<RefCell<Option<Closure<dyn FnMut()>>>> = f.clone();

    let ref_gl_clone = ref_gl.clone();

    *g.borrow_mut() = Some(Closure::wrap(Box::new(move || {
        let gl = ref_gl_clone.borrow();
        // Clear screen and draw
        gl.clear_color(0.1, 0.0, 0.15, 0.6);
        gl.clear(WebGl2RenderingContext::COLOR_BUFFER_BIT | WebGl2RenderingContext::DEPTH_BUFFER_BIT);

        gl.use_program(Some(&manager.shapes[0].shader_program));
        gl.bind_vertex_array(Some(&manager.shapes[0].vao));
        manager.camera.use_projection(&manager.shapes[0].shader_program, "CamMatrix");
        let color_uniform = gl.get_uniform_location(&manager.shapes[0].shader_program, "color");
        gl.uniform4fv_with_f32_array(color_uniform.as_ref(), &manager.shapes[0].color.to_array());


        // let camera_info = manager.camera.debug_info();
        // web_sys::console::log_1(&camera_info.into());
        
        gl.draw_elements_with_i32(
            WebGl2RenderingContext::TRIANGLES,
            manager.shapes[0].indices_len,
            WebGl2RenderingContext::UNSIGNED_INT,
            0,
        );

        check_gl_error(&gl, "hihihi error check");
        
        // Schedule next frame
        let window = window().unwrap();
        window
            .request_animation_frame(f.borrow().as_ref().unwrap().as_ref().unchecked_ref())
            .unwrap();
    }) as Box<dyn FnMut()>));

    // Start the loop
    window()
        .unwrap()
        .request_animation_frame(g.borrow().as_ref().unwrap().as_ref().unchecked_ref())
        .unwrap();
}