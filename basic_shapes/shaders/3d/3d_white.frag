#version 300 es

//this shader just colors everything white

precision highp float;
out vec4 FragColor;

void main()
{
   FragColor = vec4(1.0f, 1.0f, 1.0f, 1.0f);
}