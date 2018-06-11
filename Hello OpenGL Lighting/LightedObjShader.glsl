#version 330 core
out vec4 FragColor;

uniform vec3 objectColor;
uniform vec3 lightColor;

void main()
{
	float ambientStrength=2;
    FragColor = vec4(ambientStrength*lightColor * objectColor, 1.0);
}