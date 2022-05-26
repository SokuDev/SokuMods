//
// Created by Gegel85 on 31/10/2020
//

#include <SokuLib.hpp>
#include <shlwapi.h>

static int (SokuLib::BattleManager::*ogBattleManagerOnProcess)();
static int (SokuLib::Battle::*ogBattleOnProcess)();
static int (SokuLib::BattleClient::*ogBattleCLOnProcess)();
static int (SokuLib::BattleServer::*ogBattleSVOnProcess)();
static std::pair<unsigned, unsigned> count;

void saveCards(SokuLib::BattleManager &mgr)
{
	count.first = mgr.leftCharacterManager.cardCount;
	count.second = mgr.rightCharacterManager.cardCount;
}

void battleProcess(SokuLib::BattleManager &mgr)
{
	if (count.first > mgr.leftCharacterManager.cardCount)
		mgr.leftCharacterManager.objectBase.hp -= 1000;
	if (count.second > mgr.rightCharacterManager.cardCount)
		mgr.rightCharacterManager.objectBase.hp -= 1000;
}
/*
int __fastcall CBattleSV_OnProcess(SokuLib::BattleServer *This)
{
	saveCards();

	int ret = (This->*ogBattleSVOnProcess)();

	battleProcess();
	return ret;
}

int __fastcall CBattleCL_OnProcess(SokuLib::BattleClient *This)
{
	saveCards();

	int ret = (This->*ogBattleCLOnProcess)();

	battleProcess();
	return ret;
}

int __fastcall CBattle_OnProcess(SokuLib::Battle *This)
{
	saveCards();

	int ret = (This->*ogBattleOnProcess)();

	battleProcess();
	return ret;
}*/

int __fastcall CBattleManager_OnProcess(SokuLib::BattleManager *This)
{
	saveCards(*This);

	int ret = (This->*ogBattleManagerOnProcess)();

	battleProcess(*This);
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
	return true;
}

extern "C" int APIENTRY DllMain(HMODULE hModule, DWORD fdwReason, LPVOID lpReserved) {
	return TRUE;
}