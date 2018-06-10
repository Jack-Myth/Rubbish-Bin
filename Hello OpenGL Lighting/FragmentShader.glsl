#version 330 core

in vec2 pTextureCoordinate;
uniform sampler2D TextureBack;
uniform sampler2D TextureFront;
out vec4 FragColor;

void main()
{
	vec4 px=texture(TextureFront,pTextureCoordinate);
	float MixAlpha=1-(px.x+px.y+px.z)/3.f;
	//FragColor=vec4(MixAlpha,MixAlpha,MixAlpha,1);
	FragColor=mix(texture(TextureBack,pTextureCoordinate),texture(TextureFront,pTextureCoordinate),clamp(MixAlpha*100,0,1.f));
}