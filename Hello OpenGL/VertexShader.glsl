#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec4 aVertexColor;
layout (location =2) in vec2 aTextureCoord;
out vec4 pVertexColor;
out vec2 pTextureCoord;
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
    pTextureCoord=vec2(aPos.x+MoveDistanceX, aPos.y+MoveDistanceY)+vec2(0.5,0.7);
}

