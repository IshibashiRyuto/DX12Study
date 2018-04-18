#pragma once
#include <d3d12.h>
#include "d3dx12.h"
#include <dxgi1_4.h>
#include <vector>
#include <wrl.h>

// �萔��`
#define WINDOW_WIDTH 640
#define WINDOW_HEIGHT 480

#define FRAME_CNT 2;

// ComPtr�g�p�錾
using Microsoft::WRL::ComPtr;

class App
{
public:
	App();
	~App();

	// �f�o�C�X�̐���
	bool CreateDevice(HWND hwnd);

	// ���\�[�X�̐���
	bool CreateResource();

private:

	HWND _hwnd;							// �E�B���h�E�n���h��

	ComPtr<IDXGIFactory4> _factory{ nullptr };			// DXGI�C���^�t�F�[�X�̐����@

	D3D_FEATURE_LEVEL _level{};			// �f�o�C�X�̃t�B�[�`���[���x��
	ComPtr<ID3D12Device> _dev{ nullptr };		// �f�o�C�X�|�C���^


	ComPtr<ID3D12CommandAllocator> _commandAllocator{ nullptr };	//�R�}���h�A���P�[�^
	ComPtr<ID3D12CommandQueue> _commandQueue{ nullptr };			//�R�}���h�L���[
	ComPtr<ID3D12GraphicsCommandList> _commandList{ nullptr };		// �R�}���h���X�g

	ComPtr<IDXGISwapChain3> _swapChain;							// �X���b�v�`�F�C��

	std::vector<ComPtr<ID3D12Resource>> _renderTargets;		//�����_�[�^�[�Q�b�g�̎���
	ComPtr<ID3D12DescriptorHeap> _rtvDescriptorHeap{ nullptr };	// (RTV��)�f�B�X�N���v�^�q�[�v
	UINT _rtvDescriptorSize{ 0 };						// rtv�̂ŃX�N���v�^�T�C�Y

	ID3D12Resource* _depthBuffer{ nullptr };				//	�[�x�o�b�t�@
	ID3D12DescriptorHeap* _dsvDescriptorHeap{ nullptr };	// �[�x�o�b�t�@�̃f�B�X�N���v�^�q�[�v
	D3D12_CLEAR_VALUE _depthClearValue{};					// �[�x�o�b�t�@�̃N���A�l


	D3D12_STATIC_SAMPLER_DESC _samplerDesc{};				// �T���v���[�̐ݒ�
};

