//Texture2D<float4> tex : register(t0);
//SamplerState smp : register(s0);

cbuffer mat : register(b0)
{
    float4x4 wvp;   //World View Projection�s��
}

struct Out
{
    float4 position : SV_POSITION; // �V�X�e�����W
    float4 pos : POSITION; // ���W
    float2 uv : TEXCORD; // UV���W
};

Out VSMain(float4 pos : POSITION /*, float2 uv: TEXCORD*/) 
{
    Out result;
    pos.xy = float2(-1, 1) + pos.xy / float2(640 / 2, -480 / 2);
    result.position = mul(wvp,pos);
    //result.pos = mul(wvp, pos);
    //result.uv = uv;

    return result;
}

float4 PSMain(Out o ) : SV_Target
{
    //return tex.Sample(smp, o.uv).abgr;
    return float4(1, 1, 1, 1);

}
