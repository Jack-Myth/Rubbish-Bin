#version 330 core
out vec4 color;
in vec2 aTexCoord;

uniform samplerCube depthMap;

void main()
{             
    float depthValue = texture(depthMap,vec3(aTexCoord,1.f)).r;
	
    color = vec4(vec3(pow(depthValue,1)), 1.0);
}