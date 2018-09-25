
float4 VS(float3 inPos : POSITION) : SV_POSITION
{
	return float4(inPos,1.f);
}