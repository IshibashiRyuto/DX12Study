/*	@file	Device.h
	@brief	D3D12�f�o�C�X�N���X
	@author Ishibashi Ryuto
	@date	2018/05/22	����
*/
#pragma once
#include <d3d12.h>
#include <wrl.h>
#include <dxgi1_4.h>

// ComPtr�g�p�錾
using Microsoft::WRL::ComPtr;

class Device
{
public:
	Device();
	~Device();
private:
	ComPtr<IDXGIFactory4> _factory;
};

