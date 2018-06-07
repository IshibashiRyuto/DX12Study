#include "EffekseerRendererDX12.RenderState.h"


namespace EffekseerRendererDX12
{

	RenderState::RenderState(RendererImplemented* renderer, D3D12_COMPARISON_FUNC depthFunc)
		: m_renderer(renderer)
	{
		// カリングモードのテーブル作成
		D3D12_CULL_MODE cullTbl[] =
		{
			D3D12_CULL_MODE_BACK,
			D3D12_CULL_MODE_FRONT,
			D3D12_CULL_MODE_NONE,
		};

		// ラスタライザデスクの作成
		for (int32_t ct = 0; ct < CulTypeCount; ++ct)
		{
			ZeroMemory(&m_rStates[ct], sizeof(D3D12_RASTERIZER_DESC));
			m_rStates[ct].CullMode = cullTbl[ct];
			m_rStates[ct].FillMode = D3D12_FILL_MODE_SOLID;
			m_rStates[ct].DepthClipEnable = TRUE;
		}

		// デプスステンシルデスクの作成
		for (int32_t dt = 0; dt < DepthTestCount; ++dt)
		{
			for (int32_t dw = 0; dw < DepthWriteCount; ++dw)
			{
				ZeroMemory(&m_dStates[dt][dw], sizeof(D3D12_DEPTH_STENCIL_DESC));
				m_dStates[dt][dw].DepthEnable = dt;
				m_dStates[dt][dw].DepthWriteMask = (D3D12_DEPTH_WRITE_MASK)dw;
				m_dStates[dt][dw].DepthFunc = depthFunc;
				m_dStates[dt][dw].StencilEnable = FALSE;
			}
		}

		// ブレンドデスクの生成
		for (int32_t i = 0; i < AlphaTypeCount; ++i)
		{
			auto type = (::Effekseer::AlphaBlendType)i;
			ZeroMemory(&m_bStates[i], sizeof(D3D12_BLEND_DESC));
			m_bStates[i].AlphaToCoverageEnable = false;

			for (int32_t k = 0; k < 8; ++k)
			{
				m_bStates[i].RenderTarget[k].BlendEnable = type != ::Effekseer::AlphaBlendType::Opacity;
				m_bStates[i].RenderTarget[k].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
				m_bStates[i].RenderTarget[k].SrcBlendAlpha = D3D12_BLEND_ONE;
				m_bStates[i].RenderTarget[k].DestBlendAlpha = D3D12_BLEND_ONE;
				m_bStates[i].RenderTarget[k].BlendOpAlpha = D3D12_BLEND_OP_MAX;
				switch (type)
				{
				case ::Effekseer::AlphaBlendType::Opacity:
					m_bStates[i].RenderTarget[k].DestBlend = D3D12_BLEND_ZERO;
					m_bStates[i].RenderTarget[k].SrcBlend = D3D12_BLEND_ONE;
					m_bStates[i].RenderTarget[k].BlendOp = D3D12_BLEND_OP_ADD;
					break;
				case ::Effekseer::AlphaBlendType::Blend:
					m_bStates[i].RenderTarget[k].DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
					m_bStates[i].RenderTarget[k].SrcBlend = D3D12_BLEND_SRC_ALPHA;
					m_bStates[i].RenderTarget[k].BlendOp = D3D12_BLEND_OP_ADD;
					break;
				case ::Effekseer::AlphaBlendType::Add:
					m_bStates[i].RenderTarget[k].DestBlend = D3D12_BLEND_ONE;
					m_bStates[i].RenderTarget[k].SrcBlend = D3D12_BLEND_SRC_ALPHA;
					m_bStates[i].RenderTarget[k].BlendOp = D3D12_BLEND_OP_ADD;
					break;
				case ::Effekseer::AlphaBlendType::Sub:
					m_bStates[i].RenderTarget[k].DestBlend = D3D12_BLEND_ONE;
					m_bStates[i].RenderTarget[k].SrcBlend = D3D12_BLEND_SRC_ALPHA;
					m_bStates[i].RenderTarget[k].BlendOp = D3D12_BLEND_OP_REV_SUBTRACT;
					break;
				case ::Effekseer::AlphaBlendType::Mul:
					m_bStates[i].RenderTarget[k].DestBlend = D3D12_BLEND_SRC_COLOR;
					m_bStates[i].RenderTarget[k].SrcBlend = D3D12_BLEND_ZERO;
					m_bStates[i].RenderTarget[k].BlendOp = D3D12_BLEND_OP_ADD;
					break;
				}
			}
		}

		// サンプラの作成
		for (int32_t f = 0; f < TextureFilterCount; ++f)
		{
			for (int32_t w = 0; w < TextureWrapCount; ++w)
			{
				D3D12_TEXTURE_ADDRESS_MODE Addres[] =
				{
					D3D12_TEXTURE_ADDRESS_MODE_WRAP,
					D3D12_TEXTURE_ADDRESS_MODE_CLAMP,
				};

				D3D12_FILTER Filter[] = 
				{
					D3D12_FILTER_MIN_MAG_MIP_POINT,
					D3D12_FILTER_MIN_MAG_MIP_LINEAR,
				};

				uint32_t Anisotropic[] =
				{
					0,0,
				};

				m_sStates[f][w].Filter = Filter[f];
				m_sStates[f][w].AddressU = Addres[w];
				m_sStates[f][w].AddressV = Addres[w];
				m_sStates[f][w].AddressW = Addres[w];
				m_sStates[f][w].MipLODBias = 0.0f;
				m_sStates[f][w].MaxAnisotropy = Anisotropic[f];
				m_sStates[f][w].ComparisonFunc = D3D12_COMPARISON_FUNC_ALWAYS;
				m_sStates[f][w].BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
				m_sStates[f][w].MinLOD = 0.0f;
				m_sStates[f][w].MaxLOD = D3D12_FLOAT32_MAX;
				m_sStates[f][w].ShaderRegister = 0;
				m_sStates[f][w].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
				m_sStates[f][w].RegisterSpace = 0;
			}
		}
		// パイプラインステートデスクの初期化

	}


	RenderState::~RenderState()
	{
	}

	void RenderState::Update(bool forced)
	{
		bool changeDepth = forced;
		bool changeRasterizer = forced;
		bool changeBlend = forced;
		bool changeSamplerEither = forced;

		// デプスステンシルステートの変更
		if (m_active.DepthTest != m_next.DepthTest || forced)
		{
			changeDepth = true;
		}

		if (m_active.DepthWrite != m_next.DepthWrite || forced)
		{
			changeDepth = true;
		}

		if (changeDepth)
		{
			m_pipelineState.DepthStencilState = m_dStates[m_next.DepthTest][m_next.DepthWrite];
		}

		// ラスタライザステートの変更
		if (m_active.CullingType != m_next.CullingType || forced)
		{
			changeRasterizer = true;
		}

		if (changeRasterizer)
		{
			auto cullingType = (int32_t)m_next.CullingType;
			m_pipelineState.RasterizerState = m_rStates[cullingType];
		}

		// ブレンドステートの変更
		if (m_active.AlphaBlend != m_next.AlphaBlend || forced)
		{
			changeBlend = true;
		}

		if (changeBlend)
		{
			auto alphaBlend = (int32_t)m_next.AlphaBlend;
			m_pipelineState.BlendState = m_bStates[alphaBlend];
			float blendFactor[] = { 0.0f,0.0f,0.0f,0.0f };
			m_renderer->GetCommandList()->OMSetBlendFactor(blendFactor);
		}

		// サンプラの変更
		// ここ、ルートシグネチャに関連するんですがそれは……
		for (int32_t i = 0; i < 4; ++i)
		{
			bool changeSampler = forced;
			if (m_active.TextureFilterTypes[i] != m_next.TextureFilterTypes[i] || forced)
			{
				changeSampler = true;
			}

			if (m_active.TextureWrapTypes[i] != m_next.TextureWrapTypes[i] || forced)
			{
				changeSampler = true;
			}

			if (changeSampler)
			{
				changeSamplerEither = true;
				auto filter = (int32_t)m_next.TextureFilterTypes[i];
				auto wrap = (int32_t)m_next.TextureWrapTypes[i];

				// サンプラ変更処理
			}
		}

		// パイプラインステートの変更をコマンドに書き出し
		if (changeDepth || changeRasterizer || changeBlend || changeSamplerEither)
		{
			m_renderer->GetCommandList()->SetPipelineState(GetPipelineState());
		}
	}

	ID3D12PipelineState * RenderState::GetPipelineState()
	{
		auto it = m_pipelineStateMap.find(m_pipelineState);
		if (it == m_pipelineStateMap.end())
		{
			ID3D12PipelineState* pso;
			auto hr = m_renderer->GetDevice()->CreateGraphicsPipelineState(&m_pipelineState, IID_PPV_ARGS(&pso));
			if (FAILED(hr))
			{
#ifdef _DEBUG
				MessageBox(nullptr, TEXT("Failed Create PipelineStateObject."), TEXT("Failed"), MB_OK);
				return nullptr;
#endif
			}
			m_pipelineStateMap[m_pipelineState] = pso;
			return pso;
		}

		return m_pipelineStateMap[m_pipelineState];
	}

}