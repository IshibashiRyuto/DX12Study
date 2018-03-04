#include <Windows.h>

#include <d3d12.h>
#include "d3dx12.h"
#include <dxgi1_4.h>
#include <D3Dcompiler.h>
#include <DirectXMath.h>
#include <vector>

#include <cstdio>

using namespace DirectX;

// 定数定義
#define WINDOW_WIDTH 640
#define WINDOW_HEIGHT 480

// 構造体定義
typedef struct VERTEX {
	XMFLOAT3 pos;		//座標
}Vertex;

LRESULT WindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);

int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrevInst, LPSTR lpStr, int nCmdShow)
{

	// ウィンドウクラスの登録
	WNDCLASSEX w = {};
	w.cbSize = sizeof(WNDCLASSEX);
	w.lpfnWndProc = (WNDPROC)WindowProc;
	w.lpszClassName = TEXT("DirectXTest");
	w.hInstance = GetModuleHandle(0);
	RegisterClassEx(&w);

	// ウィンドウサイズ指定
	RECT wrc = { 0,0,WINDOW_WIDTH, WINDOW_HEIGHT };		// ウィンドウサイズの指定
	AdjustWindowRect(&wrc, WS_OVERLAPPEDWINDOW, false);//ウィンドウサイズの補正

	//ウィンドウ生成処理
	HWND hwnd;
	hwnd = CreateWindow(w.lpszClassName,
		TEXT("DX12テスト"),
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		wrc.right - wrc.left,
		wrc.bottom - wrc.top,
		nullptr,
		nullptr,
		w.hInstance,
		nullptr);

	// ウィンドウ表示
	ShowWindow(hwnd, SW_SHOW);

	// D3D12初期化処理


#ifdef _DEBUG
	{
		// デバッグレイヤーの有効化
		ID3D12Debug* debugController;
		if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController))))
		{
			debugController->EnableDebugLayer();
			debugController->Release();
			debugController = nullptr;
		}
	}
#endif

	bool isInit = true;		// 初期化成功か判別するフラグ

	// D3D12デバイスの生成

	ID3D12Device *dev = nullptr;		// デバイスポインタ
	HRESULT result = S_OK;
	D3D_FEATURE_LEVEL levels[] = {
		D3D_FEATURE_LEVEL_12_1,
		D3D_FEATURE_LEVEL_12_0,
		D3D_FEATURE_LEVEL_11_1,
		D3D_FEATURE_LEVEL_11_0,
	};	// フィーチャレベル配列

	D3D_FEATURE_LEVEL level = {};		// デバイスで使用可能なフィーチャーレベル

	for (auto l : levels)
	{
		result = D3D12CreateDevice(nullptr, l, IID_PPV_ARGS(&dev));
		if (result == S_OK)	// 最新のフィーチャーレベルでデバイスを生成出来たら
		{
			// レベルを保存してループを脱出
			level = l;
			break;
		}
	}

	if (result != S_OK)
	{
		isInit = false;
	}


	//コマンド関連変数定義
	ID3D12CommandAllocator* _commandAllocator = nullptr;	//コマンドアロケータ
	ID3D12CommandQueue* _commandQueue = nullptr;			//コマンドキュー
	ID3D12GraphicsCommandList * _commandList = nullptr;		// コマンドリスト

	// コマンドアロケータの生成処理
	result = dev->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&_commandAllocator));
	if (result != S_OK)
	{
		isInit = false;
	}

	// コマンドリストの生成処理
	result = dev->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, _commandAllocator, nullptr, IID_PPV_ARGS(&_commandList));
	if (result != S_OK)
	{
		isInit = false;
	}

	// コマンドキューの生成処理
	D3D12_COMMAND_QUEUE_DESC desc = {};
	desc.NodeMask = 0;
	desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	desc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
	desc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
	result = dev->CreateCommandQueue(&desc, IID_PPV_ARGS(&_commandQueue));
	if (result != S_OK)
	{
		isInit = false;
	}

	// スワップチェイン生成処理
	IDXGIFactory4 * factory = nullptr;		// DXGIインタフェースの生成機
	result = CreateDXGIFactory1(IID_PPV_ARGS(&factory));
	if (result != S_OK)
	{
		isInit = false;
	}

	DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};//スワップチェイン設定の項目？
	IDXGISwapChain3 *swapChain;						// スワップチェイン

	swapChainDesc.Width = WINDOW_WIDTH;
	swapChainDesc.Height = WINDOW_HEIGHT;
	swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainDesc.Stereo = false;
	swapChainDesc.SampleDesc.Count = 1;
	swapChainDesc.SampleDesc.Quality = 0;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.BufferCount = 2;
	swapChainDesc.Scaling = DXGI_SCALING_STRETCH;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	swapChainDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
	swapChainDesc.Flags = 0;


	// スワップチェーン生成
	result = factory->CreateSwapChainForHwnd(_commandQueue,
		hwnd,
		&swapChainDesc,
		nullptr,
		nullptr,
		(IDXGISwapChain1**)(&swapChain));
	if (result != S_OK)
	{
		isInit = false;
	}

	// ディスクリプタ生成処理
	// ディスクリプタヒープ用パラメタ定義
	D3D12_DESCRIPTOR_HEAP_DESC descriptorHeapDesc = {};
	descriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	descriptorHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	descriptorHeapDesc.NumDescriptors = 2;
	descriptorHeapDesc.NodeMask = 0;
	//ディスクリプタヒープ生成
	ID3D12DescriptorHeap* descriptorHeap = nullptr;
	result = dev->CreateDescriptorHeap(&descriptorHeapDesc, IID_PPV_ARGS(&descriptorHeap));
	if (result != S_OK)
	{
		isInit = false;
	}

	// ディスクリプタヒープのサイズを取得
	UINT descriptorHeapSize = 0;
	descriptorHeapSize = dev->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

	// ディスクリプタハンドルの作成
	CD3DX12_CPU_DESCRIPTOR_HANDLE descriptorHandle(descriptorHeap->GetCPUDescriptorHandleForHeapStart());


	//レンダーターゲットビューの生成
	std::vector<ID3D12Resource*> renderTargets;		//レンダーターゲットの実体
	DXGI_SWAP_CHAIN_DESC swcDesc = {};				// スワップチェインの情報
	swapChain->GetDesc(&swcDesc);

	// レンダーターゲット数を取得
	int renderTargetsNum = swcDesc.BufferCount;

	// レンダーターゲット数だけ領域確保
	renderTargets.resize(renderTargetsNum);

	// ディスクリプタ一つ当たりのサイズを取得
	int descriptorSize = dev->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

	for (int i = 0; i < renderTargetsNum; i++)
	{
		result = swapChain->GetBuffer(i, IID_PPV_ARGS(&renderTargets[i]));	//スワップチェインからキャンバスを取得
		if (result != S_OK)
		{
			isInit = false;
		}
		dev->CreateRenderTargetView(renderTargets[i], nullptr, descriptorHandle);
		descriptorHandle.Offset(descriptorSize);//ディスクリプタとキャンバス分オフセット
	}


	// ルートシグネチャの作成
	ID3D12RootSignature* rootSignature = nullptr;	//ルートシグネチャ
	ID3DBlob* signature = nullptr;					// シグネチャ
	ID3DBlob* error = nullptr;

	D3D12_ROOT_SIGNATURE_DESC rsd = {};
	rsd.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
	// シグネチャの作成
	result = D3D12SerializeRootSignature(&rsd, D3D_ROOT_SIGNATURE_VERSION_1, &signature, &error);
	if (result != S_OK)
	{
		isInit = false;
	}

	// シグネチャの情報を参照してルートシグネチャを生成
	result = dev->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&rootSignature));
	if (result != S_OK)
	{
		isInit = false;
	}

	// フェンスの実装
	ID3D12Fence *_fence = nullptr;
	UINT64 _fenceValue = 0;
	result = dev->CreateFence(_fenceValue, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&_fence));
	if (result != S_OK)
	{
		isInit = false;
	}

	if (!isInit)
	{
		MessageBox(hwnd, TEXT("DirectXの初期化に失敗しました"), TEXT("初期化失敗"), MB_OK);
		return -1;
	}



	/*三角ポリの表示*/
	// 三角ポリの頂点定義
	Vertex vertices[] = { { { 0.0f,0.7f,0.0f } },
	{ { 0.4f,-0.5f,0.0f } },
	{ { -0.4f,-0.5f,0.0f } }, };

	// 頂点レイアウトの定義
	D3D12_INPUT_ELEMENT_DESC inputLayoutDescs[] = {
		{ "POSITION",  0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }, 
	};

	// 頂点バッファ作成
	ID3D12Resource *_vertexBuffer = nullptr;
	dev->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(sizeof(vertices)),
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&_vertexBuffer));

	char* buf;


	_vertexBuffer->Map(0, nullptr, (void**)&buf);
	memcpy(buf, vertices, sizeof(vertices));
	_vertexBuffer->Unmap(0, nullptr);

	/*
	FILE *fp;
	fopen_s(&fp, "memoryDump.bin", "wb");
	fwrite(buf, sizeof(vertices), 1, fp);
	fclose(fp);
	*/

	// 頂点バッファビューの宣言
	D3D12_VERTEX_BUFFER_VIEW _vbView = {};
	_vbView.BufferLocation = _vertexBuffer->GetGPUVirtualAddress();
	_vbView.StrideInBytes = sizeof(Vertex);
	_vbView.SizeInBytes = sizeof(vertices);
	
	// シェーダの読み込み
	ID3DBlob* vertexShader = nullptr;
	ID3DBlob* pixelShader = nullptr;


	result = D3DCompileFromFile(TEXT("shader.hlsl"), 
		nullptr,
		nullptr, 
		"BaseVS",
		"vs_5_0",
		D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION,
		0,
		&vertexShader,
		nullptr);

	if (FAILED(result))
	{
		isInit = false;
	}
	result = D3DCompileFromFile(TEXT("shader.hlsl"),
		nullptr,
		nullptr,
		"BasePS",
		"ps_5_0",
		D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION,
		0,
		&pixelShader,
		nullptr);

	if (FAILED(result))
	{
		isInit = false;
	}

	// PSO初期化
	D3D12_GRAPHICS_PIPELINE_STATE_DESC gpsDesc = {};
	ID3D12PipelineState* _pipelineState = nullptr;

	gpsDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	gpsDesc.DepthStencilState.DepthEnable = FALSE;					// 深度を使用するかどうか
	gpsDesc.DepthStencilState.StencilEnable = FALSE;				// マスクを使用するかどうか
	gpsDesc.VS = CD3DX12_SHADER_BYTECODE(vertexShader);
	gpsDesc.PS = CD3DX12_SHADER_BYTECODE(pixelShader);
	gpsDesc.InputLayout.NumElements = sizeof(inputLayoutDescs) / sizeof(D3D12_INPUT_ELEMENT_DESC);
	gpsDesc.InputLayout.pInputElementDescs = inputLayoutDescs;
	gpsDesc.pRootSignature = rootSignature;
	gpsDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	gpsDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
	gpsDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	gpsDesc.NumRenderTargets = 1;
	gpsDesc.SampleDesc.Count = 1;
	gpsDesc.SampleMask = 0;

	result = dev->CreateGraphicsPipelineState(&gpsDesc, IID_PPV_ARGS(&_pipelineState));

	if (FAILED(result))
	{
		isInit = false;
	}
	/*カラークリア処理*/
	float color[4] = { 0.0f,0.0f,1.0f,1.0f };

	D3D12_VIEWPORT vp;
	vp.TopLeftX = 0;
	vp.TopLeftY = 0;
	vp.Width = WINDOW_WIDTH;
	vp.Height = WINDOW_HEIGHT;
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;



	// 処理失敗
	if (!isInit)
	{
		MessageBox(hwnd, TEXT("DirectXの初期化に失敗しました"), TEXT("初期化失敗"), MB_OK);
		return -1;
	}


	int bbIndex = 0;
	// ウィンドウズメインループ
	MSG msg = {};
	while (true)
	{
		if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
			if (msg.message == WM_QUIT)
			{
				break;
			}
		}
		else
		{


			// ルートシグニチャを設定
			_commandList->SetGraphicsRootSignature(rootSignature);

			// ビューポートのセット
			_commandList->RSSetViewports(1, &vp);

			// シザーレクトのセット
			D3D12_RECT rc = { 0,0,WINDOW_WIDTH, WINDOW_HEIGHT };
			_commandList->RSSetScissorRects(1, &rc);

			// 描画先バッファの取得
			bbIndex = swapChain->GetCurrentBackBufferIndex();
			CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(descriptorHeap->GetCPUDescriptorHandleForHeapStart(), bbIndex, descriptorSize);

			// リソースバリア
			_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(renderTargets[bbIndex], D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));


			// 描画先変更処理
			_commandList->OMSetRenderTargets(1, &rtvHandle, false, nullptr);

			// カラーの変更
			_commandList->ClearRenderTargetView(rtvHandle, color, 0, nullptr);


			// プリミティブトポロジのセット
			_commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

			// 頂点バッファビューのセット
			_commandList->IASetVertexBuffers(0, 1, &_vbView);
			_commandList->SetPipelineState(_pipelineState);

			// 頂点データのドロー
			_commandList->DrawInstanced(3, 1, 0, 0);

			// リソースバリア
			_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(renderTargets[bbIndex], D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));

			// コマンドの記録終了
			_commandList->Close();


			// コマンドリストの実行
			_commandQueue->ExecuteCommandLists(1, (ID3D12CommandList* const*)&_commandList);

			// フェンスによる「待ち」の処理実装
			++_fenceValue;
			_commandQueue->Signal(_fence, _fenceValue);
			while (_fence->GetCompletedValue() != _fenceValue)
			{
			}

			// コマンドアロケータとリストのリセット処理
			result = _commandAllocator->Reset();
			result = _commandList->Reset(_commandAllocator, _pipelineState);

			// スワップ
			swapChain->Present(1, 0);
		}
	}
	
	//デバイスのリリース
	dev->Release();

	// ウィンドウクラスの開放
	UnregisterClass(w.lpszClassName, w.hInstance);

	return 0;
}


LRESULT WindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
	// ウィンドウ破棄時にメッセージが来る
	if (msg == WM_DESTROY)
	{
		// 終了処理
		PostQuitMessage(0);
		return 0;
	}
	return DefWindowProc(hwnd, msg, wparam, lparam);	//デフォルト関数に投げる
}
