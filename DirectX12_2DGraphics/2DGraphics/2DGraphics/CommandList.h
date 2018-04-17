#pragma once
#include <d3d12.h>

#define COMMAND_ALLOCATOR_NUM 2

class CommandList
{
private:
	unsigned int _allocatorIndex;
	ID3D12CommandAllocator* _commandAllocator[COMMAND_ALLOCATOR_NUM];
	ID3D12GraphicsCommandList* _commandList{ nullptr };
public:
	CommandList();
	~CommandList();

	// コマンドリストの初期化
	bool Init(ID3D12Device* pDevice, D3D12_COMMAND_LIST_TYPE type, ID3D12PipelineState* pipelineState);
	
	// コマンドリストのクリア
	void Clear(ID3D12PipelineState* pipelineState);

	// コマンドの実行
	void Execute(ID3D12CommandQueue* pCommandQueue);

	// コマンドリスト実体の取得
	const ID3D12CommandList* GetCommandList();
};

