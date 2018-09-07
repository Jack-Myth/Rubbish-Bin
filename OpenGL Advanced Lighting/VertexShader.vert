#version 330 core

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 vNormal;
layout(location = 2) in vec2 vTextureCoordinate;

layout(std140) uniform Matrices
{
	mat4 ProjectionMatrix;
	mat4 ViewMatrix;
};
out vec2 pTextureCoordinate;
out vec3 aNormal;
out vec3 PixelPos;
uniform mat4 ModelMatrix;
uniform mat4 lightSpaceMatrix;
out mat4x4 aViewMatrix;
out vec4 PixelPosLightSpace;

void main()
{
	vec3 TargetPos=aPos;
	//TargetPos=TargetPos+vec3(gl_InstanceID/10,0,gl_InstanceID%10*0.5);
	gl_Position= ProjectionMatrix*ViewMatrix*ModelMatrix*vec4(TargetPos,1.f);
	pTextureCoordinate=vTextureCoordinate;
	aNormal=vNormal;
	PixelPos=(ViewMatrix*ModelMatrix*vec4(TargetPos,1.f)).xyz;
	aViewMatrix=ViewMatrix;
	PixelPosLightSpace= lightSpaceMatrix*vec4(aPos,1.f);
}