#version 300 es
layout(location=0) in vec2 texCoord;
layout(location=1) in vec2 position;

out vec2 TexCoord;

// uniform vec2 screenSize;

void main() {
    // Convert from screen coordinates to normalized device coordinates
    vec2 normalizedPos = (position / vec2(800, 600)) * 2.0 - 1.0;
    // Flip y-coordinate for proper text orientation (OpenGL has Y growing upwards)
    normalizedPos.y = -normalizedPos.y;
    
    // Make sure all components of gl_Position are properly set
    gl_Position = vec4(normalizedPos, 0.0, 1.0); // z=0.0, w=1.0 is critical
    
    TexCoord = texCoord;
}
