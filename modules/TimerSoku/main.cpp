//
// Created by Gegel85 on 31/10/2020
//

#include <SokuLib.hpp>
#include <shlwapi.h>

static int (SokuLib::BattleManager::*ogBattleMgrOnProcess)();
static void (SokuLib::BattleManager::*ogBattleMgrOnRender)();
static SokuLib::DrawUtils::Sprite digits;
static SokuLib::SWRFont font;
static bool init = false;
static short displayed = 0;

int __fastcall CBattleManager_OnRender(SokuLib::BattleManager *This)
{
	puts("Render");
	(This->*ogBattleMgrOnRender)();
	puts("Draw (maybe)");
	if (This->matchState < 6 && This->matchState >= 1) {
		auto timer = displayed;

		if (timer >= 10) {
			digits.setPosition({298, 80});
			digits.rect.left = timer / 10 * 11;
			digits.draw();
			digits.setPosition({320, 80});
			digits.rect.left = timer % 10 * 11;
			digits.draw();
		} else {
			digits.setPosition({309, 80});
			digits.rect.left = timer * 11;
			digits.draw();
		}
	}
	puts("Return");
	return 0;
}

int __fastcall CBattleManager_OnProcess(SokuLib::BattleManager *This)
{
	if (!init) {
		puts("Init");
		digits.texture.loadFromGame("data/battle/weatherFont000.cv0");
		digits.setSize({22, 36});
		digits.rect.width = 11;
		digits.rect.height = 18;
		init = true;
	}

	puts("Mgr");
	int ret = (This->*ogBattleMgrOnProcess)();

	puts("Check");
	if (This->matchState == -1)
		return ret;

	puts("Get timer");
	auto &timer = *(short *)&This->leftCharacterManager.objectBase.offset_0x14E;

	if (This->matchState != 2) {
		puts("Init timer");
		timer = 99 * 37;
	} else if (timer != 0) {
		puts("Tick timer");
		timer--;
		if (timer == 0) {
			puts("Timer end!");
			if (This->leftCharacterManager.objectBase.hp < This->rightCharacterManager.objectBase.hp)
				This->leftCharacterManager.objectBase.hp = 1;
			else if (This->leftCharacterManager.objectBase.hp > This->rightCharacterManager.objectBase.hp)
				This->rightCharacterManager.objectBase.hp = 1;
			else {
				This->leftCharacterManager.objectBase.hp = 1;
				This->rightCharacterManager.objectBase.hp = 1;
			}
		}
	}
	if (This->matchState <= 2 && timer % 37 == 0)
		displayed = (timer + 36) / 37;
	return ret;
}

extern "C" __declspec(dllexport) bool CheckVersion(const BYTE hash[16]) {
	return true;
}

extern "C" __declspec(dllexport) bool Initialize(HMODULE hMyModule, HMODULE hParentModule) {
	DWORD old;

#ifdef _DEBUG
	FILE *_;

	AllocConsole();
	freopen_s(&_, "CONOUT$", "w", stdout);
	freopen_s(&_, "CONOUT$", "w", stderr);
#endif

	// DWORD old;
	::VirtualProtect((PVOID)RDATA_SECTION_OFFSET, RDATA_SECTION_SIZE, PAGE_EXECUTE_WRITECOPY, &old);
	ogBattleMgrOnRender  = SokuLib::TamperDword(&SokuLib::VTable_BattleManager.onRender,  CBattleManager_OnRender);
	ogBattleMgrOnProcess = SokuLib::TamperDword(&SokuLib::VTable_BattleManager.onProcess, CBattleManager_OnProcess);
	// s_origCLogo_OnProcess   = TamperDword(vtbl_CLogo   + 4, (DWORD)CLogo_OnProcess);
	// s_origCBattle_OnProcess = TamperDword(vtbl_CBattle + 4, (DWORD)CBattle_OnProcess);
	// s_origCBattleSV_OnProcess = TamperDword(vtbl_CBattleSV + 4, (DWORD)CBattleSV_OnProcess);
	// s_origCBattleCL_OnProcess = TamperDword(vtbl_CBattleCL + 4, (DWORD)CBattleCL_OnProcess);
	// s_origCTitle_OnProcess  = TamperDword(vtbl_CTitle  + 4, (DWORD)CTitle_OnProcess);
	// s_origCSelect_OnProcess = TamperDword(vtbl_CSelect + 4, (DWORD)CSelect_OnProcess);
	::VirtualProtect((PVOID)RDATA_SECTION_OFFSET, RDATA_SECTION_SIZE, old, &old);
	::VirtualProtect((PVOID)TEXT_SECTION_OFFSET, TEXT_SECTION_SIZE, PAGE_EXECUTE_WRITECOPY, &old);
	::VirtualProtect((PVOID)TEXT_SECTION_OFFSET, TEXT_SECTION_SIZE, old, &old);

	::FlushInstructionCache(GetCurrentProcess(), nullptr, 0);
	return true;
}

extern "C" int APIENTRY DllMain(HMODULE hModule, DWORD fdwReason, LPVOID lpReserved) {
	return TRUE;
}