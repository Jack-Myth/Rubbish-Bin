#version 330 core

layout(location=0) in vec3 aPos;
layout(location = 1) in vec3 vNormal;
layout(location=2) in vec2 vTextureCoordinate;
layout(location=3) in mat4x4 ModelMatInstanced;

out vec2 pTextureCoordinate;
out vec3 aNormal;
out vec3 PixelPos;
out mat4x4 aViewMatrix;

uniform mat4 ViewMatrix,ProjectionMatrix;
uniform mat4 RotMatrix;
uniform mat4 OffsetMatrix;

void main()
{
	gl_Position= ProjectionMatrix*ViewMatrix*OffsetMatrix*RotMatrix*ModelMatInstanced*vec4(aPos,1.f);
	pTextureCoordinate=vTextureCoordinate;
	aNormal=vNormal;
	PixelPos=(ViewMatrix*OffsetMatrix*RotMatrix*ModelMatInstanced*vec4(aPos,1.f)).xyz;
	aViewMatrix=ViewMatrix;
}