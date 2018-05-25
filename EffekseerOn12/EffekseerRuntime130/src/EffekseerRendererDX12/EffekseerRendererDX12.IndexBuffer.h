#pragma once

#include "EffekseerRendererDX12.RendererImplemented.h"
#include "EffekseerRendererCommon/EffekseerRenderer.IndexBufferBase.h"
#include "EffekseerRendererDX12.DeviceObject.h"

namespace EffekseerRendererDX12
{
	class IndexBuffer
		: public ::EffekseerRenderer::IndexBufferBase
		, public DeviceObject
	{
	private:
		ID3D12Resource*		m_buffer;
		void*				m_lockedResource;

		IndexBuffer(RendererImplemented* renderer, ID3D12Resource* buffer, int maxCount, bool isDynamic);
	public:
		virtual ~IndexBuffer();

		static IndexBuffer* Create(RendererImplemented* renderer, int maxCount, bool isDynamic);

		ID3D12Resource* GetnInterface() { return m_buffer; };

		virtual void OnLostDevice();
		virtual void OnResetDevice();

		void Lock();
		void Unlock();
	};
}

