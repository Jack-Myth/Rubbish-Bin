#version 330 core
out vec4 FragColor;

in vec3 aNormal;
in vec2 pTextureCoordinate;
in vec3 PixelPos;
uniform vec3 objectColor;
uniform sampler2D TextureBack;
uniform sampler2D TextureFront;
uniform sampler2D SpecMap;
uniform mat3x3 NormalMatrix;
uniform mat3x3 VectorMatrix;
uniform vec3 ambientColor;
uniform float shininess;
uniform mat4x4 ViewMatrix;

//Directional Light
struct DirectionalLightInfo
{
	vec3 LightDir;
	vec3 diffuseColor;
	vec3 specularColor;
};

//Point Light
struct PointLightInfo
{
	vec3 LightPos;
	vec3 diffuseColor;
	vec3 specularColor;
    float linear;
    float quadratic;
};

//SpotLight
struct SpotlightInfo
{
	vec3 LightPos;
	vec3 LightDir;
	float InnerCos;
	float OutterCos;
	vec3 diffuseColor;
	vec3 specularColor;
    float linear;
    float quadratic;
};

vec4 CaculateDirectionalLight(DirectionalLightInfo DirLight);
vec4 CaculatePointLight(PointLightInfo pLight);
vec4 CaculateSpotlight(SpotlightInfo Spotlight);

uniform DirectionalLightInfo DirectionalLight;
uniform PointLightInfo PointLight[3];
uniform SpotlightInfo FlashLight;

vec3 Normal,ViewDir;
vec4 FragColorx;
void main()
{
	//物体的基本颜色(贴图)
	vec4 px=texture(TextureFront,pTextureCoordinate);
	float MixAlpha=1-(px.x+px.y+px.z)/3.f;
	MixAlpha=pow(MixAlpha*50,10);
	FragColorx=mix(texture(TextureBack,pTextureCoordinate),texture(TextureFront,pTextureCoordinate),clamp(MixAlpha*50,0,1.f));
	vec2 xtmpC=pTextureCoordinate-vec2(0.5f,0.5f);
	if(abs(xtmpC.x)>0.4||abs(xtmpC.y)>0.4)
		FragColorx=texture(TextureBack,pTextureCoordinate);
	//~~
	Normal=normalize(NormalMatrix*aNormal);
	ViewDir = normalize(vec3(0,0,0) - PixelPos);
	FragColor = vec4(ambientColor*0.1,1);
	FragColor=FragColor+CaculateDirectionalLight(DirectionalLight);
	for(int i=0;i<3;i++)
	{
		FragColor=FragColor+CaculatePointLight(PointLight[i]);
	}
	FragColor=FragColor+CaculateSpotlight(FlashLight);
}

vec4 CaculateDirectionalLight(DirectionalLightInfo DirLight)
{
	DirLight.LightDir=VectorMatrix*normalize(DirLight.LightDir);
	vec4 diff=clamp(dot(-DirLight.LightDir,Normal),0,1)*vec4(DirLight.diffuseColor,1)*FragColorx;
	vec3 ReflectDir=reflect(DirLight.LightDir,Normal);
	vec4 spec=pow(clamp(dot(ReflectDir,ViewDir),0,1.f),shininess)*vec4(DirLight.specularColor,1)*FragColorx;
	return spec+diff;
}

vec4 CaculatePointLight(PointLightInfo pLight)
{
	float Distance=length(pLight.LightPos-PixelPos);
	float Fatt=1.f/(1.f+pLight.linear*Distance+pLight.quadratic*pow(Distance,2));
	vec3 LightDir=normalize(pLight.LightPos-PixelPos);
	vec4 diff=clamp(dot(LightDir,Normal),0,1)*FragColorx*vec4(pLight.diffuseColor,1.f)*Fatt;
	vec3 reflectDir=reflect(-LightDir,Normal);
	vec4 spec=pow(clamp(dot(reflectDir,ViewDir),0,1),shininess)*vec4(pLight.specularColor,1)*FragColorx*Fatt;
	return diff+spec;
}

vec4 CaculateSpotlight(SpotlightInfo Spotlight)
{
	vec3 LightDir=normalize(Spotlight.LightPos-PixelPos);
	float Distance=length(Spotlight.LightPos-PixelPos);
	float Fatt=1.f/(1.f+Spotlight.linear*Distance+Spotlight.quadratic*pow(Distance,2));
	float theta = dot(LightDir, normalize(-Spotlight.LightDir));
	float DiffStrength=(theta-Spotlight.OutterCos)/(Spotlight.InnerCos-Spotlight.OutterCos);
	vec3 diffuse=DiffStrength * FragColorx.xyz*Fatt;
    vec3 ReflectDir = reflect(-LightDir, Normal);
    float SpecularStrength=pow(clamp(dot(ReflectDir,ViewDir),0,1),shininess)*DiffStrength;
    vec3 Specular=SpecularStrength*Spotlight.specularColor*texture(SpecMap,pTextureCoordinate).xyz;
    return vec4(diffuse*Spotlight.diffuseColor+Specular,1.f);
}