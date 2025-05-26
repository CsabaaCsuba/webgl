const canvas = document.getElementById('canvas') as HTMLCanvasElement;
const gl = canvas.getContext('webgl2') as WebGL2RenderingContext;

if (!gl) {
    throw new Error("WebGL2 not supported");
}

class Cube {
    public vao: WebGLVertexArrayObject;
    public shaderProgram: WebGLProgram;
    public position: Vec3;
    public indicesLen: number;

    constructor(position: Vec3, width: number, shaderID: WebGLProgram){
        this.position = position;
        this.vao = gl.createVertexArray();
        const vbo = gl.createBuffer();
        const ebo = gl.createBuffer();
        this.shaderProgram = shaderID;

        const a = width / 2.0;

        const vertices = new Float32Array([
            // Front face (positive Z)
            -a, -a,  a, // bottom-left
             a, -a,  a, // bottom-right
             a,  a,  a, // top-right
            -a,  a,  a, // top-left
            
            // Back face (negative Z)
            -a, -a, -a, // bottom-left
            -a,  a, -a, // top-left
             a,  a, -a, // top-right
             a, -a, -a, // bottom-right
            
            // Top face (positive Y)
            -a,  a,  a, // front-left
             a,  a,  a, // front-right
             a,  a, -a, // back-right
            -a,  a, -a, // back-left
            
            // Bottom face (negative Y)
            -a, -a,  a, // front-left
            -a, -a, -a, // back-left
             a, -a, -a, // back-right
             a, -a,  a, // front-right
            
            // Right face (positive X)
             a, -a,  a, // bottom-front
             a, -a, -a, // bottom-back
             a,  a, -a, // top-back
             a,  a,  a, // top-front
            
            // Left face (negative X)
            -a, -a,  a, // bottom-front
            -a,  a,  a, // top-front
            -a,  a, -a, // top-back
            -a, -a, -a, // bottom-back

        ]);

        const indices = new Int32Array([
            0,  1,  2,    2,  3,  0,  // Front face
            4,  5,  6,    6,  7,  4,  // Back face
            8,  9,  10,   10, 11, 8,  // Top face
            12, 13, 14,   14, 15, 12, // Bottom face
            16, 17, 18,   18, 19, 16, // Right face
            20, 21, 22,   22, 23, 20  // Left face
        ])

        this.indicesLen = indices.length;

        gl.bindVertexArray(this.vao);
        gl.bindBuffer(gl.ARRAY_BUFFER, vbo);
        gl.bufferData(gl.ARRAY_BUFFER, vertices, gl.STATIC_DRAW);

        gl.bindBuffer(gl.ELEMENT_ARRAY_BUFFER, ebo);
        gl.bufferData(gl.ELEMENT_ARRAY_BUFFER, indices, gl.STATIC_DRAW);

        gl.vertexAttribPointer(0, 3, gl.FLOAT, false, 3*4, 0);
        gl.enableVertexAttribArray(0);

    }
}

class Vec3 {
    public x: number;
    public y: number;
    public z: number;

    constructor(x: number, y:number, z:number){
        this.x = x;
        this.y = y;
        this.z = z;
    }
}


class Mat4 {
    public data: Float32Array;

    Mat4(value1: number){
        this.data = new Float32Array([
            value1,value1,value1,value1,
            value1,value1,value1,value1,
            value1,value1,value1,value1,
            value1,value1,value1,value1,
        ]);
    }

    // Mat4(v1: number, v2: number, v3:number, v4: number, v5:number, v6:number, v7:number, v8:number, 
    //     v9:number, v10:number, v11:number, v12:number, v13:number, v14:number, v15:number, v16:number){
    //     this.data = new Float32Array([
    //         v1,v1,v1,v1,
    //         v1,v1,v1,v1,
    //         v1,v1,v1,v1,
    //         v1,v1,v1,v1,
    //     ]);
    // }

    static identity(){

        [
            1, 0, 0, 0,
            0, 1, 0, 0,
            0, 0, 1, 0,
            0, 0, 0, 1,
        ]
    }
}



// === Load Shaders from HTML ===
function getShaderSource(id: string): string {
    const shaderScript = document.getElementById(id);
    if (!shaderScript || !shaderScript.textContent) {
        throw new Error(`Shader with ID '${id}' not found`);
    }
    return shaderScript.textContent;
}

function compileShader(source: string, type: GLenum): WebGLShader {
    const shader = gl.createShader(type)!;
    gl.shaderSource(shader, source);
    gl.compileShader(shader);
    if (!gl.getShaderParameter(shader, gl.COMPILE_STATUS)) {
        const error = gl.getShaderInfoLog(shader);
        gl.deleteShader(shader);
        throw new Error(`Shader compile error: ${error}`);
    }
    return shader;
}

function createProgram(vsSource: string, fsSource: string): WebGLProgram {
    const vs = compileShader(vsSource, gl.VERTEX_SHADER);
    const fs = compileShader(fsSource, gl.FRAGMENT_SHADER);
    const program = gl.createProgram()!;
    gl.attachShader(program, vs);
    gl.attachShader(program, fs);
    gl.linkProgram(program);

    if (!gl.getProgramParameter(program, gl.LINK_STATUS)) {
        const error = gl.getProgramInfoLog(program);
        gl.deleteProgram(program);
        throw new Error(`Program link error: ${error}`);
    }

    return program;
}

const vertexShaderSource = getShaderSource("vertex_shader");
const fragmentShaderSource = getShaderSource("fragment_shader");
const program = createProgram(vertexShaderSource, fragmentShaderSource);
gl.useProgram(program);


const colorLoc = gl.getUniformLocation(program, "u_color");

let cube = new Cube(new Vec3(0.0, 0.0, 0.0), 0.5, program);

// === Draw ===
function draw() {
    gl.clearColor(0.9, 0.9, 0.9, 1.0);
    gl.clear(gl.COLOR_BUFFER_BIT);

    gl.useProgram(program);
    gl.uniform4fv(colorLoc, [0.9, 0.2, 0.9, 1.0]);

    gl.bindVertexArray(cube.vao);
    gl.drawElements(gl.TRIANGLES, cube.indicesLen, gl.UNSIGNED_INT, 0);
}

// === Animate ===
let startTime = performance.now();
function animate() {
    const currentTime = performance.now();
    const elapsed = (currentTime - startTime) / 1000;
    draw();
    requestAnimationFrame(animate);
}

animate();
