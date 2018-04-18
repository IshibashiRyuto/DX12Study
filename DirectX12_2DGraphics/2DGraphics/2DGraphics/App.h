#pragma once
#include <d3d12.h>
#include "d3dx12.h"
#include <dxgi1_4.h>
#include <vector>
#include <wrl.h>

// 定数定義
#define WINDOW_WIDTH 640
#define WINDOW_HEIGHT 480

#define FRAME_CNT 2;

// ComPtr使用宣言
using Microsoft::WRL::ComPtr;

class App
{
public:
	App();
	~App();

	// デバイスの生成
	bool CreateDevice(HWND hwnd);

	// リソースの生成
	bool CreateResource();

private:

	HWND _hwnd;							// ウィンドウハンドル

	ComPtr<IDXGIFactory4> _factory{ nullptr };			// DXGIインタフェースの生成機

	D3D_FEATURE_LEVEL _level{};			// デバイスのフィーチャーレベル
	ComPtr<ID3D12Device> _dev{ nullptr };		// デバイスポインタ


	ComPtr<ID3D12CommandAllocator> _commandAllocator{ nullptr };	//コマンドアロケータ
	ComPtr<ID3D12CommandQueue> _commandQueue{ nullptr };			//コマンドキュー
	ComPtr<ID3D12GraphicsCommandList> _commandList{ nullptr };		// コマンドリスト

	ComPtr<IDXGISwapChain3> _swapChain;							// スワップチェイン

	std::vector<ComPtr<ID3D12Resource>> _renderTargets;		//レンダーターゲットの実体
	ComPtr<ID3D12DescriptorHeap> _rtvDescriptorHeap{ nullptr };	// (RTVの)ディスクリプタヒープ
	UINT _rtvDescriptorSize{ 0 };						// rtvのでスクリプタサイズ

	ID3D12Resource* _depthBuffer{ nullptr };				//	深度バッファ
	ID3D12DescriptorHeap* _dsvDescriptorHeap{ nullptr };	// 深度バッファのディスクリプタヒープ
	D3D12_CLEAR_VALUE _depthClearValue{};					// 深度バッファのクリア値


	D3D12_STATIC_SAMPLER_DESC _samplerDesc{};				// サンプラーの設定
};

