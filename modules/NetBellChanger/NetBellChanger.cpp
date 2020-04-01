#include <windows.h>
#include <mmsystem.h>
#include <shlwapi.h>

#define SWRS_USES_HASH
#include "swrs.h"

static char s_bellPath[1024];

void OnPlayNetBell(int id) {
	if (s_bellPath[0]) {
		::PlaySound(s_bellPath, NULL, SND_FILENAME | SND_ASYNC | SND_NODEFAULT);
	} else {
		PlaySEWaveBuffer(id);
	}
}

void LoadSettings(LPCSTR profilePath) {
	::GetPrivateProfileString("NetBell", "Path", "", s_bellPath, sizeof s_bellPath, profilePath);
}

extern "C" __declspec(dllexport) bool CheckVersion(const BYTE hash[16]) {
	return ::memcmp(TARGET_HASH, hash, sizeof TARGET_HASH) == 0;
}

extern "C" __declspec(dllexport) bool Initialize(HMODULE hMyModule, HMODULE hParentModule) {
	char profilePath[1024 + MAX_PATH];

	::GetModuleFileName(hMyModule, profilePath, 1024);
	::PathRemoveFileSpec(profilePath);
	::PathAppend(profilePath, "NetBellChanger.ini");
	LoadSettings(profilePath);

	DWORD dwOldProtect;
	::VirtualProtect(reinterpret_cast<LPVOID>(text_Offset), text_Size, PAGE_EXECUTE_WRITECOPY, &dwOldProtect);
	TamperNearJmpOpr(reinterpret_cast<DWORD>(PlayNetBell), reinterpret_cast<DWORD>(OnPlayNetBell));
	::VirtualProtect(reinterpret_cast<LPVOID>(text_Offset), text_Size, dwOldProtect, &dwOldProtect);
	return TRUE;
}

BOOL WINAPI DllMain(HMODULE hModule, DWORD fdwReason, LPVOID lpReserved) {
	return TRUE;
}
