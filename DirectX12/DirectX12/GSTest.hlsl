
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

	int count = 0;
	int outRangeIndex[3] = { -1,-1,-1 };
	float cutHeight = 15.0f;
	float2 cutRectMin = { 0.0f,0.0f }; // 後に定数バッファから引っ張る予定
	float2 cutRectMax = { 1.0f,1.0f }; // のちに定数バッファから引っ張る予定

    for (int i = 0; i < 3; ++i)
    {
        if (In[i].pos.y < cutHeight)
        {
			outRangeIndex[count] = i;
			count++;
        }
    }
    
	if (count == 0)
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
	else if (count == 1)
	{
		int cutVerIdx = outRangeIndex[0];
		int oldVerIdx1 = (cutVerIdx + 1) % 3;
		int oldVerIdx2 = (cutVerIdx + 2) % 3;
		float4 cutPos = In[cutVerIdx].pos;
		float4 pos1 = In[oldVerIdx1].pos;
		float4 pos2 = In[oldVerIdx2].pos;

		float4 vec1 = cutPos - pos1;
		float4 vec2 = cutPos - pos2;

		float t1 = abs(pos1.y - cutHeight) / abs(vec1.y);
		float t2 = abs(pos2.y - cutHeight) / abs(vec2.y);

		GS_Out oldPoint1, oldPoint2;
		GS_Out newPoint1, newPoint2;

		oldPoint1.pos = mul(mul(viewproj, world), pos1);
		oldPoint1.normal = In[oldVerIdx1].normal;
		oldPoint1.uv = In[oldVerIdx1].uv;
		oldPoint1.color = float4(1.0f, 1.0f, 1.0f, 1.0f);

		oldPoint2.pos = mul(mul(viewproj, world), pos2);
		oldPoint2.normal = In[oldVerIdx2].normal;
		oldPoint2.uv = In[oldVerIdx2].uv;
		oldPoint2.color = float4(1.0f, 1.0f, 1.0f, 1.0f);

		newPoint1.pos = mul(mul(viewproj, world), pos1 + t1 * vec1);
		newPoint1.normal = In[oldVerIdx1].normal;
		newPoint1.uv = lerp(In[oldVerIdx1].uv, In[cutVerIdx].uv, t1);
		newPoint1.color = float4(1.0f, 1.0f, 1.0f, 1.0f);


		newPoint2.pos = mul(mul(viewproj, world), pos2 + t2 * vec2);
		newPoint2.normal = In[oldVerIdx2].normal;
		newPoint2.uv = lerp(In[oldVerIdx2].uv, In[cutVerIdx].uv, t2);
		newPoint2.color = float4(1.0f, 1.0f, 1.0f, 1.0f);

		TriStream.Append(newPoint2);
		TriStream.Append(newPoint1);
		TriStream.Append(oldPoint1);
		TriStream.RestartStrip();
		TriStream.Append(newPoint2);
		TriStream.Append(oldPoint1);
		TriStream.Append(oldPoint2);
		TriStream.RestartStrip();
	}
	else if (count == 2)
	{
		int oldVerIdx = -1;
		for (int i = 0; i < 3; ++i)
		{
			if (i != outRangeIndex[0] && i != outRangeIndex[1])
			{
				oldVerIdx = i;
				break;
			}
		}
		int cutVerIdx1 = outRangeIndex[0];
		int cutVerIdx2 = outRangeIndex[1];

		float4 pos1 = In[oldVerIdx].pos;
		float4 cutPos1 = In[cutVerIdx1].pos;
		float4 cutPos2 = In[cutVerIdx2].pos;

		float4 vec1 = cutPos1 - pos1;
		float4 vec2 = cutPos2 - pos1;

		float t1 = abs(pos1.y - cutHeight) / abs(vec1.y);
		float t2 = abs(pos1.y - cutHeight) / abs(vec2.y);

		GS_Out oldPoint;
		GS_Out newPoint1, newPoint2;

		oldPoint.pos = mul(mul(viewproj, world), pos1);
		oldPoint.normal = In[oldVerIdx].normal;
		oldPoint.uv = In[oldVerIdx].uv;
		oldPoint.color = float4(1.0f, 1.0f, 1.0f, 1.0f);

		newPoint1.pos = mul(mul(viewproj, world), pos1 + t1 * vec1);
		newPoint1.normal = In[oldVerIdx].normal * t1 + In[cutVerIdx1].normal * (1.0f - t1) ;
		newPoint1.uv = lerp(In[oldVerIdx].uv, In[cutVerIdx1].normal, t1);
		newPoint1.color = float4(1.0f, 1.0f, 1.0f, 1.0f);


		newPoint2.pos = mul(mul(viewproj, world), pos1 + t2 * vec2);
		newPoint2.normal = In[oldVerIdx].normal * t2 + In[cutVerIdx2].normal * (1.0f -t2);
		newPoint2.uv = lerp(In[oldVerIdx].uv, In[cutVerIdx2].uv, t2);
		newPoint2.color = float4(1.0f, 1.0f, 1.0f, 1.0f);

		TriStream.Append(newPoint2);
		TriStream.Append(newPoint1);
		TriStream.Append(oldPoint);
		TriStream.RestartStrip();
	}
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
