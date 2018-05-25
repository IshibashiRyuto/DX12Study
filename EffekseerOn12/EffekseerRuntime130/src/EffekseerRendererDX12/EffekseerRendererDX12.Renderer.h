
#ifndef	__EFFEKSEERRENDERER_DX12_RENDERER_H__
#define	__EFFEKSEERRENDERER_DX12_RENDERER_H__

#include "EffekseerRendererDX12.Base.h"
#include "EffekseerRendererCommon\EffekseerRenderer.Renderer.h"

namespace EffekseerRendererDX12
{
	/*
		�e�N�X�`���Ǎ��N���X�𐶐�����
	*/
	::Effekseer::TextureLoader* CreateTextureLoader(ID3D12Device* device, ::Effekseer::FileInterface* fileInterface = nullptr);

	/*
		���f���Ǎ��N���X�𐶐�����
	*/
	::Effekseer::ModelLoader* CreateModelLoader(ID3D12Device* device, ::Effekseer::FileInterface* fileInterface = nullptr);

	// @class Renderer 
	// �`��N���X
	class Renderer
		: public ::EffekseerRenderer::Renderer
	{
	protected:
		Renderer() {}
		virtual~Renderer() {}
	public:

		/**
		@brief	�C���X�^���X�𐶐�����
		@return	�C���X�^���X
		*/
		static Renderer* Create(/*�����ɂ��Ă͒�����*/);

		// �f�o�C�X���擾����
		virtual ID3D12Device* GetDevice() = 0;

		// �w�i���擾����
		virtual Effekseer::TextureData* GetBackground() = 0;

		// �w�i��ݒ肷��
		virtual void SetBackground(/*�����ɂ��Ă͒�����*/) = 0;
	};

	// @class Model
	// ���f��

	class Model : public Effekseer::Model
	{
	private:

	public:
		ID3D12Resource*			VertexBuffer;
		ID3D12Resource*			IndexBuffer;
		int32_t					VertexCount;
		int32_t					IndexCount;
		int32_t					FaceCount;
		int32_t					ModelCount;

		Model( uint8_t* data, int32_t size )
			: Effekseer::Model(data, size)
			, VertexBuffer(nullptr)
			, IndexBuffer(nullptr)
			, VertexCount(0)
			, IndexCount(0)
			, FaceCount(0)
			, ModelCount(0)
		{
		}

		virtual ~Model()
		{
		}
	};
}

#endif