#include <windows.h>
#include <shlwapi.h>
#include <sys/stat.h>
#include <cstdio>

#define SWRS_USES_HASH
#include "swrs.h"

#define CLogo_Process(p) Ccall(p, s_origCLogo_OnProcess, int, ())()
#define CTitle_Process(p) Ccall(p, s_origCTitle_OnProcess, int, ())()
#define CBattle_Process(p) Ccall(p, s_origCBattle_OnProcess, int, ())()

static DWORD s_origCLogo_OnProcess;
static DWORD s_origCTitle_OnProcess;
static DWORD s_origCBattle_OnProcess;

static char nextFile[MAX_PATH];
static HANDLE dirIt = INVALID_HANDLE_VALUE;
static bool activated;
static bool isLast;
static bool s_autoShutdown;

int __fastcall CLogo_OnProcess(void *This) {
	int ret = CLogo_Process(This);

	if (ret == 2 && activated) {
		if (CInputManager_ReadReplay(g_inputMgr, nextFile)) {
			WIN32_FIND_DATA FindFileData;

			// 入力があったように見せかける。END
			*(BYTE *)((DWORD)g_inputMgrs + 0x74) = 0xFF;
			// リプモードにチェンジ
			SetBattleMode(3, 2);
			ret = 6;
			if (dirIt != INVALID_HANDLE_VALUE) {
				isLast = FindNextFile(dirIt, &FindFileData) == 0;
				strcpy(nextFile, __argv[1]);
				strcat(nextFile, "\\");
				strcat(nextFile, FindFileData.cFileName);
				puts(nextFile);
			}
		}
	}
	return ret;
}

int __fastcall CTitle_OnProcess(void *This) {
	int ret = CTitle_Process(This);

	if (activated) {
		WIN32_FIND_DATA FindFileData;

		if (CInputManager_ReadReplay(g_inputMgr, nextFile)) {
			// 入力があったように見せかける。END
			*(BYTE *)((DWORD)g_inputMgrs + 0x74) = 0xFF;
			// リプモードにチェンジ
			SetBattleMode(3, 2);
			ret = 6;
			if (dirIt != INVALID_HANDLE_VALUE) {
				isLast = FindNextFile(dirIt, &FindFileData) == 0;
				strcpy(nextFile, __argv[1]);
				strcat(nextFile, "\\");
				strcat(nextFile, FindFileData.cFileName);
			}
		}
	}
	return ret;
}

int __fastcall CBattle_OnProcess(void *This) {
	int ret = CBattle_Process(This);

	if (activated && ret != 5) {
		// 落とす
		if (isLast || dirIt == INVALID_HANDLE_VALUE) {
			if (s_autoShutdown) {
				ret = -1;
			}
			activated = false;
		} else
			ret = SWRSSCENE_TITLE;
	}
	return ret;
}

// 設定ロード
void LoadSettings(LPCSTR profilePath) {
	// 自動シャットダウン
	s_autoShutdown = GetPrivateProfileInt("ReplayDnD", "AutoShutdown", 1, profilePath) != 0;
}

extern "C" __declspec(dllexport) bool CheckVersion(const BYTE hash[16]) {
	return ::memcmp(TARGET_HASH, hash, sizeof TARGET_HASH) == 0;
}

extern "C" __declspec(dllexport) bool Initialize(HMODULE hMyModule, HMODULE hParentModule) {
	char profilePath[1024 + MAX_PATH];

	GetModuleFileName(hMyModule, profilePath, 1024);
	PathRemoveFileSpec(profilePath);
	PathAppend(profilePath, "ReplayDnD.ini");
	LoadSettings(profilePath);

	DWORD old;
	::VirtualProtect((PVOID)rdata_Offset, rdata_Size, PAGE_EXECUTE_WRITECOPY, &old);
	s_origCLogo_OnProcess = TamperDword(vtbl_CLogo + 4, (DWORD)CLogo_OnProcess);
	s_origCTitle_OnProcess = TamperDword(0x00857FAC + 4, (DWORD)CTitle_OnProcess);
	s_origCBattle_OnProcess = TamperDword(vtbl_CBattle + 4, (DWORD)CBattle_OnProcess);
	::VirtualProtect((PVOID)rdata_Offset, rdata_Size, old, &old);

	::FlushInstructionCache(GetCurrentProcess(), NULL, 0);

	FILE *_;

	activated = false;
	if (__argc == 2) {
		struct stat s;

		stat(__argv[1], &s);
		if(s.st_mode & S_IFDIR) {
			WIN32_FIND_DATA FindFileData;
			char buffer[MAX_PATH];

			strcpy(buffer, __argv[1]);
			strcat(buffer, "\\*.rep");
			dirIt = FindFirstFileA(buffer, &FindFileData);
			if (dirIt != INVALID_HANDLE_VALUE) {
				activated = true;
				strcpy(nextFile, __argv[1]);
				strcat(nextFile, "\\");
				strcat(nextFile, FindFileData.cFileName);
			}
		} else {
			strcpy(nextFile, __argv[1]);
			activated = true;
		}
	}
	return true;
}

extern "C" int APIENTRY DllMain(HMODULE hModule, DWORD fdwReason, LPVOID lpReserved) {
	return TRUE;
}
