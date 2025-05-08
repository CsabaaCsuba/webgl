#version 300 es
layout (location = 0) in vec2 vertex;
layout (location = 1) in vec2 texture_coordinate;

out vec2 TexCoords;

// Simplified projection: [scale_x, scale_y, offset_x, offset_y]
uniform vec4 projection;

void main() {
    // Apply the simplified 2D transformation
    vec2 transformed = vec2(
        vertex.x * projection.x + projection.z,
        vertex.y * projection.y + projection.w
    );
    gl_Position = vec4(transformed, 0.0, 1.0);
    TexCoords = texture_coordinate;
}