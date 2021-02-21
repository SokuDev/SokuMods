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
void (*s_origKeymapManager_SetInputs)();
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
KeyInput dummy;

struct Test {
	KeymapManager base;

	void handleInput()
	{
		auto &mgr = getBattleMgr();

		if (&this->base == mgr.leftCharacterManager.keyManager->keymapManager)
			return this->handlePlayerInput();
		if (mgr.rightCharacterManager.keyManager && &this->base == mgr.rightCharacterManager.keyManager->keymapManager)
			return this->handleDummyInput();
		return (void)puts("Unknown input");
	}

	void handlePlayerInput()
	{
		puts("Player input");
		memcpy(&dummy, &this->base.input, sizeof(this->base.input));
		memset(&this->base.input, 0, sizeof(this->base.input));
		//this->base.input.horizontalAxis = rand() % 3 - 1;
		//this->base.input.verticalAxis = rand() % 3 - 1;
		//this->base.input.a = rand() % 2;
		//this->base.input.b = rand() % 2;
		//this->base.input.c = rand() % 2;
		//this->base.input.d = rand() % 2;
		//this->base.input.changeCard = rand() % 2;
		//this->base.input.spellcard = rand() % 2;
	}

	void handleDummyInput()
	{
		puts("Dummy input");
		memcpy(&this->base.input, &dummy, sizeof(dummy));
	}
};

void KeymapManagerSetInputs()
{
	// super
	__asm push ecx;
	s_origKeymapManager_SetInputs();
	__asm pop ecx;
	union_cast<void (*)()>(&Test::handleInput)();
}

static void activate()
{
	DWORD old;

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

	VirtualProtect((PVOID)0x40A45E, 4, PAGE_EXECUTE_WRITECOPY, &old);
	int newOffset = (int)KeymapManagerSetInputs - 0x40A462;

	s_origKeymapManager_SetInputs = reinterpret_cast<void (*)()>(*(int *)0x40A45E + 0x40A462);
	*(int *)0x40A45E = newOffset;
	VirtualProtect((PVOID)0x40A45E, 4, old, &old);
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

	VirtualProtect((PVOID)0x40A45E, 4, PAGE_EXECUTE_WRITECOPY, &old);
	*(int *)0x40A45E = union_cast<int>(s_origKeymapManager_SetInputs) - 0x40A462;
	VirtualProtect((PVOID)0x40A45E, 4, old, &old);
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
	auto &battle = getBattleMgr();
	bool activated = false;

	//battle.leftCharacterManager.keyManager.keymapManager.horizontalAxis = 0;
	//battle.leftCharacterManager.keyManager.keymapManager.verticalAxis = 0;
	//battle.leftCharacterManager.keyManager.keymapManager.a = 0;
	//battle.leftCharacterManager.keyManager.keymapManager.b = 0;
	//battle.leftCharacterManager.keyManager.keymapManager.c = 0;
	//battle.leftCharacterManager.keyManager.keymapManager.d = 0;
	//battle.leftCharacterManager.keyManager.keymapManager.changeCard = 0;
	//battle.leftCharacterManager.keyManager.keymapManager.spellcard = 0;

	if (SokuLib::mainMode == SokuLib::BATTLE_MODE_PRACTICE)
		activated = true, activate();

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

int __fastcall CBattleManager_KO(BattleManager *This)
{
	// super
	return (This->*s_origCBattleManager_KO)();
}

int __fastcall CBattleManager_Start(BattleManager *This)
{
	// super
	return (This->*s_origCBattleManager_Start)();
}

int __fastcall CBattleManager_Render(BattleManager *This)
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
	Test test;

	AllocConsole();
	freopen_s(&_, "CONOUT$", "w", stdout);
	puts("Hello !");
	printf("%p : %p\n", &test, &test.base);
	//port = GetPrivateProfileInt("Server", "Port", 80, profilePath);
}

void hookFunctions()
{
	DWORD old;

	//Setup hooks
	VirtualProtect((PVOID)RDATA_SECTION_OFFSET, RDATA_SECTION_SIZE, PAGE_EXECUTE_WRITECOPY, &old);
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
	VirtualProtect((PVOID)RDATA_SECTION_OFFSET, RDATA_SECTION_SIZE, old, &old);

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