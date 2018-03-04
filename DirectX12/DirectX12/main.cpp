#include <Windows.h>


#include "Dx12Device.h"



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

	
	// D3D12デバイスの生成
	if (!Dx12Device::Instance()->CreateDevice(hwnd))
	{
		return -1;
	}

	/*カラークリア処理*/
	float color[4] = { 0.0f,0.0f,1.0f,1.0f };



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
			Dx12Device::Instance()->Render();
		}
	}
	
	/*終了処理*/
	Dx12Device::Instance()->Release();

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
