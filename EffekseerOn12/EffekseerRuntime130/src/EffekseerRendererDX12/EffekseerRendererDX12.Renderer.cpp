
#include "EffekseerRendererDX12.Renderer.h"
#include "EffekseerRendererDX12.RendererImplemented.h"
#include "EffekseerRendererDX12.RenderState.h"

#include "EffekseerRendererDX12.Shader.h"
#include "EffekseerRendererDX12.VertexBuffer.h"
#include "EffekseerRendererDX12.IndexBuffer.h"
#include "EffekseerRendererDX12.DeviceObject.h"
#include "EffekseerRendererDX12.ModelRenderer.h"
#include "EffekseerRendererDX12.TextureLoader.h"
#include "EffekseerRendererDX12.ModelLoader.h"

#include "../../EffekseerRendererCommon/EffekseerRenderer.SpriteRendererBase.h"
#include "../../EffekseerRendererCommon/EffekseerRenderer.RibbonRendererBase.h"
#include "../../EffekseerRendererCommon/EffekseerRenderer.RingRendererBase.h"
#include "../../EffekseerRendererCommon/EffekseerRenderer.TrackRendererBase.h"

#ifdef __EFFEKSEER_RENDERER_INTERNAL_LOADER__
#include "../../EffekseerRendererCommon/EffekseerRenderer.PngTextureLoader.h"
#endif


//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
namespace EffekseerRendererDX12
{

	namespace Standard_VS
	{
		static
#include "Shader/EffekseerRenderer.Standard_VS.h"
	}

	//-----------------------------------------------------------------------------------
	//
	//-----------------------------------------------------------------------------------
	namespace Standard_PS
	{
		static
#include "Shader/EffekseerRenderer.Standard_PS.h"
	}

	//-----------------------------------------------------------------------------------
	//
	//-----------------------------------------------------------------------------------
	namespace StandardNoTexture_PS
	{
		static
#include "Shader/EffekseerRenderer.StandardNoTexture_PS.h"
	}

	//-----------------------------------------------------------------------------------
	//
	//-----------------------------------------------------------------------------------
	namespace Standard_Distortion_VS
	{
		static
#include "Shader/EffekseerRenderer.Standard_Distortion_VS.h"
	}

	//-----------------------------------------------------------------------------------
	//
	//-----------------------------------------------------------------------------------
	namespace Standard_Distortion_PS
	{
		static
#include "Shader/EffekseerRenderer.Standard_Distortion_PS.h"
	}

	//-----------------------------------------------------------------------------------
	//
	//-----------------------------------------------------------------------------------
	namespace StandardNoTexture_Distortion_PS
	{
		static
#include "Shader/EffekseerRenderer.StandardNoTexture_Distortion_PS.h"
	}


	::Effekseer::TextureLoader* CreateTextureLoader(ID3D12Device* device, ::Effekseer::FileInterface* fileInterface)
	{
#ifdef __EFFEKSEER_RENDERER_INTERNAL_LOADER__
		return new TextureLoader(device, fileInterface);
#else
		return nullptr;
#endif
	}

	::Effekseer::ModelLoader* CreateModelLoader(ID3D12Device* device, ::Effekseer::FileInterface* fileInterface)
	{
#ifdef __EFFEKSEER_RENDERER_INTERNAL_LOADER__
		return new ModelLoader(device, fileInterface);
#else
		return nullptr;
#endif
	}

	// OriginalState関連
	/*
//----------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------------
	OriginalState::OriginalState()
		: m_blendState(NULL)
		, m_depthStencilState(NULL)
		, m_depthStencilStateRef(0)
		, m_vertexConstantBuffer(NULL)
		, m_pixelConstantBuffer(NULL)
		, m_layout(NULL)

		, m_pRasterizerState(nullptr)
		, m_pVS(nullptr)
		, m_pPS(nullptr)
		, m_pVB(nullptr)
		, m_pIB(nullptr)

	{
		for (int32_t i = 0; i < 4; i++)
		{
			m_samplers[i] = NULL;
		}

		for (int32_t i = 0; i < 4; i++)
		{
			m_psSRVs[i] = nullptr;
		}
	}

	//----------------------------------------------------------------------------------
	//
	//----------------------------------------------------------------------------------
	OriginalState::~OriginalState()
	{
		ReleaseState();
	}

	//----------------------------------------------------------------------------------
	//
	//----------------------------------------------------------------------------------
	void OriginalState::SaveState(ID3D11Device* device, ID3D11DeviceContext* context)
	{
		context->PSGetSamplers(0, 4, m_samplers);
		context->OMGetBlendState(&m_blendState, m_blendFactor, &m_blendSampleMask);
		context->OMGetDepthStencilState(&m_depthStencilState, &m_depthStencilStateRef);
		context->RSGetState(&m_pRasterizerState);

		context->VSGetConstantBuffers(0, 1, &m_vertexConstantBuffer);
		context->PSGetConstantBuffers(0, 1, &m_pixelConstantBuffer);

		context->VSGetShader(&m_pVS, nullptr, nullptr);
		context->PSGetShader(&m_pPS, nullptr, nullptr);

		context->IAGetInputLayout(&m_layout);

		context->IAGetPrimitiveTopology(&m_topology);

		context->PSGetShaderResources(0, 4, m_psSRVs);

		context->IAGetVertexBuffers(0, 1, &m_pVB, &m_vbStrides, &m_vbOffset);
		context->IAGetIndexBuffer(&m_pIB, &m_ibFormat, &m_ibOffset);
	}

	//----------------------------------------------------------------------------------
	//
	//----------------------------------------------------------------------------------
	void OriginalState::LoadState(ID3D11Device* device, ID3D11DeviceContext* context)
	{
		context->PSSetSamplers(0, 4, m_samplers);
		context->OMSetBlendState(m_blendState, m_blendFactor, m_blendSampleMask);
		context->OMSetDepthStencilState(m_depthStencilState, m_depthStencilStateRef);
		context->RSSetState(m_pRasterizerState);

		context->VSSetConstantBuffers(0, 1, &m_vertexConstantBuffer);
		context->PSSetConstantBuffers(0, 1, &m_pixelConstantBuffer);

		context->VSSetShader(m_pVS, NULL, 0);
		context->PSSetShader(m_pPS, NULL, 0);

		context->IASetInputLayout(m_layout);

		context->IASetPrimitiveTopology(m_topology);

		context->PSSetShaderResources(0, 4, m_psSRVs);

		context->IASetVertexBuffers(0, 1, &m_pVB, &m_vbStrides, &m_vbOffset);
		context->IASetIndexBuffer(m_pIB, m_ibFormat, m_ibOffset);
	}

	//----------------------------------------------------------------------------------
	//
	//----------------------------------------------------------------------------------
	void OriginalState::ReleaseState()
	{
		for (int32_t i = 0; i < 4; i++)
		{
			ES_SAFE_RELEASE(m_samplers[i]);
		}
		ES_SAFE_RELEASE(m_blendState);

		ES_SAFE_RELEASE(m_depthStencilState);

		ES_SAFE_RELEASE(m_pRasterizerState);

		ES_SAFE_RELEASE(m_vertexConstantBuffer);
		ES_SAFE_RELEASE(m_pixelConstantBuffer);

		ES_SAFE_RELEASE(m_pVS);
		ES_SAFE_RELEASE(m_pPS);

		ES_SAFE_RELEASE(m_layout);

		for (int32_t i = 0; i < 4; i++)
		{
			ES_SAFE_RELEASE(m_psSRVs[i]);
		}

		ES_SAFE_RELEASE(m_pVB);
		ES_SAFE_RELEASE(m_pIB);
	}
	*/
	//todo:以下ちょっと後回し。パイプライン系がまーじでメンドくさ過ぎるわ
	
	Renderer* Renderer::Create()
	{
		RendererImplemented* renderer = new RendererImplemented();
		if (renderer->Initialize())
		{
			return renderer;
		}
		return nullptr;
	}

	RendererImplemented::RendererImplemented(int32_t squareMaxCount)
		: m_device(nullptr)
		//, m_context( nullptr)
		, m_vertexBuffer(nullptr)
		, m_indexBuffer(nullptr)
		, m_squareMaxCount(squareMaxCount)
		, m_coordinateSystem(::Effekseer::CoordinateSystem::RH)
		, m_renderState(nullptr)
		, m_restorationOfStates(true)
		
		, m_shader(nullptr)
		, m_shader_no_texture(nullptr)
		, m_shader_distortion(nullptr)
		, m_shader_no_texture_distortion(nullptr)
		, m_standardRenderer(nullptr)
		, m_distortingCallback(nullptr)
	{
		::Effekseer::Vector3D direction(1.0f, 1.0f, 1.0f);
		SetLightDirection(direction);
		::Effekseer::Color lightColor(255, 255, 255, 255);
		SetLightColor(lightColor);
		::Effekseer::Color lightAmbient(0, 0, 0, 0);
		SetLightAmbientColor(lightAmbient);

		m_background.UserPtr = nullptr;

		m_state = new OriginalState();
#ifdef __EFFEKSEER_RENDERER_INTERNAL_LOADER__
		EffekseerRenderer::PngTextureLoader::Initialize();
#endif
	}

	RendererImplemented::~RendererImplemented()
	{
#ifdef __EFFEKSEER_RENDERER_INTERNAL_LOADER__
		EffekseerRenderer::PngTextureLoader::Finalize();
#endif

		assert(GetRef() == 0);

		ES_SAFE_DELETE(m_distortingCallback);

		auto p = (ID3D12Resource*)m_background.UserPtr;
		ES_SAFE_RELEASE(p);

		ES_SAFE_DELETE(m_standardRenderer);
		ES_SAFE_DELETE(m_shader);
		ES_SAFE_DELETE(m_shader_no_texture);
		
		ES_SAFE_DELETE(m_shader_distortion);
		ES_SAFE_DELETE(m_shader_no_texture_distortion);

		ES_SAFE_DELETE(m_state);
		
		ES_SAFE_DELETE(m_renderState);
		ES_SAFE_DELETE(m_vertexBuffer);
		ES_SAFE_DELETE(m_indexBuffer);

		assert(GetRef() == -6);
	}

	void RendererImplemented::OnLostDevice()
	{
		for (auto& device : m_deviceObjects)
		{
			device->OnLostDevice();
		}
	}

	void RendererImplemented::OnResetDevice()
	{
		for (auto& device : m_deviceObjects)
		{
			device->OnResetDevice();
		}
	}

	bool RendererImplemented::Initialize(ID3D12Device* device, D3D12_COMPARISON_FUNC depthFunc)
	{
		m_device = device;
		m_depthFunc = depthFunc;

		// 頂点の生成
		{
			// 最大でfloat*10と仮定
			m_vertexBuffer = VertexBuffer::Create(this, sizeof(float) * 10 * m_squareMaxCount * 4, true);
			if (m_vertexBuffer == nullptr)
			{
				return false;
			}
		}

		// 参照カウントの調整
		Release();

		// インデックスの生成
		{
			m_indexBuffer = IndexBuffer::Create(this, m_squareMaxCount * 6, false);
			if (m_indexBuffer == nullptr)
			{
				return false;
			}

			m_indexBuffer->Lock();

			// 標準設定で DirectX 時計回りが表, OpenGLは反時計回りが表
			for (int i = 0; i < m_squareMaxCount; ++i)
			{
				uint16_t* buf = (uint16_t*)m_indexBuffer->GetBufferDirect(6);
				buf[0] = 3 + 4 * i;
				buf[1] = 1 + 4 * i;
				buf[2] = 0 + 4 * i;
				buf[3] = 3 + 4 * i;
				buf[4] = 0 + 4 * i;
				buf[5] = 2 + 4 * i;
			}

			m_indexBuffer->Unlock();
		}
		// 参照カウントの調整
		Release();

		m_renderState = new RenderState(this, m_depthFunc);


		// シェーダ
		// 座標(3) 色(1) UV(2)
		D3D12_INPUT_ELEMENT_DESC decl[] = {
			{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
			{"NORMAL", 0, DXGI_FORMAT_R8G8B8A8_UNORM, 0, sizeof(float)*3, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
			{"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, sizeof(float)*4, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
		};

		D3D12_INPUT_ELEMENT_DESC decl_distortion[]=
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			{ "NORMAL", 0, DXGI_FORMAT_R8G8B8A8_UNORM, 0, sizeof(float) * 3, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, sizeof(float) * 4, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			{ "NORMAL", 1, DXGI_FORMAT_R32G32B32_FLOAT, 0, sizeof(float) * 6, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			{ "NORMAL", 2, DXGI_FORMAT_R32G32B32_FLOAT, 0, sizeof(float) * 9, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		};

		m_shader = Shader::Create(
			this,
			Standard_VS::g_VS,
			sizeof(Standard_VS::g_VS),
			Standard_PS::g_PS,
			sizeof(Standard_PS::g_PS),
			"StandardRenderer", decl, ARRAYSIZE(decl));

		if (m_shader == nullptr)
		{
			return false;
		}

		// 参照カウントの調整
		Release();

		m_shader_distortion = Shader::Create(
			this,
			Standard_Distortion_VS::g_VS,
			sizeof(Standard_Distortion_VS::g_VS),
			Standard_Distortion_PS::g_PS,
			sizeof(Standard_Distortion_PS::g_PS),
			"StandardRenderer Distortion", decl_distortion, ARRAYSIZE(decl_distortion));
		if (m_shader_distortion == nullptr)
		{
			return false;
		}

		// 参照カウントの調整
		Release();

		m_shader_no_texture_distortion = Shader::Create(
			this,
			Standard_Distortion_VS::g_VS,
			sizeof(Standard_Distortion_VS::g_VS),
			StandardNoTexture_Distortion_PS::g_PS,
			sizeof(StandardNoTexture_Distortion_PS::g_PS),
			"StandardRenderer No Texture Distortion",
			decl_distortion, ARRAYSIZE(decl_distortion));
		if (m_shader_no_texture_distortion == nullptr)
		{
			return false;
		}

		// 参照カウントの調整
		Release();

		m_shader->SetVertexConstantBufferSize(sizeof(Effekseer::Matrix44) * 2);
		m_shader->SetVertexRegisterCount(8);
		m_shader_no_texture->SetVertexConstantBufferSize(sizeof(Effekseer::Matrix44) * 2);
		m_shader_no_texture->SetVertexRegisterCount(8);

		m_shader_distortion->SetVertexConstantBufferSize(sizeof(Effekseer::Matrix44) * 2);
		m_shader_distortion->SetVertexRegisterCount(8);

		m_shader_distortion->SetPixelConstantBufferSize(sizeof(float) * 4);
		m_shader_distortion->SetPixelRegisterCount(1);

		m_shader_no_texture_distortion->SetVertexConstantBufferSize(sizeof(Effekseer::Matrix44) * 2);
		m_shader_no_texture_distortion->SetVertexRegisterCount(8);

		m_shader_no_texture_distortion->SetPixelConstantBufferSize(sizeof(float) * 4);
		m_shader_no_texture_distortion->SetPixelRegisterCount(1);

		m_standardRenderer = new EffekseerRenderer::StandardRenderer<RendererImplemented, Shader, Vertex, VertexDistortion>(
			this, m_shader, m_shader_no_texture, m_shader_distortion, m_shader_no_texture_distortion);

		return true;
	}

	void RendererImplemented::Destroy()
	{
		Release();
	}

	void RendererImplemented::SetRestorationOfStatesFlag(bool flag)
	{
		m_restorationOfStates = flag;
	}

	bool RendererImplemented::BeginRendering()
	{
		assert(m_device != nullptr);

		::Effekseer::Matrix44::Mul(m_cameraProj, m_camera, m_proj);

		/*
		// ステートを保存する
		if( m_restorationOfStates )
		{
			m_state->SaveState( m_device, m_context );
		}

		// ステート初期設定
		m_renderState->GetActiveState().Reset();
		m_renderState->Update( true );

		*/

		// レンダラーリセット
		m_standardRenderer->ResetAndRenderingIfRequired();

		return true;
	}

	bool RendererImplemented::EndRendering()
	{
		assert(m_device != nullptr);

		// レンダラーリセット
		m_standardRenderer->ResetAndRenderingIfRequired();

		/*
		// ステート復元
		if (m_restorationOfStates)
		{
			m_state->LoadState(m_device, m_context);
			m_state->ReleaseState();
		}
		*/

		return true;
	}

	ID3D12Device* RendererImplemented::GetDevice()
	{
		return m_device;
	}

	VertexBuffer* RendererImplemented::GetVertexBuffer()
	{
		return m_vertexBuffer;
	}

	IndexBuffer* RendererImplemented::GetIndexBuffer()
	{
		return m_indexBuffer;
	}

	int32_t RendererImplemented::GetSquareMaxCount() const
	{
		return m_squareMaxCount;
	}
}