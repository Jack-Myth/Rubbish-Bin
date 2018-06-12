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
	glm::vec3 dir=glm::vec3(1,-1,-1);
	virtual void ApplyToShader(Shader* TargetShader, std::string LightVarName) override;
};

struct FPointLight :FLight
{
	glm::vec3 pos= glm::vec3(0, 0, 0);
	float linear = 0.045f;
	float quadratic = 0.0075f;
	virtual void ApplyToShader(Shader* TargetShader, std::string LightVarName) override;
};

struct FSpotlight :FLight
{
	glm::vec3 pos= glm::vec3(0, 0, 0);
	glm::vec3 dir=glm::vec3(0,0,-1.f);
	float linear= 0.045f;
	float quadratic= 0.0075f;
	float InnerAngle=10.f;
	float OutterAngle=45.f;
	virtual void ApplyToShader(Shader* TargetShader, std::string LightVarName) override;
};