#version 330 core

in vec3 aPos;
in vec2 aTextureCoord;
in vec3 aNormal;

layout(std140) uniform Matrices
{
	mat4 ProjectionMatrix;
	mat4 ViewMatrix;
};

//Directional Light
struct DirectionalLightInfo
{
	vec3 LightDir;
	vec3 LightColor;
	vec3 diffuseColor;
	vec3 specularColor;
};

//Point Light
struct PointLightInfo
{
	vec3 LightPos;
	vec3 LightColor;
    float linear;
    float quadratic;
};

//SpotLight
struct SpotlightInfo
{
	vec3 LightPos;
	vec3 LightDir;
	vec3 LightColor;
	float InnerCos;
	float OutterCos;
    float linear;
    float quadratic;
};

const float PI =3.1415926535;

out vec4 PixelColor;

uniform bool BaseColorUseTexture;
uniform vec3 BaseColor;
uniform sampler2D BaseColorTexture;
uniform bool MetallicUseTexture;
uniform float Metallic;
uniform sampler2D MetallicTexture;
uniform bool RoughnessUseTexture;
uniform float Roughness;
uniform sampler2D RoughnessTexture;
uniform bool AOUseTexture;
uniform float AO;
uniform sampler2D AOTexture;

const int lightMaxCount = 32;

uniform DirectionalLightInfo DirLight[lightMaxCount];
uniform int DirLightCount;
uniform PointLightInfo PointLight[lightMaxCount];
uniform int PointLightCount;

uniform vec3 CameraPos;

vec3 CaculateLight(vec3 LightColor,vec3 pL/*Pixel To Light Direction*/,vec3 LightPos/*For DirectionalLight,set it equal to aPos*/);

vec3 N,V;

void main()
{
	N=normalize(aNormal); //For Normal;
	V=normalize(CameraPos-aPos); //For Pixel to Camera Direction
	vec3 Lo=vec3(0);
	for(int i=0;i<DirLightCount;i++)
		Lo+=CaculateLight(DirLight[i].LightColor,-DirLight[i].LightDir,aPos);
	for(int i=0;i<PointLightCount;i++)
		Lo+=CaculateLight(PointLight[i].LightColor,PointLight[i].LightPos-aPos,PointLight[i].LightPos);
	vec3 ambient = vec3(0.03) * BaseColor * AO;
	vec3 color = ambient + Lo;  
	color = color / (color + vec3(1.0));
	color = pow(color, vec3(1.0/2.2)); 
	PixelColor=vec4(color,1.f);
}


vec3 fresnelSchlick(float cosTheta, vec3 F0);
float DistributionGGX(vec3 N, vec3 H, float roughness);
float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness);
vec3 CaculateLight(vec3 LightColor,vec3 pL/*Pixel To Light Direction*/,vec3 LightPos/*For DirectionalLight,set it equal to aPos*/)
{
	vec3 L=normalize(pL);
	vec3 H = normalize(V + L); //Half vector between L and V
	float distance=length(LightPos-aPos);
	float attenuation=1.f/(distance==0?1.f:pow(distance,2));
	vec3 radiance=LightColor*attenuation;
	vec3 F0=vec3(0.04f); //For non-metallic Material
	F0=mix(F0,BaseColor,Metallic); //Lerp for non-metallic to metallic material;
	vec3 F=fresnelSchlick(max(dot(H, V), 0.0),F0);
	float D=DistributionGGX(N,H,Roughness);
	float G=GeometrySmith(N,V,L,Roughness);
	vec3 nominator    = D * F * G;
	float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.001; 
	vec3 specular     = nominator / denominator;  
	vec3 kS = F;
	vec3 kD = vec3(1.0) - kS; //energy conservation
	kD *= 1.0 - Metallic; //Lerp Metallic
	vec3 lambert=BaseColor/PI;
	vec3 fr=kD*lambert+specular;
	vec3 Lo=max(dot(N,V),0.f)*fr*radiance;
	return Lo;
}

vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}  

float DistributionGGX(vec3 N, vec3 H, float roughness)
{
    float a      = roughness*roughness;
    float a2     = a*a;
    float NdotH  = max(dot(N, H), 0.0);
    float NdotH2 = NdotH*NdotH;

    float nom   = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;

    return nom / max(denom, 0.001);
}

float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r*r) / 8.0;

    float nom   = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return nom / denom;
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2  = GeometrySchlickGGX(NdotV, roughness);
    float ggx1  = GeometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}