#include <windows.h>
#include <detours.h>
#include <string>
#include <Shlwapi.h>

#define SWRS_USES_HASH
#include <swrs.h>

using namespace std;

#define IS_KEY_DOWN(lParam) ((lParam & ((DWORD)1<<30)) == 0 && (lParam & ((DWORD)1<<31)) == 0)

HMODULE module;
char profilePath[1024 + MAX_PATH];

bool injected;
bool fullscreen;

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

HHOOK MessageHook = NULL;

void(WINAPI *oldExitProcess)(UINT uExitCode) = ExitProcess;
void __stdcall myExitProcess(UINT uExitCode) {
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
	return oldExitProcess(uExitCode);
}

bool wantsFullscreen = false;
void toggleFullscreen() {
	if(!window) {
		// window not loaded yet, store fullscreen request for when we inject it.
		wantsFullscreen = true;
		return;
	}
	fullscreen = !fullscreen;
	
	if (!fullscreen) {
		ShowCursor(1);
		if(barsEnabled)
			SetWindowPos(windowBars, 0, 0, 0, 0, 0, SWP_HIDEWINDOW|SWP_NOMOVE|SWP_NOSIZE|SWP_NOZORDER);

		SetWindowLong(window, GWL_STYLE, windowed_lStyle);
		SetWindowLong(window, GWL_EXSTYLE, windowed_lExStyle);
		SetWindowPos(window, HWND_NOTOPMOST, rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top, SWP_FRAMECHANGED|SWP_SHOWWINDOW);
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

LRESULT CALLBACK keyboardHook(int nCode, WPARAM wParam, LPARAM lParam)
{
	if (nCode == HC_ACTION)
	{
		if (wParam == VK_RETURN && IS_KEY_DOWN(lParam) && GetAsyncKeyState(VK_MENU)) {
			toggleFullscreen();
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

LRESULT CALLBACK injectHook(int nCode, WPARAM wParam, LPARAM lParam)
{
	if(injected) {
		return CallNextHookEx(MessageHook, nCode, wParam, lParam);
	}
	injected = true;

	window = FindWindow("th123_110a", NULL);

	windowed_lStyle = GetWindowLong(window, GWL_STYLE);
	fullscreen_lStyle = windowed_lStyle & ~(WS_CAPTION | WS_THICKFRAME | WS_MINIMIZE | WS_MAXIMIZE | WS_SYSMENU);
	windowed_lExStyle = GetWindowLong(window, GWL_EXSTYLE);
	fullscreen_lExStyle = windowed_lExStyle & ~(WS_EX_DLGMODALFRAME | WS_EX_CLIENTEDGE | WS_EX_STATICEDGE);

	LONG flags = SWP_NOMOVE | SWP_NOSIZE | SWP_FRAMECHANGED | SWP_SHOWWINDOW;
	if(sizeEnabled) {
		windowed_lStyle = windowed_lStyle | WS_SIZEBOX;
		flags &= ~SWP_NOSIZE;
	}
	if(posEnabled) {
		flags &= ~SWP_NOMOVE;
	}
	
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
	
	oldWindowProc = SetWindowLong(window, GWL_WNDPROC, (long)myWindowProc);
	SetWindowLong(window, GWL_STYLE, windowed_lStyle);
	SetWindowLong(window, GWL_EXSTYLE, windowed_lExStyle);
	SetWindowPos(window, 0, posX, posY, sizeWidth + borderX, sizeHeight + borderY, flags);
	GetWindowRect(window, &rect);

	SetWindowsHookEx(WH_KEYBOARD, keyboardHook, module, GetCurrentThreadId());
	SetWindowsHookEx(WH_CBT, windowHook, module, GetCurrentThreadId());
	HANDLE thread = CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)hotkeyThread, NULL, NULL, NULL);
	CloseHandle(thread);
	
	if(wantsFullscreen) {
		wantsFullscreen = false;
		toggleFullscreen();
	}

	UnhookWindowsHookEx(MessageHook);

	return CallNextHookEx(MessageHook, nCode, wParam, lParam);
}

extern "C" __declspec(dllexport) bool CheckVersion(const BYTE hash[16]) {
	return ::memcmp(TARGET_HASH, hash, sizeof TARGET_HASH) == 0;
}

extern "C" __declspec(dllexport) bool Initialize(HMODULE hMyModule, HMODULE hParentModule) {
	module = hMyModule;
	
	GetModuleFileName(hMyModule, profilePath, 1024);
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
	
	DWORD _;
	VirtualProtect((void *)0x00445817, 3, PAGE_EXECUTE_READWRITE, &_);
	*((char*)0x00445817) = 0x90;
	*((char*)0x00445818) = 0x90;
	*((char*)0x00445819) = 0x90;
	VirtualProtect((void *)0x004405BC, 5, PAGE_EXECUTE_READWRITE, &_);
	*((char*)0x004405BC) = 0x90;
	*((char*)0x004405BD) = 0x90;
	*((char*)0x004405BE) = 0x90;
	*((char*)0x004405BF) = 0x90;
	*((char*)0x004405C0) = 0x90;
	
	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());
	DetourAttach((void **)&oldExitProcess, (void *)myExitProcess);
	DetourAttach((void **)&oldSendMessageA, (void *)mySendMessageA);
	DetourTransactionCommit();

	if(barsEnabled)
		createBars();
	MessageHook = SetWindowsHookEx(WH_GETMESSAGE, injectHook, hMyModule, GetCurrentThreadId());
	
	return TRUE;
}

extern "C" int APIENTRY DllMain(HMODULE hModule, DWORD fdwReason, LPVOID lpReserved) {
	return TRUE;
}
