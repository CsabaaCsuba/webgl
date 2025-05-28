#version 300 es
layout(location=0) in vec3 basePos;
layout(location=1) in vec2 inputTexCoord;
uniform mat4 mvp_matrix;

out vec2 TexCoord;


void main() {
    gl_Position = mvp_matrix * vec4(basePos, 1.0);
    TexCoord = inputTexCoord;
}