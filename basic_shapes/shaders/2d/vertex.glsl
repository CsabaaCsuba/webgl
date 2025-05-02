#version 300 es

precision mediump float;
layout(location = 0) in vec3 Positions;
// out vec4 Color;

void main()
{
    gl_Position = vec4(Positions, 1.0);
    // Color = vec4(1.0, 0.0, 1.0, 1.0);
}

