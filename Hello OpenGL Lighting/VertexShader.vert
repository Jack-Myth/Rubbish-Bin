#version 330 core

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec2 vTextureCoordinate;
out vec2 pTextureCoordinate;
uniform mat4 ModelMatrix,ViewMatrix,ProjectionMatrix;

void main()
{
	gl_Position= ProjectionMatrix*ViewMatrix*ModelMatrix*vec4(aPos,1.f);
	pTextureCoordinate=vTextureCoordinate;
}