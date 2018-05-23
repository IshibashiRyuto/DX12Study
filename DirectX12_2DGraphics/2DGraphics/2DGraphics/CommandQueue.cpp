#include "CommandQueue.h"



CommandQueue::CommandQueue()
{
}


CommandQueue::~CommandQueue()
{
}

bool CommandQueue::Create(ID3D12Device * device)
{
	D3D12_COMMAND_QUEUE_DESC desc = {};
	desc.NodeMask = 0;
	desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	desc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
	desc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
	return Create(device,desc);
}

bool CommandQueue::Create(ID3D12Device * device, const D3D12_COMMAND_QUEUE_DESC& desc)
{
	auto result = device->CreateCommandQueue(&desc, IID_PPV_ARGS(&_commandQueue));
	if (result != S_OK)
	{
		MessageBox(nullptr, TEXT("Failed Create CommandQueue."), TEXT("Failed"), MB_OK);
		return false;
	}
	return true;
}
