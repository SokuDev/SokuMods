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
struct Select {};
struct Loading {};
static int (__thiscall SokuLib::BattleManager::*s_origCBattleManager_Process)();
static int (__thiscall SokuLib::BattleManager::*s_origCBattleManager_Render)();
static int (__thiscall Loading::*s_origCLoading_Process)();
static int (__thiscall Select::*s_origCSelect_Process)();
static int (__thiscall Title::*s_origCTitle_Process)();
float frameCounter = 0;



int __fastcall CTitle_OnProcess(Title *This)
{
	Practice::deactivate();

	// super
	return (This->*s_origCTitle_Process)();
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

int __fastcall CBattleManager_Render(SokuLib::BattleManager *This)
{
	// super
	int ret = (This->*s_origCBattleManager_Render)();

	if (Practice::sfmlWindow)
		Practice::render();
	return ret;
}

int __fastcall CBattleManager_OnProcess(SokuLib::BattleManager *This)
{
	int result = SokuLib::sceneId;

	if (Practice::sfmlWindow);
	else if (SokuLib::mainMode == SokuLib::BATTLE_MODE_PRACTICE || SokuLib::subMode == SokuLib::BATTLE_SUBMODE_REPLAY)
		Practice::activate();
	else
		return (This->*s_origCBattleManager_Process)();

	frameCounter += Practice::settings.requestedFrameRate / 60.f;
	while (frameCounter >= 1) {
		// super
		result = (This->*s_origCBattleManager_Process)();

		if (result > 0 && result < 4) {
			printf("CBattleManager_Process returned %i\n", result);
			frameCounter = 0;
			break;
		}

		Practice::update();
		frameCounter--;
	}
	return result;
}



// �ݒ胍�[�h
void LoadSettings(LPCSTR profilePath, LPCSTR parentPath)
{
	FILE *_;

	AllocConsole();
	freopen_s(&_, "CONOUT$", "w", stdout);
	freopen_s(&_, "CONOUT$", "w", stderr);
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
	s_origCBattleManager_Process = SokuLib::union_cast<int (SokuLib::BattleManager::*)()>(
		SokuLib::TamperDword(
			SokuLib::vtbl_CBattleManager + SokuLib::BATTLE_MGR_OFFSET_ON_PROCESS,
			reinterpret_cast<DWORD>(CBattleManager_OnProcess)
		)
	);
	s_origCBattleManager_Render = SokuLib::union_cast<int (SokuLib::BattleManager::*)()>(
		SokuLib::TamperDword(
			SokuLib::vtbl_CBattleManager + SokuLib::BATTLE_MGR_OFFSET_ON_RENDER,
			reinterpret_cast<DWORD>(CBattleManager_Render)
		)
	);
	VirtualProtect((PVOID)RDATA_SECTION_OFFSET, RDATA_SECTION_SIZE, old, &old);

	VirtualProtect((PVOID)TEXT_SECTION_OFFSET, TEXT_SECTION_SIZE, old, &old);
	int newOffset = reinterpret_cast<int>(Practice::loadDeckData) - PAYLOAD_NEXT_INSTR_DECK_INFOS;
	Practice::s_origLoadDeckData = SokuLib::union_cast<void (__stdcall *)(char *, void *, SokuLib::deckInfo &, int, SokuLib::mVC9Dequeue<short> &)>(*(int *)PAYLOAD_ADDRESS_DECK_INFOS + PAYLOAD_NEXT_INSTR_DECK_INFOS);
	*(int *)PAYLOAD_ADDRESS_DECK_INFOS = newOffset;
	VirtualProtect((PVOID)TEXT_SECTION_OFFSET, TEXT_SECTION_SIZE, old, &old);


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
	if (fdwReason == DLL_PROCESS_DETACH && Practice::settings.activated)
		Practice::settings.save();
	return TRUE;
}