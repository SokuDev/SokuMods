//
// Created by PinkySmile on 31/10/2020
//

#include <SokuLib.hpp>
#include <shlwapi.h>

static int (SokuLib::BattleManager::*ogBattleManagerOnProcess)();

void __fastcall lowerHp(SokuLib::CharacterManager &mgr)
{
	mgr.objectBase.hp -= 1000;
}

int __fastcall CBattleManager_OnProcess(SokuLib::BattleManager *This)
{
	int ret = (This->*ogBattleManagerOnProcess)();

	if (This->matchState == -1)
		return ret;
	for (int i = 0; i < This->leftCharacterManager.hand.size; i++)
		This->leftCharacterManager.hand[i].cost = 1;
	for (int i = 0; i < This->rightCharacterManager.hand.size; i++)
		This->rightCharacterManager.hand[i].cost = 1;
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
	//ogBattleOnProcess = SokuLib::TamperDword(&SokuLib::VTable_Battle.onProcess, CBattle_OnProcess);
	//ogBattleCLOnProcess = SokuLib::TamperDword(&SokuLib::VTable_BattleClient.onProcess, CBattleCL_OnProcess);
	//ogBattleSVOnProcess = SokuLib::TamperDword(&SokuLib::VTable_BattleServer.onProcess, CBattleSV_OnProcess);
	ogBattleManagerOnProcess = SokuLib::TamperDword(&SokuLib::VTable_BattleManager.onProcess, CBattleManager_OnProcess);
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
	new SokuLib::Trampoline(0x00483D68, (void (*)())lowerHp, 5);
	return true;
}

extern "C" int APIENTRY DllMain(HMODULE hModule, DWORD fdwReason, LPVOID lpReserved) {
	return TRUE;
}