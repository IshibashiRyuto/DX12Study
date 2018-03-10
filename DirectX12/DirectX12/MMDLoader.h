#pragma once
#include<string>
#include<vector>
#include<DirectXMath.h>

#define PMD_SIGNATURE "Pmd"
#define PMD_VERTEX_SIZE 38

typedef struct
{
	float version;
	char model_name[20];
	char comment[256];
	unsigned int vertexNum;
}PMD_HEADER;

#pragma pack(1)

typedef struct
{
	DirectX::XMFLOAT3 pos;			// xyz座標
	DirectX::XMFLOAT3 normal_vec;	// nx,ny,nz法線
	DirectX::XMFLOAT2 uv;			// 頂点uv座標
	unsigned short bone_num[2];			// ボーン番号1, 2(頂点が影響を受けるボーン)
	unsigned char bone_weight;			// ボーン1に与える影響度(0~100,ボーン2の影響度は100-bone_weight)
	unsigned char edge_flag;			// 0:通常,1:エッジ無効
}PMD_VERTEX;

#pragma pack()

class MMDLoader
{
public:
	~MMDLoader();

	static MMDLoader* Instance()
	{
		static MMDLoader inst;
		return &inst;
	}

	bool Load();

	const std::vector<PMD_VERTEX>& GetVertexData();

private:
	MMDLoader();
	PMD_HEADER pmdHeader;
	char pmdSignature[3];
	std::vector<PMD_VERTEX> vertex;
};

