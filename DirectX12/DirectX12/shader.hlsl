//Texture2D<float4> tex : register(t0);
//SamplerState smp : register(s0);

cbuffer mat : register(b0)
{
    float4x4 world;   //World �s��
    float4x4 viewproj; //View Projection�s��
    float3 diffuse;         // ��{�F
}

struct Out
{
    float4 position : SV_POSITION; // �V�X�e�����W
    float3 normal : NORMAL;
    float4 pos : POSITION; // ���W
    float2 uv : TEXCORD; // UV���W
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
    float3 light = normalize(float3(-1, 1, -1));// �����x�N�g��
    float brightness = dot(o.normal, light);       // �@���ƌ����x�N�g���̓��ς��Ƃ�(= �����x�N�g���Ɩ@���̊p�x�𒲂ׂ�)
    float3 diffuseColor = diffuse * brightness;
    diffuseColor = float3(brightness, brightness, brightness);
    return float4(diffuseColor, 1);

}
