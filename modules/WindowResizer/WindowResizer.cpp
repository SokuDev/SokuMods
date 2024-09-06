#include <windows.h>
#include <string>
#include <Shlwapi.h>

#define SWRS_USES_HASH
#include <swrs.h>

#include <UnionCast.hpp>
#include <Tamper.hpp>

using namespace std;

#define IS_KEY_DOWN(lParam) ((lParam & ((DWORD)1<<30)) == 0 && (lParam & ((DWORD)1<<31)) == 0)

DWORD CreateWindowExACallLoc = 0x007fb713+2;
DWORD SendMessagePtrLoc = 0x00857248;

HMODULE module;
char profilePath[1024 + MAX_PATH];

bool fullscreen;
bool sticky = false;
char stickyKey = 'P';

HWND windowBars;
HWND window;
LONG windowed_lStyle;
LONG windowed_lExStyle;
LONG fullscreen_lStyle;
LONG fullscreen_lExStyle;

static LRESULT oldWindowProc = NULL;

static int barsEnabled;

static int sizeEnabled;
static int sizeWidth;
static int sizeHeight;

static int posEnabled;
static int posX;
static int posY;

static int borderX;
static int borderY;

static const int baseWidth = 640;
static const int baseHeight = 480;

RECT rect;

void myExitProcess() {
	if (sizeEnabled && !fullscreen && (rect.left != 0 || rect.top != 0)) {
		char buff[16];
		wsprintf(buff, "%d", (rect.right - rect.left - borderX));
		WritePrivateProfileString("Size", "Width", buff, profilePath);
		wsprintf(buff, "%d", (rect.bottom - rect.top - borderY));
		WritePrivateProfileString("Size", "Height", buff, profilePath);
	}
	if (posEnabled && !fullscreen && (rect.left != 0 || rect.top != 0)) {
		char buff[16];
		wsprintf(buff, "%d", rect.left);
		WritePrivateProfileString("Position", "X", buff, profilePath);
		wsprintf(buff, "%d", rect.top);
		WritePrivateProfileString("Position", "Y", buff, profilePath);
	}
}

void toggleFullscreen() {
	if(!window) {
		// window not loaded yet, should not happen, but lets not crash.
		return;
	}
	fullscreen = !fullscreen;
	
	if (!fullscreen) {
		ShowCursor(1);
		if(barsEnabled)
			SetWindowPos(windowBars, 0, 0, 0, 0, 0, SWP_HIDEWINDOW|SWP_NOMOVE|SWP_NOSIZE|SWP_NOZORDER);

		SetWindowLong(window, GWL_STYLE, windowed_lStyle);
		SetWindowLong(window, GWL_EXSTYLE, windowed_lExStyle);
		HWND topmost = sticky? HWND_TOPMOST : HWND_NOTOPMOST;
		SetWindowPos(window, topmost, rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top, SWP_FRAMECHANGED|SWP_SHOWWINDOW);
	}
	else
	{
		GetWindowRect(window, &rect);
		
		HMONITOR hmon = MonitorFromWindow(window, MONITOR_DEFAULTTONEAREST);
		MONITORINFO mi = {sizeof(mi)};
		GetMonitorInfo(hmon, &mi);
		
		int monWidth = mi.rcMonitor.right - mi.rcMonitor.left;
		int monHeight = mi.rcMonitor.bottom - mi.rcMonitor.top;
		
		if(barsEnabled)
			SetWindowPos(windowBars, HWND_TOPMOST, mi.rcMonitor.left, mi.rcMonitor.top, mi.rcMonitor.right - mi.rcMonitor.left,mi.rcMonitor.bottom - mi.rcMonitor.top, SWP_SHOWWINDOW);
		
		int width;
		int height;
		if(monHeight * baseWidth / baseHeight <= monWidth) {
			width = monHeight * baseWidth / baseHeight;
			height = monHeight;
		} else {
			width = monWidth;
			height = monWidth * baseHeight / baseWidth;
		}
		int x = mi.rcMonitor.left + (monWidth - width) / 2;
		int y = mi.rcMonitor.top + (monHeight - height) / 2;

		SetWindowLong(window, GWL_STYLE, fullscreen_lStyle);
		SetWindowLong(window, GWL_EXSTYLE, fullscreen_lExStyle);
		SetWindowPos(window, HWND_TOPMOST, x, y, width, height, SWP_FRAMECHANGED | SWP_SHOWWINDOW);
	}
	*((char*)0x8998B0) = fullscreen;
}

void toggleSticky() {
	if(!window) {
		// window not loaded yet, should not happen, but lets not crash.
		return;
	}
	if(fullscreen) {
		return;
	}
	
	sticky = !sticky;
	
	if(sticky) {
		windowed_lExStyle |= WS_EX_TOPMOST;
		SetWindowPos(window, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
	}
	else {
		windowed_lExStyle &= ~WS_EX_TOPMOST;
		SetWindowPos(window, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
	}
	SetWindowLongPtr(window, GWL_EXSTYLE, windowed_lExStyle);
}

void toggleSize(int multiplier) {
	if(!window) {
		// window not loaded yet, should not happen, but lets not crash.
		return;
	}
	if(fullscreen) {
		return;
	}
	
	sizeWidth = baseWidth * multiplier;
	sizeHeight = MulDiv(sizeWidth, 3, 4);
	
	int fullWidth = sizeWidth + borderX;
	int fullHeight = sizeHeight + borderY;
	SetWindowPos(window, 0, 0, 0, fullWidth, fullHeight, SWP_NOMOVE | SWP_NOZORDER);
	GetWindowRect(window, &rect);
}

LRESULT CALLBACK keyboardHook(int nCode, WPARAM wParam, LPARAM lParam)
{
	if (nCode == HC_ACTION)
	{
		if (wParam == VK_RETURN && IS_KEY_DOWN(lParam) && GetAsyncKeyState(VK_MENU)) {
			toggleFullscreen();
			return 1;
		}
		if (wParam == stickyKey && IS_KEY_DOWN(lParam) && GetAsyncKeyState(VK_MENU)) {
			toggleSticky();
			return 1;
		}
		if (wParam >= '1' && wParam <= '6' && IS_KEY_DOWN(lParam) && GetAsyncKeyState(VK_MENU)) {
			int multiplier = wParam - '0';
			toggleSize(multiplier);
			return 1;
		}
	}
	return CallNextHookEx(NULL, nCode, wParam, lParam);
}

LRESULT (WINAPI *oldSendMessageA)(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam) = SendMessageA;
LRESULT WINAPI mySendMessageA(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam) {
	if(Msg == WM_SYSKEYDOWN && wParam == VK_RETURN && lParam == 0) {
		// the game calls SendMessageA(handle, WM_SYSKEYDOWN, VK_RETURN, 0) when it wants to toggle fullscreen.
		// we simply detour the call to hook that specific event, to eat it and toggle our own fullscreen.
		toggleFullscreen();
		return 0;
	} else {
		return oldSendMessageA(hWnd, Msg, wParam, lParam);
	}
}

LRESULT CALLBACK myWindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if(uMsg == WM_ACTIVATE || uMsg == WM_KILLFOCUS) {
		return DefWindowProc(hWnd, uMsg, wParam, lParam);
	}
	if (sizeEnabled && !fullscreen && uMsg == WM_SIZING) {
		RECT* window_size = (RECT*)lParam;
		window_size->right -= borderX;
		window_size->bottom -= borderY;
		int x = window_size->left;
		int y = window_size->top;
		int width = window_size->right - x;
		int height = window_size->bottom - y;
		if (width < baseWidth || height < baseHeight) {
			if (wParam == 2 || wParam == 6 || wParam == 8) {
				window_size->right = x + baseWidth;
				window_size->bottom = y + baseHeight;
			}
			else if (wParam == 1 || wParam == 3 || wParam == 4) {
				window_size->left = window_size->right - baseWidth;
				window_size->top = window_size->bottom - baseHeight;
			}
			else if (wParam == 5) {
				window_size->right = x + baseWidth;
				window_size->top = window_size->bottom - baseHeight;
			}
			else if (wParam == 7) {
				window_size->left = window_size->right - baseWidth;
				window_size->bottom = y + baseHeight;
			}
		}
		else if (wParam == 1 || wParam == 2 || wParam == 7 || wParam == 8) {
			window_size->bottom = y + (width * baseHeight / baseWidth);
		}
		else if (wParam == 4) {
			window_size->left = window_size->right - (height * baseWidth / baseHeight);
		}
		else if (wParam == 5) {
			window_size->right = window_size->left + (height * baseWidth / baseHeight);
		}
		else if (wParam == 6 || wParam == 3) {
			window_size->right = x + (height * baseWidth / baseHeight);
		}
		window_size->right += borderX;
		window_size->bottom += borderY;
	}
	if (!fullscreen && (uMsg == WM_MOVING || uMsg == WM_SIZING)) {
		RECT r = *(RECT *)lParam;
		rect = r;
	}
	return CallWindowProc((WNDPROC)oldWindowProc, hWnd, uMsg, wParam, lParam);
}

LRESULT CALLBACK windowHook(int nCode, WPARAM wParam, LPARAM lParam) {
	switch (nCode) {
	case HCBT_MINMAX:
		switch (lParam) {
		case SW_MINIMIZE:
			if (fullscreen && barsEnabled)
				ShowWindow(windowBars, SW_HIDE);
			break;
		case SW_RESTORE:
			if (fullscreen && barsEnabled)
				ShowWindow(windowBars, SW_SHOW);
			break;
		}
		break;
	}
	return CallNextHookEx(NULL, nCode, wParam, lParam);
}	

void createBars() {
	WNDCLASS wc = { 0 };
	wc.lpszClassName = "BlackBars";
	wc.lpfnWndProc = DefWindowProc;
	wc.hbrBackground = CreateSolidBrush(0x00000000);
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	RegisterClass(&wc);

	windowBars = CreateWindowEx(WS_EX_TOOLWINDOW, wc.lpszClassName, 0, WS_POPUP | WS_VISIBLE | WS_SYSMENU, 0, 0, 1, 1, 0, 0, 0, 0);
	SetWindowPos(windowBars, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
	EnableWindow(windowBars, false);
	ShowWindow(windowBars, SW_HIDE);
	
	// ugly fix for Soku going under the previous window despite having focus.
	// seems to be related to the black bars.
	// they get focus then transfer it to the previous window instead of to Soku?
	SetForegroundWindow(window);
}

void WINAPI hotkeyThread()
{
	while (true) {
		if (fullscreen && ((GetAsyncKeyState(VK_TAB) && GetAsyncKeyState(VK_MENU)) || GetAsyncKeyState(VK_LWIN) || GetAsyncKeyState(VK_RWIN))) {
			ShowWindow(window, SW_MINIMIZE);
			if(barsEnabled)
				ShowWindow(windowBars, SW_HIDE);
		}
		Sleep(100);
	}
}

HWND (__stdcall *oldCreateWindowExA)(DWORD dwExStyle, LPCSTR lpClassName, LPCSTR lpWindowName, DWORD dwStyle, int X, int Y, int nWidth, int nHeight, HWND hWndParent, HMENU hMenu, HINSTANCE hInstance, LPVOID lpParam) = CreateWindowExA;
HWND __stdcall myCreateWindowExA(DWORD dwExStyle, LPCSTR lpClassName, LPCSTR lpWindowName, DWORD dwStyle, int X, int Y, int nWidth, int nHeight, HWND hWndParent, HMENU hMenu, HINSTANCE hInstance, LPVOID lpParam) {
	windowed_lStyle = dwStyle | WS_GROUP | WS_CAPTION | WS_SYSMENU | WS_THICKFRAME;
	fullscreen_lStyle = windowed_lStyle & ~(WS_CAPTION | WS_THICKFRAME | WS_MINIMIZE | WS_MAXIMIZE | WS_SYSMENU);
	windowed_lExStyle = dwExStyle;
	fullscreen_lExStyle = windowed_lExStyle & ~(WS_EX_DLGMODALFRAME | WS_EX_CLIENTEDGE | WS_EX_STATICEDGE);
	
	{
		RECT r;
		r.top = 0;
		r.left = 0;
		r.right = baseWidth;
		r.bottom = baseHeight;
		AdjustWindowRectEx(&r, windowed_lStyle, false, windowed_lExStyle);
		borderX = r.right - r.left - baseWidth;
		borderY = r.bottom - r.top - baseHeight;
	}
	
	if(sizeEnabled) {
		nWidth = sizeWidth + borderX;
		nHeight = sizeHeight + borderY;
		windowed_lStyle = windowed_lStyle | WS_SIZEBOX;
	}
	if(posEnabled) {
		X = posX;
		Y = posY;
	}
	
	window = oldCreateWindowExA(windowed_lExStyle, lpClassName, lpWindowName, windowed_lStyle, X, Y, nWidth, nHeight, hWndParent, hMenu, hInstance, lpParam);
	
	oldWindowProc = SetWindowLong(window, GWL_WNDPROC, (long)myWindowProc);
	SetWindowsHookEx(WH_KEYBOARD, keyboardHook, module, GetCurrentThreadId());
	SetWindowsHookEx(WH_CBT, windowHook, module, GetCurrentThreadId());
	HANDLE thread = CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)hotkeyThread, NULL, NULL, NULL);
	CloseHandle(thread);
	
	SetForegroundWindow(window);
	
	if(barsEnabled) {
		createBars();
	}
	
	return window;
}
// Location pointing to myCreateWindowExA
HWND (__stdcall *myCreateWindowExAPtr)(DWORD dwExStyle, LPCSTR lpClassName, LPCSTR lpWindowName, DWORD dwStyle, int X, int Y, int nWidth, int nHeight, HWND hWndParent, HMENU hMenu, HINSTANCE hInstance, LPVOID lpParam) = myCreateWindowExA;

void loadConfig() {
	GetModuleFileName(module, profilePath, 1024);
	PathRemoveFileSpec(profilePath);
	PathAppend(profilePath, "WindowResizer.ini");
	
	sizeEnabled = GetPrivateProfileIntA("Size", "Enabled", 0, profilePath) != 0;
	if (sizeEnabled) {
		sizeWidth = GetPrivateProfileIntA("Size", "Width", baseWidth, profilePath);
		sizeHeight = ::MulDiv(sizeWidth, 3, 4);
	}
	
	posEnabled = GetPrivateProfileIntA("Position", "Enabled", 0, profilePath) != 0;
	if (posEnabled) {
		posX = GetPrivateProfileIntA("Position", "X", 0, profilePath);
		posY = GetPrivateProfileIntA("Position", "Y", 0, profilePath);
	}
	barsEnabled = GetPrivateProfileIntA("Bars", "Enabled", 1, profilePath) != 0;
	
	stickyKey = GetPrivateProfileIntA("Sticky", "Keybind", 'P', profilePath);
}

void setupHooks() {
	DWORD dwOldProtect;
	// disable resetting internal fullscreen/window state & setting to the current window state
	VirtualProtect((void *)0x00445817, 3, PAGE_EXECUTE_READWRITE, &dwOldProtect);
	*((char*)0x00445817) = 0x90;
	*((char*)0x00445818) = 0x90;
	*((char*)0x00445819) = 0x90;
	VirtualProtect((void *)0x00445817, 3, dwOldProtect, &dwOldProtect);
	VirtualProtect((void *)0x004405BC, 5, PAGE_EXECUTE_READWRITE, &dwOldProtect);
	*((char*)0x004405BC) = 0x90;
	*((char*)0x004405BD) = 0x90;
	*((char*)0x004405BE) = 0x90;
	*((char*)0x004405BF) = 0x90;
	*((char*)0x004405C0) = 0x90;
	VirtualProtect((void *)0x004405BC, 5, dwOldProtect, &dwOldProtect);
	
	// hardcode baseWidth & baseHeight values for directx viewport init instead of getting the window size
	VirtualProtect((void *)0x00414F8C, 16, PAGE_EXECUTE_READWRITE, &dwOldProtect);
	*((char*)0x00414F8C) = 0xBA;
	*((int*)0x00414F8D) = baseWidth;
	*((char*)0x00414F91) = 0x90;
	*((char*)0x00414F92) = 0x90;
	*((char*)0x00414F93) = 0x90;
	*((char*)0x00414F94) = 0xB8;
	*((int*)0x00414F95) = baseHeight;
	*((char*)0x00414F99) = 0x90;
	*((char*)0x00414F9A) = 0x90;
	*((char*)0x00414F9B) = 0x90;
	VirtualProtect((void *)0x00414F8C, 16, PAGE_EXECUTE_READWRITE, &dwOldProtect);
	
	// hook the one CreateWindowExA for the main window
	// it's call [addr] so we have to make a new memory location to store the pointer to our version of the function
	VirtualProtect((LPVOID)(CreateWindowExACallLoc), 4, PAGE_EXECUTE_READWRITE, &dwOldProtect);
	oldCreateWindowExA = *SokuLib::TamperDword(CreateWindowExACallLoc, &myCreateWindowExAPtr);
	VirtualProtect((LPVOID)(CreateWindowExACallLoc), 4, dwOldProtect, &dwOldProtect);
	
	// hook the SendMessageA, this is used in multiple places so we're hooking all of them
	// it's also call [addr] but we just change the value at addr
	VirtualProtect((LPVOID)(SendMessagePtrLoc), 4, PAGE_EXECUTE_READWRITE, &dwOldProtect);
	oldSendMessageA = SokuLib::TamperDword(SendMessagePtrLoc, mySendMessageA);
	VirtualProtect((LPVOID)(SendMessagePtrLoc), 4, dwOldProtect, &dwOldProtect);
	
	atexit(myExitProcess);
}

extern "C" __declspec(dllexport) bool CheckVersion(const BYTE hash[16]) {
	return ::memcmp(TARGET_HASH, hash, sizeof TARGET_HASH) == 0;
}

extern "C" __declspec(dllexport) bool Initialize(HMODULE hMyModule, HMODULE hParentModule) {
	module = hMyModule;
	
	loadConfig();
	setupHooks();
	
	FILE *_;

	#ifndef NDEBUG 
	AllocConsole();
	freopen_s(&_, "CONOUT$", "w", stdout);
	freopen_s(&_, "CONOUT$", "w", stderr);
	#endif
	
	return TRUE;
}

extern "C" int APIENTRY DllMain(HMODULE hModule, DWORD fdwReason, LPVOID lpReserved) {
	return TRUE;
}
