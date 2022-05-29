//
// Created by Gegel85 on 31/10/2020
//

#include <SokuLib.hpp>
#include <shlwapi.h>

static int (SokuLib::Select::*ogSelectOnProcess)();
static int (SokuLib::BattleManager::*ogBattleMgrOnProcess)();
static void (SokuLib::BattleManager::*ogBattleMgrOnRender)();
static SokuLib::CharacterManager *obj[0xC] = {nullptr};
static bool spawned = false;
static bool init = false;
static std::pair<SokuLib::CharacterManager *, int> leftBench;
static std::pair<SokuLib::CharacterManager *, int> rightBench;

int __fastcall CBattleManager_OnProcess(SokuLib::BattleManager *This)
{
	if (!spawned) {
		SokuLib::PlayerInfo p;

		puts("Not spawned. Loading both okuus");
		memset(&p, 0, sizeof(p));
		p.character = SokuLib::CHARACTER_UTSUHO;
		p.palette = 0;
		p.isRight = false;
		puts("Magic 1-1");
		((void (__thiscall *)(SokuLib::CharacterManager **, bool, SokuLib::PlayerInfo &))0x46da40)(obj, false, p);
		puts("Magic 1-2");
		(*(void (__thiscall **)(SokuLib::CharacterManager *))(*(int *)obj[0xA] + 0x44))(obj[0xA]);
		puts("Magic 1-3");
		p.isRight = true;
		puts("Magic 2-1");
		((void (__thiscall *)(SokuLib::CharacterManager **, bool, SokuLib::PlayerInfo &))0x46da40)(obj, true, p);
		puts("Magic 2-2");
		(*(void (__thiscall **)(SokuLib::CharacterManager *))(*(int *)obj[0xB] + 0x44))(obj[0xB]);
		puts("Magic 2-3");
		spawned = true;
	}
	if (This->matchState == -1)
		return (This->*ogBattleMgrOnProcess)();
	if (!init) {
		init = true;
		puts("Init okuus");

		printf("%p %p %p %p\n", ((SokuLib::CharacterManager **)This)[3], ((SokuLib::CharacterManager **)This)[4], &This->leftCharacterManager, &This->rightCharacterManager);
		leftBench.first  = ((SokuLib::CharacterManager **)This)[3];
		leftBench.second = SokuLib::leftChar;
		rightBench.first = ((SokuLib::CharacterManager **)This)[4];
		rightBench.second= SokuLib::rightChar;

		obj[0xA]->objectBase.opponent = &obj[0xB]->objectBase;
		obj[0xB]->objectBase.opponent = &obj[0xA]->objectBase;
		obj[0xA]->keyManager = ((SokuLib::CharacterManager **)This)[3]->keyManager;
		obj[0xB]->keyManager = ((SokuLib::CharacterManager **)This)[4]->keyManager;
		((SokuLib::CharacterManager **)This)[3]->keyManager = nullptr;
		((SokuLib::CharacterManager **)This)[4]->keyManager = nullptr;
		((SokuLib::CharacterManager **)This)[3] = obj[0xA];
		((SokuLib::CharacterManager **)This)[4] = obj[0xB];
		SokuLib::leftChar  = SokuLib::CHARACTER_UTSUHO;
		SokuLib::rightChar = SokuLib::CHARACTER_UTSUHO;
	}

	int ret = (This->*ogBattleMgrOnProcess)();

	//(*(int (__thiscall **)(SokuLib::CharacterManager *))(*(int *)&obj[0xA]->objectBase.vtable + 0x28))(obj[0xA]);
	//(obj[0xA]->objects.*SokuLib::union_cast<void (SokuLib::ObjListManager::*)()>(0x633ce0))();
	//(*(int (__thiscall **)(SokuLib::CharacterManager *))(*(int *)&obj[0xB]->objectBase.vtable + 0x28))(obj[0xB]);
	//(obj[0xB]->objects.*SokuLib::union_cast<void (SokuLib::ObjListManager::*)()>(0x633ce0))();

	//auto left  = ((SokuLib::CharacterManager **)This)[3];
	//auto right = ((SokuLib::CharacterManager **)This)[4];

	//if (This->matchState == 2) {
	//	((SokuLib::CharacterManager **)This)[3] = obj[0xA];
	//	reinterpret_cast<void (__thiscall *)(SokuLib::BattleManager *)>(0x47d0d0)(This);
	//	((SokuLib::CharacterManager **)This)[3] = left;
	//	((SokuLib::CharacterManager **)This)[4] = obj[0xB];
	//	reinterpret_cast<void (__thiscall *)(SokuLib::BattleManager *)>(0x47d0d0)(This);
	//	((SokuLib::CharacterManager **)This)[4] = right;
	//}
	return ret;
}

int __fastcall CSelect_OnProcess(SokuLib::Select *This)
{
	int ret = (This->*ogSelectOnProcess)();

	return ret;
}

void __fastcall CBattleManager_OnRender(SokuLib::BattleManager *This)
{
	(This->*ogBattleMgrOnRender)();
	if (spawned) {
		//TODO: Add these in SokuLib
		//(obj[0xA]->objects.*SokuLib::union_cast<void (SokuLib::ObjListManager::*)(int)>(0x59be00))(-2);
		//(obj[0xA]->objects.*SokuLib::union_cast<void (SokuLib::ObjListManager::*)(int)>(0x59be00))(-1);
		//(obj[0xA]->*SokuLib::union_cast<void (SokuLib::CharacterManager::*)()>(0x438d20))();
		//(obj[0xA]->objects.*SokuLib::union_cast<void (SokuLib::ObjListManager::*)(int)>(0x59be00))(1);
		//(obj[0xA]->objects.*SokuLib::union_cast<void (SokuLib::ObjListManager::*)(int)>(0x59be00))(2);

		//(obj[0xB]->objects.*SokuLib::union_cast<void (SokuLib::ObjListManager::*)(int)>(0x59be00))(-2);
		//(obj[0xB]->objects.*SokuLib::union_cast<void (SokuLib::ObjListManager::*)(int)>(0x59be00))(-1);
		//(obj[0xB]->*SokuLib::union_cast<void (SokuLib::CharacterManager::*)()>(0x438d20))();
		//(obj[0xB]->objects.*SokuLib::union_cast<void (SokuLib::ObjListManager::*)(int)>(0x59be00))(1);
		//(obj[0xB]->objects.*SokuLib::union_cast<void (SokuLib::ObjListManager::*)(int)>(0x59be00))(2);
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

	puts("Hello");
	// DWORD old;
	::VirtualProtect((PVOID)RDATA_SECTION_OFFSET, RDATA_SECTION_SIZE, PAGE_EXECUTE_WRITECOPY, &old);
	ogBattleMgrOnRender  = SokuLib::TamperDword(&SokuLib::VTable_BattleManager.onRender,  CBattleManager_OnRender);
	ogBattleMgrOnProcess = SokuLib::TamperDword(&SokuLib::VTable_BattleManager.onProcess, CBattleManager_OnProcess);
	ogSelectOnProcess = SokuLib::TamperDword(&SokuLib::VTable_Select.onProcess, CSelect_OnProcess);
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