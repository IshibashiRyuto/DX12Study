#pragma once
#include <d3d12.h>
#include "d3dx12.h"
#include <dxgi1_4.h>
#include <vector>

// �萔��`
#define WINDOW_WIDTH 640
#define WINDOW_HEIGHT 480

#define FRAME_CNT 2;

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

	IDXGIFactory4* _factory;			// DXGI�C���^�t�F�[�X�̐����@

	D3D_FEATURE_LEVEL _level{};			// �f�o�C�X�̃t�B�[�`���[���x��
	ID3D12Device* _dev{ nullptr };		// �f�o�C�X�|�C���^


	ID3D12CommandAllocator* _commandAllocator{ nullptr };	//�R�}���h�A���P�[�^
	ID3D12CommandQueue* _commandQueue{ nullptr };			//�R�}���h�L���[
	ID3D12GraphicsCommandList * _commandList{ nullptr };		// �R�}���h���X�g

	IDXGISwapChain3* _swapChain;							// �X���b�v�`�F�C��

	std::vector<ID3D12Resource*> _renderTargets;		//�����_�[�^�[�Q�b�g�̎���
	ID3D12DescriptorHeap* _rtvDescriptorHeap{ nullptr };	// (RTV��)�f�B�X�N���v�^�q�[�v
	UINT rtvDescriptorSize{ 0 };						// rtv�̂ŃX�N���v�^�T�C�Y

};

