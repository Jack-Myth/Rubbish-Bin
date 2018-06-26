#version 330 core

in vec3 aPos;
in vec2 aTextureCoord;

uniform sampler2D Tex0;
float offset=0.002;
float Kernel[9]=float[](2,2,2,
					 2,-15,2,
					 2,  2,2);

out vec4 fragColor;

void main()
{
	vec4 tmpfragColor=vec4(0.f);
	fragColor=vec4(vec3(1.0f)-texture(Tex0,aTextureCoord).xyz,1.f);
	/*for(float x=-1;x<=1;x++)
		for(float y=-1;y<=1;y++)
		{
			vec2 xy=vec2(x*offset,y*offset);
			int i=int((x+1)*3+(y+1));
			tmpfragColor+=(texture(Tex0,aTextureCoord+xy)*Kernel[i]);
		}*/
	fragColor=tmpfragColor;
}