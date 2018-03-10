//Texture2D<float4> tex : register(t0);
//SamplerState smp : register(s0);

cbuffer mat : register(b0)
{
    float4x4 wvp;   //World View Projection行列
}

struct Out
{
    float4 position : SV_POSITION; // システム座標
    float4 pos : POSITION; // 座標
    float2 uv : TEXCORD; // UV座標
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
