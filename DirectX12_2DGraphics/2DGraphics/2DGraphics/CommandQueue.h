/*	@file CommandQueue.h
	コマンドキュークラス

*/
#pragma once

#include<d3d12.h>
#include<wrl.h>


// ComPtr使用宣言
using Microsoft::WRL::ComPtr;

class CommandQueue
{
public:
	CommandQueue();
	~CommandQueue();
	
	/// @fn Create
	/// コマンドキューの作成
	/// @param[in]	ID3D12Device* : デバイスのポインタ
	/// @retval	bool true:生成成功,false:生成失敗
	bool Create(ID3D12Device* device);

	/// @fn Create
	/// コマンドキューの作成(パラメータ設定可)
	/// @param[in]	ID3D12Device* : デバイスのポインタ
	/// @param[in]	D3D12_COMMAND_QUEUE_DESC : コマンドキューの設定項目
	/// @retval	bool true:生成成功,false:生成失敗
	bool Create(ID3D12Device* device, const D3D12_COMMAND_QUEUE_DESC& desc);

	/// @fn Get
	/// コマンドキューのポインタを取得する
	///	@retval	ID3D12CommandQueue* :コマンドキューのポインタ
	ID3D12CommandQueue*const Get();
private:
	ComPtr<ID3D12CommandQueue> _commandQueue;
};

