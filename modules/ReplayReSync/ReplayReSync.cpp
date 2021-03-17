#include <windows.h>
#include <detours.h>
#include <process.h>
#include <shlwapi.h>
#include <swrs.h>

#define WINDOW_HANDLE 0x0089FF90
#define ADDR_AFTER_HEADER 0x42E520
#define ADDR_REPLAY_HEADER_HINT 0x898805

byte *scene_id = (byte *)0x008A0044;

DWORD mountainVaporAddress = 0x008971C0;
char newValue[4];

static wchar_t s_profilePath[1024 + MAX_PATH];

void MemoryWrite(uintptr_t address, char *value, int length) {
	DWORD oldProtect;
	VirtualProtect((PVOID)address, length, PAGE_EXECUTE_READWRITE, &oldProtect);
	memcpy((void *)address, value, length);
	VirtualProtect((PVOID)address, length, oldProtect, NULL);
}

class CDetour {
public:
	int AfterHeader();
	static int (CDetour::*ActualAfterHeader)();
};

int CDetour::AfterHeader() {
	int actual = (this->*ActualAfterHeader)();
	byte hint = *(byte *)ADDR_REPLAY_HEADER_HINT;
	if (hint == 0xCC) { // force no mountain vapor
		*newValue = 0;
		MemoryWrite(mountainVaporAddress, newValue, sizeof(newValue));
	} else if (hint == 0xCD) { // force mountain vapor
		*newValue = 11;
		MemoryWrite(mountainVaporAddress, newValue, sizeof(newValue));
	}
	return actual;
}

int (CDetour::*CDetour::ActualAfterHeader)() = union_cast<int (CDetour::*)()>(ADDR_AFTER_HEADER);

void SetMountainVapor(void *unused) {
	int disableAlert = GetPrivateProfileIntW(L"Settings", L"disable_alert", 0, s_profilePath);
	int hotkeyMV = GetPrivateProfileIntW(L"Hotkeys", L"mountain_vapor_state", 0, s_profilePath);
	int hotkeyNorm = GetPrivateProfileIntW(L"Hotkeys", L"normal_state", 0, s_profilePath);

	if (!hotkeyMV && !hotkeyNorm) {
		return;
	}

	while (true) {
		if (*scene_id == 2 && (void *)GetForegroundWindow() == *(void **)WINDOW_HANDLE) {
			if (hotkeyMV && GetAsyncKeyState(hotkeyMV)) {
				if (!disableAlert) {
					MessageBoxW(NULL, L"Mountain Vapor state is now set", L"ReplayReSync", MB_SETFOREGROUND);
				}
				*newValue = 11;
				MemoryWrite(mountainVaporAddress, newValue, sizeof(newValue));
			}
			if (hotkeyNorm && GetAsyncKeyState(hotkeyNorm)) {
				if (!disableAlert) {
					MessageBoxW(NULL, L"Normal state is now set", L"ReplayReSync", MB_SETFOREGROUND);
				}
				*newValue = 0;
				MemoryWrite(mountainVaporAddress, newValue, sizeof(newValue));
			}
		}
		Sleep(60);
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

	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());
	int (CDetour::*afterHeaderShim)() = &CDetour::AfterHeader;
	DetourAttach((void **)&CDetour::ActualAfterHeader, *(PBYTE *)&afterHeaderShim);
	DetourTransactionCommit();

	_beginthread(SetMountainVapor, 0, NULL);
	return true;
}
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved) {
	return TRUE;
}
