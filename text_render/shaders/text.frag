#version 300 es
precision mediump float;

in vec2 TexCoords;
out vec4 FragColor;

uniform sampler2D text;
uniform vec4 textColor;

void main() {
    // Sample the texture to get the glyph coverage
    float alpha = texture(text, TexCoords).r;
    
    // Discard fragments with very low alpha (optimization)
    if (alpha < 0.01)
        discard;
    
    // Final color is text color with the alpha from the font texture
    FragColor = vec4(textColor.rgb, textColor.a * alpha);
}