//
// Created by Gegel85 on 04/12/2020
//

#include <Windows.h>
#include <Shlwapi.h>
#include <SokuLib.hpp>
#include <algorithm>
#include <dinput.h>
#include <SFML/Graphics.hpp>

using namespace SokuLib;

bool activated = false;
sf::RenderWindow *sfmlWindow;
struct Title {};
struct Battle {};
struct Loading {};
struct BattleWatch {};
struct LoadingWatch {};
int (__thiscall BattleManager::*s_origCBattleManager_Render)();
int (__thiscall BattleManager::*s_origCBattleManager_Start)();
int (__thiscall BattleManager::*s_origCBattleManager_KO)();
int (__thiscall LoadingWatch::*s_origCLoadingWatch_Process)();
int (__thiscall BattleWatch::*s_origCBattleWatch_Process)();
int (__thiscall Loading::*s_origCLoading_Process)();
int (__thiscall Battle::*s_origCBattle_Process)();
int (__thiscall Title::*s_origCTitle_Process)();

int __fastcall CTitle_OnProcess(Title *This) {
	// super
	int ret = (This->*s_origCTitle_Process)();

	if (sfmlWindow) {
		delete sfmlWindow;
		sfmlWindow = nullptr;
	}
	return ret;
}

int __fastcall CBattleWatch_OnProcess(BattleWatch *This) {
	// super
	int ret = (This->*s_origCBattleWatch_Process)();

	return ret;
}

int __fastcall CBattle_OnProcess(Battle *This) {
	// super
	int ret = (This->*s_origCBattle_Process)();

	if (activated && !sfmlWindow) {
		sfmlWindow = new sf::RenderWindow{{640, 480}, "Advanced Practice Mode", sf::Style::Titlebar};
	}
	return ret;
}

int __fastcall CLoading_OnProcess(Loading *This) {
	// super
	int ret = (This->*s_origCLoading_Process)();

	if (sfmlWindow) {
		delete sfmlWindow;
		sfmlWindow = nullptr;
	}
	return ret;
}

int __fastcall CLoadingWatch_OnProcess(LoadingWatch *This) {
	// super
	int ret = (This->*s_origCLoadingWatch_Process)();

	if (sfmlWindow) {
		delete sfmlWindow;
		sfmlWindow = nullptr;
	}
	return ret;
}

int __fastcall CBattleManager_KO(BattleManager *This) {
	// super
	int ret = (This->*s_origCBattleManager_KO)();

	return ret;
}

int __fastcall CBattleManager_Start(BattleManager *This) {
	// super
	int ret = (This->*s_origCBattleManager_Start)();

	return ret;
}

int __fastcall CBattleManager_Render(BattleManager *This) {
	// super
	int ret = (This->*s_origCBattleManager_Render)();
	sf::Event event;

	if (sfmlWindow) {
		sfmlWindow->clear();
		while (sfmlWindow->pollEvent(event));
		sfmlWindow->display();
	}
	return ret;
}

// �ݒ胍�[�h
void LoadSettings(LPCSTR profilePath, LPCSTR parentPath) {
	FILE *_;

	AllocConsole();
	freopen_s(&_, "CONOUT$", "w", stdout);
	puts("Hello !");
	//port = GetPrivateProfileInt("Server", "Port", 80, profilePath);
}

void hookFunctions() {
	DWORD old;

	::VirtualProtect((PVOID)RDATA_SECTION_OFFSET, RDATA_SECTION_SIZE, PAGE_EXECUTE_WRITECOPY, &old);
	s_origCTitle_Process = union_cast<int (Title::*)()>(
		TamperDword(
			vtbl_CTitle + OFFSET_ON_PROCESS,
			reinterpret_cast<DWORD>(CTitle_OnProcess)
		)
	);
	s_origCBattle_Process = union_cast<int (Battle::*)()>(
		TamperDword(
			vtbl_CBattle + OFFSET_ON_PROCESS,
			reinterpret_cast<DWORD>(CBattle_OnProcess)
		)
	);
	s_origCLoading_Process = union_cast<int (Loading::*)()>(
		TamperDword(
			vtbl_CLoading + OFFSET_ON_PROCESS,
			reinterpret_cast<DWORD>(CLoading_OnProcess)
		)
	);
	s_origCBattleManager_Start = union_cast<int (BattleManager::*)()>(
		TamperDword(
			vtbl_CBattleManager + BATTLE_MGR_OFFSET_ON_SAY_START,
			reinterpret_cast<DWORD>(CBattleManager_Start)
		)
	);
	s_origCBattleManager_KO = union_cast<int (BattleManager::*)()>(
		TamperDword(
			vtbl_CBattleManager + BATTLE_MGR_OFFSET_ON_KO,
			reinterpret_cast<DWORD>(CBattleManager_KO)
		)
	);
	s_origCBattleManager_Render = union_cast<int (BattleManager::*)()>(
		TamperDword(
			vtbl_CBattleManager + BATTLE_MGR_OFFSET_ON_RENDER,
			reinterpret_cast<DWORD>(CBattleManager_Render)
		)
	);
	::VirtualProtect((PVOID)RDATA_SECTION_OFFSET, RDATA_SECTION_SIZE, old, &old);
	::FlushInstructionCache(GetCurrentProcess(), NULL, 0);
}

extern "C" __declspec(dllexport) bool CheckVersion(const BYTE hash[16]) {
	return true;
}

extern "C" __declspec(dllexport) bool Initialize(HMODULE hMyModule, HMODULE hParentModule) {
	char profilePath[1024 + MAX_PATH];
	char profileParent[1024 + MAX_PATH];

	GetModuleFileName(hMyModule, profilePath, 1024);
	PathRemoveFileSpec(profilePath);
	strcpy(profileParent, profilePath);
	PathAppend(profilePath, "SokuStreaming.ini");
	LoadSettings(profilePath, profileParent);

	hookFunctions();
	return true;
}

extern "C" int APIENTRY DllMain(HMODULE hModule, DWORD fdwReason, LPVOID lpReserved) {
	return TRUE;
}