#include "Light.h"

void FDirectionalLight::ApplyToShader(Shader* TargetShader, std::string LightVarName)
{
	TargetShader->SetVec3(LightVarName + ".diffuseColor", diffuse);
	TargetShader->SetVec3(LightVarName + ".specularColor", specular);
	TargetShader->SetVec3(LightVarName + ".LightDir", dir);
}

void FPointLight::ApplyToShader(Shader* TargetShader, std::string LightVarName)
{
	TargetShader->SetVec3(LightVarName + ".diffuseColor", diffuse);
	TargetShader->SetVec3(LightVarName + ".specularColor", specular);
	TargetShader->SetVec3(LightVarName + ".LightPos", pos);
	TargetShader->SetFloat(LightVarName + ".linear", linear);
	TargetShader->SetFloat(LightVarName + ".quadratic", quadratic);
}

void FSpotlight::ApplyToShader(Shader* TargetShader, std::string LightVarName)
{
	TargetShader->SetVec3(LightVarName + ".diffuseColor", diffuse);
	TargetShader->SetVec3(LightVarName + ".specularColor", specular);
	TargetShader->SetVec3(LightVarName + ".LightPos", pos);
	TargetShader->SetVec3(LightVarName + ".LightDir", dir);
	TargetShader->SetFloat(LightVarName + ".linear", linear);
	TargetShader->SetFloat(LightVarName + ".quadratic", quadratic);
	TargetShader->SetFloat(LightVarName + ".InnerCos", glm::cos(glm::radians(InnerAngle)));
	TargetShader->SetFloat(LightVarName + ".OutterCos", glm::cos(glm::radians(OutterAngle)));
}
