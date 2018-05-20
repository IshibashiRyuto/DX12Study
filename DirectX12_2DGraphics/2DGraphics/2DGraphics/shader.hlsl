//Texture2D<float4> tex : register(t0);
//SamplerState smp : register(s0);

struct VS_IN
{
    float3 pos : POSITION;   
    float3 normal : NORMAL;
    float3 offset : INSTANCE_OFFSET;
};

struct Out
{
    float4 position : SV_POSITION; // システム座標
    float3 normal : NORMAL;
    float3 pos : POSITION; // 座標
    float2 uv : TEXCORD; // UV座標
};

Out VSMain(VS_IN input) 
{
    Out result;
    result.pos = input.pos + input.offset;
    result.position = float4(result.pos, 1.0f);
    result.normal = input.normal;
    return result;
}

float4 PSMain(Out o ) : SV_Target
{
    float3 light = normalize(float3(0, 0, -1));// 光源ベクトル
    float brightness = dot(o.normal, light);       // 法線と光源ベクトルの内積をとる(= 光源ベクトルと法線の角度を調べる)
    float3 diffuseColor = float3(brightness, brightness, brightness);
    return float4(diffuseColor, 1.0f);
}
