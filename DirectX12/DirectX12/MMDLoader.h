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
	DirectX::XMFLOAT3 pos;			// xyz���W
	DirectX::XMFLOAT3 normalVec;	// nx,ny,nz�@��
	DirectX::XMFLOAT2 uv;			// ���_uv���W
	unsigned short boneNum[2];			// �{�[���ԍ�1, 2(���_���e�����󂯂�{�[��)
	unsigned char boneWeight;			// �{�[��1�ɗ^����e���x(0~100,�{�[��2�̉e���x��100-bone_weight)
	unsigned char edgeFlag;			// 0:�ʏ�,1:�G�b�W����
} PmdVertex;

typedef struct MATERIAL
{
	DirectX::XMFLOAT3 diffuseColor;		// �����F
	float alpha;						// �����E�̕s�����x
	float specularity;					
	DirectX::XMFLOAT3 specularColor;	// ����F
	DirectX::XMFLOAT3 mirrorColor;		// ���F
	unsigned char toonIndex;
	unsigned char edgeFlag;
	unsigned int faceVertexCount;		// �}�e���A�����g�p����ʒ��_���X�g�̃f�[�^��
	/*�ގ�0���珇�ɁAfaceVertexCount�����ʒ��_���X�g�̃f�[�^���g�p����*/
	char textureFileName[20];			// �g�p����e�N�X�`���������̓X�t�B�A�t�@�C���̖��O
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

	const std::vector<PmdVertex>& GetVertexData();		// ���_�f�[�^�ւ̎Q�Ƃ�Ԃ�
	const std::vector<unsigned short>& GetIndexData();	// ���_�C���f�b�N�X�f�[�^�ւ̎Q�Ƃ�Ԃ�
	const std::vector<Material>& GetMaterialData();		// �}�e���A���f�[�^�ւ̎Q�Ƃ�Ԃ�
private:
	MMDLoader();
	PMD_HEADER pmdHeader;
	char pmdSignature[3];
	unsigned int vertexCount;			// ���_��
	unsigned int faceVertexCount;		// �ʒ��_�̐�
	unsigned int materialCount;			// �}�e���A����
	std::vector<PmdVertex> vertex;		// ���_�f�[�^
	std::vector<unsigned short> faceVertexIndex;	// �ʒ��_�C���f�b�N�X�f�[�^
	std::vector<Material> materials;
};

