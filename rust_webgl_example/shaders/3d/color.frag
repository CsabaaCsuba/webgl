#version 300 es

//this shader just colors everything white

precision highp float;
out vec4 FragColor;

uniform vec4 color;

void main()
{
   FragColor = color;
}