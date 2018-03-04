
float4 BaseVS( float3 pos: POSITION) : SV_POSITION
{
	return float4(pos,1.0f);
}

float4 BasePS(float4 pos: SV_POSITION ) : SV_Target
{
	return float4(1,1,1,1);
}
