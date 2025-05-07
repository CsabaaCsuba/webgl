#version 300 es
layout (location = 0) in vec2 vertex; // <vec2 pos, vec2 tex>
layout (location = 1) in vec2 texture_coordinate;
out vec2 TexCoords;

// uniform mat4 projection;



void main() {
    gl_Position = vec4(vertex.xy, 0.0, 1.0); //could be multiplied by projection, but this is not 3d yet
    TexCoords = texture_coordinate;
}