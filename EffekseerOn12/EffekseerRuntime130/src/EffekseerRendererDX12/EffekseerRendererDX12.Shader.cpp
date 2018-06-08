#include "EffekseerRendererDX12.Shader.h"
#include "EffekseerRendererDX12.RendererImplemented.h"

namespace EffekseerRendererDX12
{
	Shader::Shader(
		RendererImplemented* renderer,
		const D3D12_SHADER_BYTECODE& vertexShader,
		const D3D12_SHADER_BYTECODE& pixelShader,
		const D3D12_INPUT_LAYOUT_DESC& vertexDeclaration,
		D3D12_INPUT_ELEMENT_DESC* inputElements,
		ID3D12DescriptorHeap * constantBufferDescriptorHeap)
		: DeviceObject(renderer)
		, m_vertexShader(vertexShader)
		, m_pixelShader(pixelShader)
		, m_vertexDeclaration(vertexDeclaration)
		, m_InputElements(inputElements)
		, m_constantBufferToVS(nullptr)
		, m_constantBufferToPS(nullptr)
		, m_vertexConstantBuffer(nullptr)
		, m_pixelConstantBuffer(nullptr)
		, m_vertexRegisterCount(0)
		, m_pixelRegisterCount(0)
		, m_constantBufferDescriptorHeap(constantBufferDescriptorHeap)
		, m_handle(D3D12_CPU_DESCRIPTOR_HANDLE{})
	{

	}

	Shader::~Shader()
	{
		ES_SAFE_DELETE_ARRAY(m_InputElements);
	}

	Shader* Shader::Create(
		RendererImplemented* renderer,
		const uint8_t vertexShader[],
		int32_t vertexShaderSize,
		const uint8_t pixelShader[],
		int32_t pixelShaderSize,
		const char* name,
		const D3D12_INPUT_ELEMENT_DESC decl[],
		int32_t layoutCount)
	{
		assert(renderer != nullptr);
		assert(renderer->GetDevice() != nullptr);

		D3D12_SHADER_BYTECODE vs;
		D3D12_SHADER_BYTECODE ps;

		vs.pShaderBytecode = vertexShader;
		vs.BytecodeLength = vertexShaderSize;

		ps.pShaderBytecode = pixelShader;
		ps.BytecodeLength = pixelShaderSize;

		D3D12_INPUT_LAYOUT_DESC vertexDeclaration{};
		D3D12_INPUT_ELEMENT_DESC* inputElements = new D3D12_INPUT_ELEMENT_DESC[layoutCount];

		for (int i = 0; i < layoutCount; ++i)
		{
			inputElements[i] = decl[i];
		}

		vertexDeclaration.NumElements = layoutCount;
		vertexDeclaration.pInputElementDescs = inputElements;

		// debug
		// ‚Æ‚è‚ ‚¦‚¸‚¢‚Á‚Ï‚¢ì‚é
		ID3D12DescriptorHeap* constantBufferDescriptorHeap;
		D3D12_DESCRIPTOR_HEAP_DESC heapDesc{};
		heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
		heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		heapDesc.NumDescriptors = (UINT)4096;
		renderer->GetDevice()->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&constantBufferDescriptorHeap));



		return new Shader(renderer, vs, ps, vertexDeclaration, inputElements,constantBufferDescriptorHeap);
	}

	void Shader::OnLostDevice()
	{
	}
	
	void Shader::OnResetDevice()
	{
	}

	void Shader::SetVertexConstantBufferSize(int32_t size)
	{
		/*
		ES_SAFE_DELETE_ARRAY(m_vertexConstantBuffer);
		m_vertexConstantBuffer = new uint8_t[size];

		D3D11_BUFFER_DESC hBufferDesc;
		hBufferDesc.ByteWidth = size;
		hBufferDesc.Usage = D3D11_USAGE_DEFAULT;
		hBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		hBufferDesc.CPUAccessFlags = 0;
		hBufferDesc.MiscFlags = 0;
		hBufferDesc.StructureByteStride = sizeof(float);

		GetRenderer()->GetDevice()->CreateBuffer(&hBufferDesc, NULL, &m_constantBufferToVS);
		*/
		ES_SAFE_DELETE_ARRAY(m_vertexConstantBuffer);
		m_vertexConstantBuffer = new uint8_t[size];
		m_cbvToVS.SizeInBytes = size;

	}

	void Shader::SetPixelConstantBufferSize(int32_t size)
	{
		/*
		ES_SAFE_DELETE_ARRAY(m_pixelConstantBuffer);
		m_pixelConstantBuffer = new uint8_t[size];

		D3D11_BUFFER_DESC hBufferDesc;
		hBufferDesc.ByteWidth = size;
		hBufferDesc.Usage = D3D11_USAGE_DEFAULT;
		hBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		hBufferDesc.CPUAccessFlags = 0;
		hBufferDesc.MiscFlags = 0;
		hBufferDesc.StructureByteStride = sizeof(float);

		GetRenderer()->GetDevice()->CreateBuffer(&hBufferDesc, NULL, &m_constantBufferToPS);
		*/
		ES_SAFE_DELETE_ARRAY(m_pixelConstantBuffer);
		m_pixelConstantBuffer = new uint8_t[size];

		D3D12_HEAP_PROPERTIES heapPropertie{};
		heapPropertie.Type = D3D12_HEAP_TYPE_UPLOAD;
		heapPropertie.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
		heapPropertie.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
		heapPropertie.VisibleNodeMask = 1;
		heapPropertie.CreationNodeMask = 1;

		D3D12_RESOURCE_DESC resourceDesc{};
		resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
		resourceDesc.Width = (size + 0xff) & ~0xff;
		resourceDesc.Height = 1;
		resourceDesc.DepthOrArraySize = 1;
		resourceDesc.SampleDesc.Count = 1;
		resourceDesc.SampleDesc.Quality = 0;
		resourceDesc.MipLevels = 1;
		resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

		GetRenderer()->GetDevice()->CreateCommittedResource(&heapPropertie,
			D3D12_HEAP_FLAG_NONE,
			&resourceDesc,
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&m_constantBufferToPS));

		m_cbvToPS.BufferLocation = m_constantBufferToPS->GetGPUVirtualAddress();
		m_cbvToPS.SizeInBytes = (size + 0xff) & ~0xff;

		if (m_handle.ptr == 0)
		{
			m_handle = m_constantBufferDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
		}
		else
		{
			m_handle.ptr += GetRenderer()->GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
		}
		GetRenderer()->GetDevice()->CreateConstantBufferView(&m_cbvToPS, m_handle);
	}

	void Shader::SetConstantBuffer()
	{
		/*
		if (m_vertexRegisterCount > 0)
		{
			GetRenderer()->GetContext()->UpdateSubresource( m_constantBufferToVS, 0, NULL, m_vertexConstantBuffer, 0, 0 );
			GetRenderer()->GetContext()->VSSetConstantBuffers(0, 1, &m_constantBufferToVS);
		}

		if (m_pixelRegisterCount > 0)
		{
			GetRenderer()->GetContext()->UpdateSubresource( m_constantBufferToPS, 0, NULL, m_pixelConstantBuffer, 0, 0 );
			GetRenderer()->GetContext()->PSSetConstantBuffers(0, 1, &m_constantBufferToPS);
		}
		*/
	}

}