/*	@file CommandQueue.h
	�R�}���h�L���[�N���X

*/
#pragma once

#include<d3d12.h>
#include<wrl.h>


// ComPtr�g�p�錾
using Microsoft::WRL::ComPtr;

class CommandQueue
{
public:
	CommandQueue();
	~CommandQueue();
	
	/// @fn Create
	/// �R�}���h�L���[�̍쐬
	/// @param[in]	ID3D12Device* : �f�o�C�X�̃|�C���^
	/// @retval	bool true:��������,false:�������s
	bool Create(ID3D12Device* device);

	/// @fn Create
	/// �R�}���h�L���[�̍쐬(�p�����[�^�ݒ��)
	/// @param[in]	ID3D12Device* : �f�o�C�X�̃|�C���^
	/// @param[in]	D3D12_COMMAND_QUEUE_DESC : �R�}���h�L���[�̐ݒ荀��
	/// @retval	bool true:��������,false:�������s
	bool Create(ID3D12Device* device, const D3D12_COMMAND_QUEUE_DESC& desc);

	/// @fn Get
	/// �R�}���h�L���[�̃|�C���^���擾����
	///	@retval	ID3D12CommandQueue* :�R�}���h�L���[�̃|�C���^
	ID3D12CommandQueue*const Get();
private:
	ComPtr<ID3D12CommandQueue> _commandQueue;
};

