#pragma once
#include <d3d12.h>
#include <wrl.h>

// ComPtr�g�p�錾
using Microsoft::WRL::ComPtr;

class CommandAllocator
{
public:
	CommandAllocator();
	~CommandAllocator();

	/// @fn Create
	/// �R�}���h�A���P�[�^���쐬����
	/// @param[in] device			D3D12�f�o�C�X
	/// @param[in] commandListType	�R�}���h���X�g�^�C�v
	/// @retval bool true:����, false:���s
	bool Create(ID3D12Device* device, D3D12_COMMAND_LIST_TYPE commandListType);

	/// @fn Get
	/// �R�}���h�A���P�[�^�̃|�C���^���擾����
	/// @retval ID3D12CommandAllocator* �R�}���h�A���P�[�^�̃|�C���^
	ID3D12CommandAllocator* Get();

	/// @fn GetComPtr
	/// �R�}���h�A���P�[�^��Com�|�C���^���擾����
	/// @retval ComPtr<ID3D12CommandAllocator> �R�}���h�A���P�[�^��COM�|�C���^
	ComPtr<ID3D12CommandAllocator> GetComPtr();
private:

	ComPtr<ID3D12CommandAllocator> _commandAllocator;	//�R�}���h�A���P�[�^
};

