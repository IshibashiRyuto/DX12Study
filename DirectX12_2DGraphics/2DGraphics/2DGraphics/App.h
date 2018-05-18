#pragma once
#include <d3d12.h>
#include "d3dx12.h"
#include <dxgi1_4.h>
#include <D3DCompiler.h>
#include <vector>
#include <wrl.h>

// 定数定義
const int WINDOW_WIDTH = 640;
const int WINDOW_HEIGHT = 480;
const int FRAME_CNT = 2;

// ComPtr使用宣言
using Microsoft::WRL::ComPtr;

class App
{
public:
	App();
	~App();

	/// @fn	Initialize
	/// 初期化処理
	/// @retval	処理が成功したか
	/// true: 成功　false:失敗
	bool Initialize(HWND hWnd);


	/// @fn Render
	/// 描画処理
	void Render();

private:

	HWND _hwnd;														// ウィンドウハンドル

	ComPtr<IDXGIFactory4> _factory{ nullptr };						// DXGIインタフェースの生成機

	D3D_FEATURE_LEVEL _level{};										// デバイスのフィーチャーレベル
	ComPtr<ID3D12Device> _dev{ nullptr };							// デバイスポインタ


	ComPtr<ID3D12CommandAllocator> _commandAllocator{ nullptr };	//コマンドアロケータ
	ComPtr<ID3D12CommandQueue> _commandQueue{ nullptr };			//コマンドキュー
	ComPtr<ID3D12GraphicsCommandList> _commandList{ nullptr };		// コマンドリスト

	ComPtr<IDXGISwapChain3> _swapChain;								// スワップチェイン

	std::vector<ComPtr<ID3D12Resource>> _renderTargets;				//レンダーターゲットの実体
	ComPtr<ID3D12DescriptorHeap> _rtvDescriptorHeap{ nullptr };		// (RTVの)ディスクリプタヒープ
	UINT _rtvDescriptorSize{ 0 };									// rtvのでスクリプタサイズ

	ComPtr<ID3D12Resource> _depthBuffer{ nullptr };					//	深度バッファ
	ComPtr<ID3D12DescriptorHeap> _dsvDescriptorHeap{ nullptr };		// 深度バッファのディスクリプタヒープ
	D3D12_CLEAR_VALUE _depthClearValue{};							// 深度バッファのクリア値


	ComPtr<ID3D12RootSignature> _rootSignature{ nullptr };			//ルートシグネチャ
	D3D12_STATIC_SAMPLER_DESC _samplerDesc{};						// サンプラーの設定
	std::vector<D3D12_INPUT_ELEMENT_DESC> _inputLayoutDescs;	// 頂点レイアウト
	ComPtr<ID3DBlob> _vertexShader{ nullptr };							// 頂点シェーダ
	ComPtr<ID3DBlob> _pixelShader{ nullptr };							// ピクセルシェーダ
	ComPtr<ID3D12PipelineState> _pipelineStateObject{ nullptr };		// パイプラインステートオブジェクト


	/// @fn	CreateDevice
	/// デバイスの生成処理
	/// @retval bool 処理が成功したか
	/// true:成功　false:失敗
	bool CreateDevice();

	/// @fn	CreateCommandAllocator
	/// コマンドアロケータの生成処理
	/// @retval bool 処理が成功したか
	/// true:成功　false:失敗
	bool CreateCommandAllocator();


	/// @fn CreateCommandQueue
	/// コマンドキューの生成処理
	/// @retval bool 処理が成功したか
	/// true:成功 false:失敗
	bool CreateCommandQueue();


	/// @fn CreateSwapchain
	/// スワップチェインの生成処理
	/// @retval bool 処理が成功したか
	/// true:成功 false:失敗
	bool CreateSwapchain();


	/// @fn CreateRenderTargetView
	/// レンダーターゲットビューの生成処理
	/// @retval bool 処理が成功したか
	/// true:成功 false:失敗
	bool CreateRenderTargetView();

	/// @fn CreateDepthBuffer
	/// デプスバッファの生成処理
	/// @retval bool 処理が成功したか
	/// true:成功 false:失敗
	bool CreateDepthBuffer();

	/// @fn CreateDepthStencilView
	/// デプスステンシルビューの生成処理
	/// @retval bool 処理が成功したか
	/// true:成功 false:失敗
	bool CreateDepthStencilView();

	/// @fn CreateRootSignature
	/// ルートシグネチャの生成処理
	/// @retval bool 処理が成功したか
	/// true:成功 false:失敗
	bool CreateRootSignature();

	/// @fn CreateResource
	/// リソースの作成
	/// @retval bool 処理が成功したか
	/// true:成功 false:失敗
	bool CreateResource();

	/// @fn ReadShader
	/// シェーダの読み込み
	/// @retval bool 読み込みが成功したか
	/// true:成功　false:失敗
	bool ReadShader();

	/// @fn CreatePipelineStateObject
	/// パイプラインステートオブジェクトの作成
	/// @retval bool 処理が成功したか
	/// true:成功 false:失敗
	bool CreatePipelineStateObject();

	/// @fn CreateCommandList
	/// コマンドリストを生成する
	/// @retval bool 処理が成功したか
	/// true:成功 false:失敗
	bool CreateCommandList();
};

