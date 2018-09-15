#version 330 core

layout(location = 0) in vec3 Pos;
layout(location = 1) in vec2 UV;
layout(location = 2) in vec3 Normal;

layout(std140) uniform Matrices
{
	mat4 ProjectionMatrix;
	mat4 ViewMatrix;
};

out vec3 aPos;
out vec2 aTextureCoord;
out vec3 aNormal;

uniform mat4x4 ModelMatrix;

void main()
{
	gl_Position=ProjectionMatrix*ViewMatrix*ModelMatrix*vec4(Pos,1.f);
	aPos=(ModelMatrix*vec4(Pos,1.f)).xyz;
	aTextureCoord=UV;
	aNormal=Normal;
}