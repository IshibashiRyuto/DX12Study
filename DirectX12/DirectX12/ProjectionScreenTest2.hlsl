
cbuffer mat : register(b0)
{
    float4x4 world; //World 行列
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
    float4 svpos : SV_POSITION;
    float4 pos : POSITION;
    float4 normal : NORMAL;
    float2 uv : TEXCORD;
};

VS_Out VSMain(float4 pos : POSITION, float3 normal : NORMAL)
{
    VS_Out result;
    result.pos = pos;
    result.normal = normal;
    result.uv = float2(0.0f, 0.0f);
    return result;
}


bool IsInRect(float2 pos, float2 rectMin, float2 rectMax)
{
    return (pos.x > rectMin.x) && (pos.x < rectMax.x) && (pos.y > rectMin.y) && (pos.y < rectMax.y);
}

bool IsCross(float pos,float vec, float targetPos)
{
    if (((targetPos <= pos) && (targetPos >= (pos + vec)))
        || ((targetPos >= pos) && (targetPos <= (pos + vec))))
    {
        return true;
    }
    return false;
}

/// ジオメトリシェーダ
[maxvertexcount(9)]
void ProjectionScreenGS(triangle GS_In input[3], inout TriangleStream<GS_Out> triStream)
{
    bool isOut = false;
    float2 cutRectPos1 = { 5.5f, 1.5f };
    float2 cutRectPos2 = { -3.5f, -1.2f };

    // loop変数
    int i;
    
    float2 cutRectMin = float2(min(cutRectPos1.x, cutRectPos2.x), min(cutRectPos1.y, cutRectPos2.y));
    
    float2 cutRectMax = float2(max(cutRectPos1.x, cutRectPos2.x), max(cutRectPos1.y, cutRectPos2.y));

    for (i = 0; i < 3; ++i)
    {
        if (!IsInRect(input[i].pos.xz, cutRectMin, cutRectMax))
        {
            isOut = true;
            break;
        }
    }

    if(isOut)
    {
        GS_In newVertex[5];
        int newVertexNum = 0;
        GS_In oldVertex[5];
        int oldVertexNum = 0;
        
        // 初期処理
        for (i = 0; i < 3; ++i)
        {
            newVertex[i] = input[i];
            ++newVertexNum;
        }

        // x下限判定
        {
            for (i = 0; i < newVertexNum; ++i)
            {
                oldVertex[i] = newVertex[i];
            }
            oldVertexNum = newVertexNum;
            newVertexNum = 0;

            for (i = 0; i < oldVertexNum; ++i)
            {
                // 自身の追加判定
                if(oldVertex[i].pos.x >= cutRectMin.x )
                {
                    newVertex[newVertexNum] = oldVertex[i];
                    newVertexNum++;
                }

                // 自身から次の頂点へのベクトル中に交点が存在するなら
                float vecX = oldVertex[(i + 1) % oldVertexNum].pos.x - oldVertex[i].pos.x;
                if (IsCross(oldVertex[i].pos.x,vecX,cutRectMin.x))
                {
                    float t = abs(cutRectMin.x - oldVertex[i].pos.x) / abs(vecX);
                    newVertex[newVertexNum].pos = oldVertex[i].pos * ( 1 - t) + oldVertex[(i + 1) % oldVertexNum].pos * t;
                    newVertex[newVertexNum].uv = oldVertex[i].uv * (1 - t) + oldVertex[(i + 1) % oldVertexNum].uv * t;
                    newVertex[newVertexNum].normal = normalize(oldVertex[i].normal * (1 - t) + oldVertex[(i + 1) % oldVertexNum].normal * t);
                    newVertexNum++;
                }
            }
        }
        // y下限判定
        {
            for (i = 0; i < newVertexNum; ++i)
            {
                oldVertex[i] = newVertex[i];
            }
            oldVertexNum = newVertexNum;
            newVertexNum = 0;

            for (i = 0; i < oldVertexNum; ++i)
            {
                // 自身の追加判定
                if (oldVertex[i].pos.z >= cutRectMin.y)
                {
                    newVertex[newVertexNum] = oldVertex[i];
                    newVertexNum++;
                }

                // 自身から次の頂点へのベクトル中に交点が存在するなら
                float vecX = oldVertex[(i + 1) % oldVertexNum].pos.z - oldVertex[i].pos.z;
                if (IsCross(oldVertex[i].pos.z, vecX, cutRectMin.y))
                {
                    float t = abs(cutRectMin.y - oldVertex[i].pos.z) / abs(vecX);
                    newVertex[newVertexNum].pos = oldVertex[i].pos * (1 - t) + oldVertex[(i + 1) % oldVertexNum].pos * t;
                    newVertex[newVertexNum].uv = oldVertex[i].uv * (1 - t) + oldVertex[(i + 1) % oldVertexNum].uv * t;
                    newVertex[newVertexNum].normal = normalize(oldVertex[i].normal * (1 - t) + oldVertex[(i + 1) % oldVertexNum].normal * t);
                    newVertexNum++;
                }
            }
        }
        
        // x上限判定
        {
            for (i = 0; i < newVertexNum; ++i)
            {
                oldVertex[i] = newVertex[i];
            }
            oldVertexNum = newVertexNum;
            newVertexNum = 0;

            for (i = 0; i < oldVertexNum; ++i)
            {
                // 自身の追加判定
                if (oldVertex[i].pos.x <= cutRectMax.x)
                {
                    newVertex[newVertexNum] = oldVertex[i];
                    newVertexNum++;
                }

                // 自身から次の頂点へのベクトル中に交点が存在するなら
                float vecX = oldVertex[(i + 1) % oldVertexNum].pos.x - oldVertex[i].pos.x;
                if (IsCross(oldVertex[i].pos.x, vecX, cutRectMax.x))
                {
                    float t = abs(cutRectMax.x - oldVertex[i].pos.x) / abs(vecX);
                    newVertex[newVertexNum].pos = oldVertex[i].pos * (1 - t) + oldVertex[(i + 1) % oldVertexNum].pos * t;
                    newVertex[newVertexNum].uv = oldVertex[i].uv * (1 - t) + oldVertex[(i + 1) % oldVertexNum].uv * t;
                    newVertex[newVertexNum].normal = normalize(oldVertex[i].normal * (1 - t) + oldVertex[(i + 1) % oldVertexNum].normal * t);
                    newVertexNum++;
                }
            }
        }
        // y下限判定
        {
            for (i = 0; i < newVertexNum; ++i)
            {
                oldVertex[i] = newVertex[i];
            }
            oldVertexNum = newVertexNum;
            newVertexNum = 0;

            for (i = 0; i < oldVertexNum; ++i)
            {
                // 自身の追加判定
                if (oldVertex[i].pos.z <= cutRectMax.y)
                {
                    newVertex[newVertexNum] = oldVertex[i];
                    newVertexNum++;
                }

                // 自身から次の頂点へのベクトル中に交点が存在するなら
                float vecX = oldVertex[(i + 1) % oldVertexNum].pos.z - oldVertex[i].pos.z;
                if (IsCross(oldVertex[i].pos.z, vecX, cutRectMax.y))
                {
                    float t = abs(cutRectMax.y - oldVertex[i].pos.z) / abs(vecX);
                    newVertex[newVertexNum].pos = oldVertex[i].pos * (1 - t) + oldVertex[(i + 1) % oldVertexNum].pos * t;
                    newVertex[newVertexNum].uv = oldVertex[i].uv * (1 - t) + oldVertex[(i + 1) % oldVertexNum].uv * t;
                    newVertex[newVertexNum].normal = normalize(oldVertex[i].normal * (1 - t) + oldVertex[(i + 1) % oldVertexNum].normal * t);
                    newVertexNum++;
                }
            }
        }

        // 出力処理
        GS_Out tri[3];
        tri[0].svpos = mul(viewproj, mul(world, newVertex[0].pos));
        tri[0].pos = tri[0].svpos;
        tri[0].uv = newVertex[0].uv;
        tri[0].normal = float4(newVertex[0].normal, 1.0f);
        
        for (i = 1; i < newVertexNum - 1; ++i)
        {
            tri[1].svpos = mul(viewproj, mul(world, newVertex[i].pos));
            tri[1].pos = tri[1].svpos;
            tri[1].uv = newVertex[i].uv;
            tri[1].normal = float4(newVertex[i].normal, 1.0f);

            tri[2].svpos = mul(viewproj, mul(world, newVertex[i + 1].pos));
            tri[2].pos = tri[2].svpos;
            tri[2].uv = newVertex[i + 1].uv;
            tri[2].normal = float4(newVertex[i + 1].normal, 1.0f);
            
            for (int j = 0; j < 3; j++)
            {
                triStream.Append(tri[j]);
            }
            triStream.RestartStrip();
        }

    }
    else
    {
        for (i = 0; i < 3; ++i)
        {
            GS_Out output;
            output.svpos = mul(viewproj, (mul(world, input[i].pos)));
            output.pos = output.svpos;
            output.uv = input[i].uv;
            output.normal = float4(input[i].normal, 1.0f);
            triStream.Append(output);
        }
        triStream.RestartStrip();
    }
    
}


typedef GS_Out PS_In;

float4 PSMain(PS_In o) : SV_Target
{
    float3 light = normalize(float3(-1, 1, -1)); // 光源ベクトル
    float brightness = dot(o.normal.xyz, light); // 法線と光源ベクトルの内積をとる(= 光源ベクトルと法線の角度を調べる)
    float3 diffuseColor = diffuse * brightness;
	// diffuseColor = float3(brightness, brightness, brightness);
    return float4(diffuseColor, 1.0f); // * o.color;
}
