#version 330 core

in vec3 gs_outColor;

out vec4 FragColor;

void main()
{
	FragColor=vec4(gs_outColor.x,gs_outColor.y,gs_outColor.z,1.f);
}