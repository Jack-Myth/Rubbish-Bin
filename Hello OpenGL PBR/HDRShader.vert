#version 330 core

layout(location=0) in vec3 aPos;
layout(location=1) in vec2 aUV;
layout(location=2) in vec3 aNormal;
layout(std140) uniform Matrices
{
	mat4 ProjectionMatrix;
	mat4 ViewMatrix;
};
out vec3 vPos;

void main()
{
	vec4 tmpPos=ProjectionMatrix*mat4x4(mat3x3(ViewMatrix))*vec4(aPos,1.f);
	gl_Position=tmpPos.xyww;
	vPos=aPos;
}