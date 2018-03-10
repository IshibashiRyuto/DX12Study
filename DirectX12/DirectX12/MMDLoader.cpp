#include "MMDLoader.h"
#include <Windows.h>


MMDLoader::MMDLoader()
{
}


MMDLoader::~MMDLoader()
{
}

bool MMDLoader::Load()
{
	FILE *fp;
	fopen_s(&fp, "�����~�N.pmd", "rb");
	
	// �t�@�C���t�H�[�}�b�g�̊m�F
	fread(pmdSignature, sizeof(pmdSignature), 1, fp);

	if (fp == nullptr)
	{
		return false;
	}

	if (pmdSignature == nullptr || strcmp(pmdSignature, PMD_SIGNATURE) != 0)
	{
		MessageBox(nullptr, TEXT("This file is not pmd.\n Please load .pmd file."), TEXT("PMDLoad Failed"), MB_OK);
		fclose(fp);
		return false;
	}

	// �w�b�_�f�[�^�ǂݍ���
	fread(&pmdHeader, sizeof(pmdHeader), 1, fp);


	// ���_���̓ǂݍ���
	vertex.resize(pmdHeader.vertexNum);
	fread(&vertex[0], (size_t)PMD_VERTEX_SIZE, vertex.size(), fp);
	/*
	for (int i = 0; i < vertex.size(); i++)
	{
		vertex[i] = PMD_VERTEX{};
		fread(&vertex[i], (size_t)PMD_VERTEX_SIZE, 1, fp);
	}
	*/
	fclose(fp);
	return true;
}

const std::vector<PMD_VERTEX>& MMDLoader::GetVertexData()
{
	return vertex;
}
