#pragma once
#include <Windows.h>

#include <d3d12.h>
#include "d3dx12.h"
#include <dxgi1_4.h>
#include <D3Dcompiler.h>
#include <DirectXMath.h>
#include <vector>

// 定数定義
#define WINDOW_WIDTH 640
#define WINDOW_HEIGHT 480

#define FRAME_CNT 2;

// 頂点構造体定義
typedef struct VERTEX {
	DirectX::XMFLOAT3 pos;		//座標
	DirectX::XMFLOAT2 uv;		//uv座標
}Vertex;

typedef struct BASE_MATRIXES
{
	DirectX::XMMATRIX world;
	DirectX::XMMATRIX viewproj;
}BaseMatrixes;

class Dx12Device
{
public:
	~Dx12Device();

	static Dx12Device* Instance()
	{
		static Dx12Device inst;
		return &inst;
	}

	bool CreateDevice(HWND hwnd);

	void Render();
	void WaitForGPU();

	void Release();

private:

	Dx12Device();
	IDXGIFactory4* _factory;			// DXGIインタフェースの生成機
	D3D_FEATURE_LEVEL _level{};			// デバイスのフィーチャーレベル
	ID3D12Device* _dev{ nullptr };		// デバイスポインタ

	ID3D12CommandAllocator* _commandAllocator{ nullptr };	//コマンドアロケータ
	ID3D12CommandQueue* _commandQueue{ nullptr };			//コマンドキュー
	ID3D12GraphicsCommandList * _commandList{ nullptr };		// コマンドリスト

	IDXGISwapChain3* _swapChain;							// スワップチェイン
	std::vector<ID3D12Resource*> _renderTargets;		//レンダーターゲットの実体
	ID3D12RootSignature* _rootSignature{ nullptr };	//ルートシグネチャ
	ID3D12DescriptorHeap* _rtvDescriptorHeap{ nullptr };	// (RTVの)ディスクリプタヒープ
	UINT rtvDescriptorSize{ 0 };						// rtvのでスクリプタサイズ

	std::vector<D3D12_INPUT_ELEMENT_DESC> _inputLayoutDescs;	// 頂点レイアウト
	ID3DBlob* vertexShader{ nullptr };							// 頂点シェーダ
	ID3DBlob* pixelShader{ nullptr };							// ピクセルシェーダ
	ID3D12PipelineState* _pipelineStateObject{ nullptr };		// パイプラインステートオブジェクト
	ID3D12Resource *_vertexBuffer{ nullptr };					// 頂点バッファ
	ID3D12Resource *_textureBuffer{ nullptr };					// テクスチャバッファ
	ID3D12DescriptorHeap* _srvDescriptorHeap{ nullptr };			// (SRVの)デスクリプタヒープ
	UINT srvHandle{ 0 };

	ID3D12Resource *_verIndexBuffer{ nullptr };					// 頂点インデックスバッファ
	D3D12_INDEX_BUFFER_VIEW _indexBufferView{};					// 頂点インデックスバッファビュー
	
	ID3D12Fence *_fence{ nullptr };
	UINT64 _fenceValue{ 0 };
	D3D12_VERTEX_BUFFER_VIEW _vbView = {};

	D3D12_STATIC_SAMPLER_DESC samplerDesc{};


	ID3D12Resource* _constantBuffer{ nullptr };				// 定数バッファ
	ID3D12DescriptorHeap* _cbvDescriptorHeap{ nullptr };	// 定数バッファビューのでスクリプタヒープ

	ID3D12Resource* _materialsConstantBuffer{ nullptr };	// マテリアル用定数バッファ
	ID3D12DescriptorHeap* _materialsCbvDescriptorHeap{ nullptr };	// マテリアル用定数バッファビューのでスクリプタヒープ

	ID3D12Resource* _depthBuffer{ nullptr };				//	深度バッファ
	ID3D12DescriptorHeap* _dsvDescriptorHeap{ nullptr };	// 深度バッファのディスクリプタヒープ
	D3D12_CLEAR_VALUE _depthClearValue{};					// 深度バッファのクリア値

	int backBufferIndex;

	HWND _hwnd;	
	
	D3D12_VIEWPORT vp = { 0,0,WINDOW_WIDTH,WINDOW_HEIGHT,0.0f,1.0f};
	D3D12_RECT rc = { 0,0,WINDOW_WIDTH, WINDOW_HEIGHT };

	BaseMatrixes *_matrixAddress;
	DirectX::XMFLOAT3 *_diffuseColorAddress;

	DirectX::XMMATRIX world;
	DirectX::XMVECTOR eye;
	DirectX::XMVECTOR target;
	DirectX::XMVECTOR upper;
	DirectX::XMMATRIX camera;
	DirectX::XMMATRIX projection;

	BaseMatrixes matrix;
};

