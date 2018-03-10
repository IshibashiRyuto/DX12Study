#include "Dx12Device.h"
#include "MMDLoader.h"


Dx12Device::Dx12Device()
{
}


Dx12Device::~Dx12Device()
{
}

bool Dx12Device::CreateDevice(HWND hwnd)
{
	// D3D12初期化処理
	HRESULT result = S_OK;			// リザルト確認
	_hwnd = hwnd;

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

	// DXGIFactoryの生成
	{
		result = CreateDXGIFactory1(IID_PPV_ARGS(&_factory));
		if (FAILED(result))
		{
			MessageBox(nullptr, TEXT("Failed Create Device."), TEXT("Failed"), MB_OK);
			return false;
		}
	}

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


		result = _factory->CreateSwapChainForHwnd(_commandQueue,
			hwnd,
			&swapChainDesc,
			nullptr,
			nullptr,
			(IDXGISwapChain1**)(&_swapChain));
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

		//ディスクリプタヒープ生成
	
		result = _dev->CreateDescriptorHeap(&descriptorHeapDesc, IID_PPV_ARGS(&_descriptorHeapRTV));
		if (FAILED(result))
		{
			MessageBox(nullptr, TEXT("Failed Create Descriptor Heap."), TEXT("Failed"), MB_OK);
			return false;
		}

		// ディスクリプタヒープのサイズを取得
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
		rtvDescriptorSize =_dev->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

		{
			// ディスクリプタハンドルの作成
			CD3DX12_CPU_DESCRIPTOR_HANDLE descriptorHandle(_descriptorHeapRTV->GetCPUDescriptorHandleForHeapStart());

			for (UINT i = 0; i < (UINT)renderTargetsNum; i++)
			{
				result = _swapChain->GetBuffer(i, IID_PPV_ARGS(&_renderTargets[i]));	//スワップチェインからキャンバスを取得
				if (FAILED(result))
				{
					MessageBox(nullptr, TEXT("Failed Create RTV."), TEXT("Failed"), MB_OK);
					return false;
				}
				_dev->CreateRenderTargetView(_renderTargets[i], nullptr, descriptorHandle);
				descriptorHandle.Offset(1, rtvDescriptorSize);//ディスクリプタとキャンバス分オフセット
			}
		}
	}

	// サンプラの設定
	{
		samplerDesc.Filter = D3D12_FILTER_MIN_MAG_LINEAR_MIP_POINT;
		samplerDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
		samplerDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
		samplerDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
		samplerDesc.MaxLOD = D3D12_FLOAT32_MAX;
		samplerDesc.MinLOD = 0.0f;
		samplerDesc.MipLODBias = 0.0f;
		samplerDesc.BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
		samplerDesc.ShaderRegister = 0;
		samplerDesc.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
		samplerDesc.RegisterSpace = 0;
		samplerDesc.MaxAnisotropy = 0;
		samplerDesc.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
	}


	// ルートシグネチャの作成
	{
		ID3DBlob* signature = nullptr;					// シグネチャ
		ID3DBlob* error = nullptr;
		// SRV用ディスクリプタレンジの設定
		D3D12_DESCRIPTOR_RANGE descriptorRangeSRV;
		descriptorRangeSRV.NumDescriptors = 1;
		descriptorRangeSRV.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
		descriptorRangeSRV.BaseShaderRegister = 0;
		descriptorRangeSRV.RegisterSpace = 0;
		descriptorRangeSRV.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

		//CBV用ディスクリプタレンジの設定
		D3D12_DESCRIPTOR_RANGE descriptorRangeCBV;
		descriptorRangeCBV.NumDescriptors = 1;
		descriptorRangeCBV.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
		descriptorRangeCBV.BaseShaderRegister = 0;
		descriptorRangeCBV.RegisterSpace = 0;
		descriptorRangeCBV.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;


		// SRV用ルートパラメータの設定
		D3D12_ROOT_PARAMETER prmSRV;
		prmSRV.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
		prmSRV.DescriptorTable = { 1,&descriptorRangeSRV };
		prmSRV.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
		

		// CBV用ルートパラメータの設定
		D3D12_ROOT_PARAMETER prmCBV;
		prmCBV.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
		prmCBV.DescriptorTable = { 1,&descriptorRangeCBV };
		prmCBV.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

		std::vector<D3D12_ROOT_PARAMETER> rootParam;
		rootParam.push_back(prmSRV);
		rootParam.push_back(prmCBV);


		CD3DX12_ROOT_SIGNATURE_DESC rsd = {};
		//rsd.Init(0, nullptr, 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);
		rsd.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
		rsd.NumParameters = rootParam.size();
		rsd.pParameters = rootParam.data();
		rsd.NumStaticSamplers = 1;
		rsd.pStaticSamplers = &samplerDesc;


		// シグネチャの作成
		result = D3D12SerializeRootSignature(&rsd, D3D_ROOT_SIGNATURE_VERSION_1, &signature, &error);
		if (result != S_OK)
		{
			MessageBox(nullptr, TEXT("Failed Create Signature."), TEXT("Failed"), MB_OK);
			return false;
		}

		// シグネチャの情報を参照してルートシグネチャを生成
		result = _dev->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&_rootSignature));
		if (result != S_OK)
		{
			MessageBox(nullptr, TEXT("Failed Create RootSignature."), TEXT("Failed"), MB_OK);
			return false;
		}
	}

	// PMDデータの読み込み
	MMDLoader::Instance()->Load();

	// 頂点レイアウトの定義
	
	_inputLayoutDescs.push_back(D3D12_INPUT_ELEMENT_DESC{ "POSITION",  0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 });
	//_inputLayoutDescs.push_back(D3D12_INPUT_ELEMENT_DESC{ "TEXCORD",  0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 });
	// シェーダの読み込み
	{
		result = D3DCompileFromFile(TEXT("shader.hlsl"),
			nullptr,
			nullptr,
			"VSMain",
			"vs_5_0",
			D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION,
			0,
			&vertexShader,
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
			&pixelShader,
			nullptr);

		if (FAILED(result))
		{
			MessageBox(nullptr, TEXT("Failed Compile PixelShader."), TEXT("Failed"), MB_OK);
			return false;
		}

	}



	// PSO初期化
	{
		D3D12_GRAPHICS_PIPELINE_STATE_DESC gpsDesc = {};

		gpsDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
		gpsDesc.DepthStencilState.DepthEnable = FALSE;					// 深度を使用するかどうか
		gpsDesc.DepthStencilState.StencilEnable = FALSE;				// マスクを使用するかどうか
		gpsDesc.VS = CD3DX12_SHADER_BYTECODE(vertexShader);
		gpsDesc.PS = CD3DX12_SHADER_BYTECODE(pixelShader);
		gpsDesc.InputLayout.NumElements = (UINT)_inputLayoutDescs.size();
		gpsDesc.InputLayout.pInputElementDescs = &_inputLayoutDescs[0];
		gpsDesc.pRootSignature = _rootSignature;
		gpsDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
		gpsDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
		gpsDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT;
		gpsDesc.NumRenderTargets = 1;
		gpsDesc.SampleDesc.Count = 1;
		gpsDesc.SampleMask = UINT_MAX;

		result = _dev->CreateGraphicsPipelineState(&gpsDesc, IID_PPV_ARGS(&_pipelineStateObject));

		if (FAILED(result))
		{
			MessageBox(nullptr, TEXT("Failed Create PipelineObject."), TEXT("Failed"), MB_OK);
			return false;
		}
	}

	// コマンドリストの生成処理
	{
		result = _dev->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, _commandAllocator, _pipelineStateObject, IID_PPV_ARGS(&_commandList));
		if (result != S_OK)
		{
			MessageBox(nullptr, TEXT("Failed Create CommandLine."), TEXT("Failed"), MB_OK);
			return false;
		}
	}
	// コマンドの記録終了
	{
		result = _commandList->Close();
		if (result != S_OK)
		{
			MessageBox(nullptr, TEXT("Failed CommandLine Close."), TEXT("Failed"), MB_OK);
			return false;
		}
	}

	/*三角ポリの表示*/
	// 三角ポリの頂点定義
	/*Vertex vertices[] = { { { 0.0f,0.7f,0.0f } },
	{ { 0.4f,-0.5f,0.0f } },
	{ { -0.4f,-0.5f,0.0f } }, };
	*/

	Vertex vertices[] = { 
		{ { WINDOW_WIDTH / 2 - 128.0f, WINDOW_HEIGHT / 2 - 128.0f,0.0f },{0.0f,0.0f} },
		{ { WINDOW_WIDTH / 2 + 128.0f, WINDOW_HEIGHT / 2 - 128.0f,0.0f },{1.0f,0.0f} },
		{ { WINDOW_WIDTH / 2 - 128.0f, WINDOW_HEIGHT / 2 + 128.0f,0.0f },{0.0f,1.0f} } ,
		{ { WINDOW_WIDTH / 2 + 128.0f, WINDOW_HEIGHT / 2 + 128.0f, 0.0f },{1.0f,1.0f} },
	};


	// MMDデータ
	std::vector<PMD_VERTEX> pmdData;
	pmdData = MMDLoader::Instance()->GetVertexData();

	// 頂点バッファ作成
	{
		result = _dev->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
			D3D12_HEAP_FLAG_NONE,
			&CD3DX12_RESOURCE_DESC::Buffer(sizeof(PMD_VERTEX) * pmdData.size()),
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&_vertexBuffer));

		if (FAILED(result))
		{
			MessageBox(nullptr, TEXT("Failed Create VertexBuffer."), TEXT("Failed"), MB_OK);
			return false;
		}
	}


	// 頂点のマップ
	char* buf;
	_vertexBuffer->Map(0, nullptr, (void**)&buf);
	memcpy(buf, &pmdData[0], pmdData.size() * sizeof(PMD_VERTEX));
	_vertexBuffer->Unmap(0, nullptr); 

	 
	/*
	FILE *fp;
	fopen_s(&fp, "memoryDump.bin", "wb");
	fwrite(buf, pmdData.size() * sizeof(PMD_VERTEX), 1, fp); 
	fclose(fp);
	*/

	// 頂点バッファビューの宣言
	_vbView.BufferLocation = _vertexBuffer->GetGPUVirtualAddress();
	_vbView.StrideInBytes = 36;// sizeof(PMD_VERTEX);
	_vbView.SizeInBytes = pmdData.size() * sizeof(PMD_VERTEX);


	/*テクスチャリソース設定*/
	{
		D3D12_RESOURCE_DESC texResourceDesc = {};
		texResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
		texResourceDesc.Width = 256;
		texResourceDesc.Height = 256;
		texResourceDesc.DepthOrArraySize = 1;
		texResourceDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		texResourceDesc.SampleDesc.Count = 1;
		texResourceDesc.Flags = D3D12_RESOURCE_FLAG_NONE;
		texResourceDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;

		D3D12_HEAP_PROPERTIES hprop = {};
		hprop.Type = D3D12_HEAP_TYPE_CUSTOM;
		hprop.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_WRITE_BACK;
		hprop.MemoryPoolPreference = D3D12_MEMORY_POOL_L0;
		hprop.CreationNodeMask = 1;
		hprop.VisibleNodeMask = 1;

		result = _dev->CreateCommittedResource(&hprop,
			D3D12_HEAP_FLAG_NONE,
			&texResourceDesc,
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&_textureBuffer));
		if (FAILED(result))
		{
			MessageBox(nullptr, TEXT("Failed Create TextureBuffer."), TEXT("Failed"), MB_OK);
			return false;
		}

		BITMAPFILEHEADER bmpFileHeader = {};
		BITMAPINFOHEADER bmpHeader = {};

		FILE *fp;
		fopen_s(&fp, "texturesample.bmp", "rb");
		fread(&bmpFileHeader, sizeof(BITMAPFILEHEADER), 1, fp);
		fread(&bmpHeader, sizeof(BITMAPINFOHEADER), 1, fp);

		std::vector<char> data;
		data.resize(bmpHeader.biSizeImage);

		int bitmapWidthSize = bmpHeader.biWidth * 4;
		for (int line = bmpHeader.biHeight - 1; line >= 0; --line)
		{
			for (int count = 0; count < bmpHeader.biWidth * 4; count += 4)
			{
				unsigned int address = line*bmpHeader.biWidth * 4;
				fread(&data[address + count], sizeof(unsigned char), 4, fp);
			}
		}
		fread(&data[0], bmpHeader.biSizeImage, 1, fp);
		fclose(fp);

		D3D12_BOX textureBox = { 0,0,0,256,256,1 };
		_textureBuffer->WriteToSubresource(0, &textureBox, &data[0], bmpHeader.biWidth*4, bmpHeader.biSizeImage);
		_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(_textureBuffer, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE));
		_commandList->Close();
		_commandQueue->ExecuteCommandLists(1, (ID3D12CommandList* const* )&_commandList);
		
	}

	// シェーダリソースビューの作成
	{
		D3D12_DESCRIPTOR_HEAP_DESC desc{};
		desc.NumDescriptors = 1;
		desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
		//CBV...ConstantBufferView
		//SRV...ShaderResourceView
		//UAV...UnorderdAccessView

		result = _dev->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&_descriptorHeapSRV));

		if (FAILED(result))
		{
			MessageBox(nullptr, TEXT("Failed Create ShaderResourceView."), TEXT("Failed"), MB_OK);
			return false;
		}

		unsigned int stride = _dev->GetDescriptorHandleIncrementSize(desc.Type);
		D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
		D3D12_CPU_DESCRIPTOR_HANDLE srvHandle{};

		srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MipLevels = 1;
		srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

		srvHandle = _descriptorHeapSRV->GetCPUDescriptorHandleForHeapStart();
		_dev->CreateShaderResourceView(_textureBuffer, &srvDesc, srvHandle);
	}

	//定数バッファの生成
	{

		world = DirectX::XMMatrixIdentity();
		eye = { 0,5,-10 };
		target = { 0,5,0 };
		upper = { 0,-1,0 };
		camera = DirectX::XMMatrixLookAtLH(eye, target, upper);

		projection = DirectX::XMMatrixPerspectiveFovLH(30.0f, 1.0f, 0.1f, 50.f);

		matrix = world * camera * projection;


		D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
		heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
		heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		heapDesc.NumDescriptors = 1;
		result = _dev->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&_cbvDescriptorHeap));

		if (FAILED(result))
		{
			MessageBox(nullptr, TEXT("Failed Create ConstantBuffer."), TEXT("Failed"), MB_OK);
			return false;
		}

		D3D12_HEAP_PROPERTIES cbvHeapProperties = {};
		cbvHeapProperties.Type = D3D12_HEAP_TYPE_UPLOAD;
		cbvHeapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
		cbvHeapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
		cbvHeapProperties.VisibleNodeMask = 1;
		cbvHeapProperties.CreationNodeMask = 1;

		D3D12_RESOURCE_DESC cbvResourceDesc = {};
		cbvResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
		cbvResourceDesc.Width = (sizeof(DirectX::XMMATRIX) + 0xff)&~0xff;
		cbvResourceDesc.Height = 1;
		cbvResourceDesc.DepthOrArraySize = 1;
		cbvResourceDesc.SampleDesc.Count = 1;
		cbvResourceDesc.MipLevels = 1;
		cbvResourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

		_dev->CreateCommittedResource(&cbvHeapProperties,
			D3D12_HEAP_FLAG_NONE,
			&cbvResourceDesc,
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&_constantBuffer));


		D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
		D3D12_CPU_DESCRIPTOR_HANDLE cbvHandle = {};

		cbvDesc.BufferLocation = _constantBuffer->GetGPUVirtualAddress();
		cbvDesc.SizeInBytes = (sizeof(DirectX::XMMATRIX) + 0xff)& ~0xff;

		cbvHandle = _cbvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
		_dev->CreateConstantBufferView(&cbvDesc, cbvHandle);


		D3D12_RANGE range = {};
		result = _constantBuffer->Map(0, &range, (void**)(&_matrixAddress));

		if (FAILED(result))
		{
			MessageBox(nullptr, TEXT("Failed Map by constantBuffer."), TEXT("Failed"), MB_OK);
			return false;
		}

		*_matrixAddress = matrix;

	}

	// フェンスの実装
	result = _dev->CreateFence(_fenceValue, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&_fence));
	if (FAILED(result))
	{
		MessageBox(nullptr, TEXT("Failed Create Fence."), TEXT("Failed"), MB_OK);
		return false;
	}
	
	WaitForGPU();
	return true;
}

void Dx12Device::Render()
{
	// ワールド座標変換行列を更新
	
	world *= DirectX::XMMatrixRotationY(3.14159265f / 120.0f);
	matrix = world * camera * projection;
	*_matrixAddress = matrix;
	
	// コマンドアロケータとリストのリセット処理
	_commandAllocator->Reset();
	_commandList->Reset(_commandAllocator, _pipelineStateObject);

	// ルートシグニチャを設定
	_commandList->SetGraphicsRootSignature(_rootSignature);


	ID3D12DescriptorHeap* descriptorHeaps[] = { _descriptorHeapSRV, _cbvDescriptorHeap };

	// SRVディスクリプタヒープの設定
	_commandList->SetDescriptorHeaps(1, (&_descriptorHeapSRV));

	// SRVディスクリプタテーブルの設定
	_commandList->SetGraphicsRootDescriptorTable(0, _descriptorHeapSRV->GetGPUDescriptorHandleForHeapStart());

	// CBVディスクリプタヒープの設定
	_commandList->SetDescriptorHeaps(1,(&_cbvDescriptorHeap));

	// CBVディスクリプタテーブルの設定
	_commandList->SetGraphicsRootDescriptorTable(1, _cbvDescriptorHeap->GetGPUDescriptorHandleForHeapStart());
	

	/*
	_commandList->SetDescriptorHeaps(2, descriptorHeaps);

	_commandList->SetGraphicsRootDescriptorTable(0, descriptorHeapSRV->GetGPUDescriptorHandleForHeapStart());
	_commandList->SetGraphicsRootDescriptorTable(1, _cbvDescriptorHeap->GetGPUDescriptorHandleForHeapStart());
	*/

	
	// ビューポートのセット
	_commandList->RSSetViewports(1, &vp);

	// シザーレクトのセット
	_commandList->RSSetScissorRects(1, &rc);

	// 描画先バッファの取得
	backBufferIndex = _swapChain->GetCurrentBackBufferIndex();

	// リソースバリア
	_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(_renderTargets[backBufferIndex], D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));


	// 描画先変更処理
	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(_descriptorHeapRTV->GetCPUDescriptorHandleForHeapStart(), backBufferIndex, rtvDescriptorSize);
	_commandList->OMSetRenderTargets(1, &rtvHandle, false, nullptr);

	float color[4] = { 0.0f,0.0f,0.0f,1.0f };

	// カラーの変更
	_commandList->ClearRenderTargetView(rtvHandle, color, 0, nullptr);


	// プリミティブトポロジのセット
	//_commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

	//PMD表示用プリミティブトポロジのセット
	_commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_POINTLIST);

	// 頂点バッファビューのセット
	_commandList->IASetVertexBuffers(0, 1, &_vbView);

	// 頂点データのドロー
	//_commandList->DrawInstanced(4, 1, 0, 0);
	_commandList->DrawInstanced(MMDLoader::Instance()->GetVertexData().size(), 1, 0, 0);
	
	// リソースバリア
	_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(_renderTargets[backBufferIndex], D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));

	// コマンドの記録終了
	_commandList->Close();


	// コマンドリストの実行
	_commandQueue->ExecuteCommandLists(1, (ID3D12CommandList* const*)&_commandList);

	// フェンスによる「待ち」の処理実装
	++_fenceValue;
	_commandQueue->Signal(_fence, _fenceValue);
	while (_fence->GetCompletedValue() != _fenceValue)
	{
	}


	// スワップ
	_swapChain->Present(1, 0);
}

void Dx12Device::WaitForGPU()
{
	_commandQueue->Signal(_fence, _fenceValue);
	auto beforeFenceValue = _fenceValue++;
	if (_fence->GetCompletedValue() < _fenceValue)
	{
		HANDLE fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
		_fence->SetEventOnCompletion(beforeFenceValue, fenceEvent);
		WaitForSingleObject(fenceEvent, INFINITE);
	}
}

void Dx12Device::Release()
{
	_dev->Release();
}

