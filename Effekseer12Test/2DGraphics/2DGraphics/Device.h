/*	@file	Device.h
	@brief	D3D12デバイスクラス
	@author Ishibashi Ryuto
	@date	2018/05/22	初版
*/
#pragma once
#include <d3d12.h>
#include <wrl.h>
#include <dxgi1_4.h>

// ComPtr使用宣言
using Microsoft::WRL::ComPtr;

class Device
{
public:
	Device();
	~Device();

	/// @fn Create
	/// デバイスを作成する
	/// @retval bool true: デバイス作成成功　false: デバイス作成失敗
	bool Create();

	/// @fn Get
	/// デバイスのポインタを取得する
	/// @retval ID3D12Device* デバイスのポインタ
	ID3D12Device* Get() const;

	/// @fn GetComPtr
	/// デバイスのCOMポインタを取得する
	/// @retval ComPtr<ID3D12Device> デバイスのComPtr
	ComPtr<ID3D12Device> GetComPtr() const;

	/// @fn GetFeatureLevel
	/// デバイスを作成したフィーチャーレベルを取得する
	/// @retval D3D_FEATURE_LEVEL デバイスのフィーチャーレベル
	D3D_FEATURE_LEVEL GetFeatureLevel() const;
private:
	D3D_FEATURE_LEVEL _level;										// デバイスのフィーチャーレベル
	ComPtr<ID3D12Device> _device;							// デバイスポインタ
};

