#version 330 core

layout(location = 0) in vec3 Pos;
layout(location = 1) in vec2 UV;
layout(location = 2) in vec3 Normal;

layout(std140) uniform Matrices
{
	mat4 ProjectionMatrix;
	mat4 ViewMatrix;
};