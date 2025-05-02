#version 300 es

precision highp float;
layout(location = 0) in vec2 BasePositions;

uniform vec2 CurrentPositions;
uniform vec2 ScreenSize;

void main()
{
    // Add the current position to the base position of each vertex
    vec2 FinalPosition = BasePositions + CurrentPositions;
    
    // Convert from pixel coordinates to normalized device coordinates
    vec2 ZeroToOne = FinalPosition / ScreenSize;
    vec2 ZeroToTwo = ZeroToOne * 2.0;
    vec2 ClipSpace = ZeroToTwo - 1.0;

    // Remember to flip the y coordinate
    gl_Position = vec4(ClipSpace.x, -ClipSpace.y, 0.0, 1.0);
}