#include "EffekseerRendererDX12.IndexBuffer.h"

namespace EffekseerRendererDX12
{

	IndexBuffer::IndexBuffer( RendererImplemented* renderer, ID3D12Resource* buffer, int maxCount, bool isDynamic )
		: DeviceObject(renderer)
		, IndexBufferBase(maxCount, isDynamic)
		, m_buffer(nullptr)
		, m_lockedResource ( nullptr )
	{
		m_lockedResource = new uint8_t[sizeof(uint16_t) * maxCount];
	}


	IndexBuffer::~IndexBuffer()
	{
	}

	IndexBuffer * IndexBuffer::Create(RendererImplemented * renderer, int maxCount, bool isDynamic)
	{
		//todo
		return nullptr;
	}

	void IndexBuffer::OnLostDevice()
	{
	}

	void IndexBuffer::OnResetDevice()
	{
	}

	void IndexBuffer::Lock()
	{
		assert(!m_isLock);
		m_isLock = true;
		m_resource = (uint8_t*)m_lockedResource;
		m_indexCount = 0;
	}

	void IndexBuffer::Unlock()
	{
		assert(m_isLock);

		if (m_isDynamic)
		{
			// todo
		}
		else
		{
			//todo
		}

		m_resource = nullptr;
		m_isLock = false;
	}

}