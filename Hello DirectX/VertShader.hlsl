cbuffer Matrices
{
	float4x4 ModelMatrix;
	float4x4 ViewMatrix;
	float4x4 ProjectionMatrix;
};


float4 VS(float3 inPos : POSITION) : SV_POSITION
{
	return float4(inPos,1.f);
}