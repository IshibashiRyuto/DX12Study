#include <Windows.h>


#include "Dx12Device.h"



LRESULT WindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);

int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrevInst, LPSTR lpStr, int nCmdShow)
{

	// �E�B���h�E�N���X�̓o�^
	WNDCLASSEX w = {};
	w.cbSize = sizeof(WNDCLASSEX);
	w.lpfnWndProc = (WNDPROC)WindowProc;
	w.lpszClassName = TEXT("DirectXTest");
	w.hInstance = GetModuleHandle(0);
	RegisterClassEx(&w);

	// �E�B���h�E�T�C�Y�w��
	RECT wrc = { 0,0,WINDOW_WIDTH, WINDOW_HEIGHT };		// �E�B���h�E�T�C�Y�̎w��
	AdjustWindowRect(&wrc, WS_OVERLAPPEDWINDOW, false);//�E�B���h�E�T�C�Y�̕␳

	//�E�B���h�E��������
	HWND hwnd;
	hwnd = CreateWindow(w.lpszClassName,
		TEXT("DX12�e�X�g"),
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

	
	// D3D12�f�o�C�X�̐���
	if (!Dx12Device::Instance()->CreateDevice(hwnd))
	{
		return -1;
	}

	/*�J���[�N���A����*/
	float color[4] = { 0.0f,0.0f,1.0f,1.0f };



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
			Dx12Device::Instance()->Render();
		}
	}
	
	/*�I������*/
	Dx12Device::Instance()->Release();

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
