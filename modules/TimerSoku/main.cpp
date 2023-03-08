//
// Created by Gegel85 on 31/10/2020
//

#include <cmath>
#include <SokuLib.hpp>
#include <shlwapi.h>

#define EXPECTED_DURATION 120
#define CRUSH_INTERVAL 5

const unsigned multiplier = std::ceil(EXPECTED_DURATION * 60 / 99);

static void (__fastcall *ogBattleMgrUpdateCounters)(void *);
static void (SokuLib::BattleManager::*ogBattleMgrOnRender)();
static SokuLib::DrawUtils::Sprite digits;
static SokuLib::SWRFont font;
static bool init = false;
static unsigned short displayed = 0;

int __fastcall CBattleManager_OnRender(SokuLib::BattleManager *This)
{
	(This->*ogBattleMgrOnRender)();
	if (SokuLib::mainMode == SokuLib::BATTLE_MODE_PRACTICE)
		return 0;
	if (This->matchState < 6 && This->matchState >= 1 && !(displayed & 0x4000)) {
		auto timer = displayed & 0xFFF;

		if (displayed & 0x8000)
			digits.tint = SokuLib::Color::Red;
		else
			digits.tint = SokuLib::Color::White;
		if (timer >= 10) {
			digits.setPosition({299, 440});
			digits.rect.left = timer / 10 * 11;
			digits.draw();
			digits.setPosition({322, 440});
			digits.rect.left = timer % 10 * 11;
			digits.draw();
		} else {
			digits.setPosition({309, 440});
			digits.rect.left = timer * 11;
			digits.draw();
		}
	}
	return 0;
}

static bool musicPlayed = false;
bool *magicLocation = nullptr;

void punish(SokuLib::CharacterManager &chr, unsigned short &timer, unsigned char &orb)
{
	if (orb == 0) {
		if (timer & 0x4000)
			return;
		timer |= 0x4000;
		puts("Activate Typhoon");
		return SokuLib::activateWeather(SokuLib::WEATHER_TYPHOON, 1);
	}
	orb--;
	puts("Crush");
	SokuLib::playSEWaveBuffer(38);
}

static void playBGM(const char *bgm, bool expectedState)
{
	if (magicLocation) {
		if (musicPlayed == expectedState)
			return (void)puts("musicPlayed == expectedState");
		if (*(unsigned *)0x8985D8 != *(unsigned *)(magicLocation + 0x20))
			return (void)printf("PlayBGM %u != %u\n", *(unsigned *)0x8985D8, *(unsigned *)(magicLocation + 0x20));
		printf("Playing %s\n", bgm);
		SokuLib::playBGM(bgm);
		puts("Done");
		musicPlayed = expectedState;
		return;
	}
	printf("Playing %s\n", bgm);
	SokuLib::playBGM(bgm);
	puts("Done");
}

void __fastcall CBattleManager_OnProcess(void *obj)
{
	SokuLib::BattleManager *This = &SokuLib::getBattleMgr();

	if (!init) {
		digits.texture.loadFromGame("data/battle/weatherFont001.cv0");
		digits.setSize({22, 36});
		digits.rect.width = 11;
		digits.rect.height = 18;
		init = true;
	}

	ogBattleMgrUpdateCounters(obj);
	if (SokuLib::mainMode == SokuLib::BATTLE_MODE_PRACTICE)
		return;

	// We detect SokuRoll's hook and we extract an address from it so we can do the same checks
	if (*(unsigned char *)0x490B29 == 0xE9) {
		auto hookAddress = *(unsigned *)0x490B2A + 0x490B29 + 5;
		auto finalHookAddress = *(unsigned *)(hookAddress + 1) + hookAddress + 5;

		magicLocation = *(bool **)(finalHookAddress + 8);
	} else
		magicLocation = nullptr;
	// Second call is to check if the game is paused
	if (This->matchState < 1 || ((bool (*)())0x0043e740)())
		return;

	auto &timer = *(unsigned short *)&This->leftCharacterManager.objectBase.offset_0x14E;
	auto *orbs = (unsigned char *)&This->rightCharacterManager.objectBase.offset_0x14E;

	if (This->matchState == 1)
		musicPlayed = false;
	if (This->matchState != 2) {
		if (timer & 0x4000)
			SokuLib::weatherCounter = 0;
		if ((timer & 0x8000) && This->currentRound != 0) {
			auto file = "data/bgm/st" + std::string(SokuLib::gameParams.musicId < 10 ? "0" : "") + std::to_string(SokuLib::gameParams.musicId) + ".ogg";

			playBGM(file.c_str(), false);
		}
		timer = 99 * multiplier;
		orbs[0] = 5;
		orbs[1] = 5;
	} else if ((timer & 0x4000) == 0) {
		timer--;
		if ((timer & 0x3FFF) == 0) {
			if (!(timer & 0x8000))
				playBGM("data/bgm/st36.ogg", true);
			timer |= 0x8000;
			timer |= CRUSH_INTERVAL * 60;
			if (This->leftCharacterManager.objectBase.hp < This->rightCharacterManager.objectBase.hp)
				punish(This->leftCharacterManager, timer, orbs[0]);
			else if (This->leftCharacterManager.objectBase.hp > This->rightCharacterManager.objectBase.hp)
				punish(This->rightCharacterManager, timer, orbs[1]);
			else {
				punish(This->leftCharacterManager, timer, orbs[0]);
				punish(This->rightCharacterManager, timer, orbs[1]);
			}
		}
	}
	if (This->matchState <= 2) {
		if (!(timer & 0x4000)) {
			if (timer & 0x8000) {
				if ((timer & 0x3FFF) % 60 == 0 && (timer & 0x3FFF) != CRUSH_INTERVAL * 60) {
					puts("Tick");
					SokuLib::playSEWaveBuffer(59);
				}
				displayed = ((timer & 0x3FFF) + 59) / 60;
			} else if (timer % multiplier == 0)
				displayed = (timer + multiplier - 1) / multiplier;
		}
		if (timer & 0x4000)
			SokuLib::weatherCounter = 999;
		if (timer & 0x8000) {
			if (This->leftCharacterManager.maxSpirit >= orbs[0] * 200) {
				This->leftCharacterManager.maxSpirit = orbs[0] * 200;
				This->leftCharacterManager.timeWithBrokenOrb = 0;
			}
			if (This->rightCharacterManager.maxSpirit >= orbs[1] * 200) {
				This->rightCharacterManager.maxSpirit = orbs[1] * 200;
				This->rightCharacterManager.timeWithBrokenOrb = 0;
			}
		}
		displayed |= timer & 0xC000;
	}
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
	// s_origCLogo_OnProcess   = TamperDword(vtbl_CLogo   + 4, (DWORD)CLogo_OnProcess);
	// s_origCBattle_OnProcess = TamperDword(vtbl_CBattle + 4, (DWORD)CBattle_OnProcess);
	// s_origCBattleSV_OnProcess = TamperDword(vtbl_CBattleSV + 4, (DWORD)CBattleSV_OnProcess);
	// s_origCBattleCL_OnProcess = TamperDword(vtbl_CBattleCL + 4, (DWORD)CBattleCL_OnProcess);
	// s_origCTitle_OnProcess  = TamperDword(vtbl_CTitle  + 4, (DWORD)CTitle_OnProcess);
	// s_origCSelect_OnProcess = TamperDword(vtbl_CSelect + 4, (DWORD)CSelect_OnProcess);
	::VirtualProtect((PVOID)RDATA_SECTION_OFFSET, RDATA_SECTION_SIZE, old, &old);
	::VirtualProtect((PVOID)TEXT_SECTION_OFFSET, TEXT_SECTION_SIZE, PAGE_EXECUTE_WRITECOPY, &old);
	ogBattleMgrUpdateCounters = SokuLib::TamperNearJmpOpr(0x004796b6, CBattleManager_OnProcess);
	::VirtualProtect((PVOID)TEXT_SECTION_OFFSET, TEXT_SECTION_SIZE, old, &old);

	::FlushInstructionCache(GetCurrentProcess(), nullptr, 0);
	return true;
}

extern "C" int APIENTRY DllMain(HMODULE hModule, DWORD fdwReason, LPVOID lpReserved) {
	return TRUE;
}