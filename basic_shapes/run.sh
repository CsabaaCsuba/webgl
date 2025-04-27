file="src/${1:-'main'}.cpp"
emcc $file -I../include -O3 -s WASM=1 -s USE_WEBGL2=1 -s USE_GLFW=3 -s FULL_ES3=1 -s FETCH=1 -s ASYNCIFY=1 -s 'ASYNCIFY_IMPORTS=["emscripten_fetch"]' -s ALLOW_MEMORY_GROWTH=1 -o triangle.js && emrun --no_browser triangle.html
