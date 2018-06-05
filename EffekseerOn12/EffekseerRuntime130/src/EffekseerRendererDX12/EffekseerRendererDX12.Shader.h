#pragma once

#include"EffekseerRendererDX12.RendererImplemented.h"
#include"EffekseerRendererDX12.DeviceObject.h"

namespace EffekseerRendererDX12
{
	class Shader
		: public DeviceObject
	{
	private:
		// TODO: PSOにまとめれそう
		/*
		ID3D11VertexShader*			m_vertexShader;
		ID3D11PixelShader*			m_pixelShader;
		ID3D11InputLayout*			m_vertexDeclaration;
		*/
		
		// TODO: ディスクリプタで管理しよう
		/*
		void*					m_vertexConstantBuffer;
		void*					m_pixelConstantBuffer;;

	int32_t					m_vertexRegisterCount;
	int32_t					m_pixelRegisterCount;
		*/

		Shader( /*
			RendererImplemented* renderer,
			ID3D11VertexShader* vertexShader,
			ID3D11PixelShader* pixelShader,
			ID3D11InputLayout* vertexDeclaration*/);

	public:
		virtual ~Shader();

		static Shader* Create(
			RendererImplemented* renderer,
			const uint8_t vertexShader[],
			int32_t vertexShaderSize,
			const uint8_t pixelShader[],
			int32_t pixelShaderSize,
			const char* name,
			const D3D12_INPUT_ELEMENT_DESC decl[],
			int32_t layoutCount);

		virtual void OnLostDevice();
		virtual void OnResetDevice();

		//TODO: このあたりをどうするかちょっと考えるわ

		/*
			どのシェーダに渡すかはルートパラメータで判断している？
			ということはルートシグネチャもここに持たせるべき？
			というかコンスタントバッファとか持ってるならここで持ってるのが妥当な気がする...
		*/
		/*
		ID3D11VertexShader* GetVertexShader() const { return m_vertexShader; }
		ID3D11PixelShader* GetPixelShader() const { return m_pixelShader; }
		ID3D11InputLayout* GetLayoutInterface() const { return m_vertexDeclaration; }
		*/

		void SetVertexConstantBufferSize(int32_t size);
		void SetPixelConstantBufferSize(int32_t size);

		void* GetVertexConstantBuffer() { return m_vertexConstantBuffer; }
		void* GetPixelConstantBuffer() { return m_pixelConstantBuffer; }

		void SetVertexRegisterCount(int32_t count) { m_vertexRegisterCount = count; }
		void SetPixelRegisterCount(int32_t count) { m_pixelRegisterCount = count; }

		void SetConstantBuffer();
	};

}

