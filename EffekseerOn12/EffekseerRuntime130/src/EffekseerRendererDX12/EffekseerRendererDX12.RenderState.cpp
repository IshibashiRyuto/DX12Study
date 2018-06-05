#include "EffekseerRendererDX12.RenderState.h"


namespace EffekseerRendererDX12
{

	RenderState::RenderState(RendererImplemented* renderer, D3D12_COMPARISON_FUNC)
		: m_renderer(renderer)
	{
	}


	RenderState::~RenderState()
	{
	}

	void RenderState::Update(bool forced)
	{

	}

}