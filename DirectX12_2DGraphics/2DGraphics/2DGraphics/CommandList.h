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

	// �R�}���h���X�g�̏�����
	bool Init(ID3D12Device* pDevice, D3D12_COMMAND_LIST_TYPE type, ID3D12PipelineState* pipelineState);
	
	// �R�}���h���X�g�̃N���A
	void Clear(ID3D12PipelineState* pipelineState);

	// �R�}���h�̎��s
	void Execute(ID3D12CommandQueue* pCommandQueue);

	// �R�}���h���X�g���̂̎擾
	const ID3D12CommandList* GetCommandList();
};

