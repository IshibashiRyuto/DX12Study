#pragma once
#include <d3d12.h>
#include "d3dx12.h"
#include <dxgi1_4.h>
#include <DirectXMath.h>
#include <D3DCompiler.h>
#include <vector>
#include <wrl.h>
#include "Device.h"
#include "CommandAllocator.h"

/*Effekseer�֌W�w�b�_�C���N���[�h*/
#include <Effekseer.h>
#include <EffekseerRendererDX12.h>
#include <EffekseerSoundXAudio2.h>

// �萔��`
const int WINDOW_WIDTH = 640;
const int WINDOW_HEIGHT = 480;
const int FRAME_CNT = 2;

// ComPtr�g�p�錾
using Microsoft::WRL::ComPtr;

// ���_
struct Vertex
{
	DirectX::XMFLOAT3 pos;			// ���W
	DirectX::XMFLOAT3 normal;
	DirectX::XMFLOAT2 uv;
};

class App
{
public:
	App();
	~App();

	/// @fn	Initialize
	/// ����������
	/// @retval	����������������
	/// true: �����@false:���s
	bool Initialize(HWND hWnd);


	/// @fn Render
	/// �`�揈��
	void Render();


	/// @fn Tarminate
	/// �I������
	void Tarminate();

private:
	const int INSTANCING_NUM = 4096;			// �I�u�W�F�N�g�̃C���X�^���V���O��

	HWND _hwnd;														// �E�B���h�E�n���h��

	ComPtr<IDXGIFactory4> _factory{ nullptr };						// DXGI�C���^�t�F�[�X�̐����@

	Device _device;													// D3D12�f�o�C�X

	CommandAllocator _commandAllocator;								// �R�}���h�A���P�[�^
	ComPtr<ID3D12CommandQueue> _commandQueue{ nullptr };			//�R�}���h�L���[
	ComPtr<ID3D12GraphicsCommandList> _commandList{ nullptr };		// �R�}���h���X�g

	ComPtr<IDXGISwapChain3> _swapChain;								// �X���b�v�`�F�C��

	std::vector<ComPtr<ID3D12Resource>> _renderTargets;				//�����_�[�^�[�Q�b�g�̎���
	ComPtr<ID3D12DescriptorHeap> _rtvDescriptorHeap{ nullptr };		// (RTV��)�f�B�X�N���v�^�q�[�v
	UINT _rtvDescriptorSize{ 0 };									// rtv�̂ŃX�N���v�^�T�C�Y

	ComPtr<ID3D12Resource> _depthBuffer{ nullptr };					//	�[�x�o�b�t�@
	ComPtr<ID3D12DescriptorHeap> _dsvDescriptorHeap{ nullptr };		// �[�x�o�b�t�@�̃f�B�X�N���v�^�q�[�v
	D3D12_CLEAR_VALUE _depthClearValue{};							// �[�x�o�b�t�@�̃N���A�l


	ComPtr<ID3D12RootSignature> _rootSignature{ nullptr };			//���[�g�V�O�l�`��
	D3D12_STATIC_SAMPLER_DESC _samplerDesc{};						// �T���v���[�̐ݒ�
	std::vector<D3D12_INPUT_ELEMENT_DESC> _inputLayoutDescs;		// ���_���C�A�E�g
	ComPtr<ID3DBlob> _vertexShader{ nullptr };						// ���_�V�F�[�_
	ComPtr<ID3DBlob> _pixelShader{ nullptr };						// �s�N�Z���V�F�[�_
	ComPtr<ID3D12PipelineState> _pipelineStateObject{ nullptr };	// �p�C�v���C���X�e�[�g�I�u�W�F�N�g

	ComPtr<ID3D12Fence> _fence;										// �t�F���X�I�u�W�F�N�g
	UINT64 _fenceValue{ 0 };										// �t�F���X�l

	ComPtr<ID3D12Resource> _vertexBuffer;							// ���_�o�b�t�@
	D3D12_VERTEX_BUFFER_VIEW _vertexBufferView;						// ���_�o�b�t�@�r���[
	ComPtr<ID3D12Resource> _instancingBuffer;						// �C���X�^���V���O�p�̃o�b�t�@
	D3D12_VERTEX_BUFFER_VIEW _instancingBufferView;					// �C���X�^���V���O�o�b�t�@�r���[
	ComPtr<ID3D12Resource> _instancingConstantBuffer;				// �C���X�^���V���O�p�̃R���X�^���g�o�b�t�@
	ComPtr<ID3D12DescriptorHeap> _icbDescHeap;						// �C���X�^���V���O�p�R���X�^���g�o�b�t�@�̃f�X�N���v�^�q�[�v

	ComPtr<ID3D12GraphicsCommandList> _bundle;								// �o���h��
	CommandAllocator _bundleAllocator;								// �o���h���p�̃A���P�[�^

	// �e�N�X�`�����\�[�X
	ComPtr<ID3D12Resource> _textureBuffer;							// �e�N�X�`���o�b�t�@
	ComPtr<ID3D12DescriptorHeap> _srvDescriptorHeap;					// �V�F�[�_���\�[�X�r���[�p�̃f�X�N���v�^�q�[�v

	/*Effekseer�e�X�g�p�����o*/
	IXAudio2* g_xa2;
	::EffekseerRenderer::Renderer* _renderer;
	::EffekseerSound::Sound* _sound;
	::Effekseer::Manager* _manager;
	Effekseer::Effect* _fire;
	Effekseer::Effect* _hit;

	Effekseer::Handle _fireHandle;
	Effekseer::Handle _hitHandle;

	Vertex vertices[4] = { {{0.0f,0.0f,0.0f}, {0.0f,0.0f,-1.0f},{0.0f,0.0f} },
	{{0.1f,0.0f, 0.0f}, { 0.0f,0.0f,-1.0f }, {1.0f,0.0f} },
	{{0.0f,-0.1f,0.0f}, { 0.0f,0.0f,-1.0f }, {0.0f,1.0f} },
	{{0.1f,-0.1f,0.0f}, { 0.0f,0.0f,-1.0f }, {1.0f,1.0f} } };											// ���_���(��)

	/// @fn	CreateDevice
	/// �f�o�C�X�̐�������
	/// @retval bool ����������������
	/// true:�����@false:���s
	bool CreateDevice();

	/// @fn	CreateCommandAllocator
	/// �R�}���h�A���P�[�^�̐�������
	/// @retval bool ����������������
	/// true:�����@false:���s
	bool CreateCommandAllocator();


	/// @fn CreateCommandQueue
	/// �R�}���h�L���[�̐�������
	/// @retval bool ����������������
	/// true:���� false:���s
	bool CreateCommandQueue();

	/// @fn CreateSwapchain
	/// �X���b�v�`�F�C���̐�������
	/// @retval bool ����������������
	/// true:���� false:���s
	bool CreateSwapchain();


	/// @fn CreateRenderTargetView
	/// �����_�[�^�[�Q�b�g�r���[�̐�������
	/// @retval bool ����������������
	/// true:���� false:���s
	bool CreateRenderTargetView();

	/// @fn CreateDepthBuffer
	/// �f�v�X�o�b�t�@�̐�������
	/// @retval bool ����������������
	/// true:���� false:���s
	bool CreateDepthBuffer();

	/// @fn CreateDepthStencilView
	/// �f�v�X�X�e���V���r���[�̐�������
	/// @retval bool ����������������
	/// true:���� false:���s
	bool CreateDepthStencilView();

	/// @fn CreateRootSignature
	/// ���[�g�V�O�l�`���̐�������
	/// @retval bool ����������������
	/// true:���� false:���s
	bool CreateRootSignature();

	/// @fn CreateResource
	/// ���\�[�X�̍쐬
	/// @retval bool ����������������
	/// true:���� false:���s
	bool CreateResource();

	/// @fn ReadShader
	/// �V�F�[�_�̓ǂݍ���
	/// @retval bool �ǂݍ��݂�����������
	/// true:�����@false:���s
	bool ReadShader();

	/// @fn CreatePipelineStateObject
	/// �p�C�v���C���X�e�[�g�I�u�W�F�N�g�̍쐬
	/// @retval bool ����������������
	/// true:���� false:���s
	bool CreatePipelineStateObject();

	/// @fn CreateCommandList
	/// �R�}���h���X�g�𐶐�����
	/// @retval bool ����������������
	/// true:���� false:���s
	bool CreateCommandList();

	/// @fn CreateBundle
	/// �o���h�����쐬����
	/// @retval bool ����������������
	/// true:���� false:���s
	bool CreateBundle();

	/// @fn SetBundle
	/// �o���h���ɃR�}���h��ς�
	void SetBundle();
	

	/// @fn CreateFence
	/// �t�F���X�𐶐�����
	/// @retval bool ����������������
	/// true:���� false:���s
	bool CreateFence();


	/// @fn CreateVertexBuffer
	/// ���_�o�b�t�@���쐬����
	/// @retval bool ����������������
	/// true:���� false:���s
	bool CreateVertexBuffer();

	/// @fn CreateInstancingBuffer
	/// �C���X�^���V���O�ɗ��p����s����i�[����o�b�t�@���쐬����
	/// @retval bool ����������������
	/// true:���� false:���s
	bool CreateInstancingBuffer();

	/// @fn CreateTextureBuffer
	/// �e�N�X�`���o�b�t�@���쐬����
	/// @retval bool ����������������
	/// true:���� false:���s
	bool CreateTextureBuffer();


	/// @fn LoadBitmapData
	/// �r�b�g�}�b�v�f�[�^�̃��[�h
	/// @retval bool ����������������
	/// true: ����, false: ���s
	bool LoadBitmapData();

	/// @fn CreateSharderResourceView
	/// �V�F�[�_���\�[�X�r���[�̍쐬
	/// @retval bool ����������������
	/// true: ����, false: ���s
	bool CreateShaderResourceView();

};
