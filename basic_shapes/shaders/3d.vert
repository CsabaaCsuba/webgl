#version 300 es

precision highp float;

layout (location = 0) in vec2 inputPos;

uniform mat4 CamMatrix;

void main()
{
   gl_Position = CamMatrix * vec4(inputPos.x, 1.0, inputPos.y, 1.0);
}