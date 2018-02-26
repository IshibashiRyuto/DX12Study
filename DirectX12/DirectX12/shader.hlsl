
float4 BaseVS( float4 pos: POSITION) : SV_POSITION
{
	return pos;
}

float4 BasePS(float4 pos: POSITION ) : SV_Target
{
	return float4(1,1,1,1);
}
