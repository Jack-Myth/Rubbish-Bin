float2 VertexPosition[]={(0,-0.5),(0.5,0.5),(-0.5,0.5)};

float4 main(uint VxID:SV_VertexID ) : SV_POSITION
{
	return float4(VertexPosition[VxID],0,1);
}