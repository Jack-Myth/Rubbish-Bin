#version 330 core
out vec4 FragColor;

in vec3 aNormal;
in vec2 pTextureCoordinate;
in vec3 PixelPos;
in mat4x4 aViewMatrix;
uniform vec3 objectColor;
uniform sampler2D TextureBack;
uniform sampler2D TextureFront;
uniform sampler2D SpecMap;
uniform mat3x3 NormalMatrix;
uniform mat3x3 VectorMatrix;
uniform float shininess;

//Point Light
uniform struct PointLightInfo
{
	vec3 LightPos;
	vec3 diffuseColor;
	vec3 ambientColor;
	vec3 specularColor;
	float constant;
    float linear;
    float quadratic;
};

//Point Light
uniform struct SpotlightInfo
{
	vec3 LightPos;
	vec3 LightDir;
	float InnerCos;
	float OutterCos;
	vec3 diffuseColor;
	vec3 ambientColor;
	vec3 specularColor;
	float constant;
    float linear;
    float quadratic;
};

uniform PointLightInfo Light;
uniform SpotlightInfo Spotlight;
void main()
{
	//物体的基本颜色(贴图)
	vec4 px=texture(TextureFront,pTextureCoordinate);
	float MixAlpha=1-(px.x+px.y+px.z)/3.f;
	MixAlpha=pow(MixAlpha*50,10);
	vec4 FragColorx=mix(texture(TextureBack,pTextureCoordinate),texture(TextureFront,pTextureCoordinate),clamp(MixAlpha*50,0,1.f));
	vec2 xtmpC=pTextureCoordinate-vec2(0.5f,0.5f);
	if(abs(xtmpC.x)>0.4||abs(xtmpC.y)>0.4)
		FragColorx=texture(TextureBack,pTextureCoordinate);
	//~~
	//PointLight
	vec3 Normal=normalize(NormalMatrix*aNormal);
	vec3 LightDir=normalize(Light.LightPos-PixelPos);
	vec3 ViewDir = normalize(vec3(0,0,0) - PixelPos);
	vec3 ReflectDir = reflect(-LightDir, Normal);
	float SpecularStrength=pow(clamp(dot(ReflectDir,ViewDir),0,1),shininess);
	float Distance=length(Light.LightPos-PixelPos);
	float Fatt=1.f/(Light.constant+Light.linear*Distance+Light.quadratic*pow(Distance,2));
	float DiffStrength=clamp(dot(Normal,LightDir),0,1)*Fatt;
	vec3 diffuse=DiffStrength * FragColorx.xyz;
	vec3 Specular=SpecularStrength*Light.specularColor*texture(SpecMap,pTextureCoordinate).xyz;
	float ambientStrength=0.1;
	FragColor = vec4(ambientStrength*Light.ambientColor*FragColorx.xyz+diffuse*Light.diffuseColor+Specular, 1.0);
	//Spotlight
	LightDir=normalize(Spotlight.LightPos-PixelPos);
	Distance=length(Spotlight.LightPos-PixelPos);
	Fatt=1.f/(Spotlight.constant+Spotlight.linear*Distance+Spotlight.quadratic*pow(Distance,2));
	float theta = dot(LightDir, normalize(-Spotlight.LightDir));
	DiffStrength=(theta-Spotlight.OutterCos)/(Spotlight.InnerCos-Spotlight.OutterCos);
	diffuse=DiffStrength * FragColorx.xyz*Fatt;
	ReflectDir = reflect(-LightDir, Normal);
	SpecularStrength=pow(clamp(dot(ReflectDir,ViewDir),0,1),shininess)*DiffStrength;
	Specular=SpecularStrength*Light.specularColor*texture(SpecMap,pTextureCoordinate).xyz;
	//if(theta>Spotlight.OutterCos)
	FragColor = FragColor+vec4(diffuse*Spotlight.diffuseColor+Specular, 1.0);
}