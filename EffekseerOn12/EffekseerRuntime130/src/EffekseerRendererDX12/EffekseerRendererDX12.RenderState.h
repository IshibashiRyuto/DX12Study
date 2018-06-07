#pragma once

#include "EffekseerRendererCommon\EffekseerRenderer.RenderStateBase.h"
#include "EffekseerRendererDX12.Base.h"
#include "EffekseerRendererDX12.RendererImplemented.h"

namespace EffekseerRendererDX12
{
	/// レンダリング情報を格納するクラス
	/// @note:もしかすると、ここにPSOを埋め込んでおいたほうがいいんじゃないの
	/// @note追記:仕様上、psoを1000個とかバカげた数作らないといけないことが発覚
	/// ちょっと考えなおそうぜ、これ
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

		D3D12_BLEND_DESC m_bStates[AlphaTypeCount];
		D3D12_RASTERIZER_DESC m_rStates[CulTypeCount];
		D3D12_DEPTH_STENCIL_DESC m_dStates[DepthTestCount][DepthWriteCount];
		D3D12_STATIC_SAMPLER_DESC m_sStates[TextureFilterCount][TextureWrapCount];
		D3D12_GRAPHICS_PIPELINE_STATE_DESC m_pipelineState;

		std::map<D3D12_GRAPHICS_PIPELINE_STATE_DESC, ID3D12PipelineState*> m_pipelineStateMap;

	public:
		RenderState(RendererImplemented* renderer, D3D12_COMPARISON_FUNC depthFunc);
		virtual ~RenderState();
		void Update(bool forced);

		// 現在のパイプラインステートに紐づけられたPSOを取得する
		ID3D12PipelineState* GetPipelineState();
	};
}

