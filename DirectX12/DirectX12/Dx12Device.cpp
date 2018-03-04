#include "Dx12Device.h"



Dx12Device::Dx12Device()
{
}


Dx12Device::~Dx12Device()
{
}

bool Dx12Device::CreateDevice(HWND hwnd)
{
	// D3D12����������
	HRESULT result = S_OK;			// ���U���g�m�F
	_hwnd = hwnd;

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

	// DXGIFactory�̐���
	{
		result = CreateDXGIFactory1(IID_PPV_ARGS(&_factory));
		if (FAILED(result))
		{
			MessageBox(nullptr, TEXT("Failed Create Device."), TEXT("Failed"), MB_OK);
			return false;
		}
	}

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


	// RTV�f�B�X�N���v�^��������
	{
		// �f�B�X�N���v�^�q�[�v�p�p�����^��`
		D3D12_DESCRIPTOR_HEAP_DESC descriptorHeapDesc = {};
		descriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
		descriptorHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		descriptorHeapDesc.NumDescriptors = FRAME_CNT;

		//�f�B�X�N���v�^�q�[�v����
	
		result = _dev->CreateDescriptorHeap(&descriptorHeapDesc, IID_PPV_ARGS(&descriptorHeap));
		if (FAILED(result))
		{
			MessageBox(nullptr, TEXT("Failed Create Descriptor Heap."), TEXT("Failed"), MB_OK);
			return false;
		}

		// �f�B�X�N���v�^�q�[�v�̃T�C�Y���擾
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
		rtvDescriptorSize =_dev->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

		{
			// �f�B�X�N���v�^�n���h���̍쐬
			CD3DX12_CPU_DESCRIPTOR_HANDLE descriptorHandle(descriptorHeap->GetCPUDescriptorHandleForHeapStart());

			for (UINT i = 0; i < (UINT)renderTargetsNum; i++)
			{
				result = _swapChain->GetBuffer(i, IID_PPV_ARGS(&_renderTargets[i]));	//�X���b�v�`�F�C������L�����o�X���擾
				if (FAILED(result))
				{
					MessageBox(nullptr, TEXT("Failed Create RTV."), TEXT("Failed"), MB_OK);
					return false;
				}
				_dev->CreateRenderTargetView(_renderTargets[i], nullptr, descriptorHandle);
				descriptorHandle.Offset(1, rtvDescriptorSize);//�f�B�X�N���v�^�ƃL�����o�X���I�t�Z�b�g
			}
		}
	}

	// ���[�g�V�O�l�`���̍쐬
	{
		ID3DBlob* signature = nullptr;					// �V�O�l�`��
		ID3DBlob* error = nullptr;

		CD3DX12_ROOT_SIGNATURE_DESC rsd = {};
		rsd.Init(0, nullptr, 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

		// �V�O�l�`���̍쐬
		result = D3D12SerializeRootSignature(&rsd, D3D_ROOT_SIGNATURE_VERSION_1, &signature, &error);
		if (result != S_OK)
		{
			MessageBox(nullptr, TEXT("Failed Create Signature."), TEXT("Failed"), MB_OK);
			return false;
		}

		// �V�O�l�`���̏����Q�Ƃ��ă��[�g�V�O�l�`���𐶐�
		result = _dev->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&_rootSignature));
		if (result != S_OK)
		{
			MessageBox(nullptr, TEXT("Failed Create RootSignature."), TEXT("Failed"), MB_OK);
			return false;
		}
	}

	// ���_���C�A�E�g�̒�`
	
	_inputLayoutDescs.push_back(D3D12_INPUT_ELEMENT_DESC{ "POSITION",  0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 });
	_inputLayoutDescs.push_back(D3D12_INPUT_ELEMENT_DESC{ "TEXCORD",  0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 });


	// �V�F�[�_�̓ǂݍ���
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

	// PSO������
	{
		D3D12_GRAPHICS_PIPELINE_STATE_DESC gpsDesc = {};

		gpsDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
		gpsDesc.DepthStencilState.DepthEnable = FALSE;					// �[�x���g�p���邩�ǂ���
		gpsDesc.DepthStencilState.StencilEnable = FALSE;				// �}�X�N���g�p���邩�ǂ���
		gpsDesc.VS = CD3DX12_SHADER_BYTECODE(vertexShader);
		gpsDesc.PS = CD3DX12_SHADER_BYTECODE(pixelShader);
		gpsDesc.InputLayout.NumElements = (UINT)_inputLayoutDescs.size();
		gpsDesc.InputLayout.pInputElementDescs = &_inputLayoutDescs[0];
		gpsDesc.pRootSignature = _rootSignature;
		gpsDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
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

	// �R�}���h���X�g�̐�������
	{
		result = _dev->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, _commandAllocator, _pipelineStateObject, IID_PPV_ARGS(&_commandList));
		if (result != S_OK)
		{
			MessageBox(nullptr, TEXT("Failed Create CommandLine."), TEXT("Failed"), MB_OK);
			return false;
		}
	}
	// �R�}���h�̋L�^�I��
	{
		result = _commandList->Close();
		if (result != S_OK)
		{
			MessageBox(nullptr, TEXT("Failed CommandLine Close."), TEXT("Failed"), MB_OK);
			return false;
		}
	}

	/*�O�p�|���̕\��*/
	// �O�p�|���̒��_��`
	/*Vertex vertices[] = { { { 0.0f,0.7f,0.0f } },
	{ { 0.4f,-0.5f,0.0f } },
	{ { -0.4f,-0.5f,0.0f } }, };
	*/

	Vertex vertices[] = { 
		{ { WINDOW_WIDTH / 2 - 128.0f, WINDOW_HEIGHT / 2 - 128.0f,0.0f },{0.0f,0.0f} },
		{ { WINDOW_WIDTH / 2 + 128.0f, WINDOW_HEIGHT / 2 - 128.0f,0.0f },{0.0f,1.0f} },
		{ { WINDOW_WIDTH / 2 - 128.0f, WINDOW_HEIGHT / 2 + 128.0f,0.0f },{1.0f,0.0f} } ,
		{ { WINDOW_WIDTH / 2 + 128.0f, WINDOW_HEIGHT / 2 + 128.0f, 0.0f },{1.0f,1.0f} },
	};



	// ���_�o�b�t�@�쐬
	{
		result = _dev->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
			D3D12_HEAP_FLAG_NONE,
			&CD3DX12_RESOURCE_DESC::Buffer(sizeof(vertices)),
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&_vertexBuffer));

		if (FAILED(result))
		{
			MessageBox(nullptr, TEXT("Failed Create VertexBuffer."), TEXT("Failed"), MB_OK);
			return false;
		}
	}
	// ���_�̃}�b�v
	char* buf;
	_vertexBuffer->Map(0, nullptr, (void**)&buf);
	memcpy(buf, vertices, sizeof(vertices));
	_vertexBuffer->Unmap(0, nullptr);

	/*
	FILE *fp;
	fopen_s(&fp, "memoryDump.bin", "wb");
	fwrite(buf, sizeof(vertices), 1, fp);
	fclose(fp);
	*/

	// ���_�o�b�t�@�r���[�̐錾
	_vbView.BufferLocation = _vertexBuffer->GetGPUVirtualAddress();
	_vbView.StrideInBytes = sizeof(Vertex);
	_vbView.SizeInBytes = sizeof(vertices);


	// �t�F���X�̎���
	result = _dev->CreateFence(_fenceValue, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&_fence));
	if (result != S_OK)
	{
		MessageBox(nullptr, TEXT("Failed Create Fence."), TEXT("Failed"), MB_OK);
		return false;
	}

	WaitForGPU();

	return true;
}

void Dx12Device::Render()
{
	// �R�}���h�A���P�[�^�ƃ��X�g�̃��Z�b�g����
	_commandAllocator->Reset();
	_commandList->Reset(_commandAllocator, _pipelineStateObject);

	// �f�B�X�N���v�^�q�[�v�̐ݒ�
	//_commandList->SetDescriptorHeaps(1, (ID3D12DescriptorHeap* const*)(&descriptorHeap));


	// ���[�g�V�O�j�`����ݒ�
	_commandList->SetGraphicsRootSignature(_rootSignature);

	// �f�B�X�N���v�^�q�[�v�e�[�u���̐ݒ�
	//_commandList->SetGraphicsRootDescriptorTable(0, descriptorHeap->GetGPUDescriptorHandleForHeapStart());


	// �r���[�|�[�g�̃Z�b�g
	_commandList->RSSetViewports(1, &vp);

	// �V�U�[���N�g�̃Z�b�g
	_commandList->RSSetScissorRects(1, &rc);

	// �`���o�b�t�@�̎擾
	backBufferIndex = _swapChain->GetCurrentBackBufferIndex();

	// ���\�[�X�o���A
	_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(_renderTargets[backBufferIndex], D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));


	// �`���ύX����
	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(descriptorHeap->GetCPUDescriptorHandleForHeapStart(), backBufferIndex, rtvDescriptorSize);
	_commandList->OMSetRenderTargets(1, &rtvHandle, false, nullptr);

	float color[4] = { 1.0f,0.0f,0.0f,1.0f };

	// �J���[�̕ύX
	_commandList->ClearRenderTargetView(rtvHandle, color, 0, nullptr);


	// �v���~�e�B�u�g�|���W�̃Z�b�g
	_commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

	// ���_�o�b�t�@�r���[�̃Z�b�g
	_commandList->IASetVertexBuffers(0, 1, &_vbView);

	// ���_�f�[�^�̃h���[
	_commandList->DrawInstanced(4, 1, 0, 0);

	// ���\�[�X�o���A
	_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(_renderTargets[backBufferIndex], D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));

	// �R�}���h�̋L�^�I��
	_commandList->Close();


	// �R�}���h���X�g�̎��s
	_commandQueue->ExecuteCommandLists(1, (ID3D12CommandList* const*)&_commandList);

	// �t�F���X�ɂ��u�҂��v�̏�������
	++_fenceValue;
	_commandQueue->Signal(_fence, _fenceValue);
	while (_fence->GetCompletedValue() != _fenceValue)
	{
	}


	// �X���b�v
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

