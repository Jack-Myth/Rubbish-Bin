#version 330 core

in vec3 vPos;

uniform samplerCube HDRImage;

out vec4 PixelColor;

void main()
{
	PixelColor=texture(HDRImage,vPos);
}