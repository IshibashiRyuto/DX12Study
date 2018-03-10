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
	DirectX::XMFLOAT3 pos;			// xyz���W
	DirectX::XMFLOAT3 normal_vec;	// nx,ny,nz�@��
	DirectX::XMFLOAT2 uv;			// ���_uv���W
	unsigned short bone_num[2];			// �{�[���ԍ�1, 2(���_���e�����󂯂�{�[��)
	unsigned char bone_weight;			// �{�[��1�ɗ^����e���x(0~100,�{�[��2�̉e���x��100-bone_weight)
	unsigned char edge_flag;			// 0:�ʏ�,1:�G�b�W����
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

