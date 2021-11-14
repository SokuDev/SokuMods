//
// Created by PinkySmile on 01/08/2021.
// Edited by PC_volt
//

#include <SokuLib.hpp>
#include <dinput.h>
#include <fstream>
static void (SokuLib::BattleManager::*og_BattleManagerOnRoundstart)();
static void (SokuLib::BattleManager::*og_BattleManagerOnRender)();
static int (SokuLib::BattleManager::*og_BattleManagerOnProcess)();
static int (SokuLib::CharacterManager::*original_onHit)(int param);
#ifndef _DEBUG
#define puts(...)
#define printf(...)

#endif

static bool loaded = false;
static SokuLib::DrawUtils::Sprite bePunish;
static SokuLib::DrawUtils::Sprite jumpPunish;
static SokuLib::DrawUtils::Sprite dashPunish;
static SokuLib::DrawUtils::Sprite attackPunish;
static SokuLib::DrawUtils::Sprite punish;

static SokuLib::SWRFont font;
static SokuLib::DrawUtils::Sprite* punishTextP1;
static SokuLib::DrawUtils::Sprite* punishTextP2;
static std::pair<unsigned, unsigned> displayCounter = {1000, 1000};

SokuLib::DrawUtils::Sprite* associatePunishSprite(SokuLib::CharacterManager &character)
{
	char* punishMessage;
	if ((character.objectBase.action >= SokuLib::ACTION_NEUTRAL_HIGH_JUMP && character.objectBase.action <= SokuLib::ACTION_FORWARD_HIGH_JUMP_FROM_GROUND_DASH)
			|| (character.objectBase.action >= SokuLib::ACTION_NEUTRAL_JUMP && character.objectBase.action <= SokuLib::ACTION_BACKWARD_JUMP))
	{
		return &jumpPunish;
	}
	else if (character.objectBase.action >= SokuLib::ACTION_FORWARD_DASH && character.objectBase.action <= SokuLib::ACTION_LILYPAD_BACKDASH)
	{
		return &dashPunish;
	}
	else if (character.objectBase.action >= SokuLib::ACTION_BE2 && character.objectBase.action <= SokuLib::ACTION_jBE6)
	{
		return &bePunish;
	}
	else if (character.objectBase.action >= SokuLib::ACTION_5A)
	{
		return &attackPunish;
	}
	else
	{
		return &punish;
	}
}

int __fastcall isHit(SokuLib::CharacterManager &character, int, int param)
{
	if (!character.objectBase.frameData->frameFlags.guardAvailable &&
		(character.objectBase.action < SokuLib::ACTION_STAND_GROUND_HIT_SMALL_HITSTUN ||
		character.objectBase.action > SokuLib::ACTION_NEUTRAL_TECH))
	{
		if (&character == &SokuLib::getBattleMgr().leftCharacterManager)
		{
			punishTextP1 = associatePunishSprite(character);
			displayCounter.first = 0;
		}
		else if (&character == &SokuLib::getBattleMgr().rightCharacterManager)
		{
			punishTextP2 = associatePunishSprite(character);
			displayCounter.second = 0;
		}
	}

	return (character.*original_onHit)(param);
}

void createPunishTextSprite(SokuLib::DrawUtils::Sprite &punishText, char* punishMessage)
{
	SokuLib::Vector2<int> realSize;
	if (!punishText.texture.createFromText(punishMessage, font, {0x1000, 50}, &realSize))
	{
		puts("Create from text failed");
	}
	punishText.setSize(realSize.to<unsigned>());
	punishText.rect.width = realSize.x;
	punishText.rect.height = realSize.y;
}

void createSprites()
{
	if (loaded)
		return;

	loaded = true;
	SokuLib::FontDescription desc;

	desc.r1 = 255;
	desc.r2 = 255;
	desc.g1 = 255;
	desc.g2 = 255;
	desc.b1 = 255;
	desc.b2 = 255;
	desc.height = 24;
	desc.weight = FW_BOLD;
	desc.italic = 0;
	desc.shadow = 4;
	desc.bufferSize = 1000000;
	desc.charSpaceX = 0;
	desc.charSpaceY = 0;
	desc.offsetX = 0;
	desc.offsetY = 0;
	desc.useOffset = 0;
	strcpy(desc.faceName, "MonoSpatialModSWR");
	font.create();
	font.setIndirect(desc);

	createPunishTextSprite(jumpPunish, "Jump Punish!");
	createPunishTextSprite(bePunish, "BE Punish!");
	createPunishTextSprite(dashPunish, "Dash Punish!");
	createPunishTextSprite(attackPunish, "Attack Punish!");
	createPunishTextSprite(punish, "Punish!");
}

int __fastcall BattleOnProcess(SokuLib::BattleManager *This)
{
	int ret = (This->*og_BattleManagerOnProcess)();

	createSprites();
	return ret;
}

void __fastcall BattleOnRender(SokuLib::BattleManager *This)
{
	(This->*og_BattleManagerOnRender)();

	if (displayCounter.first < 1000)
		displayCounter.first++;
	if (displayCounter.second < 1000)
		displayCounter.second++;

	if (displayCounter.first < 1000)
	{
		punishTextP1->setPosition({17, 67});
		punishTextP1->tint.a = min(255, max(0, (240 - static_cast<int>(displayCounter.first * 4) + 255)));
		punishTextP1->draw();
	}

	if (displayCounter.second < 1000)
	{
		int xPosMessageP2 = 640 - 17 - punishTextP2->rect.width;
		punishTextP2->setPosition({xPosMessageP2, 67});
		punishTextP2->tint.a = min(255, max(0, (240 - static_cast<int>(displayCounter.second * 4) + 255)));
		punishTextP2->draw();
	}
}

extern "C" __declspec(dllexport) bool CheckVersion(const BYTE hash[16]) {
	return memcmp(hash, SokuLib::targetHash, 16) == 0;
}

extern "C" __declspec(dllexport) bool Initialize(HMODULE hMyModule, HMODULE hParentModule)
{
	DWORD old;

#ifdef _DEBUG
	FILE *_;

	AllocConsole();
	freopen_s(&_, "CONOUT$", "w", stdout);
	freopen_s(&_, "CONOUT$", "w", stderr);
#endif

	VirtualProtect((PVOID)RDATA_SECTION_OFFSET, RDATA_SECTION_SIZE, PAGE_EXECUTE_WRITECOPY, &old);
	//og_BattleManagerOnRoundstart = SokuLib::TamperDword(&SokuLib::VTable_BattleManager.onRoundStart, BattleOnRoundstart);
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