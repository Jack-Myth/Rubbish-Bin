float2 VertexPosition[] = { (0, -0.5), (0.5, 0.5), (-0.5, 0.5) };
float3 Color[] = { (1.0, 0.0, 0.0), (0.0, 1.0, 0.0), (0.0, 0.0, 1.0) };

float4 main(uint VxID:SV_VertexID) : SV_TARGET
{
    return float4(Color[VxID], 1);

}