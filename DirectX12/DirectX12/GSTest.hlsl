
cbuffer mat : register(b0)
{
	float4x4 world;   //World 行列
	float4x4 viewproj; //View Projection行列
}

cbuffer mat : register(b1)
{
	float3 diffuse; // 基本色
}

struct VS_Out
{
float3 normal : NORMAL;
float4 pos : POSITION; // 座標
float2 uv : TEXCORD; // UV座標
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
	result.pos = pos;
	result.normal = normal;
    result.uv = float2(0.0f, 0.0f);
	return result;
}

/// ジオメトリシェーダ
[maxvertexcount(6)]
void GSMain(triangle GS_In In[3],
inout TriangleStream<GS_Out> TriStream)
{
    GS_Out Out;
    bool isUnder = true;

    for (int i = 0; i < 3; ++i)
    {
        if (In[i].pos.y < 15.0f)
        {
            isUnder = false;
            break;
        }
    }
    
    if (isUnder)
    {
        for (int i = 0; i < 3; ++i)
        {
            Out.pos = mul(mul(viewproj, world), In[i].pos);
            Out.normal = In[i].normal;
            Out.uv = In[i].uv;
            Out.color = float4(1.0f, 1.0f, 1.0f, 1.0f);
            TriStream.Append(Out);
        }
        TriStream.RestartStrip();

    }
    /*
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
    
	for (i = 0; i < 3; i++)
	{
        Out.pos = mul(mul(viewproj, world), (In[i].pos + float4(In[i].normal * 1.0f, 0.0f)));
		Out.normal = In[i].normal;
		Out.uv = In[i].uv;
		Out.color = float4(1.0f, 1.0f, 1.0f, 0.1f);

		TriStream.Append(Out);
	}

	TriStream.RestartStrip();
*/
}


typedef GS_Out PS_In;

float4 PSMain(PS_In o) : SV_Target
{
	float3 light = normalize(float3(-1, 1, -1));// 光源ベクトル
	float brightness = dot(o.normal, light);       // 法線と光源ベクトルの内積をとる(= 光源ベクトルと法線の角度を調べる)
	float3 diffuseColor = diffuse * brightness;
	// diffuseColor = float3(brightness, brightness, brightness);
    return float4(diffuseColor, 1.0f);// * o.color;
}
