#version 300 es
precision highp float;  // Use highp for better precision

in vec2 TexCoord;
out vec4 FragColor;

uniform sampler2D textTexture;

void main() {
    // Sample the texture at the interpolated texture coordinate
    float alpha = texture(textTexture, TexCoord).r;
    
    // Normal output
    FragColor = vec4(1.0, 1.0, 1.0, alpha);
}