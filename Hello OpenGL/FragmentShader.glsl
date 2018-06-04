#version 330 core
out vec4 FragColor;
uniform float DynamicColor;
in vec4 pVertexColor;
void main()
{
    float mDynamicColor=DynamicColor*2;
    //FragColor = vec4(1.0f, 0.5f, 0.2f, 1.0f);
    float gR=sin(mDynamicColor+1.5)/2.5+0.5;
    float gG=sin(mDynamicColor+0.1)/2.5+0.5;
    float gB=sin(mDynamicColor+2.4)/2.5+0.5;
    FragColor = pVertexColor;
    //FragColor = vec4(gR,gG,gB,1.0f);
} 