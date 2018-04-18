#include "App.h"



App::App()
{
}


App::~App()
{
}

bool App::CreateDevice(HWND hwnd)
{	
	// D3D12初期化処理
	HRESULT result = S_OK;			// リザルト確認
	_hwnd = hwnd;

	// DXGIFactoryの生成
	{
		result = CreateDXGIFactory1(IID_PPV_ARGS(&_factory));
		if (FAILED(result))
		{
			MessageBox(nullptr, TEXT("Failed Create Device."), TEXT("Failed"), MB_OK);
			return false;
		}
	}

#ifdef _DEBUG
	{
		// デバッグレイヤーの有効化
		ID3D12Debug* debugController;
		if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController))))
		{
			debugController->EnableDebugLayer();
			debugController->Release();
			debugController = nullptr;
		}
	}
#endif


	// D3D12デバイスの生成
	{
		D3D_FEATURE_LEVEL levels[] = {
			D3D_FEATURE_LEVEL_12_1,
			D3D_FEATURE_LEVEL_12_0,
			D3D_FEATURE_LEVEL_11_1,
			D3D_FEATURE_LEVEL_11_0,
		};	// フィーチャレベル配列

		for (auto l : levels)
		{
			result = D3D12CreateDevice(nullptr, l, IID_PPV_ARGS(&_dev));
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
	}

	// コマンドアロケータの生成処理
	{
		result = _dev->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&_commandAllocator));
		if (result != S_OK)
		{
			MessageBox(nullptr, TEXT("Failed Create CommandAllocator."), TEXT("Failed"), MB_OK);
			return false;
		}
	}


	// コマンドキューの生成処理
	{
		D3D12_COMMAND_QUEUE_DESC desc = {};
		desc.NodeMask = 0;
		desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
		desc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
		desc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
		result = _dev->CreateCommandQueue(&desc, IID_PPV_ARGS(&_commandQueue));
		if (result != S_OK)
		{
			MessageBox(nullptr, TEXT("Failed Create CommandQueue."), TEXT("Failed"), MB_OK);
			return false;
		}
	}


	// スワップチェイン生成処理
	{
		DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};//スワップチェイン設定の項目

		swapChainDesc.Width = WINDOW_WIDTH;
		swapChainDesc.Height = WINDOW_HEIGHT;
		swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		swapChainDesc.Stereo = false;
		swapChainDesc.SampleDesc.Count = 1;
		swapChainDesc.SampleDesc.Quality = 0;
		swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		swapChainDesc.BufferCount = FRAME_CNT;
		swapChainDesc.Scaling = DXGI_SCALING_STRETCH;
		swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
		swapChainDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
		swapChainDesc.Flags = 0;


		result = _factory->CreateSwapChainForHwnd(_commandQueue.Get(),
			hwnd,
			&swapChainDesc,
			nullptr,
			nullptr,
			(IDXGISwapChain1**)(_swapChain.GetAddressOf()));
		if (result != S_OK)
		{
			MessageBox(nullptr, TEXT("Failed Create CommandQueue."), TEXT("Failed"), MB_OK);
			return false;
		}
	}



	// RTVディスクリプタ生成処理
	{
		// ディスクリプタヒープ用パラメタ定義
		D3D12_DESCRIPTOR_HEAP_DESC descriptorHeapDesc = {};
		descriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
		descriptorHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		descriptorHeapDesc.NumDescriptors = FRAME_CNT;

		//rtvディスクリプタヒープ生成

		result = _dev->CreateDescriptorHeap(&descriptorHeapDesc, IID_PPV_ARGS(&_rtvDescriptorHeap));
		if (FAILED(result))
		{
			MessageBox(nullptr, TEXT("Failed Create Descriptor Heap."), TEXT("Failed"), MB_OK);
			return false;
		}

		// rtvディスクリプタヒープのサイズを取得
		UINT descriptorHeapSize = 0;
		descriptorHeapSize = _dev->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);


		//レンダーターゲットビューの生成
		DXGI_SWAP_CHAIN_DESC swcDesc = {};				// スワップチェインの情報
		_swapChain->GetDesc(&swcDesc);

		// レンダーターゲット数を取得
		int renderTargetsNum = FRAME_CNT;

		// レンダーターゲット数だけ領域確保
		_renderTargets.resize(renderTargetsNum);

		// ディスクリプタ一つ当たりのサイズを取得
		_rtvDescriptorSize = _dev->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
		{
			// ディスクリプタハンドルの作成
			CD3DX12_CPU_DESCRIPTOR_HANDLE descriptorHandle(_rtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart());

			for (UINT i = 0; i < (UINT)renderTargetsNum; i++)
			{
				result = _swapChain->GetBuffer(i, IID_PPV_ARGS(&_renderTargets[i]));	//スワップチェインからキャンバスを取得
				if (FAILED(result))
				{
					MessageBox(nullptr, TEXT("Failed Create RTV."), TEXT("Failed"), MB_OK);
					return false;
				}
				_dev->CreateRenderTargetView(_renderTargets[i].Get(), nullptr, descriptorHandle);
				descriptorHandle.Offset(1, _rtvDescriptorSize);//ディスクリプタとキャンバス分オフセット
			}
		}
	}

	// サンプラの設定
	{
		_samplerDesc.Filter = D3D12_FILTER_MIN_MAG_LINEAR_MIP_POINT;
		_samplerDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
		_samplerDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
		_samplerDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
		_samplerDesc.MaxLOD = D3D12_FLOAT32_MAX;
		_samplerDesc.MinLOD = 0.0f;
		_samplerDesc.MipLODBias = 0.0f;
		_samplerDesc.BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
		_samplerDesc.ShaderRegister = 0;
		_samplerDesc.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
		_samplerDesc.RegisterSpace = 0;
		_samplerDesc.MaxAnisotropy = 0;
		_samplerDesc.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
	}
	/*深度バッファ作成*/
	{
		D3D12_RESOURCE_DESC depthResDesc = {};
		depthResDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
		depthResDesc.Width = WINDOW_WIDTH;
		depthResDesc.Height = WINDOW_HEIGHT;
		depthResDesc.DepthOrArraySize = 1;
		depthResDesc.Format = DXGI_FORMAT_D32_FLOAT;
		depthResDesc.SampleDesc.Count = 1;
		depthResDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

		D3D12_HEAP_PROPERTIES depthHeapProp = {};
		depthHeapProp.Type = D3D12_HEAP_TYPE_DEFAULT;
		depthHeapProp.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
		depthHeapProp.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;

		_depthClearValue.DepthStencil.Depth = 1.0f;		// 深さ最大値
		_depthClearValue.Format = DXGI_FORMAT_D32_FLOAT;

		result = _dev->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
			D3D12_HEAP_FLAG_NONE,
			&depthResDesc,
			D3D12_RESOURCE_STATE_DEPTH_WRITE,
			&_depthClearValue,
			IID_PPV_ARGS(&_depthBuffer));

		if (FAILED(result))
		{
			MessageBox(nullptr, TEXT("Failed Create Depth Buffer."), TEXT("Failed"), MB_OK);
			return false;
		}
	}

	/*深度バッファビューの作成*/
	{
		D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc = {};
		D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};

		dsvHeapDesc.NumDescriptors = 1;
		dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;

		result = _dev->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(&_dsvDescriptorHeap));

		if (FAILED(result))
		{
			MessageBox(nullptr, TEXT("Failed Create Depth Stencil View."), TEXT("Failed"), MB_OK);
			return false;
		}

		dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
		dsvDesc.Format = DXGI_FORMAT_D32_FLOAT;
		_dev->CreateDepthStencilView(_depthBuffer, &dsvDesc, _dsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart());
	}

	return true;
}

bool App::CreateResource()
{
	return false;
}
