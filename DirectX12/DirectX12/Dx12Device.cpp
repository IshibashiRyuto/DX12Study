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
		rtvDescriptorSize =_dev->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
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


		// ルートシグネチャの設定
		CD3DX12_ROOT_SIGNATURE_DESC rsd = {};
		//rsd.Init(0, nullptr, 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);
		rsd.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
		rsd.NumParameters = (UINT)rootParam.size();
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

	// PMDデータの読み込み
	MMDLoader::Instance()->Load();

	// 頂点レイアウトの定義
	{
		_inputLayoutDescs.push_back(D3D12_INPUT_ELEMENT_DESC{ "POSITION",  0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 });
		_inputLayoutDescs.push_back(D3D12_INPUT_ELEMENT_DESC{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT,0,D3D12_APPEND_ALIGNED_ELEMENT,D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0 });
		//_inputLayoutDescs.push_back(D3D12_INPUT_ELEMENT_DESC{ "TEXCORD",  0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 });

	}
	
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
		gpsDesc.DepthStencilState.DepthEnable = TRUE;					// 深度を使用するかどうか
		gpsDesc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL; 
		gpsDesc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
		gpsDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT;
		gpsDesc.DepthStencilState.StencilEnable = FALSE;				// マスクを使用するかどうか
		gpsDesc.VS = CD3DX12_SHADER_BYTECODE(vertexShader);
		gpsDesc.PS = CD3DX12_SHADER_BYTECODE(pixelShader);
		gpsDesc.InputLayout.NumElements = (UINT)_inputLayoutDescs.size();
		gpsDesc.InputLayout.pInputElementDescs = &_inputLayoutDescs[0];
		gpsDesc.pRootSignature = _rootSignature;
		gpsDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
		gpsDesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
		gpsDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
		gpsDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
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

	// 頂点バッファ作成
	{

		// MMDデータ
		auto pmdVertexData = MMDLoader::Instance()->GetVertexData();
		auto pmdIndexData = MMDLoader::Instance()->GetIndexData();

		result = _dev->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
			D3D12_HEAP_FLAG_NONE,
			&CD3DX12_RESOURCE_DESC::Buffer(sizeof(PmdVertex) * pmdVertexData.size()),
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&_vertexBuffer));

		if (FAILED(result))
		{
			MessageBox(nullptr, TEXT("Failed Create VertexBuffer."), TEXT("Failed"), MB_OK);
			return false;
		}

	   // 頂点バッファビューの宣言
		_vbView.BufferLocation = _vertexBuffer->GetGPUVirtualAddress();
		_vbView.StrideInBytes = sizeof(PmdVertex);
		_vbView.SizeInBytes = (UINT)(pmdVertexData.size() * sizeof(PmdVertex));

		// 頂点のマップ
		char* buf;
		_vertexBuffer->Map(0, nullptr, (void**)&buf);
		memcpy(buf, &pmdVertexData[0], pmdVertexData.size() * sizeof(PmdVertex));
		_vertexBuffer->Unmap(0, nullptr);


		// 頂点インデックスバッファ作成
		result = _dev->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
			D3D12_HEAP_FLAG_NONE,
			&CD3DX12_RESOURCE_DESC::Buffer(sizeof(unsigned short) * pmdIndexData.size()),
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&_verIndexBuffer));

		if (FAILED(result))
		{
			MessageBox(nullptr, TEXT("Failed Create VertexIndexBuffer."), TEXT("Failed"), MB_OK);
			return false;
		}

		// 頂点インデックスバッファビューの宣言
		_indexBufferView.BufferLocation = _verIndexBuffer->GetGPUVirtualAddress();
		_indexBufferView.Format = DXGI_FORMAT_R16_UINT;
		_indexBufferView.SizeInBytes = (UINT)(pmdIndexData.size() * sizeof(short));

		// 頂点インデックスのマップ
		_verIndexBuffer->Map(0, nullptr, (void**)&buf);
		memcpy(buf, &pmdIndexData[0], pmdIndexData.size() * sizeof(short));
		_verIndexBuffer->Unmap(0, nullptr);

		/*
		// マップした頂点インデックスをバイナリとして出力
		FILE *fp;
		fopen_s(&fp, "indexData.bin", "wb");
		fwrite(buf, sizeof(short), pmdIndexData.size(), fp);
		fclose(fp);
		*/

	}

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

		result = _dev->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&_srvDescriptorHeap));

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

		srvHandle = _srvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
		_dev->CreateShaderResourceView(_textureBuffer, &srvDesc, srvHandle);
	}

	//定数バッファの生成
	{
		
		world = DirectX::XMMatrixIdentity();
		eye = { 0,10,-10 };
		target = { 0,10,0 };
		upper = { 0,-1,0 };
		camera = DirectX::XMMatrixLookAtLH(eye, target, upper);

		projection = DirectX::XMMatrixPerspectiveFovLH(3.14159265f/ 4.0f * 5.5f , 1.0f, 0.1f, 50.f);

		matrix.world = world;
		matrix.viewproj = camera * projection;


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
		cbvResourceDesc.Width = (sizeof(BaseMatrixes) + 0xff)&~0xff;
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
		cbvDesc.SizeInBytes = (sizeof(BaseMatrixes) + 0xff)& ~0xff;
	
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
	
	// material用定数バッファ作成
	{
		// モデルデータ取得
		auto materials = MMDLoader::Instance()->GetMaterialData();

		D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
		heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
		heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		heapDesc.NumDescriptors = materials.size();
		result = _dev->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&_materialsCbvDescriptorHeap));

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
		cbvResourceDesc.Width = ( (sizeof(Material::diffuseColor) + 0xff)&~0xff) * materials.size();
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
			IID_PPV_ARGS(&_materialsConstantBuffer));


		D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
		D3D12_CPU_DESCRIPTOR_HANDLE cbvHandle = {};


		cbvDesc.BufferLocation = _materialsConstantBuffer->GetGPUVirtualAddress();
		auto bufferSize = (sizeof(Material::diffuseColor) + 0xff)&~0xff;
		cbvDesc.SizeInBytes = bufferSize;


		cbvHandle = _materialsCbvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
		
		for (int i = 0; i < materials.size(); i++)
		{
			_dev->CreateConstantBufferView(&cbvDesc, cbvHandle);
			cbvDesc.BufferLocation += bufferSize;
			cbvHandle.ptr += _dev->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
		}

		D3D12_RANGE range = {};
		result = _materialsConstantBuffer->Map(0, &range, (void**)(&_diffuseColorAddress));
		if (FAILED(result))
		{
			MessageBox(nullptr, TEXT("Failed Map by constantBuffer."), TEXT("Failed"), MB_OK);
			return false;
		}

		for (int i = 0; i < materials.size(); i++)
		{
			memcpy( (_diffuseColorAddress + i), &materials[i].diffuseColor, sizeof(Material::diffuseColor));
		}
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
	matrix.world = world;
	*_matrixAddress = matrix;
	
	// コマンドアロケータとリストのリセット処理
	_commandAllocator->Reset();
	_commandList->Reset(_commandAllocator, _pipelineStateObject);


	// ルートシグニチャを設定
	_commandList->SetGraphicsRootSignature(_rootSignature);


	ID3D12DescriptorHeap* descriptorHeaps[] = { _cbvDescriptorHeap, _materialsCbvDescriptorHeap };

	// SRVディスクリプタヒープの設定
	_commandList->SetDescriptorHeaps(1, (&_srvDescriptorHeap));

	// SRVディスクリプタテーブルの設定
	_commandList->SetGraphicsRootDescriptorTable(0, _srvDescriptorHeap->GetGPUDescriptorHandleForHeapStart());

	// CBVディスクリプタヒープの設定
	//_commandList->SetDescriptorHeaps(2, descriptorHeaps);
	_commandList->SetDescriptorHeaps(1, &_cbvDescriptorHeap);

	// CBVディスクリプタテーブルの設定
	_commandList->SetGraphicsRootDescriptorTable(1, _cbvDescriptorHeap->GetGPUDescriptorHandleForHeapStart());
	

	/*
	_commandList->SetDescriptorHeaps(2, descriptorHeaps);

	_commandList->SetGraphicsRootDescriptorTable(0, _descriptorHeapSRV->GetGPUDescriptorHandleForHeapStart());
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
	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(_rtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart(), backBufferIndex, rtvDescriptorSize);
	CD3DX12_CPU_DESCRIPTOR_HANDLE dsvHandle(_dsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart());
	_commandList->OMSetRenderTargets(1, &rtvHandle, false, &dsvHandle);

	float color[4] = { 0.0f,0.0f,0.0f,1.0f };

	// レンダーターゲットビューの初期化
	_commandList->ClearRenderTargetView(rtvHandle, color, 0, nullptr);

	// dsvのクリア
	_commandList->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_DEPTH, _depthClearValue.DepthStencil.Depth, 0, 0, nullptr);

	//PMD表示用プリミティブトポロジのセット
	_commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// 頂点バッファビューのセット
	_commandList->IASetVertexBuffers(0, 1, &_vbView);

	// 頂点インデックスバッファビューをセット
	_commandList->IASetIndexBuffer(&_indexBufferView);

	// 頂点データのドロー
	
	UINT vertexOffset = 0;
	auto materials = MMDLoader::Instance()->GetMaterialData();
	
	auto handle = _materialsCbvDescriptorHeap->GetGPUDescriptorHandleForHeapStart();
	//_commandList->SetDescriptorHeaps(1, &_materialsCbvDescriptorHeap);

	for (int i = 0; i < materials.size(); i++)
	{
		//*_diffuseColorAddress = materials[i].diffuseColor;
		//_commandList->SetGraphicsRootDescriptorTable(2, handle);
		//handle.ptr += _dev->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

		_commandList->DrawIndexedInstanced(materials[i].faceVertexCount, 1, vertexOffset, 0, 0);
		vertexOffset += materials[i].faceVertexCount;
		
	}
	/*
	UINT indexNum = MMDLoader::Instance()->GetIndexData().size();
	_commandList->DrawIndexedInstanced(indexNum, 1, 0, 0, 0);
	*/
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

