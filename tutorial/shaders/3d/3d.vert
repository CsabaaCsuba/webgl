#version 300 es

precision highp float;

layout (location = 0) in vec3 inputPos;
layout (location = 1) in vec3 inputColor;
layout (location = 2) in vec2 inputTexPos;

out vec3 color;

out vec2 texCoord;

uniform float scale;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
   gl_Position = projection * view * model * vec4(inputPos, 1.0);
   color = inputColor;
   texCoord = inputTexPos;
}