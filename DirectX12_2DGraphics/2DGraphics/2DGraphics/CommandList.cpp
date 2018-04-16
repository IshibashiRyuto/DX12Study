#include "CommandList.h"



CommandList::CommandList()
{
	_allocatorIndex = 0;
}


CommandList::~CommandList()
{
}

bool CommandList::Init(ID3D12Device * pDevice, D3D12_COMMAND_LIST_TYPE type, ID3D12PipelineState * pPipelineState)
{
	if (pDevice == nullptr)
	{
		return false;
	}

	// コマンドアロケータ生成
	for (int i = 0; i < COMMAND_ALLOCATOR_NUM; i++)
	{
		auto result = pDevice->CreateCommandAllocator(type, IID_PPV_ARGS(&_commandAllocator[i]));
		if (FAILED(result))
		{
			return false;
		}
	}

	// コマンドリストの生成
	{
		auto result = pDevice->CreateCommandList(0, type, _commandAllocator[0], pPipelineState, IID_PPV_ARGS(&_commandList));
		if (FAILED(result))
		{
			return false;
		}
	}
	
	return true;
}

void CommandList::Clear(ID3D12PipelineState * pipelineState)
{
	_commandAllocator[_allocatorIndex]->Reset();
	_allocatorIndex++;
	if (_allocatorIndex == COMMAND_ALLOCATOR_NUM)
	{
		_allocatorIndex = 0;
	}
	_commandList->Reset(_commandAllocator[_allocatorIndex], pipelineState);
}

void CommandList::Execute(ID3D12CommandQueue * pCommandQueue)
{
	_commandList->Close();
	pCommandQueue->ExecuteCommandLists(1, (ID3D12CommandList* const*)&_commandList);
}
