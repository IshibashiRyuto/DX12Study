#include "App.h"
#include <math.h>
#include <time.h>
#include <random>

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
{// D3D12����������
	HRESULT result = S_OK;			// ���U���g�m�F
	_hwnd = hWnd;

	// DXGIFactory�̐���
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
		// �f�o�b�O���C���[�̗L����
		ID3D12Debug* debugController;
		if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController))))
		{
			debugController->EnableDebugLayer();
			debugController->Release();
			debugController = nullptr;
		}
	}
#endif


	// D3D12�f�o�C�X�̐���
	if (!CreateDevice())
	{
		return false;
	}

	// �R�}���h�A���P�[�^�̐�������
	if(!CreateCommandAllocator())
	{
		return false;
	}


	// �R�}���h�L���[�̐�������
	if (!CreateCommandQueue())
	{
		return false;
	}


	// �X���b�v�`�F�C����������
	if (!CreateSwapchain())
	{
		return false;
	}



	// RTV�f�B�X�N���v�^��������
	if (!CreateRenderTargetView())
	{
		return false;
	}

	/*�[�x�o�b�t�@�쐬*/
	if (!CreateDepthBuffer())
	{
		return false;
	}

	/*�[�x�o�b�t�@�r���[�̍쐬*/
	if (!CreateDepthStencilView())
	{
		return false;
	}

	// �T���v���̐ݒ�
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

	// ���[�g�V�O�l�`���̍쐬
	if (!CreateRootSignature())
	{
		return false;
	}

	// �t�F���X�̍쐬
	if (!CreateFence())
	{
		return false;
	}

	// ���\�[�X�̍쐬
	if (!CreateResource())
	{
		return false;
	}


	return true;
}

void App::Render()
{
	// �R�}���h���X�g�̏���������
	_commandAllocator->Reset();
	_commandList->Reset(_commandAllocator.Get(), _pipelineStateObject.Get());
	_commandList->SetGraphicsRootSignature(_rootSignature.Get());

	// �`��͈͂̐ݒ�
	D3D12_VIEWPORT vp = { 0,0,WINDOW_WIDTH,WINDOW_HEIGHT,0.0f,1.0f };
	D3D12_RECT rc = { 0,0,WINDOW_WIDTH, WINDOW_HEIGHT };
	_commandList->RSSetViewports(1, &vp);
	_commandList->RSSetScissorRects(1, &rc);

	//�`���ύX����
	int backBufferIndex = _swapChain->GetCurrentBackBufferIndex();
	_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(_renderTargets[backBufferIndex].Get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));
	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(_rtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart(), backBufferIndex, _rtvDescriptorSize);
	CD3DX12_CPU_DESCRIPTOR_HANDLE dsvHandle(_dsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart());
	_commandList->OMSetRenderTargets(1, &rtvHandle, false, &dsvHandle);

	float color[4] = { 0.0f,0.0f,0.0f,1.0f };

	// �`�揉��������
	_commandList->ClearRenderTargetView(rtvHandle, color, 0, nullptr);
	_commandList->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_DEPTH, _depthClearValue.DepthStencil.Depth, 0, 0, nullptr);
	//_commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

	// CBV
	_commandList->SetDescriptorHeaps(1, _icbDescHeap.GetAddressOf());
	_commandList->SetGraphicsRootDescriptorTable(0, _icbDescHeap->GetGPUDescriptorHandleForHeapStart());

	// �o���h���̎��s
	_commandList->ExecuteBundle(_bundle.Get());

	// �`��I������
	_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(_renderTargets[backBufferIndex].Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));
	_commandList->Close();

	// �`��R�}���h�̎��s
	ID3D12CommandList* commandLists[] = { _commandList.Get() };
	_commandQueue->ExecuteCommandLists(_countof(commandLists), commandLists);

	// �t�F���X�ɂ��ҋ@����
	++_fenceValue;
	_commandQueue->Signal(_fence.Get(), _fenceValue);
	while (_fence->GetCompletedValue() != _fenceValue)
	{
	}


	// ��ʂ̃X���b�v
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
	};	// �t�B�[�`�����x���z��

	for (auto l : levels)
	{
		result = D3D12CreateDevice(nullptr, l, IID_PPV_ARGS(&_dev));
		if (result == S_OK)	// �ŐV�̃t�B�[�`���[���x���Ńf�o�C�X�𐶐��o������
		{
			// ���x����ۑ����ă��[�v��E�o
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
	DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};//�X���b�v�`�F�C���ݒ�̍���

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
		// �f�B�X�N���v�^�q�[�v�p�p�����^��`
		D3D12_DESCRIPTOR_HEAP_DESC descriptorHeapDesc = {};
		descriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
		descriptorHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		descriptorHeapDesc.NumDescriptors = FRAME_CNT;

		//rtv�f�B�X�N���v�^�q�[�v����

		auto result = _dev->CreateDescriptorHeap(&descriptorHeapDesc, IID_PPV_ARGS(&_rtvDescriptorHeap));
		if (FAILED(result))
		{
			MessageBox(nullptr, TEXT("Failed Create RTV Descriptor Heap."), TEXT("Failed"), MB_OK);
			return false;
		}

		// rtv�f�B�X�N���v�^�q�[�v�̃T�C�Y���擾
		UINT descriptorHeapSize = 0;
		descriptorHeapSize = _dev->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);


		//�����_�[�^�[�Q�b�g�r���[�̐���
		DXGI_SWAP_CHAIN_DESC swcDesc = {};				// �X���b�v�`�F�C���̏��
		_swapChain->GetDesc(&swcDesc);

		// �����_�[�^�[�Q�b�g�����擾
		int renderTargetsNum = FRAME_CNT;

		// �����_�[�^�[�Q�b�g�������̈�m��
		_renderTargets.resize(renderTargetsNum);

		// �f�B�X�N���v�^�������̃T�C�Y���擾
		_rtvDescriptorSize = _dev->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
		{
			// �f�B�X�N���v�^�n���h���̍쐬
			CD3DX12_CPU_DESCRIPTOR_HANDLE descriptorHandle(_rtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart());

			for (UINT i = 0; i < (UINT)renderTargetsNum; i++)
			{
				result = _swapChain->GetBuffer(i, IID_PPV_ARGS(&_renderTargets[i]));	//�X���b�v�`�F�C������L�����o�X���擾
				if (FAILED(result))
				{
					MessageBox(nullptr, TEXT("Failed Create RTV."), TEXT("Failed"), MB_OK);
					return false;
				}
				_dev->CreateRenderTargetView(_renderTargets[i].Get(), nullptr, descriptorHandle);
				descriptorHandle.Offset(1, _rtvDescriptorSize);//�f�B�X�N���v�^�ƃL�����o�X���I�t�Z�b�g
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

	_depthClearValue.DepthStencil.Depth = 1.0f;		// �[���ő�l
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
	ID3DBlob* signature = nullptr;					// �V�O�l�`��
	ID3DBlob* error = nullptr;

	// �T���v��
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

	std::vector<D3D12_ROOT_PARAMETER> rootParam;	// ���[�g�p�����[�^

	// CBV�p�f�B�X�N���v�^�����W�̐ݒ�
	D3D12_DESCRIPTOR_RANGE range;
	range.NumDescriptors = 1;
	range.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
	range.BaseShaderRegister = 0;
	range.RegisterSpace = 0;
	range.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	// SRV�p�f�B�X�N���v�^�����W�̐ݒ�
	D3D12_DESCRIPTOR_RANGE descriptorRangeSRV;
	descriptorRangeSRV.NumDescriptors = 1;
	descriptorRangeSRV.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	descriptorRangeSRV.BaseShaderRegister = 0;
	descriptorRangeSRV.RegisterSpace = 0;
	descriptorRangeSRV.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	// CBV�p���[�g�p�����[�^�̐ݒ�
	D3D12_ROOT_PARAMETER prm;
	prm.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	prm.DescriptorTable = { 1,&range };
	prm.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	// SRV�p���[�g�p�����[�^�̐ݒ�
	D3D12_ROOT_PARAMETER prmSRV;
	prmSRV.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	prmSRV.DescriptorTable = { 1,&descriptorRangeSRV };
	prmSRV.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;


	rootParam.resize(2);
	rootParam[0] = prm;
	rootParam[1] = prmSRV;

	// ���[�g�V�O�l�`���̐ݒ�
	CD3DX12_ROOT_SIGNATURE_DESC rsd = {};
	rsd.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
	rsd.NumParameters = (UINT)rootParam.size();
	rsd.pParameters = rootParam.data();
	rsd.NumStaticSamplers = 1;
	rsd.pStaticSamplers = &_samplerDesc;


	// �V�O�l�`���̍쐬
	auto result = D3D12SerializeRootSignature(&rsd, D3D_ROOT_SIGNATURE_VERSION_1, &signature, &error);
	if (result != S_OK)
	{
		MessageBox(nullptr, TEXT("Failed Create Signature."), TEXT("Failed"), MB_OK);
		return false;
	}

	// �V�O�l�`���̏����Q�Ƃ��ă��[�g�V�O�l�`���𐶐�
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
	// ���_���C�A�E�g�̒�`
	{
		_inputLayoutDescs.push_back(D3D12_INPUT_ELEMENT_DESC{ "POSITION",  0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 });
		_inputLayoutDescs.push_back(D3D12_INPUT_ELEMENT_DESC{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT,0,D3D12_APPEND_ALIGNED_ELEMENT,D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0 });
		_inputLayoutDescs.push_back(D3D12_INPUT_ELEMENT_DESC{ "TEXCORD",  0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 });
		_inputLayoutDescs.push_back(D3D12_INPUT_ELEMENT_DESC{ "INSTANCE_INDEX", 0, DXGI_FORMAT_R32_UINT, 1,D3D12_APPEND_ALIGNED_ELEMENT,D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA, 1 });
	}

	// �V�F�[�_�̓ǂݍ���
	if (!ReadShader())
	{
		return false;
	}

	// �p�C�v���C���X�e�[�g�I�u�W�F�N�g�̐���
	if (!CreatePipelineStateObject())
	{
		return false;
	}
	
	/// �R�}���h���X�g�̐���
	if (!CreateCommandList())
	{
		return false;
	}

	// �o���h���̍쐬
	if (!CreateBundle())
	{
		return false;
	}


	// ���_�o�b�t�@�̍쐬
	if (!CreateVertexBuffer())
	{
		return false;
	}

	// �C���X�^���V���O�o�b�t�@�̍쐬
	if (!CreateInstancingBuffer())
	{
		return false;
	}
	// �e�N�X�`���o�b�t�@�̍쐬
	if (!CreateTextureBuffer())
	{
		return false;
	}

	// �e�N�X�`���f�[�^�̓ǂݍ���
	if (!LoadBitmapData())
	{
		return false;
	}

	// �V�F�[�_���\�[�X�r���[�̍쐬
	if (!CreateShaderResourceView())
	{
		return false;
	}


	// �o���h���֕`��R�}���h�𗬂�����
	SetBundle();
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
	gpsDesc.DepthStencilState.DepthEnable = TRUE;					// �[�x���g�p���邩�ǂ���
	gpsDesc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
	gpsDesc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
	gpsDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT;
	gpsDesc.DepthStencilState.StencilEnable = FALSE;				// stencil�}�X�N���g�p���邩�ǂ���
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

bool App::CreateBundle()
{
	HRESULT result = S_OK;

	result = _dev->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_BUNDLE, IID_PPV_ARGS(_bundleAllocator.GetAddressOf()));
	if (result != S_OK)
	{
		MessageBox(nullptr, TEXT("Failed Create BundleAllocator."), TEXT("Failed"), MB_OK);
		return false;
	}
	_bundleAllocator->Reset();
	result = _dev->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_BUNDLE, _bundleAllocator.Get(), _pipelineStateObject.Get(), IID_PPV_ARGS(_bundle.GetAddressOf()));
	if (result != S_OK)
	{
		MessageBox(nullptr, TEXT("Failed Create Bundle."), TEXT("Failed"), MB_OK);
		return false;
	}

	_bundle->Close();
	return true;
}

void App::SetBundle()
{
	// ������
	_bundle->Reset(_bundleAllocator.Get(), _pipelineStateObject.Get());
	_bundle->SetGraphicsRootSignature(_rootSignature.Get());

	//�Ƃۂ낶������
	_bundle->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
	// �f�[�^�Z�b�g

	//	VertexBuffer��p�����C���X�^���V���O

	D3D12_VERTEX_BUFFER_VIEW vbViews[2] = { _vertexBufferView, _instancingBufferView };
	_bundle->IASetVertexBuffers(0, 2, vbViews);


	// �`�揈��
	_bundle->DrawInstanced(4, INSTANCING_NUM, 0, 0);

	// �o���h���̃N���[�Y
	_bundle->Close();
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
	//vertexBuffer��
	auto result = _dev->CreateCommittedResource
	(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(sizeof(unsigned int)*INSTANCING_NUM),
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(_instancingBuffer.GetAddressOf()));
	
	if (FAILED(result))
	{
		MessageBox(nullptr, TEXT("Failed InstancingBuffer."), TEXT("Failed"), MB_OK);
		return false;
	}

	unsigned  *buf;
	_instancingBuffer->Map(0, nullptr, (void**)&buf);
	for (unsigned int i = 0; i < (unsigned int)INSTANCING_NUM; ++i)
	{
		*buf = i;
		++buf;
	}
	_instancingBuffer->Unmap(0, nullptr);

	_instancingBufferView.BufferLocation = _instancingBuffer->GetGPUVirtualAddress();
	_instancingBufferView.StrideInBytes = sizeof(unsigned int);
	_instancingBufferView.SizeInBytes = sizeof(unsigned int)*INSTANCING_NUM;

	//constantBuffer��
	D3D12_DESCRIPTOR_HEAP_DESC desc{};
	desc.NumDescriptors = 1;
	desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	result = _dev->CreateDescriptorHeap(&desc, IID_PPV_ARGS(_icbDescHeap.GetAddressOf()));

	if (FAILED(result))
	{
		MessageBox(nullptr, TEXT("Failed Create ConstantBuffer Descriptor Heap."), TEXT("Failed"), MB_OK);
		return false;
	}

	D3D12_HEAP_PROPERTIES heapProp = {};
	heapProp.Type = D3D12_HEAP_TYPE_UPLOAD;
	heapProp.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	heapProp.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	heapProp.VisibleNodeMask = 1;
	heapProp.CreationNodeMask = 1;

	D3D12_RESOURCE_DESC resourceDesc = {};
	resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	resourceDesc.Width = (sizeof(DirectX::XMFLOAT4) * INSTANCING_NUM + 0xff)&~0xff;		//256byteAllinment
	resourceDesc.Height = 1;
	resourceDesc.DepthOrArraySize = 1;
	resourceDesc.SampleDesc.Count = 1;
	resourceDesc.MipLevels = 1;
	resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;


	result = _dev->CreateCommittedResource(&heapProp,
		D3D12_HEAP_FLAG_NONE,
		&resourceDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(_instancingConstantBuffer.GetAddressOf()) );

	if (FAILED(result))
	{
		MessageBox(nullptr, TEXT("Failed Create ConstantBuffer."), TEXT("Failed"), MB_OK);
		return false;
	}

	D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
	D3D12_CPU_DESCRIPTOR_HANDLE cbvHandle = {};

	cbvDesc.BufferLocation = _instancingConstantBuffer->GetGPUVirtualAddress();
	cbvDesc.SizeInBytes = (sizeof(DirectX::XMFLOAT4) * INSTANCING_NUM + 0xff)&~0xff;

	cbvHandle = _icbDescHeap->GetCPUDescriptorHandleForHeapStart();
	
	_dev->CreateConstantBufferView(&cbvDesc, cbvHandle);
	

	DirectX::XMFLOAT4 *cBuf = nullptr;
	D3D12_RANGE range = {};
	result = _instancingConstantBuffer->Map(0, &range, (void**)(&cBuf));

	if (FAILED(result))
	{
		auto reason = _dev->GetDeviceRemovedReason();
		MessageBox(nullptr, TEXT("Failed Map by constantBuffer."), TEXT("Failed"), MB_OK);
		return false;
	}

	std::random_device seed_gen;
	std::mt19937 engine(seed_gen());
	std::uniform_real_distribution<float> point(-1.0f, 1.0f);

	for (int i = 0; i < INSTANCING_NUM; ++i)
	{
		float x, y;

		x = point(engine);
		y = point(engine);
		DirectX::XMFLOAT4 pos(x, y, 0.0f,0.0f);
		*cBuf = pos;
		++cBuf;
	}

	return true;
}

bool App::CreateTextureBuffer()
{
	HRESULT result = {};

	D3D12_RESOURCE_DESC texResourceDesc = {};
	texResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	texResourceDesc.Width = 256;	// ���ߑł�
	texResourceDesc.Height = 256;	// ���ߑł�
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
		nullptr, IID_PPV_ARGS(_textureBuffer.GetAddressOf()));

	if (FAILED(result))
	{
		MessageBox(nullptr, TEXT("Failed Create Texture Buffer."), TEXT("Failed"), MB_OK);
		return false;
	}

	return true;
}

bool App::LoadBitmapData()
{
	BITMAPFILEHEADER bitmapFileHeader = {};
	BITMAPINFOHEADER bitmapInfoHeader = {};
	FILE *fp;
	
	auto err = fopen_s(&fp, "texturesample.bmp", "rb");

	if (err != 0)
	{
		MessageBox(nullptr, TEXT("Failed Open Bitmap File."), TEXT("Failed"), MB_OK);
		return false;
	}

	fread(&bitmapFileHeader, sizeof(BITMAPFILEHEADER), 1, fp);
	fread(&bitmapInfoHeader, sizeof(BITMAPINFOHEADER), 1, fp);

	std::vector<char> data;
	data.resize(bitmapInfoHeader.biSizeImage);
	fread(&data[0], sizeof(char)*bitmapInfoHeader.biSizeImage, 1, fp);
	fclose(fp);
	D3D12_BOX box{ 0,0,1,256,256,0 };
	_textureBuffer->WriteToSubresource(0, &box, &data[0], 256 * sizeof(char), sizeof(char)*bitmapInfoHeader.biSizeImage);

	_commandAllocator->Reset();
	_commandList->Reset(_commandAllocator.Get(), _pipelineStateObject.Get());
	_commandList->SetGraphicsRootSignature(_rootSignature.Get());

	_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(_textureBuffer.Get(), D3D12_RESOURCE_STATE_COPY_DEST,
		D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE));
	_commandList->Close();

	ID3D12CommandList* commandLists[] = { _commandList.Get() };
	_commandQueue->ExecuteCommandLists(_countof(commandLists), commandLists);

	_commandQueue->Signal(_fence.Get(), ++_fenceValue);
	while (_fence->GetCompletedValue() != _fenceValue);
	return true;
}

bool App::CreateShaderResourceView()
{
	D3D12_DESCRIPTOR_HEAP_DESC desc{};
	desc.NumDescriptors = 1;
	desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;

	auto result = _dev->CreateDescriptorHeap(&desc, IID_PPV_ARGS(_srvDescriptorHeap.GetAddressOf()));

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
	_dev->CreateShaderResourceView(_textureBuffer.Get(), &srvDesc, srvHandle);

	return true;
}
