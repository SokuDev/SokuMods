//
// Created by PinkySmile on 01/08/2021.
//

#include <fstream>
#include <dinput.h>
#include <SokuLib.hpp>

static SokuLib::SWRFont font;
static SokuLib::DrawUtils::Sprite punishText;
static std::pair<unsigned, unsigned> dispCtr = {1000, 1000};
static void (SokuLib::BattleManager::*og_BattleManagerOnRender)();
static int (SokuLib::BattleManager::*og_BattleManagerOnProcess)();

#ifndef _DEBUG
#define puts(...)
#define printf(...)
#endif

static int (SokuLib::CharacterManager::*original_onHit)(int param);

bool __fastcall isHit(SokuLib::CharacterManager &character, int, int param)
{
	if (
		!character.objectBase.frameData.frameFlags.guardAvailable &&
		(character.objectBase.action < SokuLib::ACTION_STAND_GROUND_HIT_SMALL_HITSTUN ||
		character.objectBase.action > SokuLib::ACTION_NEUTRAL_TECH)
	) {
		if (&character == &SokuLib::getBattleMgr().leftCharacterManager)
			dispCtr.second = 0;
		else if (&character == &SokuLib::getBattleMgr().rightCharacterManager)
			dispCtr.first = 0;
	}
	return (character.*original_onHit)(param);
}

int __fastcall BattleOnProcess(SokuLib::BattleManager *This)
{
	int ret = (This->*og_BattleManagerOnProcess)();

	if (!punishText.texture.hasTexture()) {
		SokuLib::FontDescription desc;

		desc.r1 = 255;
		desc.r2 = 255;
		desc.g1 = 0;
		desc.g2 = 0;
		desc.b1 = 0;
		desc.b2 = 0;
		desc.height = 24;
		desc.weight = FW_BOLD;
		desc.italic = 0;
		desc.shadow = 2;
		desc.bufferSize = 1000000;
		desc.charSpaceX = 0;
		desc.charSpaceY = 0;
		desc.offsetX = 0;
		desc.offsetY = 0;
		desc.useOffset = 0;
		strcpy(desc.faceName, "Tahoma");
		font.create();
		font.setIndirect(desc);

		punishText.texture.createFromText("Punish !", font, {0x1000, 50});
		punishText.setSize(punishText.texture.getSize());
		punishText.rect.width = punishText.texture.getSize().x;
		punishText.rect.height = punishText.texture.getSize().y;
	}
	if (dispCtr.first < 1000)
		dispCtr.first++;
	if (dispCtr.second < 1000)
		dispCtr.second++;
	return ret;
}

void __fastcall BattleOnRender(SokuLib::BattleManager *This)
{
	(This->*og_BattleManagerOnRender)();

	punishText.setPosition({10, 54});
	punishText.tint.a = min(255, max(0, (240 - static_cast<int>(dispCtr.first * 4) + 255)));
	punishText.draw();
	punishText.setPosition({532, 54});
	punishText.tint.a = min(255, max(0, (240 - static_cast<int>(dispCtr.second * 4) + 255)));
	punishText.draw();
}

// �ݒ胍�[�h
void LoadSettings()
{
}

extern "C" __declspec(dllexport) bool CheckVersion(const BYTE hash[16]) {
	return memcmp(hash, SokuLib::targetHash, 16) == 0;
}

extern "C" __declspec(dllexport) bool Initialize(HMODULE hMyModule, HMODULE hParentModule)
{
	DWORD old;

	LoadSettings();

	VirtualProtect((PVOID)RDATA_SECTION_OFFSET, RDATA_SECTION_SIZE, PAGE_EXECUTE_WRITECOPY, &old);
	og_BattleManagerOnProcess = SokuLib::TamperDword(&SokuLib::VTable_BattleManager.onProcess, BattleOnProcess);
	og_BattleManagerOnRender  = SokuLib::TamperDword(&SokuLib::VTable_BattleManager.onRender,  BattleOnRender);
	VirtualProtect((PVOID)RDATA_SECTION_OFFSET, RDATA_SECTION_SIZE, old, &old);

	VirtualProtect((PVOID)TEXT_SECTION_OFFSET, TEXT_SECTION_SIZE, PAGE_EXECUTE_WRITECOPY, &old);
	original_onHit = SokuLib::union_cast<int (SokuLib::CharacterManager::*)(int)>(SokuLib::TamperNearJmpOpr(0x47c5a9, reinterpret_cast<DWORD>(isHit)));
	VirtualProtect((PVOID)TEXT_SECTION_OFFSET, TEXT_SECTION_SIZE, old, &old);

	FlushInstructionCache(GetCurrentProcess(), nullptr, 0);
	return true;
}

extern "C" int APIENTRY DllMain(HMODULE hModule, DWORD fdwReason, LPVOID lpReserved)
{
	return TRUE;
}