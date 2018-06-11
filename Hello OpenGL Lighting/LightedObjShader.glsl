#version 330 core
out vec4 FragColor;

in vec3 aNormal;
in vec2 pTextureCoordinate;
in vec3 PixelPos;
in mat4x4 aViewMatrix;
uniform vec3 objectColor;
uniform vec3 lightColor;
uniform vec3 LightPos;
uniform sampler2D TextureBack;
uniform sampler2D TextureFront;
uniform mat3x3 NormalMatrix;

void main()
{
	//物体的基本颜色(贴图)
	vec4 px=texture(TextureFront,pTextureCoordinate);
	float MixAlpha=1-(px.x+px.y+px.z)/3.f;
	vec4 FragColorx=mix(texture(TextureBack,pTextureCoordinate),texture(TextureFront,pTextureCoordinate),clamp(MixAlpha*100,0,1.f));
	//~~
	float specularStrength=0.5;		//高光
	vec3 Normal=normalize(NormalMatrix*aNormal);
	vec3 LightDir=normalize(LightPos-PixelPos);
	vec3 ViewDir = normalize(vec3(0,0,0) - PixelPos);
	vec3 ReflectDir = reflect(-LightDir, Normal);
	float SpecularLightStrength=pow(clamp(dot(ReflectDir,ViewDir),0,1),32);
	float DiffStrength=clamp(dot(Normal,LightDir),0,1);
	vec3 diffuse=DiffStrength*lightColor;
	vec3 Specular=SpecularLightStrength*lightColor;
	float ambientStrength=0.1;
    FragColor = vec4((ambientStrength+diffuse+Specular) * objectColor, 1.0);
}