//
// Created by PinkySmile on 14/03/2023.
//
//
// Created by Gegel85 on 31/10/2020
//

#include <cmath>
#include <SokuLib.hpp>
#include <shlwapi.h>


static void (__fastcall *ogBattleMgrUpdateCounters)(void *);
static const char effectTable[] = {
	0,  // WEATHER_SUNNY
	0,  // WEATHER_DRIZZLE
	0,  // WEATHER_CLOUDY
	2,  // WEATHER_BLUE_SKY
	6,  // WEATHER_HAIL
	0,  // WEATHER_SPRING_HAZE
	0,  // WEATHER_HEAVY_FOG
	16, // WEATHER_SNOW
	20, // WEATHER_SUN_SHOWER
	0,  // WEATHER_SPRINKLE
	0,  // WEATHER_TEMPEST
	14, // WEATHER_MOUNTAIN_VAPOR
	20, // WEATHER_RIVER_MIST
	6,  // WEATHER_TYPHOON
	7,  // WEATHER_CALM
	16, // WEATHER_DIAMOND_DUST
	0,  // WEATHER_DUST_STORM
	0,  // WEATHER_SCORCHING_SUN
	2,  // WEATHER_MONSOON
	0,  // WEATHER_AURORA
	14, // WEATHER_TWILIGHT
	0,  // WEATHER_CLEAR
};

void checkShock(SokuLib::CharacterManager &chr, SokuLib::CharacterManager &op)
{
	auto FUN_00438ce0 = reinterpret_cast<void (__thiscall *)(SokuLib::CharacterManager &, unsigned, float, float, unsigned, unsigned)>(0x438ce0);

	if (op.timeStop)
		return;
	if (op.objectBase.action >= SokuLib::ACTION_GRABBED && op.objectBase.action < 120 && op.objectBase.position.y != 0)
		return;
	if (chr.objectBase.action < SokuLib::ACTION_5A)
		return;
	if (chr.objectBase.action == SokuLib::ACTION_SYSTEM_CARD && chr.timeStop)
		return;
	if (chr.keyMap.a > 4 || chr.keyMap.a == 0)
		return;
	if (chr.keyMap.d > 4 || chr.keyMap.d == 0)
		return;
	if (chr.maxSpirit == 0)
		return;
	if (
		chr.objectBase.action == SokuLib::ACTION_66A &&
		chr.objectBase.actionBlockId == 0 &&
		chr.objectBase.frameCount < 4 &&
		chr.keyMap.d >= chr.keyMap.a
	)
		return;
	if (SokuLib::activeWeather != SokuLib::WEATHER_SUNNY) {
		chr.maxSpirit -= 200;
		chr.currentSpirit = chr.maxSpirit;
		chr.timeWithBrokenOrb = 0;
	}
	if (chr.currentSpirit < 200)
		chr.currentSpirit = 0;
	else
		chr.currentSpirit -= 200;
	if (chr.objectBase.position.y == 0) {
		chr.objectBase.action = SokuLib::ACTION_SYSTEM_CARD;
		chr.timeStop = 65;
		chr.grabInvulTimer = 60;
		chr.meleeInvulTimer = 60;
		chr.projectileInvulTimer = 60;
		SokuLib::playSEWaveBuffer(23);
		FUN_00438ce0(chr, 115, chr.objectBase.position.x, chr.objectBase.position.y + 120, 1, 1);
	} else {
		chr.objectBase.action = SokuLib::ACTION_FALLING;
		chr.timeStop += 5;
		chr.objectBase.hitstop += 5;
		FUN_00438ce0(chr, 69, chr.objectBase.position.x, chr.objectBase.position.y + 120, 1, 1);
	}
	chr.objectBase.speed = {0, 0};
	chr.objectBase.gravity = {0, 0};
	chr.objectBase.animate();
	chr.objectBase.renderInfos.scale = {1, 1};
	chr.objectBase.renderInfos.xRotation = 0;
	chr.objectBase.renderInfos.yRotation = 0;
	chr.objectBase.renderInfos.zRotation = 0;
	chr.correction |= 0x18;
	chr.offset_0x7CD[3] = 0;
	/*if (SokuLib::activeWeather == SokuLib::WEATHER_CLEAR && SokuLib::weatherCounter % 90 == 1)
		chr.offset_0x7CD[3] = 14;
	else if (SokuLib::activeWeather == SokuLib::WEATHER_AURORA)
		chr.offset_0x7CD[3] = effectTable[SokuLib::weatherCounter % SokuLib::WEATHER_CLEAR];
	else
		chr.offset_0x7CD[3] = effectTable[SokuLib::activeWeather];*/
	if (op.objectBase.action >= SokuLib::ACTION_GRABBED && op.objectBase.action < 120) {
		op.objectBase.action = SokuLib::ACTION_STAND_GROUND_HIT_HUGE_HITSTUN;
		if (op.objectBase.hitstop > 360)
			op.objectBase.hitstop = 360;
		op.objectBase.animate();
		op.objectBase.speed = {0, 0};
		op.objectBase.gravity = {0, 0};
		op.objectBase.renderInfos.scale = {1, 1};
		op.objectBase.renderInfos.xRotation = 0;
		op.objectBase.renderInfos.yRotation = 0;
		op.objectBase.renderInfos.zRotation = 0;
	}
}

void __fastcall CBattleManager_OnProcess(void *obj)
{
	SokuLib::BattleManager *This = &SokuLib::getBattleMgr();

	ogBattleMgrUpdateCounters(obj);
	checkShock(This->leftCharacterManager, This->rightCharacterManager);
	checkShock(This->rightCharacterManager, This->leftCharacterManager);
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
	//ogBattleMgrOnRender  = SokuLib::TamperDword(&SokuLib::VTable_BattleManager.onRender,  CBattleManager_OnRender);
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