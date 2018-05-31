//Texture2D<float4> tex : register(t0);
//SamplerState smp : register(s0);

cbuffer mat : register(b0)
{
	float4x4 world;   //World �s��
	float4x4 viewproj; //View Projection�s��
}

cbuffer mat : register(b1)
{
	float3 diffuse; // ��{�F
}
/*
struct Out
{
	float4 position : SV_POSITION; // �V�X�e�����W
	float3 normal : NORMAL;
	float4 pos : POSITION; // ���W
	float2 uv : TEXCORD; // UV���W
};
*/

struct VS_Out
{
float4 position : POSITION; // �V�X�e�����W
float3 normal : NORMAL;
float4 pos : POSITION; // ���W
float2 uv : TEXCORD; // UV���W
};

typedef VS_Out GS_In;

struct GS_Out
{
float4 pos  : SV_POSITION;
float3 normal : NORMAL;
float2 uv : TEXCORD;
float4 color : COLOR;
};

VS_Out VSMain(float4 pos : POSITION, float3 normal : NORMAL)
{
	VS_Out result;
	result.position = mul(mul(viewproj, world), pos);
	result.pos = pos;
	result.normal = mul(world, float4(normal, 0));
	return result;
}

/// �W�I���g���V�F�[�_
[maxvertexcount(6)]
void GSMain(triangle GS_In In[3],
inout TriangleStream<GS_Out> TriStream)
{
	GS_Out Out;
	
	int i;

	for (i = 0; i < 3; i++)
	{
		Out.pos = mul(mul(viewproj, world), In[i].pos);
		Out.normal = In[i].normal;
		Out.uv = In[i].uv;
		Out.color = float4(1.0f, 1.0f, 1.0f, 1.0f);

		TriStream.Append(Out);
	}

	TriStream.RestartStrip();

	// ���_���쐬����
	for (i = 0; i < 3; i++)
	{
		Out.pos = mul(mul(viewproj, world), (In[i].pos + In[i].normal*2.0f) );
		Out.normal = In[i].normal;
		Out.uv = In[i].uv;
		Out.color = float4(1.0f, 1.0f, 1.0f, 0.5f);

		TriStream.Append(Out);
	}

	TriStream.RestartStrip();
}


typedef GS_Out PS_In;

float4 PSMain(PS_In o) : SV_Target
{
	float3 light = normalize(float3(-1, 1, -1));// �����x�N�g��
	float brightness = dot(o.normal, light);       // �@���ƌ����x�N�g���̓��ς��Ƃ�(= �����x�N�g���Ɩ@���̊p�x�𒲂ׂ�)
	float3 diffuseColor = diffuse * brightness;
	// diffuseColor = float3(brightness, brightness, brightness);
	return float4(diffuseColor, 1) * o.color;
}
