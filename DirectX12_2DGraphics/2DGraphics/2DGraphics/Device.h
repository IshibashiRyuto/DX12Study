/*	@file	Device.h
	@brief	D3D12�f�o�C�X�N���X
	@author Ishibashi Ryuto
	@date	2018/05/22	����
*/
#pragma once
#include <d3d12.h>
#include <wrl.h>
#include <dxgi1_4.h>

// ComPtr�g�p�錾
using Microsoft::WRL::ComPtr;

class Device
{
public:
	Device();
	~Device();

	/// @fn Create
	/// �f�o�C�X���쐬����
	/// @retval bool true: �f�o�C�X�쐬�����@false: �f�o�C�X�쐬���s
	bool Create();

	/// @fn Get
	/// �f�o�C�X�̃|�C���^���擾����
	/// @retval ID3D12Device* �f�o�C�X�̃|�C���^
	ID3D12Device* Get() const;

	/// @fn GetComPtr
	/// �f�o�C�X��COM�|�C���^���擾����
	/// @retval ComPtr<ID3D12Device> �f�o�C�X��ComPtr
	ComPtr<ID3D12Device> GetComPtr() const;

	/// @fn GetFeatureLevel
	/// �f�o�C�X���쐬�����t�B�[�`���[���x�����擾����
	/// @retval D3D_FEATURE_LEVEL �f�o�C�X�̃t�B�[�`���[���x��
	D3D_FEATURE_LEVEL GetFeatureLevel() const;
private:
	D3D_FEATURE_LEVEL _level;										// �f�o�C�X�̃t�B�[�`���[���x��
	ComPtr<ID3D12Device> _device;							// �f�o�C�X�|�C���^
};

