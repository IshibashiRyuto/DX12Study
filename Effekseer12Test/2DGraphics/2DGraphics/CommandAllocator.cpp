#include "CommandAllocator.h"



CommandAllocator::CommandAllocator()
{
}


CommandAllocator::~CommandAllocator()
{
}

bool CommandAllocator::Create(ID3D12Device * device, D3D12_COMMAND_LIST_TYPE commandListType)
{
	HRESULT result;
	result = device->CreateCommandAllocator(commandListType, IID_PPV_ARGS(&_commandAllocator));
	if (result != S_OK)
	{
		MessageBox(nullptr, TEXT("Failed Create CommandAllocator."), TEXT("Failed"), MB_OK);
		return false;
	}
	return true;
}

ID3D12CommandAllocator *const CommandAllocator::Get()
{
	return _commandAllocator.Get();
}

ComPtr<ID3D12CommandAllocator> CommandAllocator::GetComPtr()
{
	return _commandAllocator;
}
