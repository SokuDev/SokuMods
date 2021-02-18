//
// Created by Gegel85 on 04/12/2020
//

#include <Windows.h>
#include <Shlwapi.h>
#include <SokuLib.hpp>
#include <algorithm>
#include <dinput.h>
#include <SFML/Graphics.hpp>
#include "Gui.hpp"

using namespace SokuLib;

struct Title {};
struct Battle {};
struct Select {};
struct Loading {};
struct BattleWatch {};
struct LoadingWatch {};

sf::RenderWindow *sfmlWindow;
int (__thiscall BattleManager::*s_origCBattleManager_Render)();
int (__thiscall BattleManager::*s_origCBattleManager_Start)();
int (__thiscall BattleManager::*s_origCBattleManager_KO)();
int (__thiscall LoadingWatch::*s_origCLoadingWatch_Process)();
int (__thiscall BattleWatch::*s_origCBattleWatch_Process)();
int (__thiscall Loading::*s_origCLoading_Process)();
int (__thiscall Battle::*s_origCBattle_Process)();
int (__thiscall Select::*s_origCSelect_Process)();
int (__thiscall Title::*s_origCTitle_Process)();
char profilePath[1024 + MAX_PATH];
char profileParent[1024 + MAX_PATH];

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

	if (SokuLib::mainMode == SokuLib::BATTLE_MODE_PRACTICE && !sfmlWindow) {
		sfmlWindow = new sf::RenderWindow{{640, 480}, "Advanced Practice Mode", sf::Style::Titlebar};
		Practice::init(profileParent);
		Practice::gui.setTarget(*sfmlWindow);
		try {
			Practice::loadAllGuiElements(profileParent);
		} catch (std::exception &e) {
			puts(e.what());
			throw;
		}
	}
	return ret;
}

int __fastcall CSelect_OnProcess(Select *This) {
	// super
	int ret = (This->*s_origCSelect_Process)();

	if (sfmlWindow) {
		delete sfmlWindow;
		sfmlWindow = nullptr;
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
		sfmlWindow->clear(sf::Color(0xAA, 0xAA, 0xAA));
		try {
			Practice::updateGuiState();
		} catch (std::exception &e) {
			puts(e.what());
			throw;
		}
		while (sfmlWindow->pollEvent(event))
			Practice::gui.handleEvent(event);
		Practice::gui.draw();
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

	//Setup hooks
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
	s_origCSelect_Process = union_cast<int (Select::*)()>(
		TamperDword(
			vtbl_CSelect + OFFSET_ON_PROCESS,
			reinterpret_cast<DWORD>(CSelect_OnProcess)
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

	//Bypass the basic practice features by messing up the check for practice mode.
	::VirtualProtect((PVOID)0x42A333, 1, PAGE_EXECUTE_WRITECOPY, &old);
	// cmp eax, 08 -> cmp eax, 0A
	*(char *)0x42A333 = 10;
	::VirtualProtect((PVOID)0x42A333, 1, old, &old);

	::FlushInstructionCache(GetCurrentProcess(), NULL, 0);
}

extern "C" __declspec(dllexport) bool CheckVersion(const BYTE hash[16])
{
	return true;
}

extern "C" __declspec(dllexport) bool Initialize(HMODULE hMyModule, HMODULE hParentModule)
{
	GetModuleFileName(hMyModule, profilePath, 1024);
	PathRemoveFileSpec(profilePath);
	strcpy(profileParent, profilePath);
	PathAppend(profilePath, "AdvancedPracticeMode.ini");
	LoadSettings(profilePath, profileParent);
	hookFunctions();
	return true;
}

extern "C" int APIENTRY DllMain(HMODULE hModule, DWORD fdwReason, LPVOID lpReserved)
{
	return TRUE;
}