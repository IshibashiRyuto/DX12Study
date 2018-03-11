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
	fopen_s(&fp, "初音ミク.pmd", "rb");
	
	// ファイルフォーマットの確認
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

	// ヘッダデータ読み込み
	fread(&pmdHeader, sizeof(pmdHeader), 1, fp);

	
	// 頂点情報の読み込み
	fread(&vertexCount, sizeof(int), 1, fp);
	vertex.resize(vertexCount);
	fread(&vertex[0], sizeof(PMD_VERTEX), vertex.size(), fp);
	
	// 頂点インデックス情報読み込み
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