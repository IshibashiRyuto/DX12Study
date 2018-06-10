#include "Device.h"



Device::Device() : _device(nullptr)
{
}


Device::~Device()
{
}

bool Device::Create()
{
		HRESULT result;
		D3D_FEATURE_LEVEL levels[] = {
			D3D_FEATURE_LEVEL_12_1,
			D3D_FEATURE_LEVEL_12_0,
			D3D_FEATURE_LEVEL_11_1,
			D3D_FEATURE_LEVEL_11_0,
		};	// フィーチャレベル配列

		for (auto l : levels)
		{
			result = D3D12CreateDevice(nullptr, l, IID_PPV_ARGS(&_device));
			if (result == S_OK)	// 最新のフィーチャーレベルでデバイスを生成出来たら
			{
				// レベルを保存してループを脱出
				_level = l;
				break;
			}
		}

		if (result != S_OK)
		{
			MessageBox(nullptr, TEXT("Failed Create Device."), TEXT("Failed"), MB_OK);
			return false;
		}

	return true;
}

ID3D12Device * Device::Get() const
{
	return _device.Get();
}

ComPtr<ID3D12Device> Device::GetComPtr() const
{
	return _device;
}

D3D_FEATURE_LEVEL Device::GetFeatureLevel() const
{
	return _level;
}
