#pragma once

#include"EffekseerRendererDX12.RendererImplemented.h"
#include"EffekseerRendererDX12.DeviceObject.h"
#include <wrl.h>

using Microsoft::WRL::ComPtr;

namespace EffekseerRendererDX12
{
	class Shader
		: public DeviceObject
	{
	private:
		D3D12_SHADER_BYTECODE m_vertexShader;
		D3D12_SHADER_BYTECODE m_pixelShader;
		D3D12_INPUT_ELEMENT_DESC *m_InputElements;
		D3D12_INPUT_LAYOUT_DESC m_vertexDeclaration;
		

		// TODO: ディスクリプタで管理しよう
		void*					m_vertexConstantBuffer;
		void*					m_pixelConstantBuffer;
		void*					m_vertexConstantBufferStart;
		void*					m_pixelConstantBufferStart;

		int32_t					m_vertexRegisterCount;
		int32_t					m_pixelRegisterCount;

		int32_t					m_vertexBufferSize;
		int32_t					m_pixelBufferSize;

		ID3D12DescriptorHeap* m_constantBufferDescriptorHeap;
		D3D12_CPU_DESCRIPTOR_HANDLE m_handle;

		ComPtr<ID3D12Resource> m_constantBufferToVS;
		D3D12_CONSTANT_BUFFER_VIEW_DESC m_cbvToVS;
		ComPtr<ID3D12Resource> m_constantBufferToPS;
		D3D12_CONSTANT_BUFFER_VIEW_DESC m_cbvToPS;

		Shader(RendererImplemented* renderer,
			const D3D12_SHADER_BYTECODE& vertexShader,
			const D3D12_SHADER_BYTECODE& pixelShader,
			const D3D12_INPUT_LAYOUT_DESC& vertexDeclaration,
			D3D12_INPUT_ELEMENT_DESC* inputElements);

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

		D3D12_SHADER_BYTECODE& GetVertexShader() { return m_vertexShader; }
		D3D12_SHADER_BYTECODE& GetPixelShader() { return m_pixelShader; }
		D3D12_INPUT_LAYOUT_DESC& GetLayoutInterface() { return m_vertexDeclaration; }

		void SetVertexConstantBufferSize(int32_t size);
		void SetPixelConstantBufferSize(int32_t size);

		void* GetVertexConstantBuffer() { return m_vertexConstantBuffer; }
		void* GetPixelConstantBuffer() { return m_pixelConstantBuffer; }

		void SetVertexRegisterCount(int32_t count) { m_vertexRegisterCount = count; }
		void SetPixelRegisterCount(int32_t count) { m_pixelRegisterCount = count; }

		void SetConstantBuffer();

		void ResetConstantBuffer();

		ID3D12DescriptorHeap* GetDescriptorHeap() { return m_constantBufferDescriptorHeap; }

	};

}

