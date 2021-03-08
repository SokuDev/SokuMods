#include <windows.h>
#include "swrs.h"
#include <detours.h>
#include <process.h>
#include <shlobj.h>
#include <shlwapi.h>

#define CLogo_Process(p) Ccall(p, s_origCLogo_OnProcess, int, ())()
#define CBattle_Process(p) Ccall(p, s_origCBattle_OnProcess, int, ())()

#define ADDR_SET_VOLUME 0x00403D10

class CDetour {
public:
	void SetVolume(float volume);
	static void (CDetour::*ActualSetVolume)(float volume);
};

void CDetour::SetVolume(float volume) {
	(this->*ActualSetVolume)(0);
}

void (CDetour::*CDetour::ActualSetVolume)(float) = union_cast<void (CDetour::*)(float)>(ADDR_SET_VOLUME);

static DWORD s_origCLogo_OnProcess;
static DWORD s_origCBattle_OnProcess;

static bool s_swrapt;
static bool s_autoShutdown;
static bool s_muteMusic;

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
	s_muteMusic = GetPrivateProfileInt("ReplayDnD", "MuteMusic", 1, profilePath) != 0;
}

void load_thread(void *unused) {
	HKEY key;
	if (RegCreateKeyExW(HKEY_CURRENT_USER, L"Software\\Classes\\.rep", 0, NULL, 0, KEY_SET_VALUE | KEY_WOW64_64KEY, NULL, &key, NULL)) {
		return;
	}
	if (RegSetKeyValueW(key, NULL, NULL, REG_SZ, L"SokuReplay", sizeof(L"SokuReplay"))) {
		return;
	}
	RegCloseKey(key);
	if (RegCreateKeyExW(HKEY_CURRENT_USER, L"Software\\Classes\\SokuReplay", 0, NULL, 0, KEY_SET_VALUE | KEY_WOW64_64KEY, NULL, &key, NULL)) {
		return;
	}
	if (RegSetKeyValueW(key, NULL, NULL, REG_SZ, L"Soku Replay", sizeof(L"Soku Replay"))) {
		return;
	}
	if (RegSetKeyValueW(key, NULL, L"AlwaysShowExt", REG_SZ, L"1", sizeof(L"1"))) {
		return;
	}
	RegCloseKey(key);
	if (RegCreateKeyExW(HKEY_CURRENT_USER, L"Software\\Classes\\SokuReplay\\Shell\\Open", 0, NULL, 0, KEY_SET_VALUE | KEY_WOW64_64KEY, NULL, &key, NULL)) {
		return;
	}
	if (RegSetKeyValueW(key, NULL, NULL, REG_SZ, L"Watch in Soku", sizeof(L"Watch in Soku"))) {
		return;
	}
	RegCloseKey(key);
	wchar_t exe_path[MAX_PATH + 1024];
	exe_path[0] = L'"';
	if (!GetModuleFileNameW(NULL, &exe_path[1], MAX_PATH + 1023)) {
		return;
	}
	DWORD disposition;
	if (RegCreateKeyExW(
				HKEY_CURRENT_USER, L"Software\\Classes\\SokuReplay\\DefaultIcon", 0, NULL, 0, KEY_SET_VALUE | KEY_WOW64_64KEY, NULL, &key, &disposition)) {
		return;
	}
	if (RegSetKeyValueW(key, NULL, NULL, REG_SZ, &exe_path[1], 2 * (wcslen(exe_path) + 1))) {
		return;
	}
	RegCloseKey(key);
	if (RegCreateKeyExW(
				HKEY_CURRENT_USER, L"Software\\Classes\\SokuReplay\\Shell\\Open\\Command", 0, NULL, 0, KEY_SET_VALUE | KEY_WOW64_64KEY, NULL, &key, NULL)) {
		return;
	}
	StrCatW(exe_path, L"\" \"%1\"");
	if (RegSetKeyValueW(key, NULL, NULL, REG_SZ, exe_path, 2 * (wcslen(exe_path) + 1))) {
		return;
	}
	RegCloseKey(key);
	if (disposition == REG_CREATED_NEW_KEY) {
		SHChangeNotify(SHCNE_ASSOCCHANGED, SHCNF_IDLIST, NULL, NULL);
	}
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

	// allow running multiple instances until we can switch a
	// running game to a replay
	VirtualProtect((void *)0x007FB5C8, 1, PAGE_EXECUTE_WRITECOPY, &old);
	*(char *)(0x007FB5C8) = 0xB8;
	VirtualProtect((void *)0x007FB5C8, 1, old, &old);

	::FlushInstructionCache(GetCurrentProcess(), NULL, 0);

	if (s_muteMusic) {
		DetourTransactionBegin();
		DetourUpdateThread(GetCurrentThread());
		void (CDetour::*setVolumeShim)(float) = &CDetour::SetVolume;
		DetourAttach(&(PVOID &)CDetour::ActualSetVolume, *(PBYTE *)&setVolumeShim);
		DetourTransactionCommit();
	}

	_beginthread(load_thread, 0, NULL);
	return true;
}

extern "C" int APIENTRY DllMain(HMODULE hModule, DWORD fdwReason, LPVOID lpReserved) {
	return TRUE;
}
