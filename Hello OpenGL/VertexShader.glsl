#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec4 aVertexColor;
layout (location =2) in vec2 aTextureCoord;
out vec4 pVertexColor;
out vec2 pTextureCoord;
uniform float DynamicColor;
uniform mat4 TransformMatrix;
uniform mat4 RotationMatrix;

float rand(vec2 co)
{
  return fract(sin(dot(co.xy, vec2(12.9898,78.233))) * 43758.5453);
}

void main()
{
    mat4 mTransformMatrix=TransformMatrix*sin(DynamicColor);
    float MoveDistanceX = sin(DynamicColor*2)/2;
    float MoveDistanceY = sin(DynamicColor*4)/3;
    float noiseX=rand(vec2(aPos.x,aPos.y));
    float noiseY=rand(vec2(-aPos.x,-aPos.y));
    vec4 maPos=vec4(aPos,1.f)*mTransformMatrix*RotationMatrix;
    gl_Position = vec4(maPos.x+MoveDistanceX, maPos.y+MoveDistanceY, maPos.z, 1.0f);
    pVertexColor=aVertexColor;
    //maPos=maPos*-mTransformMatrix*-RotationMatrix;
    pTextureCoord=vec2(maPos.x+MoveDistanceX, maPos.y+MoveDistanceY)+vec2(0.5,0.6f);
}

