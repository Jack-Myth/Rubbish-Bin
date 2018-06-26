#version 330 core

in vec3 aPos;
in vec2 aTextureCoord;

uniform sampler2D Tex0;
float offset=0.001;
float KernelX[9]=float[](-1,0,1,
						 -2,0,2,
						 -1,0,1);
float KernelY[9]=float[](1,2,1,
						 0,0,0,
						-1,-2,-1);

out vec4 fragColor;

void main()
{
	vec4 tmpfragColorX=vec4(0.f);
	vec4 tmpfragColorY=vec4(0.f);
	for(float x=-1;x<=1;x++)
		for(float y=-1;y<=1;y++)
		{
			vec2 xy=vec2(x*offset,y*offset);
			int i=int((x+1)*3+(y+1));
			tmpfragColorX+=(texture(Tex0,aTextureCoord+xy)*KernelX[i]);
			tmpfragColorY+=(texture(Tex0,aTextureCoord+xy)*KernelY[i]);
		}
	fragColor=vec4(sqrt(tmpfragColorX*tmpfragColorX+tmpfragColorY*tmpfragColorY).xyz,1);
}