//Texture2D<float4> tex : register(t0);
//SamplerState smp : register(s0);

cbuffer mat : register(b0)
{
    float4x4 world;   //World 行列
    float4x4 viewproj; //View Projection行列
    float3 diffuse;         // 基本色
}

struct Out
{
    float4 position : SV_POSITION; // システム座標
    float3 normal : NORMAL;
    float4 pos : POSITION; // 座標
    float2 uv : TEXCORD; // UV座標
};

Out VSMain(float4 pos : POSITION ,float3 normal : NORMAL) 
{
    Out result;
    result.position = mul(mul(viewproj, world), pos);
    result.pos = pos;
    result.normal = mul(world, float4(normal, 0));
    return result;
}

float4 PSMain(Out o ) : SV_Target
{
    float3 light = normalize(float3(-1, 1, -1));// 光源ベクトル
    float brightness = dot(o.normal, light);       // 法線と光源ベクトルの内積をとる(= 光源ベクトルと法線の角度を調べる)
    float3 diffuseColor = diffuse * brightness;
    diffuseColor = float3(brightness, brightness, brightness);
    return float4(diffuseColor, 1);

}
