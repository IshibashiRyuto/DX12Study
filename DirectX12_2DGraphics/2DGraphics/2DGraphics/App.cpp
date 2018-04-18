#include "App.h"



App::App()
{
}


App::~App()
{
}

bool App::CreateDevice(HWND hwnd)
{	
	// D3D12����������
	HRESULT result = S_OK;			// ���U���g�m�F
	_hwnd = hwnd;

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
	{
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
	}

	// �R�}���h�A���P�[�^�̐�������
	{
		result = _dev->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&_commandAllocator));
		if (result != S_OK)
		{
			MessageBox(nullptr, TEXT("Failed Create CommandAllocator."), TEXT("Failed"), MB_OK);
			return false;
		}
	}


	// �R�}���h�L���[�̐�������
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


	// �X���b�v�`�F�C����������
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



	// RTV�f�B�X�N���v�^��������
	{
		// �f�B�X�N���v�^�q�[�v�p�p�����^��`
		D3D12_DESCRIPTOR_HEAP_DESC descriptorHeapDesc = {};
		descriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
		descriptorHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		descriptorHeapDesc.NumDescriptors = FRAME_CNT;

		//rtv�f�B�X�N���v�^�q�[�v����

		result = _dev->CreateDescriptorHeap(&descriptorHeapDesc, IID_PPV_ARGS(&_rtvDescriptorHeap));
		if (FAILED(result))
		{
			MessageBox(nullptr, TEXT("Failed Create Descriptor Heap."), TEXT("Failed"), MB_OK);
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
	/*�[�x�o�b�t�@�쐬*/
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

	/*�[�x�o�b�t�@�r���[�̍쐬*/
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
