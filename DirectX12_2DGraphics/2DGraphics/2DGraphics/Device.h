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
private:
	ComPtr<IDXGIFactory4> _factory;
};

