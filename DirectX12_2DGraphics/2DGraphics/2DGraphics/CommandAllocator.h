#pragma once
#include <d3d12.h>
#include <wrl.h>

// ComPtr使用宣言
using Microsoft::WRL::ComPtr;

class CommandAllocator
{
public:
	CommandAllocator();
	~CommandAllocator();

	/// @fn Create
	/// コマンドアロケータを作成する
	/// @param[in] device			D3D12デバイス
	/// @param[in] commandListType	コマンドリストタイプ
	/// @retval bool true:成功, false:失敗
	bool Create(ID3D12Device* device, D3D12_COMMAND_LIST_TYPE commandListType);

	/// @fn Get
	/// コマンドアロケータのポインタを取得する
	/// @retval ID3D12CommandAllocator* コマンドアロケータのポインタ
	ID3D12CommandAllocator* Get();

	/// @fn GetComPtr
	/// コマンドアロケータのComポインタを取得する
	/// @retval ComPtr<ID3D12CommandAllocator> コマンドアロケータのCOMポインタ
	ComPtr<ID3D12CommandAllocator> GetComPtr();
private:

	ComPtr<ID3D12CommandAllocator> _commandAllocator;	//コマンドアロケータ
};

