#include <windows.h>
#include <d3d9.h>
#include <detours.h>
#include <shlwapi.h>

const BYTE TARGET_HASH[16] = {0xdf, 0x35, 0xd1, 0xfb, 0xc7, 0xb5, 0x83, 0x31, 0x7a, 0xda, 0xbe, 0x8c, 0xd9, 0xf5, 0x3b, 0x2e};

HWND(WINAPI *oldCreateWindowExA)
(DWORD dwExStyle, LPCSTR lpClassName, LPCSTR lpWindowName, DWORD dwStyle, int x, int y, int nWidth, int nHeight, HWND hWndParent, HMENU hMenu,
	HINSTANCE hInstance, LPVOID lpParam)
	= CreateWindowExA;
BOOL(WINAPI *oldGetWindowInfo)(HWND hwnd, PWINDOWINFO pwi) = GetWindowInfo;
IDirect3D9 *(WINAPI *oldDirect3DCreate9)(UINT SDKVersion) = Direct3DCreate9;
HRESULT(WINAPI *oldCreateDevice)
(LPVOID IDirect3D9, UINT Adapter, D3DDEVTYPE DeviceType, HWND hFocusWindow, DWORD BehaviorFlags, D3DPRESENT_PARAMETERS *pPresentationParameters,
	IDirect3DDevice9 **ppReturnedDeviceInterface)
	= NULL;

HWND WINAPI myCreateWindowExA(DWORD dwExStyle, LPCSTR lpClassName, LPCSTR lpWindowName, DWORD dwStyle, int x, int y, int nWidth, int nHeight, HWND hWndParent,
	HMENU hMenu, HINSTANCE hInstance, LPVOID lpParam) {
	// use WS_POPUP instead of WS_OVERLAPPED for borderless and hiding taskbar
	dwStyle = WS_POPUP;

	HMONITOR hmon = MonitorFromWindow(hWndParent, MONITOR_DEFAULTTONEAREST);
	MONITORINFO mi = {sizeof(mi)};
	GetMonitorInfo(hmon, &mi);

	return oldCreateWindowExA(dwExStyle, lpClassName, lpWindowName, dwStyle, mi.rcMonitor.left, mi.rcMonitor.top, mi.rcMonitor.right - mi.rcMonitor.left,
		mi.rcMonitor.bottom - mi.rcMonitor.top, hWndParent, hMenu, hInstance, lpParam);
}

BOOL WINAPI myGetWindowInfo(HWND hwnd, PWINDOWINFO pwi) {
	oldGetWindowInfo(hwnd, pwi);
	int w = pwi->rcClient.right - pwi->rcClient.left;
	int h = pwi->rcClient.bottom - pwi->rcClient.top;

	pwi->rcClient.left = 0;
	pwi->rcClient.top = 0;
	if (w * 480 / 640 > h) {
		pwi->rcClient.right = 480 * w / h;
		pwi->rcClient.bottom = 480;
	} else {
		pwi->rcClient.right = 640;
		pwi->rcClient.bottom = 640 * h / w;
	}
	return TRUE;
}

HRESULT WINAPI myCreateDevice(LPVOID IDirect3D9, UINT Adapter, D3DDEVTYPE DeviceType, HWND hFocusWindow, DWORD BehaviorFlags,
	D3DPRESENT_PARAMETERS *pPresentationParameters, IDirect3DDevice9 **ppReturnedDeviceInterface) {
	HRESULT result = oldCreateDevice(IDirect3D9, Adapter, DeviceType, hFocusWindow, BehaviorFlags, pPresentationParameters, ppReturnedDeviceInterface);
	if (FAILED(result)) {
		return result;
	}
	D3DVIEWPORT9 viewData = {0, 0, 640, 480, 0.0f, 1.0f};
	(*ppReturnedDeviceInterface)->SetViewport(&viewData);
	return result;
}

IDirect3D9 *WINAPI myDirect3DCreate9(UINT SDKVersion) {
	IDirect3D9 *direct3D9 = oldDirect3DCreate9(SDKVersion);

	LPVOID ptrCreateDevice = *(((LPVOID *)(*(LPVOID *)(direct3D9))) + 16);
	oldCreateDevice = (HRESULT(WINAPI *)(LPVOID, UINT, D3DDEVTYPE, HWND, DWORD, D3DPRESENT_PARAMETERS *, IDirect3DDevice9 **))ptrCreateDevice;

	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());
	DetourAttach(&(PVOID &)oldCreateDevice, myCreateDevice);
	DetourTransactionCommit();

	return direct3D9;
}

extern "C" {
__declspec(dllexport) bool CheckVersion(const BYTE hash[16]) {
	return ::memcmp(TARGET_HASH, hash, sizeof TARGET_HASH) == 0;
}

__declspec(dllexport) bool Initialize(HMODULE hMyModule, HMODULE hParentModule) {
	DetourRestoreAfterWith();

	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());
	DetourAttach(&(PVOID &)oldCreateWindowExA, myCreateWindowExA);
	DetourAttach(&(PVOID &)oldGetWindowInfo, myGetWindowInfo);
	DetourAttach(&(PVOID &)oldDirect3DCreate9, myDirect3DCreate9);
	DetourTransactionCommit();

	return true;
}
}
