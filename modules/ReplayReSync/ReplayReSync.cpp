#include <windows.h>
#include <process.h>
#include <shlwapi.h>

static wchar_t s_profilePath[1024 + MAX_PATH];

void MemoryWrite(uintptr_t address, char *value, int length) {
	DWORD oldProtect;
	VirtualProtect((PVOID)address, length, PAGE_EXECUTE_READWRITE, &oldProtect);
	memcpy((void *)address, value, length);
	VirtualProtect((PVOID)address, length, oldProtect, NULL);
}

void SetMountainVapor(void *unused) {
	int disableAlert = GetPrivateProfileIntW(L"Settings", L"disable_alert", 0, s_profilePath);
	int hotkeyMV = GetPrivateProfileIntW(L"Hotkeys", L"mountain_vapor_state", 0, s_profilePath);
	int hotkeyNorm = GetPrivateProfileIntW(L"Hotkeys", L"normal_state", 0, s_profilePath);

	DWORD mountainVaporAddress = 0x008971C0;
	char newValue[4];

	while (true) {
		if (GetAsyncKeyState(hotkeyMV)) {
			if (!disableAlert) {
				MessageBoxW(NULL, L"Mountain Vapor state is now set", L"ReplayReSync", MB_SETFOREGROUND);
			}
			*newValue = 11;
			MemoryWrite(mountainVaporAddress, newValue, sizeof(newValue));
		}
		if (GetAsyncKeyState(hotkeyNorm)) {
			if (!disableAlert) {
				MessageBoxW(NULL, L"Normal state is now set", L"ReplayReSync", MB_SETFOREGROUND);
			}
			*newValue = 0;
			MemoryWrite(mountainVaporAddress, newValue, sizeof(newValue));
		}
		Sleep(100);
	}
}

extern "C" {
__declspec(dllexport) bool CheckVersion(const BYTE hash[16]) {
	return true;
}

__declspec(dllexport) bool Initialize(HMODULE hMyModule, HMODULE hParentModule) {
	GetModuleFileNameW(hMyModule, s_profilePath, 1024);
	PathRemoveFileSpecW(s_profilePath);
	PathAppendW(s_profilePath, L"ReplayReSync.ini");

	_beginthread(SetMountainVapor, 0, NULL);
	return true;
}
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved) {
	return TRUE;
}
