#version 330 core
out vec4 FragColor;
uniform float DynamicColor;
uniform sampler2D MyTexture;
in vec4 pVertexColor;
in vec2 pTextureCoord;
void main()
{
    float mDynamicColor=DynamicColor*2;
    //FragColor = vec4(1.0f, 0.5f, 0.2f, 1.0f);
    float gR=sin(mDynamicColor+1.5)/2.5+0.5;
    float gG=sin(mDynamicColor+0.1)/2.5+0.5;
    float gB=sin(mDynamicColor+2.4)/2.5+0.5;
    //FragColor = pVertexColor;
    //FragColor=vec4(pTextureCoord.xy,1,1);
    if(pTextureCoord==vec2(0,0))
        FragColor = pVertexColor;
    else
        //FragColor = vec4(pTextureCoord,1,1);
        FragColor = vec4(texture(MyTexture,pTextureCoord).xyz,pVertexColor.z);
    //FragColor = vec4(gR,gG,gB,1.0f);
} 