#pragma once

// for overlay
#include <d3d9.h>//d3d11
#include <d3dx9.h>//D3DX11
#include <d3dx9core.h>//D3DX11core
#include <Dwmapi.h> 

#pragma comment(lib, "d3d9.lib")
#pragma comment(lib, "d3dx9.lib")
#pragma comment(lib, "Dwmapi.lib")

#include <Windows.h>
#include <TlHelp32.h>
#include <iostream>
#include <string>
#include <fstream>
using namespace std;

LRESULT __stdcall WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
//void LogWinAPI(string str);
//void LogWin(string str);

class Overlay
{
	HWND OverlayGame = 0;
	int Owidth = 0;
	int Oheight = 0;
	int Dwidth = 0;
	int Dheight = 0;
	HINSTANCE hInstance;
	RECT rect;
	HWND CrWin;
	int nCmdShow;
	IDirect3DDevice9* d3dDevice = 0;
	bool FullScreen = 0;

public:

	Overlay(HWND OG, HINSTANCE hInst, int CmdShow)
	{
		OverlayGame = OG;
		hInstance = hInst;
		nCmdShow = CmdShow;
	}

	void FindWidthHeight()
	{
		GetWindowRect(OverlayGame, &rect);
		Owidth = rect.right - rect.left;
		Oheight = rect.bottom - rect.top;
		cout << "Owidth: " << Owidth << "  Oheight: " << Oheight << endl;
	}

	void DisplayWidthHeight()
	{
		RECT desktop;
		const HWND hDesktop = GetDesktopWindow();
		GetWindowRect(hDesktop, &desktop);
		Dwidth = desktop.right;
		Dheight = desktop.bottom;
		cout << "Dwidth: " << Dwidth << "  Dheight: " << Dheight << endl;
	}

	void GlassyWindow()
	{
		WNDCLASSEX wnd;//create window
		ZeroMemory(&wnd, sizeof(WNDCLASSEX));
		//GhInstance;
		wnd.cbSize = sizeof(WNDCLASSEX);
		wnd.style = CS_HREDRAW | CS_VREDRAW;//CS_GLOBALCLASS
		wnd.lpfnWndProc = WindowProc;//callback function 
		wnd.hInstance = hInstance;
		wnd.hCursor = LoadCursor(hInstance, IDC_ARROW);
		wnd.hbrBackground = (HBRUSH)RGB(0, 0, 0);
		wnd.lpszClassName = "WindowClass";
		wnd.lpszMenuName = "";

		if (RegisterClassEx(&wnd) == NULL)
		{
			LogWin("Eror RegisterClassEx: ");
		}
		else
		{
			LogWin("RegisterClassEx succesful!");
		}
		//проверка на окно игры 
		//if (Owidth <= 160 && Oheight <= 30)//w = 159 h = 27
		//{
		//	FullScreen = 1;
		//	CrWin = CreateWindowEx(0,
		//		"WindowClass",
		//		"",
		//		WS_EX_TOPMOST | WS_POPUP,//WS_BORDER// WS_OVERLAPPEDWINDOW | WS_VISIBLE// // WS_EX_TOPMOST | WS_POPUP//WS_OVERLAPPEDWINDOW | WS_POPUP
		//		rect.left, rect.top,
		//		Dwidth, Dheight,
		//		NULL,
		//		NULL,
		//		hInstance,
		//		NULL);
		//	// ? точно ли так ?
		//	//SetWindowLong(CrWin, GWL_EXSTYLE, (int)GetWindowLong(CrWin, GWL_EXSTYLE) | WS_EX_TRANSPARENT | WS_EX_LAYERED);//WS_EX_LAYERED | WS_EX_TRANSPARENT//WS_EX_TOPMOST
		//	//SetLayeredWindowAttributes(CrWin, RGB(0, 0, 0), 0, LWA_COLORKEY);
		//	//ShowWindow(CrWin, nCmdShow);//nCmdShow    //SW_SHOWMAXIMIZED

		//	if (CrWin == NULL)
		//	{
		//		LogWinAPI("Eror CrWin : ");
		//	}
		//	MSG msg;
		//	while (GetMessage(&msg, NULL, 0, 0))
		//	{
		//		TranslateMessage(&msg);
		//		DispatchMessage(&msg);
		//	}

		//}
		if(true)//else
		{
			FullScreen = 0;
			CrWin = CreateWindowEx(0,
				"WindowClass",
				"",
				WS_EX_TOPMOST | WS_POPUP,//WS_BORDER WS_OVERLAPPEDWINDOW | WS_VISIBLE
				rect.left, rect.top,
				Owidth, Oheight,
				NULL,
				NULL,
				hInstance,
				NULL);
			if (CrWin == NULL)
			{
				LogWinAPI("Eror CrWin : ");
			}
			//else if(CrWin != NULL)
			//{
			//	MSG msg;
			//	while (GetMessage(&msg, NULL, 0, 0))//PM_REMOVE
			//	{
			//		TranslateMessage(&msg);
			//		DispatchMessage(&msg);
			//	}
			//}
		}

	}

	void InitializationDirectX()
	{
		if (FullScreen==NULL)
		{
			LPDIRECT3D9 m_pD3D = 0;
			m_pD3D = Direct3DCreate9(D3D_SDK_VERSION);
			if (m_pD3D == NULL)
			{
				LogWin("D3D_SDK_VERSION false!");
			}
			D3DDISPLAYMODE d3ddm = { 0 };
			m_pD3D->GetAdapterDisplayMode(0, &d3ddm);


			D3DPRESENT_PARAMETERS d3dpp;
			ZeroMemory(&d3dpp, sizeof(d3dpp));

			d3dpp.Windowed = 1;//0 - FullScreen
			d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
			d3dpp.hDeviceWindow = CrWin;
			d3dpp.BackBufferFormat = D3DFMT_X8R8G8B8;//d3ddm.Format //D3DFMT_A8R8G8B8
			d3dpp.BackBufferWidth = Owidth;
			d3dpp.BackBufferHeight = Oheight - 15;//-15		
			d3dpp.EnableAutoDepthStencil = TRUE;
			d3dpp.AutoDepthStencilFormat = D3DFMT_D16;//игнорируется если EnableAutoDepthStencil true

			HRESULT d3dev = 0;
			d3dev = m_pD3D->CreateDevice(D3DADAPTER_DEFAULT,
				D3DDEVTYPE_HAL,
				CrWin,
				D3DCREATE_SOFTWARE_VERTEXPROCESSING,//D3DCREATE_SOFTWARE_VERTEXPROCESSING//D3DCREATE_HARDWARE_VERTEXPROCESSING
				&d3dpp,
				&d3dDevice);

			if (d3dev == D3D_OK) { LogWin("d3d is GOOD"); }
			else
			{
				LogWin("d3d is BAD: CrWin= ");
				if (d3dev == D3DERR_DEVICELOST) { LogWin("Eror D3DERR_DEVICELOST"); }
				if (d3dev == D3DERR_INVALIDCALL) { LogWin("Eror D3DERR_INVALIDCALL"); }
				if (d3dev == D3DERR_NOTAVAILABLE) { LogWin("Eror D3DERR_NOTAVAILABLE"); }
				if (d3dev == D3DERR_OUTOFVIDEOMEMORY) { LogWin("Eror D3DERR_OUTOFVIDEOMEMORY"); }
			}
		}
		//else if(FullScreen==TRUE){}
	}

	DWORD* GiveMeCrWinandIDirect3DDevice9(HWND &crwin)//, IDirect3DDevice9 *IDirect
	{
		crwin = CrWin;
		return reinterpret_cast<DWORD*>(d3dDevice);
	}

	~Overlay() {}
};

LRESULT __stdcall WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)//WPARAM LPARAM
{
	switch (message)
	{
	case WM_DESTROY:
	{
		PostQuitMessage(0);
		return 0;
	}
	break;
	}
	return DefWindowProc(hWnd, message, wParam, lParam);
}


//void LogWinAPI(string str)
//{
//	ofstream Logi("AlfaToC-LOG.txt", fstream::app);
//	Logi << str + " №" << GetLastError() << endl;
//	Logi.close();
//}
//
//void LogWin(string str)
//{
//	ofstream Logi("AlfaToC-LOG.txt", fstream::app);
//	Logi << str << endl;
//	Logi.close();
//}
