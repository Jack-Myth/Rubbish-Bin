#version 330 core
layout(triangles) in;
layout(triangle_strip,max_vertices =3) out;

in vec2 gTextureCoordinate[];
in vec3 gNormal[];
in vec3 gPixelPos[];
uniform mat4 ModelMatrix;
out mat3 TBN;
out vec2 pTextureCoordinate;
out vec3 aNormal;
out vec3 PixelPos;

void main()
{
	vec3 edge1 = (gl_in[1].gl_Position - gl_in[0].gl_Position).xyz;
	vec3 edge2 = (gl_in[2].gl_Position - gl_in[0].gl_Position).xyz;
	vec2 deltaUV1 = gTextureCoordinate[1]-gTextureCoordinate[0];
	vec2 deltaUV2 = gTextureCoordinate[2]-gTextureCoordinate[0];
	vec3 tangent1,bitangent1;
	float f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);

	tangent1.x = f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
	tangent1.y = f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
	tangent1.z = f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);
	tangent1 = normalize(tangent1);

	bitangent1.x = f * (-deltaUV2.x * edge1.x + deltaUV1.x * edge2.x);
	bitangent1.y = f * (-deltaUV2.x * edge1.y + deltaUV1.x * edge2.y);
	bitangent1.z = f * (-deltaUV2.x * edge1.z + deltaUV1.x * edge2.z);	
	bitangent1 = normalize(bitangent1);  
	vec3 T = normalize(vec3(ModelMatrix * vec4(tangent1,   0.0)));
    vec3 B = normalize(vec3(ModelMatrix * vec4(bitangent1, 0.0)));
	for(int i=0;i<3;i++)
	{
		vec3 N = normalize(vec3(ModelMatrix * vec4(gNormal[i],    0.0)));
		pTextureCoordinate=gTextureCoordinate[i];
		aNormal=gNormal[i];
		PixelPos=gPixelPos[i];
		TBN = mat3(T, B, N);
		gl_Position=gl_in[i].gl_Position;
		EmitVertex();
	}
	EndPrimitive();
}