#include<Windows.h>
#include "App.h"

#define WINDOW_WIDTH 640
#define WINDOW_HEIGHT 480

LRESULT WindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);

int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrev, LPSTR lpStr, int nCmdWnd)
{
	// アプリケーションクラスの実体を生成
	App app;

	// ウィンドウクラスの登録
	WNDCLASSEX w = {};
	w.cbSize = sizeof(WNDCLASSEX);
	w.lpfnWndProc = (WNDPROC)WindowProc;
	w.lpszClassName = TEXT("2DGraphicsOnDirectX12");
	w.hInstance = GetModuleHandle(0);
	RegisterClassEx(&w);

	// ウィンドウサイズ指定
	RECT wrc = { 0,0,WINDOW_WIDTH, WINDOW_HEIGHT };		// ウィンドウサイズの指定
	AdjustWindowRect(&wrc, WS_OVERLAPPEDWINDOW, false);//ウィンドウサイズの補正

													   //ウィンドウ生成処理
	HWND hwnd;
	hwnd = CreateWindow(w.lpszClassName,
		TEXT("2DGraphicsOnDx12"),
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

	// アプリケーションのデバイス生成
	if (!app.Initialize(hwnd))
	{
		MessageBox(hwnd, TEXT("デバイスの生成に失敗しました"), TEXT("mistake!"),MB_OK);
		return -1;
	}

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
			app.Render();
		}
	}


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
