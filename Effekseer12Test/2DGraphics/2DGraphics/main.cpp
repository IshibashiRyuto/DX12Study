#include<Windows.h>
#include "App.h"

#define WINDOW_WIDTH 640
#define WINDOW_HEIGHT 480

LRESULT WindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);

int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrev, LPSTR lpStr, int nCmdWnd)
{
	// �A�v���P�[�V�����N���X�̎��̂𐶐�
	App app;

	// �E�B���h�E�N���X�̓o�^
	WNDCLASSEX w = {};
	w.cbSize = sizeof(WNDCLASSEX);
	w.lpfnWndProc = (WNDPROC)WindowProc;
	w.lpszClassName = TEXT("2DGraphicsOnDirectX12");
	w.hInstance = GetModuleHandle(0);
	RegisterClassEx(&w);

	// �E�B���h�E�T�C�Y�w��
	RECT wrc = { 0,0,WINDOW_WIDTH, WINDOW_HEIGHT };		// �E�B���h�E�T�C�Y�̎w��
	AdjustWindowRect(&wrc, WS_OVERLAPPEDWINDOW, false);//�E�B���h�E�T�C�Y�̕␳

													   //�E�B���h�E��������
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

	// �E�B���h�E�\��
	ShowWindow(hwnd, SW_SHOW);

	// �A�v���P�[�V�����̃f�o�C�X����
	if (!app.Initialize(hwnd))
	{
		MessageBox(hwnd, TEXT("�f�o�C�X�̐����Ɏ��s���܂���"), TEXT("mistake!"),MB_OK);
		return -1;
	}

	// �E�B���h�E�Y���C�����[�v
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


	// �E�B���h�E�N���X�̊J��
	UnregisterClass(w.lpszClassName, w.hInstance);


	return 0;
}

LRESULT WindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
	// �E�B���h�E�j�����Ƀ��b�Z�[�W������
	if (msg == WM_DESTROY)
	{
		// �I������
		PostQuitMessage(0);
		return 0;
	}
	return DefWindowProc(hwnd, msg, wparam, lparam);	//�f�t�H���g�֐��ɓ�����
}
