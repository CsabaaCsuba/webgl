#version 300 es
precision mediump float;

uniform sampler2D textTexture;
in vec2 TexCoord;
out vec4 FragColor;

void main() {
    FragColor = texture(textTexture, TexCoord); // Texture already includes RGBA data
}
