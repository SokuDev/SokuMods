#include <windows.h>
#include <shlwapi.h>

#define SWRS_USES_HASH
#include "swrs.h"

#define CLogo_Process(p) Ccall(p, s_origCLogo_OnProcess, int, ())()
#define CBattle_Process(p) Ccall(p, s_origCBattle_OnProcess, int, ())()

static DWORD s_origCLogo_OnProcess;
static DWORD s_origCBattle_OnProcess;

static bool s_swrapt;
static bool s_autoShutdown;

int __fastcall CLogo_OnProcess(void *This) {
	if (CInputManager_ReadReplay(g_inputMgr, __argv[1])) {
		s_swrapt = true;
		// 入力があったように見せかける。END
		*(BYTE *)((DWORD)g_inputMgrs + 0x74) = 0xFF;
		// リプモードにチェンジ
		SetBattleMode(3, 2);
		return 6;
	}
	return CLogo_Process(This);
}

int __fastcall CBattle_OnProcess(void *This) {
	int ret = CBattle_Process(This);
	if (s_swrapt && ret != 5) {
		s_swrapt = false;
		if (s_autoShutdown) {
			// 落とす
			ret = -1;
		}
	}
	return ret;
}

// 設定ロード
void LoadSettings(LPCSTR profilePath) {
	// 自動シャットダウン
	s_autoShutdown = GetPrivateProfileInt("ReplayDnD", "AutoShutdown", 1, profilePath) != 0;
}

extern "C" __declspec(dllexport) bool CheckVersion(const BYTE hash[16]) {
	return true;
}

extern "C" __declspec(dllexport) bool Initialize(HMODULE hMyModule, HMODULE hParentModule) {
	if (__argc != 2 || !StrStrI(__argv[1], "rep")) {
		return true;
	}

	char profilePath[1024 + MAX_PATH];

	GetModuleFileName(hMyModule, profilePath, 1024);
	PathRemoveFileSpec(profilePath);
	PathAppend(profilePath, "ReplayDnD.ini");
	LoadSettings(profilePath);

	DWORD old;
	::VirtualProtect((PVOID)rdata_Offset, rdata_Size, PAGE_EXECUTE_WRITECOPY, &old);
	s_origCLogo_OnProcess = TamperDword(vtbl_CLogo + 4, (DWORD)CLogo_OnProcess);
	s_origCBattle_OnProcess = TamperDword(vtbl_CBattle + 4, (DWORD)CBattle_OnProcess);
	::VirtualProtect((PVOID)rdata_Offset, rdata_Size, old, &old);

	::FlushInstructionCache(GetCurrentProcess(), NULL, 0);

	return true;
}

extern "C" int APIENTRY DllMain(HMODULE hModule, DWORD fdwReason, LPVOID lpReserved) {
	return TRUE;
}
