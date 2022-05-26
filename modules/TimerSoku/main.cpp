//
// Created by Gegel85 on 31/10/2020
//

#include <SokuLib.hpp>
#include <shlwapi.h>

static int (SokuLib::BattleManager::*ogBattleMgrOnProcess)();
static void (SokuLib::BattleManager::*ogBattleMgrOnRender)();
static SokuLib::DrawUtils::Sprite timerSprite;
static unsigned timer = 99 * 37;
static SokuLib::SWRFont font;
static bool init = false;

void __fastcall CBattleManager_OnRender(SokuLib::BattleManager *This)
{
	(This->*ogBattleMgrOnRender)();
	if (This->matchState < 6)
		timerSprite.draw();
}

int __fastcall CBattleManager_OnProcess(SokuLib::BattleManager *This)
{
	if (!init) {
		SokuLib::FontDescription desc;

		desc.r1 = 255;
		desc.r2 = 255;
		desc.g1 = 20;
		desc.g2 = 20;
		desc.b1 = 20;
		desc.b2 = 20;
		desc.height = 30;
		desc.weight = FW_NORMAL;
		desc.italic = 0;
		desc.shadow = 1;
		desc.bufferSize = 1000000;
		desc.charSpaceX = 0;
		desc.charSpaceY = 0;
		desc.offsetX = 0;
		desc.offsetY = 0;
		desc.useOffset = 0;
		strcpy(desc.faceName, "MonoSpatialModSWR");
		desc.weight = FW_REGULAR;
		font.create();
		font.setIndirect(desc);
		init = true;
	}

	int ret = (This->*ogBattleMgrOnProcess)();
	SokuLib::Vector2i size;

	if (This->matchState != 2) {
		timer = 99 * 37;
	} else if (timer != 0) {
		timer--;
		if (timer == 0) {
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
	if (This->matchState <= 2 && timer % 37 == 0) {
		timerSprite.texture.createFromText(std::to_string((timer + 36) / 37).c_str(), font, {100, 60}, &size);
		timerSprite.setPosition({640 / 2 - size.x / 2, 80});
		timerSprite.setSize(size.to<unsigned>());
		timerSprite.rect.width = size.x;
		timerSprite.rect.height = size.y;
	}
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