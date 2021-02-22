//
// Created by Gegel85 on 04/12/2020
//

#include <Windows.h>
#include <Shlwapi.h>
#include <SokuLib.hpp>
#include <algorithm>
#include <dinput.h>
#include "Logic.hpp"
#include "State.hpp"

struct Title {};
struct Battle {};
struct Select {};
struct Loading {};
struct BattleWatch {};
struct LoadingWatch {};

int (__thiscall SokuLib::BattleManager::*s_origCBattleManager_Render)();
int (__thiscall SokuLib::BattleManager::*s_origCBattleManager_Start)();
int (__thiscall SokuLib::BattleManager::*s_origCBattleManager_KO)();
int (__thiscall LoadingWatch::*s_origCLoadingWatch_Process)();
int (__thiscall BattleWatch::*s_origCBattleWatch_Process)();
int (__thiscall Loading::*s_origCLoading_Process)();
int (__thiscall Battle::*s_origCBattle_Process)();
int (__thiscall Select::*s_origCSelect_Process)();
int (__thiscall Title::*s_origCTitle_Process)();

int __fastcall CTitle_OnProcess(Title *This)
{
	Practice::deactivate();

	// super
	return (This->*s_origCTitle_Process)();
}

int __fastcall CBattleWatch_OnProcess(BattleWatch *This)
{
	// super
	return (This->*s_origCBattleWatch_Process)();
}

int __fastcall CBattle_OnProcess(Battle *This)
{
	auto &battle = SokuLib::getBattleMgr();

	if (SokuLib::mainMode == SokuLib::BATTLE_MODE_PRACTICE)
		Practice::activate();

	// super
	int ret = (This->*s_origCBattle_Process)();

	return ret;
}

int __fastcall CSelect_OnProcess(Select *This)
{
	Practice::deactivate();

	// super
	return (This->*s_origCSelect_Process)();
}

int __fastcall CLoading_OnProcess(Loading *This)
{
	Practice::deactivate();

	// super
	return (This->*s_origCLoading_Process)();
}

int __fastcall CLoadingWatch_OnProcess(LoadingWatch *This)
{
	Practice::deactivate();

	// super
	return (This->*s_origCLoadingWatch_Process)();
}

int __fastcall CBattleManager_KO(SokuLib::BattleManager *This)
{
	// super
	return (This->*s_origCBattleManager_KO)();
}

int __fastcall CBattleManager_Start(SokuLib::BattleManager *This)
{
	// super
	return (This->*s_origCBattleManager_Start)();
}

int __fastcall CBattleManager_Render(SokuLib::BattleManager *This)
{
	// super
	int ret = (This->*s_origCBattleManager_Render)();

	Practice::render();
	return ret;
}

// �ݒ胍�[�h
void LoadSettings(LPCSTR profilePath, LPCSTR parentPath)
{
	FILE *_;

	AllocConsole();
	freopen_s(&_, "CONOUT$", "w", stdout);
	puts("Hello !");
	//port = GetPrivateProfileInt("Server", "Port", 80, profilePath);
}

void hookFunctions()
{
	DWORD old;

	//Setup hooks
	VirtualProtect((PVOID)RDATA_SECTION_OFFSET, RDATA_SECTION_SIZE, PAGE_EXECUTE_WRITECOPY, &old);
	s_origCTitle_Process = SokuLib::union_cast<int (Title::*)()>(
		SokuLib::TamperDword(
			SokuLib::vtbl_CTitle + SokuLib::OFFSET_ON_PROCESS,
			reinterpret_cast<DWORD>(CTitle_OnProcess)
		)
	);
	s_origCBattle_Process = SokuLib::union_cast<int (Battle::*)()>(
		SokuLib::TamperDword(
			SokuLib::vtbl_CBattle + SokuLib::OFFSET_ON_PROCESS,
			reinterpret_cast<DWORD>(CBattle_OnProcess)
		)
	);
	s_origCSelect_Process = SokuLib::union_cast<int (Select::*)()>(
		SokuLib::TamperDword(
			SokuLib::vtbl_CSelect + SokuLib::OFFSET_ON_PROCESS,
			reinterpret_cast<DWORD>(CSelect_OnProcess)
		)
	);
	s_origCLoading_Process = SokuLib::union_cast<int (Loading::*)()>(
		SokuLib::TamperDword(
			SokuLib::vtbl_CLoading + SokuLib::OFFSET_ON_PROCESS,
			reinterpret_cast<DWORD>(CLoading_OnProcess)
		)
	);
	s_origCBattleManager_Start = SokuLib::union_cast<int (SokuLib::BattleManager::*)()>(
		SokuLib::TamperDword(
			SokuLib::vtbl_CBattleManager + SokuLib::BATTLE_MGR_OFFSET_ON_SAY_START,
			reinterpret_cast<DWORD>(CBattleManager_Start)
		)
	);
	s_origCBattleManager_KO = SokuLib::union_cast<int (SokuLib::BattleManager::*)()>(
		SokuLib::TamperDword(
			SokuLib::vtbl_CBattleManager + SokuLib::BATTLE_MGR_OFFSET_ON_KO,
			reinterpret_cast<DWORD>(CBattleManager_KO)
		)
	);
	s_origCBattleManager_Render = SokuLib::union_cast<int (SokuLib::BattleManager::*)()>(
		SokuLib::TamperDword(
			SokuLib::vtbl_CBattleManager + SokuLib::BATTLE_MGR_OFFSET_ON_RENDER,
			reinterpret_cast<DWORD>(CBattleManager_Render)
		)
	);
	VirtualProtect((PVOID)RDATA_SECTION_OFFSET, RDATA_SECTION_SIZE, old, &old);

	::FlushInstructionCache(GetCurrentProcess(), nullptr, 0);
}

extern "C" __declspec(dllexport) bool CheckVersion(const BYTE hash[16])
{
	return true;
}

extern "C" __declspec(dllexport) bool Initialize(HMODULE hMyModule, HMODULE hParentModule)
{
	GetModuleFileName(hMyModule, Practice::profilePath, 1024);
	PathRemoveFileSpec(Practice::profilePath);
	strcpy(Practice::profileParent, Practice::profilePath);
	PathAppend(Practice::profilePath, "AdvancedPracticeMode.ini");
	LoadSettings(Practice::profilePath, Practice::profileParent);
	hookFunctions();
	return true;
}

extern "C" int APIENTRY DllMain(HMODULE hModule, DWORD fdwReason, LPVOID lpReserved)
{
	return TRUE;
}