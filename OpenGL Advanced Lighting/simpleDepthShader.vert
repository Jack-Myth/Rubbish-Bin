#version 330 core
layout(location = 0) in vec3 position;
layout(location = 1) in vec3 vNormal;
layout(location = 2) in vec2 vTextureCoordinate;

uniform mat4 model;

void main()
{
    gl_Position =  model * vec4(position, 1.0f);
}