#pragma once
#include <Windows.h>
#include <iostream>
#include <string>
#include <Psapi.h>
#include <fstream>
using namespace std;

typedef HWND(WINAPI *mn_FindWindowW)(
	_In_opt_ LPCTSTR lpClassName,
	_In_opt_ LPCTSTR lpWindowName
	);

typedef DWORD(WINAPI *mn_GetWindowThreadProcessId)(
	_In_      HWND    hWnd,
	_Out_opt_ LPDWORD lpdwProcessId
	);

typedef HANDLE(WINAPI *mn_OpenProcess)(
	_In_ DWORD dwDesiredAccess,
	_In_ BOOL  bInheritHandle,
	_In_ DWORD dwProcessId
	);

typedef BOOL(WINAPI *mn_EnumProcessModulesEx)(
	_In_  HANDLE  hProcess,
	_Out_ HMODULE *lphModule,
	_In_  DWORD   cb,
	_Out_ LPDWORD lpcbNeeded,
	_In_  DWORD   dwFilterFlag
	);

typedef DWORD(WINAPI *mn_GetModuleFileNameEx)(
	_In_     HANDLE  hProcess,
	_In_opt_ HMODULE hModule,
	_Out_    LPTSTR  lpFilename,
	_In_     DWORD   nSize
	);

typedef BOOL(WINAPI *mn_ReadProcessMemory)(
	_In_  HANDLE  hProcess,
	_In_  LPCVOID lpBaseAddress,
	_Out_ LPVOID  lpBuffer,
	_In_  SIZE_T  nSize,
	_Out_ SIZE_T  *lpNumberOfBytesRead
	);

void LogWinAPI(string str)
{
	ofstream Logi("AlfaToC-LOG.txt", fstream::app);
	Logi << str + " №" << GetLastError() << endl;
	Logi.close();
}

void LogWin(string str)
{
	ofstream Logi("AlfaToC-LOG.txt", fstream::app);
	Logi << str << endl;
	Logi.close();
}

string XORCrypt(string str, unsigned char key)// unsigned char key = 3
{
	string result = "";
	for (char c : str)
	{
		result += c ^ key;
	}
	return result;
}

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



class WinApiDLL
{
	//private:
	//public:
	int Flag = 0;

	string n_FWW = "EjmgTjmgltB";
	string n_GWTPI = "DfwTjmgltWkqfbgSql`fppJg";
	string n_OP = "LsfmSql`fpp";
	unsigned char key = 3;
	string n_EPME = "FmvnSql`fppNlgvofpF{";
	string n_GMFNE = "DfwNlgvofEjofMbnfF{B";
	string n_RPM = "QfbgSql`fppNfnlqz";

	mn_FindWindowW MFindWindowA = 0;
	mn_GetWindowThreadProcessId MGetWindowThreadProcessId = 0;
	mn_OpenProcess MOpenProcess = 0;
	mn_EnumProcessModulesEx MEnumProcessModulesEx = 0;
	mn_GetModuleFileNameEx MGetModuleFileNameExA = 0;
	mn_ReadProcessMemory MReadProcessMemory = 0;

	HMODULE hUser32 = 0;
	HMODULE hKernel32 = 0;
	HMODULE hPsapi = 0;
public:

	WinApiDLL() //конструктор
	{
		
		//Flag = flag;
		hUser32 = LoadLibrary(TEXT("user32.dll"));
		hKernel32 = GetModuleHandle(TEXT("kernel32.dll"));
		hPsapi = LoadLibrary(TEXT("psapi.dll"));
		if (hUser32 == NULL)
		{
			cout << "In WinApiDLL:  LoadLibrary(user32.dll) eror №" << GetLastError() << endl;
		}
		if (hKernel32 == NULL)
		{
			cout << "In WinApiDLL:  GetModuleHandle(Kernel32.dll) eror №" << GetLastError() << endl;
		}
		if (hPsapi == NULL)
		{
			cout << "In WinApiDLL:  LoadLibrary(Psapi.dll) eror №" << GetLastError() << endl;
		}
	}

	void InitializationWords()// мне кажется это глупо , но вдруг будет статический анализ кода античитом на шаблонные слова(врятли но!)
	{
		n_FWW = XORCrypt(n_FWW, key);
		//LogWin(n_FWW);
		n_GWTPI = XORCrypt(n_GWTPI, key);
		//LogWin(n_GWTPI);
		n_OP = XORCrypt(n_OP, key);
		//LogWin(n_OP);
		n_EPME = XORCrypt(n_EPME, key);
		//LogWin(n_EPME);
		n_GMFNE = XORCrypt(n_GMFNE, key);
		//LogWin(n_GMFNE);
		n_RPM = XORCrypt(n_RPM, key);
		//LogWin(n_RPM);
	}

	bool InitializationFunction()
	{

		MFindWindowA = (mn_FindWindowW)GetProcAddress(hUser32, n_FWW.c_str());
		if (MFindWindowA == NULL)
		{
			LogWinAPI("In WinApiDLL:  InitializationFunction(FWA) ");
			return 0;
		}

		MGetWindowThreadProcessId = (mn_GetWindowThreadProcessId)GetProcAddress(hUser32, n_GWTPI.c_str());
		if (MGetWindowThreadProcessId == NULL)
		{
			LogWinAPI("In WinApiDLL:  InitializationFunction(GWTPI) ");
			return 0;
		}

		MOpenProcess = (mn_OpenProcess)GetProcAddress(hKernel32, n_OP.c_str());
		if (MOpenProcess == NULL)
		{
			LogWinAPI("In WinApiDLL:  InitializationFunction(OP) ");
			return 0;
		}

		MEnumProcessModulesEx = (mn_EnumProcessModulesEx)GetProcAddress(hPsapi, n_EPME.c_str());
		if (MEnumProcessModulesEx == NULL)
		{
			LogWinAPI("In WinApiDLL:  InitializationFunction(EPME) ");
			return 0;
		}

		MGetModuleFileNameExA = (mn_GetModuleFileNameEx)GetProcAddress(hPsapi, n_GMFNE.c_str());
		if (MGetModuleFileNameExA == NULL)
		{
			LogWinAPI("In WinApiDLL:  InitializationFunction(GMFNEA) ");
			return 0;
		}

		MReadProcessMemory = (mn_ReadProcessMemory)GetProcAddress(hKernel32, n_RPM.c_str());
		if (MReadProcessMemory == NULL)
		{
			LogWinAPI("In WinApiDLL:  InitializationFunction(RPM) ");
			return 0;
		}

		n_FWW = XORCrypt(n_FWW, key);
		n_GWTPI = XORCrypt(n_GWTPI, key);
		n_OP = XORCrypt(n_OP, key);
		n_EPME = XORCrypt(n_EPME, key);
		n_GMFNE = XORCrypt(n_GMFNE, key);
		n_RPM = XORCrypt(n_RPM, key);

		return true;
	}

	HWND _FindWindowA(LPCTSTR lpClassName, LPCTSTR lpWindowName)
	{
		return MFindWindowA(lpClassName, lpWindowName);
	}

	DWORD _GetWindowThreadProcessId(HWND hWnd, LPDWORD lpdwProcessId)
	{
		return MGetWindowThreadProcessId(hWnd, lpdwProcessId);
	}

	HANDLE _OpenProcess(DWORD dwDesiredAccess, BOOL  bInheritHandle, DWORD dwProcessId)
	{
		return MOpenProcess(dwDesiredAccess, bInheritHandle, dwProcessId);
	}

	BOOL _EnumProcessModulesEx(HANDLE  hProcess, HMODULE *lphModule, DWORD cb, LPDWORD lpcbNeeded, DWORD   dwFilterFlag)
	{
		return MEnumProcessModulesEx(hProcess, lphModule, cb, lpcbNeeded, dwFilterFlag);
	}

	DWORD _GetModuleFileNameExA(HANDLE hProcess, HMODULE hModule, LPTSTR lpFilename, DWORD  nSize)
	{
		return MGetModuleFileNameExA(hProcess, hModule, lpFilename, nSize);
	}

	BOOL _ReadProcessMemory(HANDLE hProcess, LPCVOID lpBaseAddress, LPVOID lpBuffer, SIZE_T nSize, SIZE_T *lpNumberOfBytesRead)
	{
		return MReadProcessMemory(hProcess, lpBaseAddress, lpBuffer, nSize, lpNumberOfBytesRead);
	}

	~WinApiDLL() { FreeLibrary(hUser32);  FreeLibrary(hPsapi); };//деструктор
};
