#pragma once
#include "glm/glm.hpp"
#include "Shader.h"

struct FLight
{
	glm::vec3 diffuse;
	glm::vec3 specular;
	virtual void ApplyToShader(Shader* TargetShader, std::string LightVarName) = 0;
};

struct FDirectionalLight :FLight
{
	glm::vec3 dir;
	virtual void ApplyToShader(Shader* TargetShader, std::string LightVarName) override;
};

struct FPointLight :FLight
{
	glm::vec3 pos;
	float linear;
	float quadratic;
	virtual void ApplyToShader(Shader* TargetShader, std::string LightVarName) override;
};

struct FSpotlight :FLight
{
	glm::vec3 pos;
	glm::vec3 dir;
	float linear;
	float quadratic;
	float InnerAngle;
	float OutterAngle;
	virtual void ApplyToShader(Shader* TargetShader, std::string LightVarName) override;
};