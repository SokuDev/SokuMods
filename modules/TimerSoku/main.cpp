//
// Created by Gegel85 on 31/10/2020
//

#include <SokuLib.hpp>
#include <shlwapi.h>

#define EXPECTED_DURATION 120
#define CRUSH_INTERVAL 5

const unsigned multiplier = std::ceil(EXPECTED_DURATION * 60 / 99);

static int (SokuLib::BattleManager::*ogBattleMgrOnProcess)();
static void (SokuLib::BattleManager::*ogBattleMgrOnRender)();
static SokuLib::DrawUtils::Sprite digits;
static SokuLib::SWRFont font;
static bool init = false;
static unsigned short displayed = 0;

int __fastcall CBattleManager_OnRender(SokuLib::BattleManager *This)
{
	(This->*ogBattleMgrOnRender)();
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

void punish(SokuLib::CharacterManager &chr, unsigned short &timer, unsigned char &orb)
{
	if (orb == 0) {
		timer |= 0x4000;
		return SokuLib::activateWeather(SokuLib::WEATHER_TYPHOON, 1);
	}
	SokuLib::playSEWaveBuffer(38);
	orb--;
}

int __fastcall CBattleManager_OnProcess(SokuLib::BattleManager *This)
{
	if (!init) {
		digits.texture.loadFromGame("data/battle/weatherFont001.cv0");
		digits.setSize({22, 36});
		digits.rect.width = 11;
		digits.rect.height = 18;
		init = true;
	}

	int ret = (This->*ogBattleMgrOnProcess)();

	if (This->matchState < 1)
		return ret;

	auto &timer = *(unsigned short *)&This->leftCharacterManager.objectBase.offset_0x14E;
	auto *orbs = (unsigned char *)&This->rightCharacterManager.objectBase.offset_0x14E;

	if (This->matchState != 2) {
		if (timer & 0x4000)
			SokuLib::weatherCounter = 0;
		if (timer & 0x8000) {
			//auto file = "data/bgm/st" + std::string(SokuLib::gameParams.musicId < 10 ? "0" : "") + std::to_string(SokuLib::gameParams.musicId) + ".ogg";

			//printf("Playing %s\n", file.c_str());
			//SokuLib::playBGM(file.c_str());
			//puts("Done");
		}
		timer = 99 * multiplier;
		orbs[0] = 5;
		orbs[1] = 5;
	} else if ((timer & 0x4000) == 0) {
		timer--;
		if ((timer & 0x3FFF) == 0) {
			if (!(timer & 0x8000)) {
				//puts("Playing data/bgm/st36.ogg");
				//SokuLib::playBGM("data/bgm/st36.ogg");
				//puts("Done");
			}
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
				if ((timer & 0x3FFF) % 60 == 0 && (timer & 0x3FFF) != CRUSH_INTERVAL * 60)
					SokuLib::playSEWaveBuffer(59);
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