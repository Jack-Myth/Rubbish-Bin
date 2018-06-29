#version 330 core

layout(location=0) in vec3 mPos;
layout(location=1) in vec3 mNormal;
layout(location=2) in vec2 mTextureCoord;

out vec3 TexCoords;

uniform mat4 ViewMatrix,ProjectionMatrix;

void main()
{
	TexCoords=mPos;
	gl_Position=ProjectionMatrix*mat4x4(mat3x3(ViewMatrix))*vec4(mPos,1.f);
}