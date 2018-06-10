
#include "EffekseerRendererDX12.RendererImplemented.h"
#include "EffekseerRendererDX12.TextureLoader.h"

#include "../../EffekseerRendererCommon/EffekseerRenderer.DXTK.DDSTextureLoader.h"
#include "../../EffekseerRendererCommon/EffekseerRenderer.PngTextureLoader.h"
#include "../../EffekseerRendererCommon/EffekseerRenderer.DDSTextureLoader.h"

namespace EffekseerRendererDX12
{
	TextureLoader::TextureLoader(ID3D12Device* device, ::Effekseer::FileInterface* fileInterface)
		: m_fileInterface(fileInterface)
		, device(device)
	{
		ES_SAFE_ADDREF(device);
		if (fileInterface == nullptr)
		{
			m_fileInterface = &m_defaultFileInterface;
		}
#ifdef __EFFEKSEER_RENDERER_INTERNAL_LOADER__
		EffekseerRenderer::PngTextureLoader::Initialize();
#endif
	}

	TextureLoader::~TextureLoader()
	{
#ifdef __EFFEKSEER_RENDERER_INTERNAL_LOADER__
		EffekseerRenderer::PngTextureLoader::Finalize();
#endif

		ES_SAFE_RELEASE(device);
	}

	Effekseer::TextureData* TextureLoader::Load(const EFK_CHAR* path, ::Effekseer::TextureType textureType)
	{
		std::shared_ptr<::Effekseer::FileReader>
			reader(m_fileInterface->OpenRead(path));
		if (reader)
		{
			// テクスチャ読み込み処理
		}

		return nullptr;
	}

	void TextureLoader::Unload(Effekseer::TextureData* data)
	{
		if (data != nullptr && data->UserPtr != nullptr)
		{
			//テクスチャデータの削除処理
			//auto texture = (ID3D12ShaderResourceView*)data->UserPtr;
			//texture->Release();
		}

		if (data != nullptr)
		{
			delete data;
		}
	}
}