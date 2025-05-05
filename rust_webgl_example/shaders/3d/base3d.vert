#version 300 es

precision highp float;

layout (location = 0) in vec3 inputPos;

uniform mat4 CamMatrix;

// uniform mat4 Rotation;

void main()
{
   gl_Position = CamMatrix * vec4(inputPos.x, inputPos.y*(600.0/800.0), inputPos.z, 1.0);
}