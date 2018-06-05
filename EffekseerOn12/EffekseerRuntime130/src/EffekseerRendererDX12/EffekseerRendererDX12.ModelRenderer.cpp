#include "EffekseerRendererDX12.RendererImplemented.h"
#include "EffekseerRendererDX12.RenderState.h"

#include "EffekseerRendererDX12.VertexBuffer.h"
#include "EffekseerRendererDX12.IndexBuffer.h"
#include "EffekseerRendererDX12.ModelRenderer.h"
#include "EffekseerRendererDX12.Shader.h"

namespace EffekseerRendererDX12
{
	namespace ShaderLightingTextureNormal_
	{
		static
#include "Shader/EffekseerRenderer.ModelRenderer.ShaderLightingTextureNormal_VS.h"

			static
#include "Shader/EffekseerRenderer.ModelRenderer.ShaderLightingTextureNormal_PS.h"

	}
	//-----------------------------------------------------------------------------------
	//
	//-----------------------------------------------------------------------------------
	namespace ShaderLightingNormal_
	{
		static
#include "Shader/EffekseerRenderer.ModelRenderer.ShaderLightingNormal_VS.h"

			static
#include "Shader/EffekseerRenderer.ModelRenderer.ShaderLightingNormal_PS.h"
	}
	//-----------------------------------------------------------------------------------
	//
	//-----------------------------------------------------------------------------------
	namespace ShaderLightingTexture_
	{
		static
#include "Shader/EffekseerRenderer.ModelRenderer.ShaderLightingTexture_VS.h"

			static
#include "Shader/EffekseerRenderer.ModelRenderer.ShaderLightingTexture_PS.h"
	}
	//-----------------------------------------------------------------------------------
	//
	//-----------------------------------------------------------------------------------
	namespace ShaderLighting_
	{
		static
#include "Shader/EffekseerRenderer.ModelRenderer.ShaderLighting_VS.h"

			static
#include "Shader/EffekseerRenderer.ModelRenderer.ShaderLighting_PS.h"
	}
	//-----------------------------------------------------------------------------------
	//
	//-----------------------------------------------------------------------------------
	namespace ShaderTexture_
	{
		static
#include "Shader/EffekseerRenderer.ModelRenderer.ShaderTexture_VS.h"

			static
#include "Shader/EffekseerRenderer.ModelRenderer.ShaderTexture_PS.h"
	}
	//-----------------------------------------------------------------------------------
	//
	//-----------------------------------------------------------------------------------
	namespace Shader_
	{
		static
#include "Shader/EffekseerRenderer.ModelRenderer.Shader_VS.h"

			static
#include "Shader/EffekseerRenderer.ModelRenderer.Shader_PS.h"
	}

	//-----------------------------------------------------------------------------------
	//
	//-----------------------------------------------------------------------------------
	namespace ShaderDistortionTexture_
	{
		static
#include "Shader/EffekseerRenderer.ModelRenderer.ShaderDistortion_VS.h"

			static
#include "Shader/EffekseerRenderer.ModelRenderer.ShaderDistortionTexture_PS.h"
	}

	//-----------------------------------------------------------------------------------
	//
	//-----------------------------------------------------------------------------------
	namespace ShaderDistortion_
	{
		static
#include "Shader/EffekseerRenderer.ModelRenderer.ShaderDistortion_VS.h"

			static
#include "Shader/EffekseerRenderer.ModelRenderer.ShaderDistortion_PS.h"
	}

	ModelRenderer::ModelRenderer(RendererImplemented* renderer,
		Shader* shader_lighting_texture_normal,
		Shader* shader_lighting_normal,
		Shader* shader_lighting_texture,
		Shader* shader_lighting,
		Shader* shader_texture,
		Shader* shader,
		Shader* shader_distortion_texture,
		Shader* shader_distortion)
		: m_renderer(renderer)
		, m_shader_lighting_texture_normal(shader_lighting_texture_normal)
		, m_shader_lighting_normal(shader_lighting_normal)
		, m_shader_lighting_texture(shader_lighting_texture)
		, m_shader_lighting(shader_lighting)
		, m_shader_texture(shader_texture)
		, m_shader(shader)
		, m_shader_distortion_texture(shader_distortion_texture)
		, m_shader_distortion(shader_distortion)
	{

		Shader* shaders[6];
		shaders[0] = m_shader_lighting_texture_normal;
		shaders[1] = m_shader_lighting_normal;
		shaders[2] = m_shader_lighting_texture;
		shaders[3] = m_shader_lighting;
		shaders[4] = m_shader_texture;
		shaders[5] = m_shader;

		for (int32_t i = 0; i < 6; i++)
		{
			shaders[i]->SetVertexConstantBufferSize(sizeof(::EffekseerRenderer::ModelRendererVertexConstantBuffer<40>));
			shaders[i]->SetVertexRegisterCount(sizeof(::EffekseerRenderer::ModelRendererVertexConstantBuffer<40>) / (sizeof(float) * 4));
			shaders[i]->SetPixelConstantBufferSize(sizeof(::EffekseerRenderer::ModelRendererPixelConstantBuffer));
			shaders[i]->SetPixelRegisterCount(sizeof(::EffekseerRenderer::ModelRendererPixelConstantBuffer) / (sizeof(float) * 4));
		}

		m_shader_distortion_texture->SetVertexConstantBufferSize(sizeof(::EffekseerRenderer::ModelRendererVertexConstantBuffer<40>));
		m_shader_distortion_texture->SetVertexRegisterCount(sizeof(::EffekseerRenderer::ModelRendererVertexConstantBuffer<40>) / (sizeof(float) * 4));
		m_shader_distortion_texture->SetPixelConstantBufferSize(sizeof(float) * 4);
		m_shader_distortion_texture->SetPixelRegisterCount(1);

		m_shader_distortion->SetVertexConstantBufferSize(sizeof(::EffekseerRenderer::ModelRendererVertexConstantBuffer<40>));
		m_shader_distortion->SetVertexRegisterCount(sizeof(::EffekseerRenderer::ModelRendererVertexConstantBuffer<40>) / (sizeof(float) * 4));
		m_shader_distortion->SetPixelConstantBufferSize(sizeof(float) * 4);
		m_shader_distortion->SetPixelRegisterCount(1);
		//todo ShaderÇÃé¿ëïÇ…ìÔÇ†ÇËÅ@àÍéûí‚é~
	}

}