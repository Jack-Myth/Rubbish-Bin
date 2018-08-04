#version 330 core
layout(points) in;
in VS_OUT
{
	vec3 aVerticeColor;
} gs_in[];	

layout(triangle_strip,max_vertices=5) out;
out vec3 gs_outColor;

void buildHouse(vec4 position)
{
	gs_outColor=gs_in[0].aVerticeColor;
	gl_Position=position+vec4(-0.2, -0.2, 0.0, 0.0);    // 1:左下
	EmitVertex();
	gl_Position=position+vec4(0.2,-0.2,0,0);//2:右下
	EmitVertex();
	gl_Position=position+vec4(-0.2,0.2,0,0);//左上
	EmitVertex();
	gl_Position=position+vec4(0.2,0.2,0,0);//右上
	EmitVertex();
	gl_Position=position+vec4(0,0.4,0,0);//顶部
	EmitVertex();
	EndPrimitive();
}

void main()
{
	buildHouse(gl_in[0].gl_Position);
}