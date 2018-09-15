#version 330 core

layout(location = 0) in vec3 Pos;
layout(location = 1) in vec2 UV;
layout(location = 2) in vec3 Normal;

out vec3 aPos;
out vec2 aTextureCoord;
out vec3 aNormal;

layout(std140) uniform Matrices
{
	mat4 ProjectionMatrix;
	mat4 ViewMatrix;
};

uniform mat4 ModelMatrix;

void main()
{
	gl_Position=ProjectionMatrix*ViewMatrix*ModelMatrix*vec4(aPos,1);
	aPos=(ModelMatrix*vec4(Pos,1.f)).xyz;
	aTextureCoord=UV;
	aNormal=Normal;
}