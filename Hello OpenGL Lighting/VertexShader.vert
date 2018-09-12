#version 330 core

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec2 vTextureCoordinate;
layout(location = 2) in vec3 vNormal;
out vec2 gTextureCoordinate;
out vec3 gNormal;
out vec3 gPixelPos;
uniform mat4 ModelMatrix,ViewMatrix,ProjectionMatrix;

void main()
{
	gl_Position= ProjectionMatrix*ViewMatrix*ModelMatrix*vec4(aPos,1.f);
	gTextureCoordinate=vTextureCoordinate;
	gNormal=vNormal;
	gPixelPos=(ViewMatrix*ModelMatrix*vec4(aPos,1.f)).xyz;
}