#include <windows.h>
#include "swrs.h"
#include <cstdio>
#include <detours.h>
#include <process.h>
#include <shlobj.h>
#include <shlwapi.h>
#include <sys/stat.h>

#define CLogo_Process(p) Ccall(p, s_origCLogo_OnProcess, int, ())()
#define CTitle_Process(p) Ccall(p, s_origCTitle_OnProcess, int, ())()
#define CBattle_Process(p) Ccall(p, s_origCBattle_OnProcess, int, ())()

#define ADDR_SET_VOLUME 0x00403D10

#define WARN(fmt, ...) \
	{ \
		size_t needed = _snwprintf(NULL, 0, fmt, ##__VA_ARGS__); \
		wchar_t *buf = (wchar_t *)malloc((needed + 1) * 2); \
		_snwprintf(buf, (needed + 1), fmt, ##__VA_ARGS__); \
		MessageBoxW(NULL, buf, L"ReplayDnD", MB_ICONWARNING | MB_OK); \
		free(buf); \
	}

class CDetour {
public:
	void SetVolume(float volume);
	static void (CDetour::*ActualSetVolume)(float volume);
};

void CDetour::SetVolume(float volume) {
	(this->*ActualSetVolume)(0);
}

void (CDetour::*CDetour::ActualSetVolume)(float) = union_cast<void (CDetour::*)(float)>(ADDR_SET_VOLUME);

char replay_path_pointer_fake[MAX_PATH];
wchar_t *replay_path_pointer_actual;

HANDLE(__stdcall *actual_CreateFileA)
(LPCSTR lpFileName, DWORD dwDesiredAccess, DWORD dwShareMode, LPSECURITY_ATTRIBUTES lpSecurityAttributes, DWORD dwCreationDisposition,
	DWORD dwFlagsAndAttributes, HANDLE hTemplateFile)
	= CreateFileA;

HANDLE __stdcall my_CreateFileA(char *lpFileName, DWORD dwDesiredAccess, DWORD dwShareMode, LPSECURITY_ATTRIBUTES lpSecurityAttributes,
	DWORD dwCreationDisposition, DWORD dwFlagsAndAttributes, HANDLE hTemplateFile) {
	if (lpFileName && !strcmp(lpFileName, replay_path_pointer_fake)) {
		return CreateFileW(
			replay_path_pointer_actual, dwDesiredAccess, dwShareMode, lpSecurityAttributes, dwCreationDisposition, dwFlagsAndAttributes, hTemplateFile);
	}
	return actual_CreateFileA(lpFileName, dwDesiredAccess, dwShareMode, lpSecurityAttributes, dwCreationDisposition, dwFlagsAndAttributes, hTemplateFile);
}

wchar_t **wargv;

static DWORD s_origCLogo_OnProcess;
static DWORD s_origCTitle_OnProcess;
static DWORD s_origCBattle_OnProcess;

static wchar_t next_file[MAX_PATH];
static HANDLE dir_it = INVALID_HANDLE_VALUE;
static bool dir_last;

static bool s_autoShutdown;
static bool s_muteMusic;

int __fastcall CLogo_OnProcess(void *This) {
	if (next_file[0]) {
		replay_path_pointer_actual = next_file;
		WideCharToMultiByte(CP_ACP, 0, replay_path_pointer_actual, -1, replay_path_pointer_fake, MAX_PATH, NULL, NULL);
		if (CInputManager_ReadReplay(g_inputMgr, replay_path_pointer_fake)) {
			// 入力があったように見せかける。END
			*(BYTE *)((DWORD)g_inputMgrs + 0x74) = 0xFF;
			// リプモードにチェンジ
			SetBattleMode(3, 2);

			if (dir_it != INVALID_HANDLE_VALUE) {
				WIN32_FIND_DATAW find_data;
				if (FindNextFileW(dir_it, &find_data)) {
					StrCpyW(next_file, wargv[1]);
					StrCatW(next_file, L"\\");
					StrCatW(next_file, find_data.cFileName);
				} else {
					dir_last = true;
				}
			}
			return 6;
		} else {
			WARN(L"Failed loading %s", next_file);
		}
	}
	return CLogo_Process(This);
}

int __fastcall CTitle_OnProcess(void *This) {
	if (next_file[0]) {
		replay_path_pointer_actual = next_file;
		WideCharToMultiByte(CP_ACP, 0, replay_path_pointer_actual, -1, replay_path_pointer_fake, MAX_PATH, NULL, NULL);
		if (CInputManager_ReadReplay(g_inputMgr, replay_path_pointer_fake)) {
			// 入力があったように見せかける。END
			*(BYTE *)((DWORD)g_inputMgrs + 0x74) = 0xFF;
			// リプモードにチェンジ
			SetBattleMode(3, 2);

			if (dir_it != INVALID_HANDLE_VALUE) {
				WIN32_FIND_DATAW find_data;
				if (FindNextFileW(dir_it, &find_data)) {
					StrCpyW(next_file, wargv[1]);
					StrCatW(next_file, L"\\");
					StrCatW(next_file, find_data.cFileName);
				} else {
					dir_last = true;
				}
			}
			return 6;
		} else {
			WARN(L"Failed loading %s", next_file);
		}
	}
	return CTitle_Process(This);
}

int __fastcall CBattle_OnProcess(void *This) {
	int ret = CBattle_Process(This);
	if (next_file[0] && ret != 5) {
		if (dir_last || dir_it == INVALID_HANDLE_VALUE) {
			if (s_autoShutdown) {
				// 落とす
				ret = -1;
			}
			next_file[0] = '\0';
		} else {
			ret = SWRSSCENE_TITLE;
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
	if (RegSetValueExW(key, NULL, 0, REG_SZ, (BYTE*)L"SokuReplay", sizeof(L"SokuReplay"))) {
		return;
	}
	RegCloseKey(key);
	if (RegCreateKeyExW(HKEY_CURRENT_USER, L"Software\\Classes\\SokuReplay", 0, NULL, 0, KEY_SET_VALUE | KEY_WOW64_64KEY, NULL, &key, NULL)) {
		return;
	}
	if (RegSetValueExW(key, NULL, 0, REG_SZ, (BYTE*)L"Soku Replay", sizeof(L"Soku Replay"))) {
		return;
	}
	if (RegSetValueExW(key, L"AlwaysShowExt", 0, REG_SZ, (BYTE*)L"1", sizeof(L"1"))) {
		return;
	}
	RegCloseKey(key);
	if (RegCreateKeyExW(HKEY_CURRENT_USER, L"Software\\Classes\\SokuReplay\\Shell\\Open", 0, NULL, 0, KEY_SET_VALUE | KEY_WOW64_64KEY, NULL, &key, NULL)) {
		return;
	}
	if (RegSetValueExW(key, NULL, 0, REG_SZ, (BYTE*)L"Watch in Soku", sizeof(L"Watch in Soku"))) {
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
	if (RegSetValueExW(key, NULL, 0, REG_SZ, (BYTE*)(&exe_path[1]), 2 * (wcslen(exe_path) + 1))) {
		return;
	}
	RegCloseKey(key);
	if (RegCreateKeyExW(
				HKEY_CURRENT_USER, L"Software\\Classes\\SokuReplay\\Shell\\Open\\Command", 0, NULL, 0, KEY_SET_VALUE | KEY_WOW64_64KEY, NULL, &key, NULL)) {
		return;
	}
	StrCatW(exe_path, L"\" \"%1\"");
	if (RegSetValueExW(key, NULL, 0, REG_SZ, (BYTE*)exe_path, 2 * (wcslen(exe_path) + 1))) {
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
	_beginthread(load_thread, 0, NULL);

	if (__argc != 2) {
		return true;
	}

	int _;
	wargv = CommandLineToArgvW(GetCommandLineW(), &_);

	struct _stat64 s;
	if (_wstat64(wargv[1], &s)) {
		return true;
	}
	if (s.st_mode & S_IFDIR) {
		WIN32_FIND_DATAW find_data;
		wchar_t buf[MAX_PATH];
		StrCpyW(buf, wargv[1]);
		StrCatW(buf, L"\\*.rep");
		dir_it = FindFirstFileW(buf, &find_data);
		if (dir_it == INVALID_HANDLE_VALUE) {
			return true;
		}
		StrCpyW(next_file, wargv[1]);
		StrCatW(next_file, L"\\");
		StrCatW(next_file, find_data.cFileName);
	} else if (StrStrIW(wargv[1], L".rep")) {
		StrCpyW(next_file, wargv[1]);
	} else {
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
	s_origCTitle_OnProcess = TamperDword(vtbl_CTitle + 4, (DWORD)CTitle_OnProcess);
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
		DetourAttach((void **)&CDetour::ActualSetVolume, *(PBYTE *)&setVolumeShim);
		DetourAttach((void **)&actual_CreateFileA, my_CreateFileA);
		DetourTransactionCommit();
	}
	return true;
}

extern "C" int APIENTRY DllMain(HMODULE hModule, DWORD fdwReason, LPVOID lpReserved) {
	return TRUE;
}
