
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

bool IsInTriangle(float2 pos, float2 triPos1, float2 triPos2, float2 triPos3)
{
    float3 vec1 = cross(float3(pos - triPos1, 0.0f), float3(triPos2 - triPos1, 0.0f));
    float3 vec2 = cross(float3(pos - triPos2, 0.0f), float3(triPos3 - triPos2, 0.0f));
    float3 vec3 = cross(float3(pos - triPos3, 0.0f), float3(triPos1 - triPos3, 0.0f));

    float dot1 = dot(vec1, vec2);
    if (dot1 < 0)
    {
        return false;
    }

    float dot2 = dot(vec1, vec3);

    if (dot2 < 0)
    {
        return false;
    }

    return true;
}

bool IsCross(float2 pos1, float2 vec1, float2 pos2, float2 vec2)
{
    float3 crossVec1 = cross(float3(pos2 - pos1, 0.0f), float3(vec1, 0.0f));
    float3 crossVec2 = cross(float3((pos2 + vec2) - pos1, 0.0f), float3(vec1, 0.0f));

    if (dot(crossVec1, crossVec2) > 0.0f)
    {
        return false;
    }

    crossVec1 = cross(float3(pos1 - pos2, 0.0f), float3(vec2, 0.0f));
    crossVec2 = cross(float3((pos1 + vec1) - pos2, 0.0f), float3(vec2, 0.0f));

    if (dot(crossVec1, crossVec2) > 0.0f)
    {
        return false;
    }

    return true;
}

float Cross2D(float2 vec1, float2 vec2)
{
    return vec1.x * vec2.y - vec1.y * vec2.x;
}

float CalcCrossPoint(float2 calcPos, float2 calcVec, float2 targetPos, float2 targetVec)
{
    return (Cross2D((calcPos - targetPos), targetVec) / Cross2D(targetVec, calcVec));

}

float3 CalcBarycentricCoordinates(float3 pos1, float3 pos2, float3 pos3, float3 targetPos)
{
    float3 barycentric = float3(0.0f, 0.0f, 0.0f);

    float3 d1 = pos1 - pos3;
    float3 d2 = pos2 - pos3;
    float3 d = targetPos - pos3;
    float d1x = dot(d1, d1);
    float d1y = dot(d1, d2);
    float d2x = d1y;
    float d2y = dot(d2, d2);
    float dx = dot(d, d1);
    float dy = dot(d, d2);

    float detT = d1x * d2y - d1y * d2x;

    barycentric.x = (dx * d2y - dy * d2x) / detT;
    barycentric.y = (d1x * dy - d1y * dx) / detT;
    barycentric.z = 1 - barycentric.x - barycentric.y;

    return barycentric;
}
/// ジオメトリシェーダ

[maxvertexcount(9)]
void ProjectionScreenGS(triangle GS_In input[3], inout TriangleStream<GS_Out> triStream)
{
	
    uint inVerNum = 0;
    uint outVerNum = 0;
    uint inVerIdx[3] = { 0, 0, 0 };
    uint outVerIdx[3] = { 0, 0, 0 };

    float2 cutRectPos1 = { 1.5f,2.0f };
    float2 cutRectPos2 = { -1.5f, -2.0f };

    float2 cutRect[4] =
    {
        { min(cutRectPos1.x, cutRectPos2.x), min(cutRectPos1.y, cutRectPos2.y) },
        { max(cutRectPos1.x, cutRectPos2.x), min(cutRectPos1.y, cutRectPos2.y) },
        { max(cutRectPos1.x, cutRectPos2.x), max(cutRectPos1.y, cutRectPos2.y) },
        { min(cutRectPos1.x, cutRectPos2.x), max(cutRectPos1.y, cutRectPos2.y) }
    };


    for (int i = 0; i < 3; ++i)
    {
        if (IsInRect(input[i].pos.xz, cutRect[0], cutRect[2]))
        {
            inVerIdx[inVerNum] = i;
            ++inVerNum;
        }
        else
        {
            outVerIdx[outVerNum] = i;
            ++outVerNum;
        }
    }
    
   
    if (outVerNum == 0)
    {
        for (int i = 0; i < 3; ++i)
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
    else if (outVerNum == 1)
    {
        float2 triPos[3];
        uint i = 0, j = 0;
        GS_Out output[5];
        uint outputVerIdx = 0;
        uint gsOutputNum = 4;
        bool isInTri = false;
        uint inTriPointIdx = 0;
        
        // 平面に潰した三角形の頂点情報を保存
        for (i = 0; i < 3; ++i)
        {
            triPos[i] = input[i].pos.xz;
        }

        // 矩形頂点の内包確認
        for (i = 0; i < 4; ++i)
        {
            if (IsInTriangle(cutRect[i], triPos[0], triPos[1], triPos[2]))
            {
                gsOutputNum = 5;
                inTriPointIdx = i;
                isInTri = true;
                break;
            }
        }
        
        // 矩形内頂点情報を出力頂点情報に保存
        for (i = 0; i < inVerNum; ++i)
        {
            output[outputVerIdx].svpos = mul(viewproj, (mul(world,input[inVerIdx[i]].pos)));
            output[outputVerIdx].pos = output[outputVerIdx].svpos;
            output[outputVerIdx].uv = input[inVerIdx[i]].uv;
			
          
            output[outputVerIdx].normal = float4(input[i].normal, 1.0f);
            ++outputVerIdx;
        }

        // ポリゴン辺と矩形辺の交差判定
        {
            float3 outPos = input[outVerIdx[0]].pos.xyz;
            float3 vec[2] =
            {
                { input[inVerIdx[0]].pos.xyz - input[outVerIdx[0]].pos.xyz },
                { input[inVerIdx[1]].pos.xyz - input[outVerIdx[0]].pos.xyz },
            };

            for (i = 0; i < 4; ++i)
            {
                float2 targetPos = cutRect[i];
                float2 targetVec = cutRect[(i + 1) % 4] - targetPos;

                 //交差してたら交差点に出力頂点生成
                for (j = 0; j < inVerNum; j++)
                {
                    if (IsCross(outPos.xz, vec[j].xz, targetPos, targetVec))
                    {
                        float t = CalcCrossPoint(outPos.xz, vec[j].xz, targetPos, targetVec);

                        output[outputVerIdx].svpos = mul(viewproj, (mul(world, float4(outPos + vec[j] * t, 1.0f))));
                        output[outputVerIdx].pos = output[outputVerIdx].svpos;
                        output[outputVerIdx].uv = input[outVerIdx[0]].uv * t + input[inVerIdx[j]].uv * (1 - t);
			
                        output[outputVerIdx].normal = float4( normalize(input[outVerIdx[0]].normal * t + input[inVerIdx[j]].normal * (1 - t) ), 1.0f);
                        ++outputVerIdx;
                    }
                    
                    //if (isInTri)
                    //{
                    //    // 三角形内包頂点を出力頂点に追加
                    //    float2 inTriVec = cutRect[inTriPointIdx] - outPos.xz;
                    //    float t2 = (inTriVec.y - (inTriVec.x * vec[0].y) / vec[0].x) / (vec[1].y - (vec[1].x * vec[0].y) / vec[0].x);
                    //    float t1 = (inTriVec.x - t2 * vec[1].x) / vec[0].x;

                    //    float3 outputPosition = outPos + vec[0] * t1 + vec[1] * t2;
                    //    // 重心比計算
                    //    float3 barycentric = CalcBarycentricCoordinates(input[0].pos.xyz, input[1].pos.xyz, input[2].pos.xyz, outputPosition);
                        
                    //    output[outputVerIdx].svpos = mul(viewproj, (mul(world, float4(outputPosition, 1))));
                    //    output[outputVerIdx].pos = output[outputVerIdx].svpos;
                    //    output[outputVerIdx].uv = barycentric.x * input[0].uv + barycentric.y * input[1].uv + barycentric.z * input[2].uv;
			
                      
                    //    output[outputVerIdx].svpos = mul(viewproj, (mul(world, input[inVerIdx[i]].pos)));
                    //    ++outputVerIdx;
                    //    break;
                    //}
                }
            }
        }
        
        // 出力処理
        if (isInTri)
        {
            triStream.Append(output[0]);
            triStream.Append(output[1]);
            triStream.Append(output[2]);
            triStream.RestartStrip();
            
            triStream.Append(output[1]);
            triStream.Append(output[3]);
            triStream.Append(output[2]);
            triStream.RestartStrip();
            
            triStream.Append(output[2]);
            triStream.Append(output[3]);
            triStream.Append(output[4]);
            triStream.RestartStrip();
        }
        else
        {
                triStream.Append(output[0]);
                triStream.Append(output[1]);
                triStream.Append(output[2]);
                triStream.RestartStrip();
            
                triStream.Append(output[1]);
                triStream.Append(output[3]);
                triStream.Append(output[2]);
                triStream.RestartStrip();
        }
    }
    else if(outVerNum == 2)
    {
        float2 triPos[3];
        uint i = 0, j = 0;
        GS_Out output[5];
        uint outputVerIdx = 0;
        uint gsOutputNum = 3;
        bool isInTri = false;
        uint inTriPointIdx = 0;
        
        // 平面に潰した三角形の頂点情報を保存
        for (i = 0; i < 3; ++i)
        {
            triPos[i] = input[i].pos.xz;
        }

        // 矩形頂点の内包確認
        for (i = 0; i < 4; ++i)
        {
            if (IsInTriangle(cutRect[i], triPos[0], triPos[1], triPos[2]))
            {
                gsOutputNum = 4;
                inTriPointIdx = i;
                isInTri = true;
                break;
            }
        }
        
        // 矩形内頂点情報を出力頂点情報に保存
        for (i = 0; i < inVerNum; ++i)
        {
            output[outputVerIdx].svpos = mul(viewproj, (mul(world, input[inVerIdx[i]].pos)));
            output[outputVerIdx].pos = output[outputVerIdx].svpos;
            output[outputVerIdx].uv = input[inVerIdx[i]].uv;
			
          
            output[outputVerIdx].normal = float4(input[i].normal, 1.0f);
            ++outputVerIdx;
        }

        // ポリゴン辺と矩形辺の交差判定
        {
            float3 outPos = input[outVerIdx[0]].pos.xyz;
            float3 vec[2] =
            {
                { input[inVerIdx[0]].pos.xyz - input[outVerIdx[0]].pos.xyz },
                { input[inVerIdx[1]].pos.xyz - input[outVerIdx[0]].pos.xyz },
            };

            for (i = 0; i < 4; ++i)
            {
                float2 targetPos = cutRect[i];
                float2 targetVec = cutRect[(i + 1) % 4] - targetPos;

                 //交差してたら交差点に出力頂点生成
                for (j = 0; j < inVerNum; j++)
                {
                    if (IsCross(outPos.xz, vec[j].xz, targetPos, targetVec))
                    {
                        float t = CalcCrossPoint(outPos.xz, vec[j].xz, targetPos, targetVec);

                        output[outputVerIdx].svpos = mul(viewproj, (mul(world, float4(outPos + vec[j] * t, 1.0f))));
                        output[outputVerIdx].pos = output[outputVerIdx].svpos;
                        output[outputVerIdx].uv = input[outVerIdx[0]].uv * t + input[inVerIdx[j]].uv * (1 - t);
			
                        output[outputVerIdx].normal = float4(normalize(input[outVerIdx[0]].normal * t + input[inVerIdx[j]].normal * (1 - t)), 1.0f);
                        ++outputVerIdx;
                    }
                    
                    //if (isInTri)
                    //{
                    //    // 三角形内包頂点を出力頂点に追加
                    //    float2 inTriVec = cutRect[inTriPointIdx] - outPos.xz;
                    //    float t2 = (inTriVec.y - (inTriVec.x * vec[0].y) / vec[0].x) / (vec[1].y - (vec[1].x * vec[0].y) / vec[0].x);
                    //    float t1 = (inTriVec.x - t2 * vec[1].x) / vec[0].x;

                    //    float3 outputPosition = outPos + vec[0] * t1 + vec[1] * t2;
                    //    // 重心比計算
                    //    float3 barycentric = CalcBarycentricCoordinates(input[0].pos.xyz, input[1].pos.xyz, input[2].pos.xyz, outputPosition);
                        
                    //    output[outputVerIdx].svpos = mul(viewproj, (mul(world, float4(outputPosition, 1))));
                    //    output[outputVerIdx].pos = output[outputVerIdx].svpos;
                    //    output[outputVerIdx].uv = barycentric.x * input[0].uv + barycentric.y * input[1].uv + barycentric.z * input[2].uv;
			
                      
                    //    output[outputVerIdx].svpos = mul(viewproj, (mul(world, input[inVerIdx[i]].pos)));
                    //    ++outputVerIdx;
                    //    break;
                    //}
                }
            }
        }
        
        // 出力処理
        if (isInTri)
        {
            triStream.Append(output[0]);
            triStream.Append(output[1]);
            triStream.Append(output[2]);
            triStream.RestartStrip();
            
            triStream.Append(output[1]);
            triStream.Append(output[3]);
            triStream.Append(output[2]);
            triStream.RestartStrip();
            
            triStream.Append(output[2]);
            triStream.Append(output[3]);
            triStream.Append(output[4]);
            triStream.RestartStrip();
        }
        else
        {
            triStream.Append(output[0]);
            triStream.Append(output[1]);
            triStream.Append(output[2]);
            triStream.RestartStrip();
            
            triStream.Append(output[1]);
            triStream.Append(output[3]);
            triStream.Append(output[2]);
            triStream.RestartStrip();
        }
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
