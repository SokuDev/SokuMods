#include <windows.h>
#include <shlwapi.h>
#include <stdbool.h>
#include <stdint.h>

HANDLE self;

bool load() {
	wchar_t dll_path[MAX_PATH];
	wchar_t sdk_path[MAX_PATH];
	if (!GetModuleFileNameW(self, dll_path, sizeof(dll_path))) {
		return false;
	}
	if (!PathRemoveFileSpecW(dll_path)) {
		return false;
	}
	if (!StrCpyW(sdk_path, dll_path)) {
		return false;
	}
	if (!PathAppendW(sdk_path, L"discord_game_sdk.dll")) {
		return false;
	}
	if (!PathAppendW(dll_path, L"DiscordIntegration.dat")) {
		return false;
	}
	if (!LoadLibraryExW(sdk_path, NULL, 0)) {
		return false;
	}
	HANDLE dll = LoadLibraryExW(dll_path, NULL, 0);
	if (!dll) {
		return false;
	}
	bool (*Initialize)(HMODULE, HMODULE) = (void *)GetProcAddress(dll, "Initialize");
	if (Initialize == NULL || !Initialize(dll, self)) {
		FreeLibrary(dll);
		return false;
	}
	return true;
}

__declspec(dllexport) bool CheckVersion(const BYTE hash[16]) {
	return true;
}

__declspec(dllexport) bool Initialize(HMODULE hSelf, HMODULE hInjector) {
	if (!load()) {
		return false;
	}
	return true;
}

bool WINAPI DllMain(HINSTANCE hInst, DWORD dwReason, LPVOID reserved) {
	if (dwReason == DLL_PROCESS_ATTACH) {
		self = hInst;
	}
	return true;
}
