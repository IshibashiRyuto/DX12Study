#pragma once

#include "EffekseerRendererCommon\EffekseerRenderer.RenderStateBase.h"
#include "EffekseerRendererDX12.Base.h"
#include "EffekseerRendererDX12.RendererImplemented.h"

namespace EffekseerRendererDX12
{
	/// レンダリング情報を格納するクラス
	/// @note:もしかすると、ここにPSOを埋め込んでおいたほうがいいんじゃないの
	class RenderState
		: public ::EffekseerRenderer::RenderStateBase
	{
	private:

		static const int32_t		DepthTestCount = 2;
		static const int32_t		DepthWriteCount = 2;
		static const int32_t		CulTypeCount = 3;
		static const int32_t		AlphaTypeCount = 5;
		static const int32_t		TextureFilterCount = 2;
		static const int32_t		TextureWrapCount = 2;

		RendererImplemented*		m_renderer;
		/*
		ID3D11RasterizerState*		m_rStates[CulTypeCount];
		ID3D11DepthStencilState*	m_dStates[DepthTestCount][DepthWriteCount];
		ID3D11BlendState*			m_bStates[AlphaTypeCount];
		ID3D11SamplerState*			m_sStates[TextureFilterCount][TextureWrapCount];
		*/
	public:
		RenderState(RendererImplemented* renderer, D3D12_COMPARISON_FUNC depthFunc);
		virtual ~RenderState();
		void Update(bool forced);
	};
}

