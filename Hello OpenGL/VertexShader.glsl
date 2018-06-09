#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec4 aVertexColor;
layout (location =2) in vec2 aTextureCoord;
out vec4 pVertexColor;
out vec2 pTextureCoord;
uniform float DynamicColor;
uniform mat4 TransformMatrix;
uniform mat4 ModelMatrix;
uniform mat4 ViewMatrix;
uniform mat4 ProjectionMatrix;

float rand(vec2 co)
{
  return fract(sin(dot(co.xy, vec2(12.9898,78.233))) * 43758.5453);
}

void main()
{
    float MoveDistanceX = sin(DynamicColor*2)/2;
    float MoveDistanceY = sin(DynamicColor*4)/3;
    pTextureCoord=vec2(aPos.x+MoveDistanceX, aPos.y+MoveDistanceY)+vec2(0.5,0.6f);
    gl_Position =ProjectionMatrix* ViewMatrix*ModelMatrix*vec4(aPos,1.f);
    return;
    mat4 mTransformMatrix=TransformMatrix*sin(DynamicColor);
    float noiseX=rand(vec2(aPos.x,aPos.y));
    float noiseY=rand(vec2(-aPos.x,-aPos.y));
    vec4 maPos=vec4(aPos,1.f)*mTransformMatrix;
    gl_Position = vec4(maPos.x+MoveDistanceX, maPos.y+MoveDistanceY, maPos.z, 1.0f);
    pVertexColor=aVertexColor;
    pTextureCoord=vec2(maPos.x+MoveDistanceX, maPos.y+MoveDistanceY)+vec2(0.5,0.6f);
}

