#pragma once
#include <d3d12.h>
#include "d3dx12.h"
#include <dxgi1_4.h>
#include <D3DCompiler.h>
#include <vector>
#include <wrl.h>

// �萔��`
const int WINDOW_WIDTH = 640;
const int WINDOW_HEIGHT = 480;
const int FRAME_CNT = 2;

// ComPtr�g�p�錾
using Microsoft::WRL::ComPtr;

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

private:

	HWND _hwnd;														// �E�B���h�E�n���h��

	ComPtr<IDXGIFactory4> _factory{ nullptr };						// DXGI�C���^�t�F�[�X�̐����@

	D3D_FEATURE_LEVEL _level{};										// �f�o�C�X�̃t�B�[�`���[���x��
	ComPtr<ID3D12Device> _dev{ nullptr };							// �f�o�C�X�|�C���^


	ComPtr<ID3D12CommandAllocator> _commandAllocator{ nullptr };	//�R�}���h�A���P�[�^
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
	std::vector<D3D12_INPUT_ELEMENT_DESC> _inputLayoutDescs;	// ���_���C�A�E�g
	ComPtr<ID3DBlob> _vertexShader{ nullptr };							// ���_�V�F�[�_
	ComPtr<ID3DBlob> _pixelShader{ nullptr };							// �s�N�Z���V�F�[�_
	ComPtr<ID3D12PipelineState> _pipelineStateObject{ nullptr };		// �p�C�v���C���X�e�[�g�I�u�W�F�N�g


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
};

