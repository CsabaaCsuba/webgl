use web_sys::{WebGl2RenderingContext, WebGlProgram, WebGlShader};
use wasm_bindgen::JsCast;
use wasm_bindgen_futures::JsFuture;
use web_sys::{Request, RequestInit, RequestMode, Response};

pub async fn make_shader(gl: &WebGl2RenderingContext, vertex_path: &str, fragment_path: &str) -> Result<WebGlProgram, String>{
    let vertex_code = fetch_shader_code(vertex_path).await?;
    // if let Err(e) = 
    let fragment_code = fetch_shader_code(fragment_path).await?;

    let vertex = compile_shader(gl, WebGl2RenderingContext::VERTEX_SHADER, &vertex_code)?;
    let fragment = compile_shader(gl, WebGl2RenderingContext::FRAGMENT_SHADER, &fragment_code)?;

    let program = link_program(gl, &vertex, &fragment)?;

    return Ok(program);

}

pub fn compile_shader(
    gl: &WebGl2RenderingContext,
    shader_type: u32,
    source: &str,
) -> Result<WebGlShader, String> {
    let shader = gl
        .create_shader(shader_type)
        .ok_or("Unable to create shader object")?;
    gl.shader_source(&shader, source);
    gl.compile_shader(&shader);
    if gl
        .get_shader_parameter(&shader, WebGl2RenderingContext::COMPILE_STATUS)
        .as_bool()
        .unwrap_or(false)
    {
        Ok(shader)
    } else {
        Err(gl.get_shader_info_log(&shader).unwrap_or_default())
    }
}

pub fn link_program(
    gl: &WebGl2RenderingContext,
    vertex_shader: &WebGlShader,
    fragment_shader: &WebGlShader,
) -> Result<WebGlProgram, String> {
    let program = gl
        .create_program()
        .ok_or("Unable to create shader program")?;
    gl.attach_shader(&program, vertex_shader);
    gl.attach_shader(&program, fragment_shader);
    gl.link_program(&program);
    if gl
        .get_program_parameter(&program, WebGl2RenderingContext::LINK_STATUS)
        .as_bool()
        .unwrap_or(false)
    {
        Ok(program)
    } else {
        Err(gl.get_program_info_log(&program).unwrap_or_default())
    }
}

pub async fn fetch_shader_code(path: &str) -> Result<String, String> {
    let mut opts = RequestInit::new();
    opts.set_method("GET");
    opts.set_mode(RequestMode::Cors);

    let request = Request::new_with_str_and_init(path, &opts)
        .map_err(|err| format!("Failed to create request: {:?}", err))?;

    let window = web_sys::window().ok_or("No window object found")?;
    let resp_value = JsFuture::from(window.fetch_with_request(&request))
        .await
        .map_err(|err| format!("Failed to fetch: {:?}", err))?;

    let resp: Response = resp_value
        .dyn_into()
        .map_err(|err| format!("Failed to cast response: {:?}", err))?;

    if !resp.ok() {
        return Err(format!("HTTP error: {}", resp.status()));
    }

    let text_promise = resp
        .text()
        .map_err(|err| format!("Failed to get text promise: {:?}", err))?;
    let text_js_value = JsFuture::from(text_promise)
        .await
        .map_err(|err| format!("Failed to resolve text promise: {:?}", err))?;

    text_js_value
        .as_string()
        .ok_or_else(|| "Response text is not a string".to_string())
}