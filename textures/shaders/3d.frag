#version 300 es
precision highp float;  // Use highp for better precision

in vec2 TexCoord;
out vec4 FragColor;

uniform sampler2D ourTexture;

void main() {
    // Sample the texture at the interpolated texture coordinate
    vec4 texColor = texture(ourTexture, TexCoord);
    
    // If you want to debug texture coordinates:
    // FragColor = vec4(TexCoord, 0.0, 1.0);  // This will show the texture coordinates as colors
    
    // Normal output
    FragColor = texColor;
}