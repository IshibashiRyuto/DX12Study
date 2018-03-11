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
	fread(&vertexCount, sizeof(int), 1, fp);
	vertex.resize(vertexCount);
	fread(&vertex[0], sizeof(PMD_VERTEX), vertex.size(), fp);
	
	// ���_�C���f�b�N�X���ǂݍ���
	fread(&faceVertexCount, sizeof(int), 1, fp);
	faceVertexIndex.resize(faceVertexCount);
	fread(&faceVertexIndex[0], sizeof(short), faceVertexCount, fp);

	fclose(fp);
	return true;
}

const std::vector<PMD_VERTEX>& MMDLoader::GetVertexData()
{
	return vertex;
}

const std::vector<unsigned short>& MMDLoader::GetIndexData()
{
	return faceVertexIndex;
}