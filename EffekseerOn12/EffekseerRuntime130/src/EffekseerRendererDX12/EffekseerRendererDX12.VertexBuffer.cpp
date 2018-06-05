#include "EffekseerRendererDX12.VertexBuffer.h"

namespace EffekseerRendererDX12
{
	VertexBuffer::VertexBuffer(RendererImplemented* renderer, ID3D12Resource* buffer, int size, bool isDynamic)
		: DeviceObject(renderer)
		, VertexBufferBase(size, isDynamic)
		, m_buffer(buffer)
		, m_vertexRingOffset(0)
		, m_ringBufferLock(false)
		, m_ringLockedOffset(0)
		, m_ringLockedSize(0)
	{
		m_lockedResource = new uint8_t[size];
	}

	VertexBuffer::~VertexBuffer()
	{
		ES_SAFE_DELETE_ARRAY(m_lockedResource);
		ES_SAFE_RELEASE(m_buffer);
	}

	VertexBuffer* VertexBuffer::Create(RendererImplemented* renderer, int size, bool isDynamic)
	{
	}

	void VertexBuffer::OnLostDevice()
	{
	}

	void VertexBuffer::OnResetDevice()
	{
	}
	void VertexBuffer::Lock()
	{
	}
	bool VertexBuffer::RingBufferLock(int32_t size, int32_t & offset, void *& data)
	{
		return false;
	}
	bool VertexBuffer::TryRingBufferLock(int32_t size, int32_t & offset, void *& data)
	{
		return false;
	}
	void VertexBuffer::Unlock()
	{
	}
}

