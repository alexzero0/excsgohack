//начало программы 
#define _CRT_SECURE_NO_WARNINGS

/*--------------------------------------\
|1) Сделать оверлей (AlfaOverlay)   +   |
|2) Вызывать все функции через dll	+   |
|3) Сделать ВХ и ХР бар             +   |
|4) Сделать запуск от Админа        +   |
|5) затереть PE                         |
|6) Resource Hacker  version info       |
|                                       |
|                                       |
\_____________________________________ */


#include <Windows.h>
#include "WinAPIDLL.h"
#include "Overlay.h"
#include "Offsets.h"


//Объявление глобальных переменных
HINSTANCE GhInstance, GhPrevInstance;
LPSTR GlpCmdLine; int GnCmdShow;
bool ThreadGUI = TRUE;
WinApiDLL WAD;
DWORD clientdll = 0;//адрес dll
HANDLE HandleGame = 0;// ключ процесса (дескриптор)
HWND HwndFindGame = 0;// хендл окна игры
bool ButtonVX = 0;
float view_matrix[4][4];

//Объявление потоковых функций
static DWORD WINAPI StartGUI(void* lpParam);
static DWORD WINAPI EasyHack(void* lpParam);
// Объявление StartGUI функций
LONG WINAPI WndProc(HWND hwnd, UINT Message, WPARAM wparam, LPARAM lparam);


// Объявление EasyHack функций
DWORD* InitLibH(HWND &HOverlay);
bool FindGameDll();
int ReadMemory(LPCVOID addr, int num, void* buf);
void get_view_matrix();
int world_to_screen(float* coord, RECT rect, float* pOut, float* Rost, int hp);
int WorldToScreen(float* coord, RECT rect, float* pOut, float* Rost);
float to2(float* to, float* rost);

// Объявление Вспомогательных функций
void LogWinAPI(string str);
void LogWin(string str);
string XORCrypt(string str, unsigned char key);// unsigned char key = 3


int __stdcall WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	/*
	1) запуск GUI
	
	2) Расшифрофка и Зашифровка важных названий, так же поиск нужных функций 
	
	3) ловить флаги с GUI и запускать код
	*/
	GhInstance = hInstance;
	GhPrevInstance = hPrevInstance;
	GlpCmdLine = lpCmdLine;
	GnCmdShow = nCmdShow;


	CreateThread(0, 0, StartGUI, 0, 0, 0);
	CreateThread(0, 0, EasyHack, 0, 0, 0);

	while (ThreadGUI)
	{
		Sleep(3000);
	}
}

static DWORD WINAPI StartGUI(void* lpParam)
{
	HWND hwnd; // дескриптор окна
	MSG msg;   // структура сообщения
	WNDCLASS w; // структура класса окна
	memset(&w, 0, sizeof(WNDCLASS)); // очистка памяти для структуры
	w.style = CS_HREDRAW | CS_VREDRAW;
	w.lpfnWndProc = WndProc;
	w.hInstance = GhInstance;
	w.hbrBackground = CreateSolidBrush(0x00FFFFFF);//CreateSolidBrush(0x00FFFFFF)//HBRUSH(COLOR_GRAYTEXT+13)
	w.lpszClassName = "MyClassGO";
	RegisterClass(&w); // регистрация класса окна

					   // Создание окна
	hwnd = CreateWindow("MyClassGO",
		"AlfaToC",
		WS_OVERLAPPED | WS_BORDER | WS_SYSMENU,
		500, 300, 500, 145,
		NULL, NULL, GhInstance, NULL);

	SetWindowLong(hwnd, GWL_EXSTYLE, GetWindowLong(hwnd, GWL_EXSTYLE) | WS_EX_LAYERED);
	SetLayeredWindowAttributes(hwnd, 0, 190, LWA_ALPHA);

	ShowWindow(hwnd, GnCmdShow); // отображение окна
	UpdateWindow(hwnd);         // перерисовка окна
								// Цикл обработки сообщений
	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	ThreadGUI = !ThreadGUI;
	return msg.wParam;
}

static DWORD WINAPI EasyHack(void* lpParam)
{
	/*
	1) WinApi
	2) Overlay
	3) OffsetsGame	
	*/
	HWND hWndOverlay;
	DWORD* HelpIDirect3=0;
	HelpIDirect3 = InitLibH(hWndOverlay);
	IDirect3DDevice9 *IDirect3 = reinterpret_cast<IDirect3DDevice9*>(HelpIDirect3);

	RECT SizeWindowGame;
	int Owidth, Oheight;

	ID3DXLine *line;
	D3DXCreateLine(IDirect3, &line);
	line->SetWidth(1.0f);
	line->SetPattern(0xffffffff);
	line->SetAntialias(FALSE);
	D3DXVECTOR2 box[5];

	SetWindowLong(hWndOverlay, GWL_EXSTYLE, (int)GetWindowLong(hWndOverlay, GWL_EXSTYLE) | WS_EX_TRANSPARENT | WS_EX_LAYERED);//WS_EX_LAYERED | WS_EX_TRANSPARENT//WS_EX_TOPMOST
	SetLayeredWindowAttributes(hWndOverlay, RGB(0, 0, 0), 0, LWA_COLORKEY);
	ShowWindow(hWndOverlay, GnCmdShow);

	for (;; Sleep(10))
	{
		while (ButtonVX)
		{
			Sleep(10);
			GetWindowRect(HwndFindGame, &SizeWindowGame);
			Owidth = SizeWindowGame.right - SizeWindowGame.left;
			Oheight = SizeWindowGame.bottom - SizeWindowGame.top;
			::SetWindowPos(hWndOverlay, HWND_TOPMOST, SizeWindowGame.left, SizeWindowGame.top, Owidth, Oheight, SWP_SHOWWINDOW);//SWP_SHOWWINDOW//SWP_NOMOVE|SWP_NOSIZE//SWP_SHOWWINDOW|SWP_NOSIZE

			IDirect3->Clear(0, NULL, D3DCLEAR_TARGET, D3DCOLOR_ARGB(0, 0, 0, 0), 1.0f, 0);
			IDirect3->BeginScene();

			DWORD Player = 0;
			int hp = 0, team_play = 0;
			float coord[3] = { 0 }, pOut[4] = { 0 }, Rost[4] = { 0 }, forhp[3] = { 0 };

			get_view_matrix();

			for (int i = 0; i < 10; i++)
			{
				ReadMemory((LPCVOID)(clientdll + Offset::StrucrPlayer + i * Offset::DistanceStruct), 4, &Player);
				//cout << Player << endl;
				if (Player == 0) continue;

				ReadMemory((LPCVOID)(Player + Offset::PlayerCoordX), 12, &coord);
				//cout << coord << endl;
				/*if (coord[0] == 0 || coord[1] == 0)
				continue;*/

				ReadMemory((LPCVOID)(Player + Offset::Health), 4, &hp);

				if ((hp <= 0) || (hp > 101)) continue;

				ReadMemory((LPCVOID)(Player + Offset::PlayerTeam), 4, &team_play);

				if (team_play == 2 || team_play == 3)
				{
					/*cout << "Player#" << i << " : " << endl;
					cout << "\t" << "team_play : " << team_play << " hp : " << hp << endl;
					cout << "\t" << coord[0] << " : " << coord[1] << " : " << coord[2]<<endl;*/
					if (team_play == 2)
					{
						get_view_matrix();
						if ((WorldToScreen(coord, SizeWindowGame, pOut, Rost) == 1) && ((hp > 0) && (hp < 101)))// (hp < 0)//(WorldToScreen(coord, SizeWindowGame, pOut, Rost) == 1)//(world_to_screen(coord, pOut, SizeWindowGame) == 1)
						{
							float heig = to2(pOut, Rost);
							float weght = heig / 4;
							box[0] = D3DXVECTOR2(pOut[0] - weght, pOut[1]);//
							box[1] = D3DXVECTOR2(pOut[0] + weght, pOut[1]);//pOut[2], pOut[3]
							box[2] = D3DXVECTOR2(Rost[0] + weght, Rost[1]);//Rost[0], Rost[1]
							box[3] = D3DXVECTOR2(Rost[0] - weght, Rost[1]);//Rost[2], Rost[3]
							box[4] = D3DXVECTOR2(pOut[0] - weght, pOut[1]);//pOut[0], pOut[1]
							line->Begin();
							line->Draw(box, 5, D3DCOLOR_ARGB(255, 255, 0, 0));
							line->End();
						}
					}
					if (team_play == 3)
					{
						get_view_matrix();
						if ((WorldToScreen(coord, SizeWindowGame, pOut, Rost) == 1) && ((hp > 0) && (hp < 101)))// (hp < 0)//(WorldToScreen(coord, SizeWindowGame, pOut, Rost) == 1)//(world_to_screen(coord, pOut, SizeWindowGame) == 1)
						{
							float heig = to2(pOut, Rost);
							float weght = heig / 4;
							box[0] = D3DXVECTOR2(pOut[0] - weght, pOut[1]);//-
							box[1] = D3DXVECTOR2(pOut[0] + weght, pOut[1]);//pOut[2], pOut[3]+
							box[2] = D3DXVECTOR2(Rost[0] + weght, Rost[1]);//Rost[0], Rost[1]+
							box[3] = D3DXVECTOR2(Rost[0] - weght, Rost[1]);//Rost[2], Rost[3]-
							box[4] = D3DXVECTOR2(pOut[0] - weght, pOut[1]);//pOut[0], pOut[1]-
							line->Begin();
							line->Draw(box, 5, D3DCOLOR_ARGB(255, 0, 0, 255));
							line->End();
							/*
							box[0] = D3DXVECTOR2(pOut[0] - weght, pOut[1]);//-
							box[1] = D3DXVECTOR2(pOut[0] + weght, pOut[1]);//pOut[2], pOut[3]+
							box[2] = D3DXVECTOR2(Rost[0] + weght, Rost[1]);//Rost[0], Rost[1]+
							box[3] = D3DXVECTOR2(Rost[0] - weght, Rost[1]);//Rost[2], Rost[3]-
							box[4] = D3DXVECTOR2(pOut[0] - weght, pOut[1]);//pOut[0], pOut[1]-
							*/
						}
					}
					get_view_matrix();
					if ((world_to_screen(coord, SizeWindowGame, pOut, Rost, hp) == 1) && ((hp > 0) && (hp < 101)))// (hp < 0)//(WorldToScreen(coord, SizeWindowGame, pOut, Rost) == 1)//(world_to_screen(coord, pOut, SizeWindowGame) == 1)
					{
						float heig = to2(pOut, Rost);
						float weght = heig / 3;
						box[0] = D3DXVECTOR2(pOut[0] - weght, pOut[1]);
						box[1] = D3DXVECTOR2(Rost[0] - weght, Rost[2]);
						/*
						box[0] = D3DXVECTOR2(pOut[0] - weght, pOut[2]);
						box[1] = D3DXVECTOR2(Rost[0] - weght, Rost[1]);
						*/


						line->Begin();
						line->Draw(box, 2, D3DCOLOR_ARGB(255, 0, 255, 0));
						line->End();
					}

				}
			}
			IDirect3->EndScene();
			IDirect3->Present(NULL, NULL, NULL, NULL);
		}
		while (!ButtonVX)
		{
			IDirect3->Clear(0, NULL, D3DCLEAR_TARGET, D3DCOLOR_ARGB(0, 0, 0, 0), 0.01f, 0);
			IDirect3->BeginScene();
			IDirect3->EndScene();
			IDirect3->Present(NULL, NULL, NULL, NULL);
			Sleep(100);
		}
	}
	line->Release();


	return 0;
}


//============ Функции StartGUI =================
// Функция обработки сообщений
LONG WINAPI WndProc(HWND hwnd, UINT Message, WPARAM wparam, LPARAM lparam)
{
	HDC hdc;
	HINSTANCE hInst;//trig,rad,neon
	PAINTSTRUCT ps;
	static HWND hBtn0, /* hBtn1,*/ hBtn2, hBtn3, hBtn4, hBtn5, hBtn6, hBtn7; // дескриптор кнопки
	static HWND hEdt1, hEdt2; // дескрипторы полей редактирования
	static HWND hStat, hStat1, hStat2; // дескриптор статического текста

	switch (Message)
	{
	case WM_CREATE: // сообщение создания окна
		hInst = ((LPCREATESTRUCT)lparam)->hInstance; // дескриптор приложения

		hBtn2 = CreateWindow("button", "Меню",
			WS_CHILD | WS_VISIBLE | WS_BORDER,
			5, 1, 475, 20,
			hwnd, 0, hInst, NULL);
		ShowWindow(hBtn2, SW_SHOWNORMAL);

		//// Создаем и показываем поле текста для результата
		hStat = CreateWindow("static", "привет %username% \n как твои дела ?",
			WS_CHILD | WS_VISIBLE | ES_CENTER,
			30, 30, 420, 70,
			hwnd, 0, hInst, NULL);
		ShowWindow(hStat, SW_SHOWNORMAL);

		hBtn0 = CreateWindow("button", "Присоедениться к игре",
			WS_CHILD | WS_VISIBLE | WS_BORDER | ES_CENTER,
			0, 21, 486, 20,
			hwnd, 0, hInst, NULL);
		ShowWindow(hBtn0, SW_HIDE);

		hEdt1 = CreateWindow("static", "",
			WS_CHILD | WS_VISIBLE | WS_BORDER | ES_RIGHT,
			240, 42, 124, 20,
			hwnd, 0, hInst, NULL);
		ShowWindow(hEdt1, SW_HIDE);

		hStat1 = CreateWindow("static", "Процесс игры : ",
			WS_CHILD | WS_VISIBLE | ES_LEFT,
			136, 42, 100, 20,
			hwnd, 0, hInst, NULL);
		ShowWindow(hStat1, SW_HIDE);

		hStat2 = CreateWindow("static", "                         -=Read=-                      |                      -=Write=-",
			WS_CHILD | WS_VISIBLE | WS_BORDER,
			1, 64, 482, 20,
			hwnd, 0, hInst, NULL);
		ShowWindow(hStat2, SW_HIDE);

		hBtn4 = CreateWindow("button", "Overlay&&D3D+VX",
			WS_CHILD | WS_VISIBLE | WS_BORDER,
			70, 85, 120, 20,
			hwnd, 0, hInst, NULL);
		ShowWindow(hBtn4, SW_HIDE);

		hBtn5 = CreateWindow("button", "Triger",
			WS_CHILD | WS_VISIBLE | WS_BORDER,
			250, 85, 80, 20,
			hwnd, 0, hInst, NULL);
		ShowWindow(hBtn5, SW_HIDE);

		hBtn6 = CreateWindow("button", "Radar",
			WS_CHILD | WS_VISIBLE | WS_BORDER,
			330, 85, 80, 20,
			hwnd, 0, hInst, NULL);
		ShowWindow(hBtn6, SW_HIDE);

		hBtn7 = CreateWindow("button", "ESP",
			WS_CHILD | WS_VISIBLE | WS_BORDER,
			410, 85, 76, 20,
			hwnd, 0, hInst, NULL);
		ShowWindow(hBtn7, SW_HIDE);

		/*hEdt2 = CreateWindow("static", "",
		WS_CHILD | WS_VISIBLE | WS_BORDER | ES_RIGHT,
		360, 63, 124, 20,
		hwnd, 0, hInst, NULL);
		ShowWindow(hEdt2, SW_HIDE);*/

		break;

	case WM_COMMAND:  // сообщение о команде

		if (lparam == (LPARAM)hBtn2)
		{
			ShowWindow(hStat, SW_HIDE);
			ShowWindow(hBtn0, SW_SHOWNORMAL);
		}
		if (lparam == (LPARAM)hBtn0)
		{
			ShowWindow(hEdt1, SW_SHOWNORMAL);
			ShowWindow(hStat1, SW_SHOWNORMAL);
			ShowWindow(hBtn3, SW_SHOWNORMAL);
			ShowWindow(hStat2, SW_SHOWNORMAL);
			ShowWindow(hBtn4, SW_SHOWNORMAL);
			ShowWindow(hBtn5, SW_SHOWNORMAL);
			ShowWindow(hBtn6, SW_SHOWNORMAL);
			ShowWindow(hBtn7, SW_SHOWNORMAL);
		}
		if (lparam == (LPARAM)hBtn4)
		{
			ButtonVX = !ButtonVX;
		}
		break;
	case WM_DESTROY: // закрытие окна
		PostQuitMessage(0);
		break;
	default: // обработка сообщения по умолчанию
		return DefWindowProc(hwnd, Message, wparam, lparam);
	}
	return 0;
}

//============ Функции EasyHack =================
DWORD* InitLibH(HWND &HOverlay)
{
	WAD.InitializationWords();
	if (WAD.InitializationFunction() == NULL)
	{
		LogWin("\tEror InitializationFunction ");
	}
	if (FindGameDll() == 0)
	{
		LogWin("\tFALSE FindGameDll ");
	}
	Overlay OverlayGame(HwndFindGame, GhInstance, GnCmdShow);
	OverlayGame.FindWidthHeight();
	OverlayGame.DisplayWidthHeight();
	OverlayGame.GlassyWindow();	
	OverlayGame.InitializationDirectX();

	HWND hWndOverlay;
	DWORD* HelpIDirect3;

	HelpIDirect3 = OverlayGame.GiveMeCrWinandIDirect3DDevice9(hWndOverlay);// CrWin
	HOverlay = hWndOverlay;
	return HelpIDirect3;// посмотреть под отладкой 
	//IDirect3DDevice9 *IDirect3 = reinterpret_cast<IDirect3DDevice9*>(HelpIDirect3);

}

bool FindGameDll()
{
	DWORD GameID = 0;
	HMODULE HmGameDll[1024];
	/*
	@lvmwfq.Pwqjhf9#Dolabo#Leefmpjuf : Counter-Strike: Global Offensive
	`ojfmw-goo : client.dll
	EjmgTjmgltB : FindWindowA
	DfwTjmgltWkqfbgSql`fppJg : GetWindowThreadProcessId
	LsfmSql`fpp : OpenProcess
	FmvnSql`fppNlgvofpF{ : EnumProcessModulesEx
	DfwNlgvofEjofMbnfF{ : GetModuleFileNameEx
	QfbgSql`fppNfnlqz : ReadProcessMemory
	*/
	string FindGame = XORCrypt("@lvmwfq.Pwqjhf9#Dolabo#Leefmpjuf", 3);
	string CLIENT = XORCrypt("`ojfmw-goo", 3);


	HwndFindGame = WAD._FindWindowA(0, FindGame.c_str());
	if (HwndFindGame == NULL)
	{
		LogWinAPI("Eror :_FWA ");
		return false;
	}

	WAD._GetWindowThreadProcessId(HwndFindGame, &GameID);
	LogWin("GID: "+ GameID);

	HandleGame = WAD._OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, 0, GameID);
	if (HandleGame == NULL)
	{
		LogWinAPI("Eror _OP: ");
		return false;
	}

	if (WAD._EnumProcessModulesEx(HandleGame, HmGameDll, sizeof(HmGameDll), &GameID, LIST_MODULES_32BIT) == FALSE)
	{
		LogWinAPI("Eror _EPME : ");
		return 0;
	}
	else
	{
		int g = 0;
		for (int i = 0; i < (GameID / sizeof(HMODULE)); i++)
		{
			TCHAR szModName[1024];
			if (WAD._GetModuleFileNameExA(HandleGame, HmGameDll[i], szModName, (sizeof(szModName) / sizeof(TCHAR))))
			{
				if (strstr(szModName, CLIENT.c_str()) != 0)// написать свое сравнение!
				{
					if (g == 0) { g++; continue; }
					//LogWin("cl base : " + HmGameDll[i]);
					clientdll = (DWORD)HmGameDll[i];
					//LogWin("cl base : " + clientdll);
					break;
				}
				if (clientdll != NULL)
				{
					return true;
				}
			}
		}
		return false;
	}
}

int ReadMemory(LPCVOID addr, int num, void* buf)
{
	SIZE_T sz = 0;
	int r = WAD._ReadProcessMemory(HandleGame, addr, buf, num, &sz);
	if (r == 0 || sz == 0)
	{
		LogWinAPI("Eror in RPMy : ");
		return 0;
	}
	return 1;
}

void get_view_matrix()
{
	DWORD VM = 0;
	ReadMemory((LPCVOID)(clientdll + 0x1EBA28 + 0x1), 4, &VM);
	ReadMemory((LPCVOID)(VM + 0x284), 64, &view_matrix);
	//cout << "\n adr viewM : " << Offset::ViewMatrix;
	//ReadProcessMemory((LPCVOID)(/*enginedll +*/ Offset::ViewMatrix), 64, &view_matrix);
	/*cout << "\n" << view_matrix[0][0] << " : " << view_matrix[0][1] << " : " << view_matrix[0][2] << " : " << view_matrix[0][3]  << endl;
	cout <<         view_matrix[1][0] << " : " << view_matrix[1][1] << " : " << view_matrix[1][2] << " : " << view_matrix[1][3]  << endl;
	cout <<         view_matrix[2][0] << " : " << view_matrix[2][1] << " : " << view_matrix[2][2] << " : " << view_matrix[2][3]  << endl;
	cout <<         view_matrix[3][0] << " : " << view_matrix[3][1] << " : " << view_matrix[3][2] << " : " << view_matrix[3][3]  << endl;*/
}

int world_to_screen(float* coord, RECT rect, float* pOut, float* Rost, int hp)
{
	float w = 0.0f;
	pOut[0] = view_matrix[0][0] * coord[0] + view_matrix[0][1] * coord[1] + view_matrix[0][2] * (coord[2] + 0) + view_matrix[0][3];
	pOut[1] = view_matrix[1][0] * coord[0] + view_matrix[1][1] * coord[1] + view_matrix[1][2] * (coord[2] + 0) + view_matrix[1][3];

	/*if (hp > 90)
	{
		pOut[2] = view_matrix[1][0] * coord[0] + view_matrix[1][1] * coord[1] + view_matrix[1][2] * (coord[2] + 0) + view_matrix[1][3];
	}
	if (hp <= 90 && hp > 80)
	{
		pOut[2] = view_matrix[1][0] * coord[0] + view_matrix[1][1] * coord[1] + view_matrix[1][2] * ((coord[2] + 0) + 10 * 0.75) + view_matrix[1][3];
	}
	if (hp <= 80 && hp > 70)
	{
		pOut[2] = view_matrix[1][0] * coord[0] + view_matrix[1][1] * coord[1] + view_matrix[1][2] * ((coord[2] + 0) + 20 * 0.75) + view_matrix[1][3];
	}
	if (hp <= 70 && hp > 60)
	{
		pOut[2] = view_matrix[1][0] * coord[0] + view_matrix[1][1] * coord[1] + view_matrix[1][2] * ((coord[2] + 0) + 30 * 0.75) + view_matrix[1][3];
	}
	if (hp <= 60 && hp > 50)
	{
		pOut[2] = view_matrix[1][0] * coord[0] + view_matrix[1][1] * coord[1] + view_matrix[1][2] * ((coord[2] + 0) + 40 * 0.75) + view_matrix[1][3];
	}
	if (hp <= 50 && hp > 40)
	{
		pOut[2] = view_matrix[1][0] * coord[0] + view_matrix[1][1] * coord[1] + view_matrix[1][2] * ((coord[2] + 0) + 50 * 0.75) + view_matrix[1][3];
	}
	if (hp <= 40 && hp > 30)
	{
		pOut[2] = view_matrix[1][0] * coord[0] + view_matrix[1][1] * coord[1] + view_matrix[1][2] * ((coord[2] + 0) + 60 * 0.75) + view_matrix[1][3];
	}
	if (hp <= 30 && hp > 20)
	{
		pOut[2] = view_matrix[1][0] * coord[0] + view_matrix[1][1] * coord[1] + view_matrix[1][2] * ((coord[2] + 0) + 70 * 0.75) + view_matrix[1][3];
	}
	if (hp <= 20 && hp > 10)
	{
		pOut[2] = view_matrix[1][0] * coord[0] + view_matrix[1][1] * coord[1] + view_matrix[1][2] * ((coord[2] + 0) + 80 * 0.75) + view_matrix[1][3];
	}
	if (hp <= 10 && hp > 0)
	{
		pOut[2] = view_matrix[1][0] * coord[0] + view_matrix[1][1] * coord[1] + view_matrix[1][2] * ((coord[2] + 0) + 90 * 0.75) + view_matrix[1][3];
	}*/

	w = view_matrix[3][0] * coord[0] + view_matrix[3][1] * coord[1] + view_matrix[3][2] * (coord[2] + 0) + view_matrix[3][3];
	if (w < 0.01f) { return 0; }
	float intw = 1.0f / w;
	pOut[0] *= intw;
	pOut[1] *= intw;
	//pOut[2] *= intw;//<----
	int weight = (int)(rect.right - rect.left);//(rect.right - rect.left);
	int height = (int)(rect.bottom - rect.top);//(rect.bottom - rect.top);
	float x = weight / 2;
	float y = height / 2;
	//float yh = height / 2;//<----------
	x += 0.5*pOut[0] * weight + 0.5;
	y -= 0.5*pOut[1] * height + 0.5;
	//yh -= 0.5*pOut[2] * height + 0.5;//<-------
	pOut[0] = x;//+ rect.left
	pOut[1] = y;//+ rect.top
	//pOut[2] = yh;

	//----------------------------------------------=-------------
	float w1 = 0.0f;
	Rost[0] = view_matrix[0][0] * coord[0] + view_matrix[0][1] * coord[1] + view_matrix[0][2] * (coord[2] + 70) + view_matrix[0][3];
	Rost[1] = view_matrix[1][0] * coord[0] + view_matrix[1][1] * coord[1] + view_matrix[1][2] * (coord[2] + 70) + view_matrix[1][3];

	if (hp > 90)
	{
		Rost[2] = view_matrix[1][0] * coord[0] + view_matrix[1][1] * coord[1] + view_matrix[1][2] * (coord[2] + 70) + view_matrix[1][3];
	}
	if (hp <= 90 && hp > 80)
	{
		Rost[2] = view_matrix[1][0] * coord[0] + view_matrix[1][1] * coord[1] + view_matrix[1][2] * ((coord[2] + 70) - 10 * 0.75) + view_matrix[1][3];
	}
	if (hp <= 80 && hp > 70)
	{
		Rost[2] = view_matrix[1][0] * coord[0] + view_matrix[1][1] * coord[1] + view_matrix[1][2] * ((coord[2] + 70) - 20 * 0.75) + view_matrix[1][3];
	}
	if (hp <= 70 && hp > 60)
	{
		Rost[2] = view_matrix[1][0] * coord[0] + view_matrix[1][1] * coord[1] + view_matrix[1][2] * ((coord[2] + 70) - 30 * 0.75) + view_matrix[1][3];
	}
	if (hp <= 60 && hp > 50)
	{
		Rost[2] = view_matrix[1][0] * coord[0] + view_matrix[1][1] * coord[1] + view_matrix[1][2] * ((coord[2] + 70) - 40 * 0.75) + view_matrix[1][3];
	}
	if (hp <= 50 && hp > 40)
	{
		Rost[2] = view_matrix[1][0] * coord[0] + view_matrix[1][1] * coord[1] + view_matrix[1][2] * ((coord[2] + 70) - 50 * 0.75) + view_matrix[1][3];
	}
	if (hp <= 40 && hp > 30)
	{
		Rost[2] = view_matrix[1][0] * coord[0] + view_matrix[1][1] * coord[1] + view_matrix[1][2] * ((coord[2] + 70) - 60 * 0.75) + view_matrix[1][3];
	}
	if (hp <= 30 && hp > 20)
	{
		Rost[2] = view_matrix[1][0] * coord[0] + view_matrix[1][1] * coord[1] + view_matrix[1][2] * ((coord[2] + 70) - 70 * 0.75) + view_matrix[1][3];
	}
	if (hp <= 20 && hp > 10)
	{
		Rost[2] = view_matrix[1][0] * coord[0] + view_matrix[1][1] * coord[1] + view_matrix[1][2] * ((coord[2] + 70) - 80 * 0.75) + view_matrix[1][3];
	}
	if (hp <= 10 && hp > 0)
	{
		Rost[2] = view_matrix[1][0] * coord[0] + view_matrix[1][1] * coord[1] + view_matrix[1][2] * ((coord[2] + 70) - 90 * 0.75) + view_matrix[1][3];
	}

	w1   =    view_matrix[3][0] * coord[0] + view_matrix[3][1] * coord[1] + view_matrix[3][2] * (coord[2] + 70) + view_matrix[3][3];
	if (w1 < 0.01f) { return 0; }
	float intw1 = 1.0f / w1;
	Rost[0] *= intw1;
	Rost[1] *= intw1;
	Rost[2] *= intw1;//<----
	int weight1 = (int)(rect.right - rect.left);//(rect.right - rect.left);
	int height1 = (int)(rect.bottom - rect.top);//(rect.bottom - rect.top);
	float x1 = weight1 / 2;
	float y1 = height1 / 2;
	float yh1 = height1 / 2;//<----------
	x1 += 0.5*Rost[0] * weight1 + 0.5;
	y1 -= 0.5*Rost[1] * height1 + 0.5;
	yh1 -= 0.5*Rost[2] * height1 + 0.5;//<-------
	Rost[0] = x1;//+ rect.left
	Rost[1] = y1;//+ rect.top
	Rost[2] = yh1;
	return 1;
}

int WorldToScreen(float* coord, RECT rect, float* pOut, float* Rost)
{
	float w = 0.0f;
	pOut[0] = view_matrix[0][0] * coord[0] + view_matrix[0][1] * coord[1] + view_matrix[0][2] * (coord[2] - 5) + view_matrix[0][3];
	pOut[1] = view_matrix[1][0] * coord[0] + view_matrix[1][1] * coord[1] + view_matrix[1][2] * (coord[2] - 5) + view_matrix[1][3];
	w = view_matrix[3][0] * coord[0] + view_matrix[3][1] * coord[1] + view_matrix[3][2] * (coord[2] - 5) + view_matrix[3][3];
	if (w < 0.01f) { return 0; }
	float intw = 1.0f / w;
	pOut[0] *= intw;
	pOut[1] *= intw;
	int weight = (int)(rect.right - rect.left);//(rect.right - rect.left);
	int height = (int)(rect.bottom - rect.top);//(rect.bottom - rect.top);
	float x = weight / 2;
	float y = height / 2;
	x += 0.5*pOut[0] * weight + 0.5;
	y -= 0.5*pOut[1] * height + 0.5;
	pOut[0] = x;//+ rect.left
	pOut[1] = y;//+ rect.top
				//----------------------------------------------=-------------
	float w1 = 0.0f;
	Rost[0] = view_matrix[0][0] * coord[0] + view_matrix[0][1] * coord[1] + view_matrix[0][2] * (coord[2] + 70) + view_matrix[0][3];
	Rost[1] = view_matrix[1][0] * coord[0] + view_matrix[1][1] * coord[1] + view_matrix[1][2] * (coord[2] + 70) + view_matrix[1][3];
	w1 = view_matrix[3][0] * coord[0] + view_matrix[3][1] * coord[1] + view_matrix[3][2] * (coord[2] + 70) + view_matrix[3][3];
	if (w1 < 0.01f) { return 0; }
	float intw1 = 1.0f / w1;
	Rost[0] *= intw1;
	Rost[1] *= intw1;
	int weight1 = (int)(rect.right - rect.left);//(rect.right - rect.left);
	int height1 = (int)(rect.bottom - rect.top);//(rect.bottom - rect.top);
	float x1 = weight1 / 2;
	float y1 = height1 / 2;
	x1 += 0.5*Rost[0] * weight1 + 0.5;
	y1 -= 0.5*Rost[1] * height1 + 0.5;
	Rost[0] = x1;//+ rect.left
	Rost[1] = y1;//+ rect.top
	return 1;
}

float to2(float* to, float* rost)//float* to//определеям высоту персонажа
{
	return sqrt(pow(to[1] - rost[1], 2));
}
