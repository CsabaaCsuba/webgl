#version 300 es

precision highp float;

in vec3 color_out;
// uniform vec4 color;
out vec4 out_color;


void main(){

    out_color = vec4(color_out, 1.0);
}