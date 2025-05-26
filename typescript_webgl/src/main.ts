const canvas = document.getElementById("canvas") as HTMLCanvasElement | null;
if (canvas === null) throw new Error("Could not find canvas element");

const gl = canvas.getContext("webgl2");
if (gl === null) throw new Error("Could not get WebGL context");

gl.viewport(0, 0, canvas.width, canvas.height);

gl.clearColor(0.5, 0.5, 0.5, 1.0);
gl.enable(gl.DEPTH_TEST);
gl.clear(gl.COLOR_BUFFER_BIT);

const VAO = gl.createVertexArray();
gl.bindVertexArray(VAO);

const triangleData = [
    0.9, 0.9,
    0.0, -0.9,
    -0.9, 0.9
]

const VBO = gl.createBuffer()
VBO.bufferData()