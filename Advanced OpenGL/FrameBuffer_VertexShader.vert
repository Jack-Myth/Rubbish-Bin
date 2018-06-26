#version 330 core

layout(location=0) in vec3 mPos;
layout(location=1) in vec3 mNormal;
layout(location=2) in vec2 mTextureCoord;

out vec3 aPos;
out vec2 aTextureCoord;

void main()
{
	gl_Position=vec4(2*(mPos-0.5),1.f);
	aPos=mPos;
	aTextureCoord=mTextureCoord;
}