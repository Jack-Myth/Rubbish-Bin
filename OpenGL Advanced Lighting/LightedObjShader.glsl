#version 330 core
out vec4 FragColor;

in vec3 aNormal;
in vec2 pTextureCoordinate;
in vec3 PixelPos;
in mat4x4 aViewMatrix;
in vec4 PixelPosLightSpace;
uniform sampler2D DiffuseMap;
uniform sampler2D SpecularMap;
uniform samplerCube Skybox;
uniform sampler2D ShadowMap;
uniform mat3x3 NormalMatrix;
uniform mat3x3 VectorMatrix;
uniform vec3 ambientColor;
uniform float shininess;

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
float CaculateShadow(vec4 pixelPosLightSpace);

uniform DirectionalLightInfo DirectionalLight;
uniform PointLightInfo PointLight[3];
uniform SpotlightInfo FlashLight;
uniform bool UseDiffuseMap;
uniform bool UseSpecluarMap;
uniform bool UseDepthVisualization;
uniform bool UseSkyboxReflection;
uniform bool UseSkyboxReflectionAsDiffuseMap;
uniform bool UseSkyboxRefractionAsDiffuseMap;

vec3 Normal,ViewDir,ReflectDir,RefractDir,HalfwayDir;
vec4 FragColorx;
vec4 DiffuseMapPixelColor;
vec4 SpecularMapPixelColor;
vec4 PixelDiffuseColor;
vec4 PixelSpecularColor;
void main()
{
	if(UseDepthVisualization)
	{
		float z = gl_FragCoord.z * 2.0 - 1.0;
		float linearDepth = (2.0 * 0.001f * 10000.f) / (10000.f + 0.001f - z * (10000.f - 0.001f));
		FragColor = vec4(vec3(linearDepth/50.f), 1.0);
		return;
	}
	//Object Normal
	Normal=normalize(NormalMatrix*aNormal);
	ViewDir = normalize(vec3(0,0,0) - PixelPos);
	ReflectDir=reflect(-ViewDir,Normal);
	RefractDir=refract(-ViewDir,Normal,1.0f/1.33f);
	if(UseDiffuseMap)
	{
		if(UseSkyboxRefractionAsDiffuseMap&&UseSkyboxReflectionAsDiffuseMap)
		{
			DiffuseMapPixelColor=texture(Skybox,RefractDir)+texture(Skybox,ReflectDir)*0.5;
		}
		else if(UseSkyboxReflectionAsDiffuseMap)
			DiffuseMapPixelColor=texture(Skybox,ReflectDir);
		else if(UseSkyboxRefractionAsDiffuseMap)
			DiffuseMapPixelColor=texture(Skybox,RefractDir);
		else
		{
			DiffuseMapPixelColor=texture(DiffuseMap,pTextureCoordinate);
			FragColor=vec4(ambientColor*0.2,1)*texture(DiffuseMap,pTextureCoordinate);
		}
	}
	else
		DiffuseMapPixelColor=vec4(0.5,0.5,0.5,1);
	if(UseSpecluarMap)
		SpecularMapPixelColor=texture(SpecularMap,pTextureCoordinate);
	else if(UseSkyboxReflection)
		SpecularMapPixelColor=texture(Skybox,ReflectDir)*0.5;
	else
		SpecularMapPixelColor=vec4(0.5,0.5,0.5,1);
	if(!UseSkyboxRefractionAsDiffuseMap)
	{
		PixelDiffuseColor=max(CaculateDirectionalLight(DirectionalLight),0);
		//FragColor=FragColor + max(CaculateDirectionalLight(DirectionalLight),0);
		for(int i=0;i<3;i++)
		{
			FragColor=FragColor + max(CaculatePointLight(PointLight[i]),0);
		}
		FragColor=FragColor+max(CaculateSpotlight(FlashLight),0);
	}
	else
	{
		FragColor=FragColor+max(CaculateSpotlight(FlashLight)*0.2,0);
	}
	FragColor+=vec4((1-CaculateShadow(PixelPosLightSpace))*PixelDiffuseColor);
	FragColor.a=DiffuseMapPixelColor.a;
}

vec4 CaculateDirectionalLight(DirectionalLightInfo DirLight)
{
	DirLight.LightDir=VectorMatrix*normalize(DirLight.LightDir);
	vec4 diff=clamp(dot(-DirLight.LightDir,Normal),0,1)*vec4(DirLight.diffuseColor,1)*DiffuseMapPixelColor;
	//vec3 ReflectDir=reflect(DirLight.LightDir,Normal);
	vec3 HalfDir=normalize(normalize(-DirLight.LightDir)+normalize(ViewDir));
	vec4 spec=pow(clamp(dot(HalfDir,Normal),0,1.f),shininess)*vec4(DirLight.specularColor,1)*SpecularMapPixelColor;
	return spec+diff;
}

vec4 CaculatePointLight(PointLightInfo pLight)
{
	float Distance=length(pLight.LightPos-PixelPos);
	float Fatt=1.f/(1.f+pLight.linear*Distance+pLight.quadratic*pow(Distance,2));
	vec3 LightDir=normalize(pLight.LightPos-PixelPos);
	vec4 diff=clamp(dot(LightDir,Normal),0,1)*vec4(pLight.diffuseColor,1.f)*Fatt*DiffuseMapPixelColor;
	//vec3 reflectDir=reflect(-LightDir,Normal);
	vec3 HalfDir=normalize(normalize(LightDir)+normalize(ViewDir));
	vec4 spec=pow(max(dot(HalfDir,Normal),0),shininess)*vec4(pLight.specularColor,1)*Fatt*SpecularMapPixelColor;
	return diff+spec;
}

vec4 CaculateSpotlight(SpotlightInfo Spotlight)
{
	vec3 LightDir=normalize(Spotlight.LightPos-PixelPos);
	float Distance=length(Spotlight.LightPos-PixelPos);
	float Fatt=1.f/(1.f+Spotlight.linear*Distance+Spotlight.quadratic*pow(Distance,2));
	float theta = dot(LightDir, normalize(-Spotlight.LightDir));
	float DiffStrength=(theta-Spotlight.OutterCos)/(Spotlight.InnerCos-Spotlight.OutterCos);
	vec3 diffuse=DiffStrength *Fatt*DiffuseMapPixelColor.xyz;
    //vec3 ReflectDir = reflect(-LightDir, Normal);
	vec3 HalfDir=normalize(normalize(LightDir)+normalize(ViewDir));
    float SpecularStrength=pow(max(dot(HalfDir,Normal),0),shininess)*DiffStrength;
    vec3 Specular=SpecularStrength*Spotlight.specularColor*SpecularMapPixelColor.xyz;
    return vec4(diffuse*Spotlight.diffuseColor+Specular,1.f);
}

float CaculateShadow(vec4 pixelPosLightSpace)
{
	vec3 projCoords = pixelPosLightSpace.xyz / pixelPosLightSpace.w;
	//vec3 projCoords = pixelPosLightSpace.xyz;
	projCoords = projCoords * 0.5 + 0.5;
	//float closestDepth=0;
	float closestDepth = texture(ShadowMap, projCoords.xy).r;
	float currentDepth = projCoords.z;
	//return 1;
	return currentDepth>closestDepth?1.f:0.f;
}