struct Out
{
    float4 position : SV_POSITION; // システム座標
    float4 pos : POSITION; // 座標
    float2 uv : TEXCORD; // UV座標
};

Out VSMain(float4 position : POSITION, float2 uv:TEXCORD)
{
    Out result;

    result.position = position;
    result.position.xy = float2(-1, 1) + position.xy / float2(640 / 2, -480 / 2);

    return result;
}

float4 PSMain(Out input) : SV_Target
{
    return float4(input.uv, 1, 1);

}
