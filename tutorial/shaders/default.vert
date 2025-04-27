#version 300 es

precision highp float;

layout (location = 0) in vec3 inputPos;
layout(location=1)  in vec3 inputColor;

out vec3 color;

void main()
{
   gl_Position = vec4(inputPos.x, inputPos.y, inputPos.z, 1.0);
   color = inputColor;
}