#include <windows.h>
#include <detours.h>
#include <shlwapi.h>

static char s_profilePath[1024 + MAX_PATH];

static bool s_sizeEnabled;
static int s_sizeWidth;
static int s_sizeHeight;
static int s_sizeWindowWidth;
static int s_sizeWindowHeight;

static bool s_posEnabled;
static int s_posX;
static int s_posY;
static WNDPROC s_origWndProc;

void(WINAPI *oldExitProcess)(UINT uExitCode) = ExitProcess;
ATOM(WINAPI *oldRegisterClassExA)(const WNDCLASSEXA *pWcex) = RegisterClassExA;
HWND(WINAPI *oldCreateWindowExA)
(DWORD dwExStyle, LPCSTR lpClassName, LPCSTR lpWindowName, DWORD dwStyle, int x, int y, int nWidth, int nHeight, HWND hWndParent, HMENU hMenu,
	HINSTANCE hInstance, LPVOID lpParam)
	= CreateWindowExA;
BOOL(WINAPI *oldSetWindowPos)(HWND hWnd, HWND hWndInsertAfter, int X, int Y, int cx, int cy, UINT uFlags) = SetWindowPos;
BOOL(WINAPI *oldGetWindowInfo)(HWND hwnd, PWINDOWINFO pwi) = GetWindowInfo;

void ClientToScreen(HWND hWnd, LPRECT lpRect) {
	POINT point;
	point.x = lpRect->left;
	point.y = lpRect->top;
	::ClientToScreen(hWnd, &point);
	lpRect->left = point.x;
	lpRect->top = point.y;
	point.x = lpRect->right;
	point.y = lpRect->bottom;
	::ClientToScreen(hWnd, &point);
	lpRect->right = point.x;
	lpRect->bottom = point.y;
}

LRESULT CALLBACK myWindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	if (s_sizeEnabled && uMsg == WM_SIZING) {
		RECT &rct = *(RECT *)lParam;

		// 新しいウィンドウ領域からクライアント領域を求める
		WINDOWPOS wndpos;
		wndpos.x = rct.left;
		wndpos.y = rct.top;
		wndpos.cx = rct.right - rct.left;
		wndpos.cy = rct.bottom - rct.top;
		wndpos.hwnd = hWnd;
		wndpos.hwndInsertAfter = NULL;
		wndpos.flags = SWP_NOZORDER | SWP_NOMOVE;

		NCCALCSIZE_PARAMS nccp;
		nccp.lppos = &wndpos;
		nccp.rgrc[0] = rct;
		::GetWindowRect(hWnd, &nccp.rgrc[1]);
		::GetClientRect(hWnd, &nccp.rgrc[2]);
		::ClientToScreen(hWnd, &nccp.rgrc[1]);
		::ClientToScreen(hWnd, &nccp.rgrc[2]);

		::SendMessage(hWnd, WM_NCCALCSIZE, TRUE, (LPARAM)&nccp);

		//クライアント領域を3:4補正して対応するウィンドウ領域を求める
		rct = nccp.rgrc[0];
		switch (wParam) {
		case WMSZ_LEFT:
		case WMSZ_RIGHT:
			rct.bottom = MulDiv(rct.right - rct.left, 3, 4) + rct.top;
			break;
		case WMSZ_TOPLEFT:
		case WMSZ_BOTTOMLEFT:
			rct.left = -MulDiv(rct.bottom - rct.top, 4, 3) + rct.right;
			break;
		default:
			rct.right = MulDiv(rct.bottom - rct.top, 4, 3) + rct.left;
			break;
		}
		s_sizeWidth = rct.right - rct.left;
		s_sizeHeight = rct.bottom - rct.top;

		DWORD style = ::GetWindowLong(hWnd, GWL_STYLE);
		DWORD exStyle = ::GetWindowLong(hWnd, GWL_EXSTYLE);
		::AdjustWindowRectEx(&rct, style, FALSE, exStyle);

		s_sizeWindowWidth = rct.right - rct.left;
		s_sizeWindowHeight = rct.bottom - rct.top;
		s_posX = rct.left;
		s_posY = rct.top;

		return TRUE;
	} else if (s_posEnabled && uMsg == WM_MOVING) {
		RECT &rct = *(RECT *)lParam;
		s_posX = rct.left;
		s_posY = rct.top;
	}

	return s_origWndProc(hWnd, uMsg, wParam, lParam);
}

BOOL __stdcall mySetWindowPos(HWND hWnd, HWND hWndInsertAfter, int X, int Y, int cx, int cy, UINT uFlags) {
	if (hWndInsertAfter != HWND_TOP) {
		if (s_sizeEnabled) {
			uFlags &= ~SWP_NOSIZE;
			cx = s_sizeWindowWidth;
			cy = s_sizeWindowHeight;
		}
		if (s_posEnabled) {
			if (s_posX != CW_USEDEFAULT)
				X = s_posX;
			if (s_posY != CW_USEDEFAULT)
				Y = s_posY;
		}
	}
	return oldSetWindowPos(hWnd, hWndInsertAfter, X, Y, cx, cy, uFlags);
}

HWND __stdcall myCreateWindowExA(DWORD dwExStyle, LPCSTR lpClassName, LPCSTR lpWindowName, DWORD dwStyle, int x, int y, int nWidth, int nHeight,
	HWND hWndParent, HMENU hMenu, HINSTANCE hInstance, LPVOID lpParam) {
	if (s_sizeEnabled) {
		dwStyle = WS_OVERLAPPED | WS_GROUP | WS_CAPTION | WS_SYSMENU | WS_THICKFRAME;
		RECT rct = {0, 0, s_sizeWidth, s_sizeHeight};
		::AdjustWindowRectEx(&rct, dwStyle, FALSE, dwExStyle);
		nWidth = s_sizeWindowWidth = rct.right - rct.left;
		nHeight = s_sizeWindowHeight = rct.bottom - rct.top;
	}

	if (s_posEnabled) {
		x = s_posX;
		y = s_posY;
	}

	return oldCreateWindowExA(dwExStyle, lpClassName, lpWindowName, dwStyle, x, y, nWidth, nHeight, hWndParent, hMenu, hInstance, lpParam);
}

ATOM __stdcall myRegisterClassExA(WNDCLASSEXA *pWcex) {
	s_origWndProc = pWcex->lpfnWndProc;
	pWcex->lpfnWndProc = myWindowProc;
	return oldRegisterClassExA(pWcex);
}

BOOL __stdcall myGetWindowInfo(HWND hwnd, PWINDOWINFO pwi) {
	pwi->rcClient.left = pwi->rcClient.top = 0;
	pwi->rcClient.right = 640;
	pwi->rcClient.bottom = 480;
	return TRUE;
}

void __stdcall myExitProcess(UINT uExitCode) {
	if (s_sizeEnabled) {
		char buff[16];
		::wsprintf(buff, "%d", s_sizeWidth);
		::WritePrivateProfileString("Size", "Width", buff, s_profilePath);
		::wsprintf(buff, "%d", s_sizeHeight);
		::WritePrivateProfileString("Size", "Height", buff, s_profilePath);
	}
	if (s_posEnabled) {
		char buff[16];
		::wsprintf(buff, "%d", s_posX);
		::WritePrivateProfileString("Position", "X", buff, s_profilePath);
		::wsprintf(buff, "%d", s_posY);
		::WritePrivateProfileString("Position", "Y", buff, s_profilePath);
	}
	return oldExitProcess(uExitCode);
}

void LoadSettings(LPCSTR profilePath) {
	s_sizeEnabled = ::GetPrivateProfileInt("Size", "Enabled", 0, profilePath) != 0;
	if (s_sizeEnabled) {
		s_sizeWidth = ::GetPrivateProfileInt("Size", "Width", 640, profilePath);
		s_sizeHeight = ::MulDiv(s_sizeWidth, 3, 4);
	}
	s_posEnabled = GetPrivateProfileInt("Position", "Enabled", 0, profilePath) != 0;
	if (s_posEnabled) {
		s_posX = ::GetPrivateProfileInt("Position", "X", CW_USEDEFAULT, profilePath);
		s_posY = ::GetPrivateProfileInt("Position", "Y", CW_USEDEFAULT, profilePath);
	}
}

extern "C" __declspec(dllexport) bool CheckVersion(const BYTE hash[16]) {
	return true;
}

extern "C" __declspec(dllexport) bool Initialize(HMODULE hMyModule, HMODULE hParentModule) {
	::GetModuleFileName(hMyModule, s_profilePath, 1024);
	::PathRemoveFileSpec(s_profilePath);
	::PathAppend(s_profilePath, "WindowResizer.ini");
	::LoadSettings(s_profilePath);

	DetourRestoreAfterWith();

	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());
	DetourAttach(&(PVOID &)oldExitProcess, myExitProcess);
	DetourAttach(&(PVOID &)oldRegisterClassExA, myRegisterClassExA);
	DetourAttach(&(PVOID &)oldCreateWindowExA, myCreateWindowExA);
	DetourAttach(&(PVOID &)oldSetWindowPos, mySetWindowPos);
	DetourAttach(&(PVOID &)oldGetWindowInfo, myGetWindowInfo);
	DetourTransactionCommit();
	return TRUE;
}

extern "C" int APIENTRY DllMain(HMODULE hModule, DWORD fdwReason, LPVOID lpReserved) {
	return TRUE;
}
