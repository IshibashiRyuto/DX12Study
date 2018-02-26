#include <Windows.h>

#include <d3d12.h>
#include "d3dx12.h"
#include <dxgi1_4.h>
#include <D3Dcompiler.h>
#include <DirectXMath.h>
#include <vector>

using namespace DirectX;

// �萔��`
#define WINDOW_WIDTH 640
#define WINDOW_HEIGHT 480

// �\���̒�`
typedef struct VERTEX {
	XMFLOAT3 pos;		//���W
}Vertex;

LRESULT WindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);

int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrevInst, LPSTR lpStr, int nCmdShow)
{

	// �E�B���h�E�N���X�̓o�^
	WNDCLASSEX w = {};
	w.cbSize = sizeof(WNDCLASSEX);
	w.lpfnWndProc = (WNDPROC)WindowProc;
	w.lpszClassName = TEXT("DirectXTest");
	w.hInstance = GetModuleHandle(0);
	RegisterClassEx(&w);

	// �E�B���h�E�T�C�Y�w��
	RECT wrc = { 0,0,WINDOW_WIDTH, WINDOW_HEIGHT };		// �E�B���h�E�T�C�Y�̎w��
	AdjustWindowRect(&wrc, WS_OVERLAPPEDWINDOW, false);//�E�B���h�E�T�C�Y�̕␳

	//�E�B���h�E��������
	HWND hwnd;
	hwnd = CreateWindow(w.lpszClassName,
		TEXT("DX12�e�X�g"),
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		wrc.right - wrc.left,
		wrc.bottom - wrc.top,
		nullptr,
		nullptr,
		w.hInstance,
		nullptr);

	// �E�B���h�E�\��
	ShowWindow(hwnd, SW_SHOW);

	// D3D12����������
	bool isInit = true;		// ���������������ʂ���t���O

	// D3D12�f�o�C�X�̐���

	ID3D12Device *dev = nullptr;		// �f�o�C�X�|�C���^
	HRESULT result = S_OK;
	D3D_FEATURE_LEVEL levels[] = {
		D3D_FEATURE_LEVEL_12_1,
		D3D_FEATURE_LEVEL_12_0,
		D3D_FEATURE_LEVEL_11_1,
		D3D_FEATURE_LEVEL_11_0,
	};	// �t�B�[�`�����x���z��

	D3D_FEATURE_LEVEL level = {};		// �f�o�C�X�Ŏg�p�\�ȃt�B�[�`���[���x��

	for (auto l : levels)
	{
		result = D3D12CreateDevice(nullptr, l, IID_PPV_ARGS(&dev));
		if (result == S_OK)	// �ŐV�̃t�B�[�`���[���x���Ńf�o�C�X�𐶐��o������
		{
			// ���x����ۑ����ă��[�v��E�o
			level = l;
			break;
		}
	}

	if (result != S_OK)
	{
		isInit = false;
	}


	//�R�}���h�֘A�ϐ���`
	ID3D12CommandAllocator* _commandAllocator = nullptr;	//�R�}���h�A���P�[�^
	ID3D12CommandQueue* _commandQueue = nullptr;			//�R�}���h�L���[
	ID3D12GraphicsCommandList * _commandList = nullptr;		// �R�}���h���X�g

	// �R�}���h�A���P�[�^�̐�������
	result = dev->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&_commandAllocator));
	if (result != S_OK)
	{
		isInit = false;
	}

	// �R�}���h���X�g�̐�������
	result = dev->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, _commandAllocator, nullptr, IID_PPV_ARGS(&_commandList));
	if (result != S_OK)
	{
		isInit = false;
	}

	// �R�}���h�L���[�̐�������
	D3D12_COMMAND_QUEUE_DESC desc = {};
	desc.NodeMask = 0;
	desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	desc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
	desc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
	result = dev->CreateCommandQueue(&desc, IID_PPV_ARGS(&_commandQueue));
	if (result != S_OK)
	{
		isInit = false;
	}

	// �X���b�v�`�F�C����������
	IDXGIFactory4 * factory = nullptr;		// DXGI�C���^�t�F�[�X�̐����@
	result = CreateDXGIFactory1(IID_PPV_ARGS(&factory));
	if (result != S_OK)
	{
		isInit = false;
	}

	DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};//�X���b�v�`�F�C���ݒ�̍��ځH
	IDXGISwapChain3 *swapChain;						// �X���b�v�`�F�C��

	swapChainDesc.Width = WINDOW_WIDTH;
	swapChainDesc.Height = WINDOW_HEIGHT;
	swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainDesc.Stereo = false;
	swapChainDesc.SampleDesc.Count = 1;
	swapChainDesc.SampleDesc.Quality = 0;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.BufferCount = 2;
	swapChainDesc.Scaling = DXGI_SCALING_STRETCH;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	swapChainDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
	swapChainDesc.Flags = 0;


	// �X���b�v�`�F�[������
	result = factory->CreateSwapChainForHwnd(_commandQueue,
		hwnd,
		&swapChainDesc,
		nullptr,
		nullptr,
		(IDXGISwapChain1**)(&swapChain));
	if (result != S_OK)
	{
		isInit = false;
	}

	// �f�B�X�N���v�^��������
	// �f�B�X�N���v�^�q�[�v�p�p�����^��`
	D3D12_DESCRIPTOR_HEAP_DESC descriptorHeapDesc = {};
	descriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	descriptorHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	descriptorHeapDesc.NumDescriptors = 2;
	descriptorHeapDesc.NodeMask = 0;
	//�f�B�X�N���v�^�q�[�v����
	ID3D12DescriptorHeap* descriptorHeap = nullptr;
	result = dev->CreateDescriptorHeap(&descriptorHeapDesc, IID_PPV_ARGS(&descriptorHeap));
	if (result != S_OK)
	{
		isInit = false;
	}

	// �f�B�X�N���v�^�q�[�v�̃T�C�Y���擾
	UINT descriptorHeapSize = 0;
	descriptorHeapSize = dev->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

	// �f�B�X�N���v�^�n���h���̍쐬
	CD3DX12_CPU_DESCRIPTOR_HANDLE descriptorHandle(descriptorHeap->GetCPUDescriptorHandleForHeapStart());


	//�����_�[�^�[�Q�b�g�r���[�̐���
	std::vector<ID3D12Resource*> renderTargets;		//�����_�[�^�[�Q�b�g�̎���
	DXGI_SWAP_CHAIN_DESC swcDesc = {};				// �X���b�v�`�F�C���̏��
	swapChain->GetDesc(&swcDesc);

	// �����_�[�^�[�Q�b�g�����擾
	int renderTargetsNum = swcDesc.BufferCount;

	// �����_�[�^�[�Q�b�g�������̈�m��
	renderTargets.resize(renderTargetsNum);

	// �f�B�X�N���v�^�������̃T�C�Y���擾
	int descriptorSize = dev->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

	for (int i = 0; i < renderTargetsNum; i++)
	{
		result = swapChain->GetBuffer(i, IID_PPV_ARGS(&renderTargets[i]));	//�X���b�v�`�F�C������L�����o�X���擾
		if (result != S_OK)
		{
			isInit = false;
		}
		dev->CreateRenderTargetView(renderTargets[i], nullptr, descriptorHandle);
		descriptorHandle.Offset(descriptorSize);//�f�B�X�N���v�^�ƃL�����o�X���I�t�Z�b�g
	}


	// ���[�g�V�O�l�`���̍쐬
	ID3D12RootSignature* rootSignature = nullptr;	//���[�g�V�O�l�`��
	ID3DBlob* signature = nullptr;					// �V�O�l�`��
	ID3DBlob* error = nullptr;

	D3D12_ROOT_SIGNATURE_DESC rsd = {};
	rsd.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
	// �V�O�l�`���̍쐬
	result = D3D12SerializeRootSignature(&rsd, D3D_ROOT_SIGNATURE_VERSION_1, &signature, &error);
	if (result != S_OK)
	{
		isInit = false;
	}

	// �V�O�l�`���̏����Q�Ƃ��ă��[�g�V�O�l�`���𐶐�
	result = dev->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&rootSignature));
	if (result != S_OK)
	{
		isInit = false;
	}

	// �t�F���X�̎���
	ID3D12Fence *_fence = nullptr;
	UINT64 _fenceValue = 0;
	result = dev->CreateFence(_fenceValue, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&_fence));
	if (result != S_OK)
	{
		isInit = false;
	}

	if (!isInit)
	{
		MessageBox(hwnd, TEXT("DirectX�̏������Ɏ��s���܂���"), TEXT("���������s"), MB_OK);
		return -1;
	}

	/*
#if defined(_DEBUG)
	// DirectX12�̃f�o�b�O���C���[��L���ɂ���
	{
		ID3D12Debug	*debugController;
		if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController)))) {
			debugController->EnableDebugLayer();
		}
	}
#endif
*/

	/*�O�p�|���̕\��*/
	// �O�p�|���̒��_��`
	Vertex vertices[] = { { { 0.0f,0.0f,0.0f } },
	{ { 1.0f,0.0f,0.0f } },
	{ { 0.0f,-1.0f,0.0f } } };

	// ���_���C�A�E�g�̒�`
	D3D12_INPUT_ELEMENT_DESC element[] = {
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, D3D12_APPEND_ALIGNED_ELEMENT, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0, }
	};

	// ���_�o�b�t�@�쐬
	ID3D12Resource * _vertexBuffer = nullptr;
	dev->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(sizeof(vertices)),
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&_vertexBuffer));
	char* buf;

	_vertexBuffer->Map(0, nullptr, (void**)&buf);
	memcpy(buf, vertices, sizeof(vertices));
	_vertexBuffer->Unmap(0, nullptr);
	

	// ���_�o�b�t�@�r���[�̐錾
	D3D12_VERTEX_BUFFER_VIEW _vbView = {};
	_vbView.BufferLocation = _vertexBuffer->GetGPUVirtualAddress();
	_vbView.StrideInBytes = sizeof(Vertex);
	_vbView.SizeInBytes = sizeof(vertices);
	
	// �V�F�[�_�̓ǂݍ���
	ID3DBlob* vertexShader = nullptr;
	ID3DBlob* pixelShader = nullptr;


	result = D3DCompileFromFile(TEXT("shader.hlsl"), 
		nullptr,
		nullptr, 
		"BaseVS",
		"vs_5_0",
		D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION,
		0,
		&vertexShader,
		nullptr);

	if (FAILED(result))
	{
		isInit = false;
	}
	result = D3DCompileFromFile(TEXT("shader.hlsl"),
		nullptr,
		nullptr,
		"BasePS",
		"ps_5_0",
		D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION,
		0,
		&pixelShader,
		nullptr);

	if (FAILED(result))
	{
		isInit = false;
	}

	//PSO������
	D3D12_GRAPHICS_PIPELINE_STATE_DESC gpsDesc;


	/*�J���[�N���A����*/
	float color[4] = { 1.0f,0.0f,0.0f,1.0f };

	D3D12_VIEWPORT vp;
	vp.TopLeftX = 0;
	vp.TopLeftY = 0;
	vp.Width = WINDOW_WIDTH;
	vp.Height = WINDOW_HEIGHT;
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;



	// �������s
	if (!isInit)
	{
		MessageBox(hwnd, TEXT("DirectX�̏������Ɏ��s���܂���"), TEXT("���������s"), MB_OK);
		return -1;
	}


	int bbIndex = 0;
	// �E�B���h�E�Y���C�����[�v
	MSG msg = {};
	while (true)
	{
		// �R�}���h�A���P�[�^�ƃ��X�g�̃��Z�b�g����
		result = _commandAllocator->Reset();
		result = _commandList->Reset(_commandAllocator, nullptr);


		// �`���ύX����
		bbIndex = swapChain->GetCurrentBackBufferIndex();
		CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(descriptorHeap->GetCPUDescriptorHandleForHeapStart(), bbIndex, descriptorSize);
		_commandList->OMSetRenderTargets(1, &rtvHandle, false, nullptr);

		// �J���[�̕ύX
		//_commandList->RSSetViewports(1, &vp);
		_commandList->ClearRenderTargetView(rtvHandle, color, 0, nullptr);
		_commandList->Close();
		
		// �R�}���h���X�g�̎��s
		_commandQueue->ExecuteCommandLists(1, (ID3D12CommandList* const*)&_commandList);
		

		// �t�F���X�ɂ��u�҂��v�̏�������
		++_fenceValue;
		_commandQueue->Signal(_fence, _fenceValue); 
		while (_fence->GetCompletedValue() != _fenceValue)
		{
		}


		if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		if (msg.message == WM_QUIT)
		{
			break;
		}
		// �X���b�v
		swapChain->Present(1, 0);



	}
	
	//�f�o�C�X�̃����[�X
	dev->Release();

	// �E�B���h�E�N���X�̊J��
	UnregisterClass(w.lpszClassName, w.hInstance);

	return 0;
}


LRESULT WindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
	// �E�B���h�E�j�����Ƀ��b�Z�[�W������
	if (msg == WM_DESTROY)
	{
		// �I������
		PostQuitMessage(0);
		return 0;
	}
	return DefWindowProc(hwnd, msg, wparam, lparam);	//�f�t�H���g�֐��ɓ�����
}
