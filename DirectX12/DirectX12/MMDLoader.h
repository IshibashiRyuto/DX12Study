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
}PMD_HEADER;

#pragma pack(1)

typedef struct PMD_VERTEX
{
	DirectX::XMFLOAT3 pos;			// xyz座標
	DirectX::XMFLOAT3 normalVec;	// nx,ny,nz法線
	DirectX::XMFLOAT2 uv;			// 頂点uv座標
	unsigned short boneNum[2];			// ボーン番号1, 2(頂点が影響を受けるボーン)
	unsigned char boneWeight;			// ボーン1に与える影響度(0~100,ボーン2の影響度は100-bone_weight)
	unsigned char edgeFlag;			// 0:通常,1:エッジ無効
} PmdVertex;

typedef struct MATERIAL
{
	DirectX::XMFLOAT3 diffuseColor;		// 減衰色
	float alpha;						// 減衰職の不透明度
	float specularity;					
	DirectX::XMFLOAT3 specularColor;	// 光沢色
	DirectX::XMFLOAT3 mirrorColor;		// 環境色
	unsigned char toonIndex;
	unsigned char edgeFlag;
	unsigned int faceVertexCount;		// マテリアルが使用する面頂点リストのデータ数
	/*材質0から順に、faceVertexCount分ずつ面頂点リストのデータを使用する*/
	char textureFileName[20];			// 使用するテクスチャもしくはスフィアファイルの名前
} Material;

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

	const std::vector<PmdVertex>& GetVertexData();		// 頂点データへの参照を返す
	const std::vector<unsigned short>& GetIndexData();	// 頂点インデックスデータへの参照を返す
	const std::vector<Material>& GetMaterialData();		// マテリアルデータへの参照を返す
private:
	MMDLoader();
	PMD_HEADER pmdHeader;
	char pmdSignature[3];
	unsigned int vertexCount;			// 頂点数
	unsigned int faceVertexCount;		// 面頂点の数
	unsigned int materialCount;			// マテリアル数
	std::vector<PmdVertex> vertex;		// 頂点データ
	std::vector<unsigned short> faceVertexIndex;	// 面頂点インデックスデータ
	std::vector<Material> materials;
};

