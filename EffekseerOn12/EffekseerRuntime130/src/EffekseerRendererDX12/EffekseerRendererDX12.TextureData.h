#pragma once
#include <d3d12.h>
#include <wrl.h>


namespace EffekseerRendererDX12
{
	class TextureData
	{
	private:
		D3D12_SHADER_RESOURCE_VIEW_DESC m_srvDesc;
		ID3D12Resource* m_resource;

		TextureData(ID3D12Resource* resource, D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc);
		

	public:
		ID3D12Resource* GetResource() { return m_resource; }
		const D3D12_SHADER_RESOURCE_VIEW_DESC& GetShaderResourceView() { return m_srvDesc; }

		static TextureData* Create(	ID3D12Resource *resource);
		~TextureData() {}
	};
}

