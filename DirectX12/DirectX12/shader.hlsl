struct Out
{
    float4 position : SV_POSITION; // �V�X�e�����W
    float4 pos : POSITION; // ���W
    float2 uv : TEXCORD; // UV���W
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
