#version 300 es

precision highp float;

layout (location = 0) in vec3 inputPos;

uniform mat4 CamMatrix;

void main()
{
   gl_Position = CamMatrix * vec4(inputPos.x, inputPos.y, inputPos.z, 1.0);
}