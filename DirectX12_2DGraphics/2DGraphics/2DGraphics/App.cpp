#include "App.h"
#include <math.h>

#pragma comment(lib,"d3d12.lib")
#pragma comment(lib,"dxgi.lib")
#pragma comment(lib,"d3dCompiler.lib")

App::App()
{
}


App::~App()
{
}



bool App::Initialize(HWND hWnd)
{// D3D12初期化処理
	HRESULT result = S_OK;			// リザルト確認
	_hwnd = hWnd;

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
	if (!CreateDevice())
	{
		return false;
	}

	// コマンドアロケータの生成処理
	if(!CreateCommandAllocator())
	{
		return false;
	}


	// コマンドキューの生成処理
	if (!CreateCommandQueue())
	{
		return false;
	}


	// スワップチェイン生成処理
	if (!CreateSwapchain())
	{
		return false;
	}



	// RTVディスクリプタ生成処理
	if (!CreateRenderTargetView())
	{
		return false;
	}

	/*深度バッファ作成*/
	if (!CreateDepthBuffer())
	{
		return false;
	}

	/*深度バッファビューの作成*/
	if (!CreateDepthStencilView())
	{
		return false;
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

	// ルートシグネチャの作成
	if (!CreateRootSignature())
	{
		return false;
	}

	// リソースの作成
	if (!CreateResource())
	{
		return false;
	}

	// フェンスの作成
	if (!CreateFence())
	{
		return false;
	}

	return true;
}

void App::Render()
{
	// コマンドリストの初期化処理
	_commandAllocator->Reset();
	_commandList->Reset(_commandAllocator.Get(), _pipelineStateObject.Get());
	_commandList->SetGraphicsRootSignature(_rootSignature.Get());

	// 描画範囲の設定
	D3D12_VIEWPORT vp = { 0,0,WINDOW_WIDTH,WINDOW_HEIGHT,0.0f,1.0f };
	D3D12_RECT rc = { 0,0,WINDOW_WIDTH, WINDOW_HEIGHT };
	_commandList->RSSetViewports(1, &vp);
	_commandList->RSSetScissorRects(1, &rc);

	//描画先変更処理
	int backBufferIndex = _swapChain->GetCurrentBackBufferIndex();
	_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(_renderTargets[backBufferIndex].Get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));
	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(_rtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart(), backBufferIndex, _rtvDescriptorSize);
	CD3DX12_CPU_DESCRIPTOR_HANDLE dsvHandle(_dsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart());
	_commandList->OMSetRenderTargets(1, &rtvHandle, false, &dsvHandle);

	float color[4] = { 0.0f,0.0f,0.0f,1.0f };

	// 描画初期化処理
	_commandList->ClearRenderTargetView(rtvHandle, color, 0, nullptr);
	_commandList->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_DEPTH, _depthClearValue.DepthStencil.Depth, 0, 0, nullptr);
	_commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// データセット
	D3D12_VERTEX_BUFFER_VIEW vbViews[2] = { _vertexBufferView, _instancingBufferView };
	_commandList->IASetVertexBuffers(0, 2, vbViews);

	// 描画処理
	_commandList->DrawInstanced(3, INSTANCING_NUM, 0, 0);

	// 描画終了処理
	_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(_renderTargets[backBufferIndex].Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));
	_commandList->Close();

	// 描画コマンドの実行
	ID3D12CommandList* commandLists[] = { _commandList.Get() };
	_commandQueue->ExecuteCommandLists(_countof(commandLists), commandLists);

	// フェンスによる待機処理
	++_fenceValue;
	_commandQueue->Signal(_fence.Get(), _fenceValue);
	while (_fence->GetCompletedValue() != _fenceValue)
	{
	}


	// 画面のスワップ
	_swapChain->Present(1, 0);
}


bool App::CreateDevice()
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

	return true;
}

bool App::CreateCommandAllocator()
{
	HRESULT result;
	result = _dev->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&_commandAllocator));
	if (result != S_OK)
	{
		MessageBox(nullptr, TEXT("Failed Create CommandAllocator."), TEXT("Failed"), MB_OK);
		return false;
	}
	return true;
}

bool App::CreateCommandQueue()
{

	D3D12_COMMAND_QUEUE_DESC desc = {};
	desc.NodeMask = 0;
	desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	desc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
	desc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
	auto result = _dev->CreateCommandQueue(&desc, IID_PPV_ARGS(&_commandQueue));
	if (result != S_OK)
	{
		MessageBox(nullptr, TEXT("Failed Create CommandQueue."), TEXT("Failed"), MB_OK);
		return false;
	}
	return true;
}

bool App::CreateSwapchain()
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


	auto result = _factory->CreateSwapChainForHwnd(_commandQueue.Get(),
		_hwnd,
		&swapChainDesc,
		nullptr,
		nullptr,
		(IDXGISwapChain1**)(_swapChain.GetAddressOf()));
	if (result != S_OK)
	{
		MessageBox(nullptr, TEXT("Failed Create CommandQueue."), TEXT("Failed"), MB_OK);
		return false;
	}
	return true;
}

bool App::CreateRenderTargetView()
{
	{
		// ディスクリプタヒープ用パラメタ定義
		D3D12_DESCRIPTOR_HEAP_DESC descriptorHeapDesc = {};
		descriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
		descriptorHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		descriptorHeapDesc.NumDescriptors = FRAME_CNT;

		//rtvディスクリプタヒープ生成

		auto result = _dev->CreateDescriptorHeap(&descriptorHeapDesc, IID_PPV_ARGS(&_rtvDescriptorHeap));
		if (FAILED(result))
		{
			MessageBox(nullptr, TEXT("Failed Create RTV Descriptor Heap."), TEXT("Failed"), MB_OK);
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
	return true;
}

bool App::CreateDepthBuffer()
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

	auto result = _dev->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
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

	return true;
}

bool App::CreateDepthStencilView()
{
	D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc = {};
	D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};

	dsvHeapDesc.NumDescriptors = 1;
	dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;

	auto result = _dev->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(&_dsvDescriptorHeap));

	if (FAILED(result))
	{
		MessageBox(nullptr, TEXT("Failed Create Depth Stencil View."), TEXT("Failed"), MB_OK);
		return false;
	}

	dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	dsvDesc.Format = DXGI_FORMAT_D32_FLOAT;
	_dev->CreateDepthStencilView(_depthBuffer.Get(), &dsvDesc, _dsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart());
	return true;
}

bool App::CreateRootSignature()
{
	ID3DBlob* signature = nullptr;					// シグネチャ
	ID3DBlob* error = nullptr;

	std::vector<D3D12_ROOT_PARAMETER> rootParam;

	// ルートシグネチャの設定
	CD3DX12_ROOT_SIGNATURE_DESC rsd = {};
	rsd.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
	rsd.NumParameters = (UINT)rootParam.size();
	rsd.pParameters = rootParam.data();
	rsd.NumStaticSamplers = 1;
	rsd.pStaticSamplers = &_samplerDesc;


	// シグネチャの作成
	auto result = D3D12SerializeRootSignature(&rsd, D3D_ROOT_SIGNATURE_VERSION_1, &signature, &error);
	if (result != S_OK)
	{
		MessageBox(nullptr, TEXT("Failed Create Signature."), TEXT("Failed"), MB_OK);
		return false;
	}

	// シグネチャの情報を参照してルートシグネチャを生成
	result = _dev->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(_rootSignature.GetAddressOf()));
	if (result != S_OK)
	{
		MessageBox(nullptr, TEXT("Failed Create RootSignature."), TEXT("Failed"), MB_OK);
		return false;
	}

	return true;
}

bool App::CreateResource()
{
	// 頂点レイアウトの定義
	{
		_inputLayoutDescs.push_back(D3D12_INPUT_ELEMENT_DESC{ "POSITION",  0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 });
		_inputLayoutDescs.push_back(D3D12_INPUT_ELEMENT_DESC{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT,0,D3D12_APPEND_ALIGNED_ELEMENT,D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0 });
		//_inputLayoutDescs.push_back(D3D12_INPUT_ELEMENT_DESC{ "TEXCORD",  0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 });
		_inputLayoutDescs.push_back(D3D12_INPUT_ELEMENT_DESC{ "INSTANCE_OFFSET", 0, DXGI_FORMAT_R32G32B32_FLOAT, 1,D3D12_APPEND_ALIGNED_ELEMENT,D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA, 1 });
	}

	// シェーダの読み込み
	if (!ReadShader())
	{
		return false;
	}

	// パイプラインステートオブジェクトの生成
	if (!CreatePipelineStateObject())
	{
		return false;
	}
	
	/// コマンドリストの生成
	if (!CreateCommandList())
	{
		return false;
	}


	// 頂点バッファの作成
	if (!CreateVertexBuffer())
	{
		return false;
	}

	// インスタンシングバッファの作成
	if (!CreateInstancingBuffer())
	{
		return false;
	}

	return true;
}

bool App::ReadShader()
{
	auto result = D3DCompileFromFile(TEXT("shader.hlsl"),
		nullptr,
		nullptr,
		"VSMain",
		"vs_5_0",
		D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION,
		0,
		&_vertexShader,
		nullptr);

	if (FAILED(result))
	{
		MessageBox(nullptr, TEXT("Failed Compile VertexShader."), TEXT("Failed"), MB_OK);
		return false;
	}
	result = D3DCompileFromFile(TEXT("shader.hlsl"),
		nullptr,
		nullptr,
		"PSMain",
		"ps_5_0",
		D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION,
		0,
		&_pixelShader,
		nullptr);

	if (FAILED(result))
	{
		MessageBox(nullptr, TEXT("Failed Compile PixelShader."), TEXT("Failed"), MB_OK);
		return false;
	}
	return true;
}

bool App::CreatePipelineStateObject()

{
	D3D12_GRAPHICS_PIPELINE_STATE_DESC gpsDesc = {};

	gpsDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	gpsDesc.DepthStencilState.DepthEnable = TRUE;					// 深度を使用するかどうか
	gpsDesc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
	gpsDesc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
	gpsDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT;
	gpsDesc.DepthStencilState.StencilEnable = FALSE;				// stencilマスクを使用するかどうか
	gpsDesc.VS = CD3DX12_SHADER_BYTECODE(_vertexShader.Get());
	gpsDesc.PS = CD3DX12_SHADER_BYTECODE(_pixelShader.Get());
	gpsDesc.InputLayout.NumElements = (UINT)_inputLayoutDescs.size();
	gpsDesc.InputLayout.pInputElementDescs = &_inputLayoutDescs[0];
	gpsDesc.pRootSignature = _rootSignature.Get();
	gpsDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	gpsDesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
	gpsDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
	gpsDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	gpsDesc.NumRenderTargets = 1;
	gpsDesc.SampleDesc.Count = 1;
	gpsDesc.SampleMask = UINT_MAX;

	auto result = _dev->CreateGraphicsPipelineState(&gpsDesc, IID_PPV_ARGS(_pipelineStateObject.GetAddressOf()));

	if (FAILED(result))
	{
		MessageBox(nullptr, TEXT("Failed Create PipelineObject."), TEXT("Failed"), MB_OK);
		return false;
	}
	return true;
}

bool App::CreateCommandList()
{
	auto result = _dev->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, _commandAllocator.Get(), _pipelineStateObject.Get(), IID_PPV_ARGS(_commandList.GetAddressOf()));
	if (result != S_OK)
	{
		MessageBox(nullptr, TEXT("Failed Create CommandLine."), TEXT("Failed"), MB_OK);
		return false;
	}

	result = _commandList->Close();
	if (result != S_OK)
	{
		MessageBox(nullptr, TEXT("Failed CommandLine Close."), TEXT("Failed"), MB_OK);
		return false;
	}
	return true;
}

bool App::CreateFence()
{
	auto result = _dev->CreateFence(_fenceValue, D3D12_FENCE_FLAGS::D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(_fence.GetAddressOf()));
	if (FAILED(result))
	{
		MessageBox(nullptr, TEXT("Failed Create Fence."), TEXT("Failed"), MB_OK);
		return false;
	}
	return true;
}

bool App::CreateVertexBuffer()
{
	auto result = _dev->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(sizeof(vertices)),
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(_vertexBuffer.GetAddressOf()));

	if (FAILED(result))
	{
		MessageBox(nullptr, TEXT("Failed Create VertexBuffer."), TEXT("Failed"), MB_OK);
		return false;
	}
	char *buf;

	_vertexBuffer->Map(0, nullptr, (void**)&buf);
	memcpy(buf, &vertices, sizeof(vertices));
	_vertexBuffer->Unmap(0, nullptr);

	_vertexBufferView.BufferLocation = _vertexBuffer->GetGPUVirtualAddress();
	_vertexBufferView.StrideInBytes = sizeof(Vertex);
	_vertexBufferView.SizeInBytes = sizeof(vertices);

	return true;
}

bool App::CreateInstancingBuffer()
{
	auto result = _dev->CreateCommittedResource
	(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(sizeof(DirectX::XMFLOAT3)*INSTANCING_NUM),
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(_instancingBuffer.GetAddressOf()));
	
	if (FAILED(result))
	{
		MessageBox(nullptr, TEXT("Failed InstancingBuffer."), TEXT("Failed"), MB_OK);
		return false;
	}

	DirectX::XMFLOAT3 *buf;
	_instancingBuffer->Map(0, nullptr, (void**)&buf);
	for (int i = 0; i < INSTANCING_NUM; ++i)
	{
		float x = (float)rand() / (float)RAND_MAX;
		float y = (float)rand() / (float)RAND_MAX;
		x = x * 2 - 1.0f;
		y = y * 2 - 1.0f;
		

		DirectX::XMFLOAT3 pos(x, y, 0.0f);
		*buf = pos;
		++buf;
	}
	_instancingBuffer->Unmap(0, nullptr);

	_instancingBufferView.BufferLocation = _instancingBuffer->GetGPUVirtualAddress();
	_instancingBufferView.StrideInBytes = sizeof(DirectX::XMFLOAT3);
	_instancingBufferView.SizeInBytes = sizeof(DirectX::XMFLOAT3)*INSTANCING_NUM;
	return true;
}
