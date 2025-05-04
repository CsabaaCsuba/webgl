use wasm_bindgen::JsCast;
use wasm_bindgen_futures::JsFuture;
use web_sys::{Request, RequestInit, RequestMode, Response};

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
