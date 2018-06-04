#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec4 aVertexColor;
out vec4 pVertexColor;
uniform float DynamicColor;

float rand(vec2 co)
{
  return fract(sin(dot(co.xy, vec2(12.9898,78.233))) * 43758.5453);
}

void main()
{
    float MoveDistanceX = sin(DynamicColor*2)/2;
    float MoveDistanceY = sin(DynamicColor*4)/3;
    float noiseX=rand(vec2(aPos.x,aPos.y));
    float noiseY=rand(vec2(-aPos.x,-aPos.y));
    gl_Position = vec4(aPos.x+MoveDistanceX, aPos.y+MoveDistanceY, aPos.z, 1.0);
    pVertexColor=aVertexColor;
}

