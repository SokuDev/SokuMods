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

#define PAYLOAD_ADDRESS_PLAYER 0x40A45E
#define PAYLOAD_NEXT_INSTR_PLAYER (PAYLOAD_ADDRESS_PLAYER + 4)
#define PAYLOAD_ADDRESS_DUMMY 0x48224E
#define PAYLOAD_NEXT_INSTR_DUMMY (PAYLOAD_ADDRESS_DUMMY + 4)

struct Title {};
struct Battle {};
struct Select {};
struct Loading {};
struct BattleWatch {};
struct LoadingWatch {};

sf::RenderWindow *sfmlWindow;
void (*s_origKeymapManager_SetInputs)();
void (*s_origDummy_SetInputs)();
int (__thiscall SokuLib::BattleManager::*s_origCBattleManager_Render)();
int (__thiscall SokuLib::BattleManager::*s_origCBattleManager_Start)();
int (__thiscall SokuLib::BattleManager::*s_origCBattleManager_KO)();
int (__thiscall LoadingWatch::*s_origCLoadingWatch_Process)();
int (__thiscall BattleWatch::*s_origCBattleWatch_Process)();
int (__thiscall Loading::*s_origCLoading_Process)();
int (__thiscall Battle::*s_origCBattle_Process)();
int (__thiscall Select::*s_origCSelect_Process)();
int (__thiscall Title::*s_origCTitle_Process)();
char profilePath[1024 + MAX_PATH];
char profileParent[1024 + MAX_PATH];
SokuLib::KeyInput dummy;

struct TestKeyMapMgr {
	SokuLib::KeymapManager base;

	void handleInput()
	{
		auto &mgr = SokuLib::getBattleMgr();

		if (&this->base == mgr.leftCharacterManager.keyManager->keymapManager)
			return this->handlePlayerInput();
	}

	void handlePlayerInput()
	{
		puts("Player input");
		memcpy(&dummy, &this->base.input, sizeof(this->base.input));
		memset(&this->base.input, 0, sizeof(this->base.input));
	}
};

struct Dummy {
	SokuLib::CharacterManager base;

	void handleInput()
	{
		memcpy(&this->base.keyMap, &dummy, sizeof(dummy));
	}
};

void KeymapManagerSetInputs()
{
	__asm push ecx;
	s_origKeymapManager_SetInputs();
	__asm pop ecx;
	SokuLib::union_cast<void (*)()>(&TestKeyMapMgr::handleInput)();
}

void __fastcall dummySetInputs()
{
	SokuLib::union_cast<void (*)()>(&Dummy::handleInput)();
}

static void activate()
{
	DWORD old;
	int newOffset;

	if (sfmlWindow)
		return;

	sfmlWindow = new sf::RenderWindow{{640, 480}, "Advanced Practice Mode", sf::Style::Titlebar};
	Practice::init(profileParent);
	Practice::gui.setTarget(*sfmlWindow);
	try {
		Practice::loadAllGuiElements(profileParent);
	} catch (std::exception &e) {
		puts(e.what());
		throw;
	}

	//Bypass the basic practice features by messing up the check for practice mode.
	VirtualProtect((PVOID)0x42A333, 1, PAGE_EXECUTE_WRITECOPY, &old);
	// cmp eax, 08 -> cmp eax, 0A
	*(char *)0x42A333 = 10;
	VirtualProtect((PVOID)0x42A333, 1, old, &old);

	VirtualProtect((PVOID)PAYLOAD_ADDRESS_PLAYER, 4, PAGE_EXECUTE_WRITECOPY, &old);
	newOffset = (int)KeymapManagerSetInputs - PAYLOAD_NEXT_INSTR_PLAYER;
	s_origKeymapManager_SetInputs = reinterpret_cast<void (*)()>(*(int *)PAYLOAD_ADDRESS_PLAYER + PAYLOAD_NEXT_INSTR_PLAYER);
	*(int *)PAYLOAD_ADDRESS_PLAYER = newOffset;
	VirtualProtect((PVOID)PAYLOAD_ADDRESS_PLAYER, 4, old, &old);

	VirtualProtect((PVOID)PAYLOAD_ADDRESS_DUMMY, 4, PAGE_EXECUTE_WRITECOPY, &old);
	newOffset = (int)dummySetInputs - PAYLOAD_NEXT_INSTR_DUMMY;
	s_origDummy_SetInputs = reinterpret_cast<void (*)()>(*(int *)PAYLOAD_ADDRESS_DUMMY + PAYLOAD_NEXT_INSTR_DUMMY);
	*(int *)PAYLOAD_ADDRESS_DUMMY = newOffset;
	VirtualProtect((PVOID)PAYLOAD_ADDRESS_DUMMY, 4, old, &old);
}

static void deactivate()
{
	DWORD old;

	if (!sfmlWindow)
		return;

	delete sfmlWindow;
	sfmlWindow = nullptr;
	VirtualProtect((PVOID)0x42A333, 1, PAGE_EXECUTE_WRITECOPY, &old);
	*(char *)0x42A333 = 8;
	VirtualProtect((PVOID)0x42A333, 1, old, &old);

	VirtualProtect((PVOID)PAYLOAD_ADDRESS_PLAYER, 4, PAGE_EXECUTE_WRITECOPY, &old);
	*(int *)PAYLOAD_ADDRESS_PLAYER = SokuLib::union_cast<int>(s_origKeymapManager_SetInputs) - PAYLOAD_NEXT_INSTR_PLAYER;
	VirtualProtect((PVOID)PAYLOAD_ADDRESS_PLAYER, 4, old, &old);

	VirtualProtect((PVOID)PAYLOAD_ADDRESS_DUMMY, 4, PAGE_EXECUTE_WRITECOPY, &old);
	*(int *)PAYLOAD_ADDRESS_DUMMY = SokuLib::union_cast<int>(s_origDummy_SetInputs) - PAYLOAD_NEXT_INSTR_DUMMY;
	VirtualProtect((PVOID)PAYLOAD_ADDRESS_DUMMY, 4, old, &old);
}

int __fastcall CTitle_OnProcess(Title *This)
{
	deactivate();

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
	bool activated = false;

	//battle.leftCharacterManager.keyManager.keymapManager.horizontalAxis = 0;
	//battle.leftCharacterManager.keyManager.keymapManager.verticalAxis = 0;
	//battle.leftCharacterManager.keyManager.keymapManager.a = 0;
	//battle.leftCharacterManager.keyManager.keymapManager.b = 0;
	//battle.leftCharacterManager.keyManager.keymapManager.c = 0;
	//battle.leftCharacterManager.keyManager.keymapManager.d = 0;
	//battle.leftCharacterManager.keyManager.keymapManager.changeCard = 0;
	//battle.leftCharacterManager.keyManager.keymapManager.spellcard = 0;

	if (SokuLib::mainMode == SokuLib::BATTLE_MODE_PRACTICE) {
		//__asm ;
		//(0x42A4C9);
		activated = true, activate();
	}

	memcpy(&battle.rightCharacterManager.keyMap, &dummy, sizeof(dummy));
	// super
	int ret = (This->*s_origCBattle_Process)();

	if (activated) {
		//if (!battle.rightCharacterManager.keyManager.keymapManager) {
		//	battle.rightCharacterManager.keyManager.keymapManager = New<KeymapManager>(sizeof(*battle.rightCharacterManager.keyManager.keymapManager));
		//	memcpy(
		//		battle.rightCharacterManager.keyManager.keymapManager,
		//		battle.leftCharacterManager.keyManager.keymapManager,
		//		sizeof(*battle.rightCharacterManager.keyManager.keymapManager)
		//	);
		//}
	}
	return ret;
}

int __fastcall CSelect_OnProcess(Select *This)
{
	deactivate();

	// super
	return (This->*s_origCSelect_Process)();
}

int __fastcall CLoading_OnProcess(Loading *This)
{
	deactivate();

	// super
	return (This->*s_origCLoading_Process)();
}

int __fastcall CLoadingWatch_OnProcess(LoadingWatch *This)
{
	deactivate();

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