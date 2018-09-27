cbuffer Matrices
{
	float4x4 ModelMatrix;
	float4x4 ViewMatrix;
	float4x4 ProjectionMatrix;
};

float4 VS(float3 inPos : POSITION, float3 inNormal : NORMAL, float2 inUV : UV) : SV_POSITION
{
	return mul(mul(mul(float4(inPos,1.f),ModelMatrix),ViewMatrix),ProjectionMatrix);
	//return mul(float4(inPos,1.f),ModelMatrix);
}

float4 PS(float4 WorldPosition : SV_POSITION) : SV_TARGET
{
	return float4(0,0,1,1);
}