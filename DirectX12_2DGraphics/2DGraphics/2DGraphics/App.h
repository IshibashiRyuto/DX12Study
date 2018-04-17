#pragma once
#include <d3d12.h>
#include "d3dx12.h"
#include <dxgi1_4.h>
#include <vector>

// 定数定義
#define WINDOW_WIDTH 640
#define WINDOW_HEIGHT 480

#define FRAME_CNT 2;

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

	IDXGIFactory4* _factory;			// DXGIインタフェースの生成機

	D3D_FEATURE_LEVEL _level{};			// デバイスのフィーチャーレベル
	ID3D12Device* _dev{ nullptr };		// デバイスポインタ


	ID3D12CommandAllocator* _commandAllocator{ nullptr };	//コマンドアロケータ
	ID3D12CommandQueue* _commandQueue{ nullptr };			//コマンドキュー
	ID3D12GraphicsCommandList * _commandList{ nullptr };		// コマンドリスト

	IDXGISwapChain3* _swapChain;							// スワップチェイン

	std::vector<ID3D12Resource*> _renderTargets;		//レンダーターゲットの実体
	ID3D12DescriptorHeap* _rtvDescriptorHeap{ nullptr };	// (RTVの)ディスクリプタヒープ
	UINT rtvDescriptorSize{ 0 };						// rtvのでスクリプタサイズ

};

