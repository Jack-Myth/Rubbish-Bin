#version 330 core

in vec3 aPos;
in vec2 aTextureCoord;

uniform sampler2D Tex0;
float offset=0.002;
float Kernel[9]=float[](1,2,1,
					 2,4,2,
					 1,  2,1);

out vec4 fragColor;

void main()
{
	vec4 tmpfragColor=vec4(0.f);
	for(float x=-1;x<=1;x++)
		for(float y=-1;y<=1;y++)
		{
			vec2 xy=vec2(x*offset,y*offset);
			int i=int((x+1)*3+(y+1));
			tmpfragColor+=(texture(Tex0,aTextureCoord+xy)*Kernel[i]);
		}
	fragColor=tmpfragColor/16;
}