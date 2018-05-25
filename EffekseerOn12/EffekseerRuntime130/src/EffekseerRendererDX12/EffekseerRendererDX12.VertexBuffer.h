
#pragma once
///	Include
#include "EffekseerRendererDX12.RendererImplemented.h"
#include "EffekseerRendererCommon\EffekseerRenderer.VertexBufferBase.h"
#include "EffekseerRendererDX12.DeviceObject.h"

namespace EffekseerRendererDX12
{
	class VertexBuffer
		: public DeviceObject
		, public ::EffekseerRenderer::VertexBufferBase
	{
	private:
		ID3D12Resource*		m_buffer;
		void*				m_lockedResource;

		uint32_t			m_vertexRingOffset;
		bool				m_ringBufferLock;

		int32_t				m_ringLockedOffset;
		int32_t				m_ringLockedSize;

		VertexBuffer(RendererImplemented* renderer, ID3D12Resource* buffer, int size, bool isDynamic);
	public:
		virtual ~VertexBuffer();

		static VertexBuffer* Create(RendererImplemented* renderer, int size, bool isDynamic);

		ID3D12Resource* GetInterface() { return m_buffer; }

		virtual void OnLostDevice();
		virtual void OnResetDevice();

		void Lock();
		bool RingBufferLoc(int32_t size, int32_t& offset, void*& data);
		bool TryRingBufferLock(int32_t size, int32_t& offset, void*& data);
		void Unlock();

	};
}


