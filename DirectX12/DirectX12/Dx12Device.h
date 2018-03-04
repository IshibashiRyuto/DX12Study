#pragma once
#include <Windows.h>

#include <d3d12.h>
#include <d3dx12.h>
#include <dxgi1_4.h>
#include <D3Dcompiler.h>
#include <DirectXMath.h>
#include <vector>

// �萔��`
#define WINDOW_WIDTH 640
#define WINDOW_HEIGHT 480

#define FRAME_CNT 2;

// ���_�\���̒�`
typedef struct VERTEX {
	DirectX::XMFLOAT3 pos;		//���W
	DirectX::XMFLOAT2 uv;		//uv���W
}Vertex;

class Dx12Device
{
public:
	~Dx12Device();

	static Dx12Device* Instance()
	{
		static Dx12Device inst;
		return &inst;
	}

	bool CreateDevice(HWND hwnd);

	void Render();
	void WaitForGPU();

	void Release();

private:

	Dx12Device();
	IDXGIFactory4* _factory;			// DXGI�C���^�t�F�[�X�̐����@
	D3D_FEATURE_LEVEL _level{};			// �f�o�C�X�̃t�B�[�`���[���x��
	ID3D12Device* _dev = nullptr;		// �f�o�C�X�|�C���^
	
	ID3D12CommandAllocator* _commandAllocator = nullptr;	//�R�}���h�A���P�[�^
	ID3D12CommandQueue* _commandQueue = nullptr;			//�R�}���h�L���[
	ID3D12GraphicsCommandList * _commandList = nullptr;		// �R�}���h���X�g

	IDXGISwapChain3* _swapChain;							// �X���b�v�`�F�C��
	std::vector<ID3D12Resource*> _renderTargets;		//�����_�[�^�[�Q�b�g�̎���
	ID3D12RootSignature* _rootSignature = nullptr;	//���[�g�V�O�l�`��
	ID3D12DescriptorHeap* descriptorHeap = nullptr;	// �f�B�X�N���v�^�q�[�v
	UINT rtvDescriptorSize{ 0 };						// rtv�̂ŃX�N���v�^�T�C�Y

	std::vector<D3D12_INPUT_ELEMENT_DESC> _inputLayoutDescs;	// ���_���C�A�E�g
	ID3DBlob* vertexShader = nullptr;							// ���_�V�F�[�_
	ID3DBlob* pixelShader = nullptr;							// �s�N�Z���V�F�[�_
	ID3D12PipelineState* _pipelineStateObject{ nullptr };		// �p�C�v���C���X�e�[�g�I�u�W�F�N�g
	ID3D12Resource *_vertexBuffer = nullptr;					// ���_�o�b�t�@

	ID3D12Fence *_fence{ nullptr };
	UINT64 _fenceValue{ 0 };
	D3D12_VERTEX_BUFFER_VIEW _vbView = {};

	int backBufferIndex;

	HWND _hwnd;	
	
	D3D12_VIEWPORT vp = { 0,0,WINDOW_WIDTH,WINDOW_HEIGHT,0.0f,1.0f};
	D3D12_RECT rc = { 0,0,WINDOW_WIDTH, WINDOW_HEIGHT };

};
