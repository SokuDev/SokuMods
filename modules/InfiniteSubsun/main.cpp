//
// Created by Gegel85 on 31/10/2020
//

#include <SokuLib.hpp>
#include <shlwapi.h>

static int (SokuLib::Select::*ogSelectOnProcess)();
static int (SokuLib::Battle::*ogBattleOnProcess)();
static int (SokuLib::Battle::*ogBattleOnRender)();
static SokuLib::CharacterManager *obj[0xC] = {nullptr};
static bool spawned = false;
static std::pair<bool, bool> pushed = {false, false};

int __fastcall CBattle_OnProcess(SokuLib::Battle *This)
{
	if (!spawned) {
		SokuLib::PlayerInfo p;

		puts("Not spawned. Loading both okuus");
		memset(&p, 0, sizeof(p));
		pushed = {false, false};
		p.character = SokuLib::CHARACTER_UTSUHO;
		p.palette = 0;
		p.isRight = false;
		puts("Magic 1-1");
		((void (__thiscall *)(SokuLib::CharacterManager **, bool, SokuLib::PlayerInfo &))0x46da40)(obj, false, p);
		puts("Magic 1-2");
		(*(void (__thiscall **)(SokuLib::CharacterManager *))(*(int *)obj[0xA] + 0x44))(obj[0xA]);
		puts("Magic 1-3");
		obj[0xA]->objectBase.opponent = &SokuLib::getBattleMgr().rightCharacterManager.objectBase;
		p.isRight = true;
		puts("Magic 2-1");
		((void (__thiscall *)(SokuLib::CharacterManager **, bool, SokuLib::PlayerInfo &))0x46da40)(obj, true, p);
		puts("Magic 2-2");
		(*(void (__thiscall **)(SokuLib::CharacterManager *))(*(int *)obj[0xB] + 0x44))(obj[0xB]);
		puts("Magic 2-3");
		obj[0xB]->objectBase.opponent = &SokuLib::getBattleMgr().leftCharacterManager.objectBase;
		obj[0xA]->objectBase.action = SokuLib::ACTION_USING_SC_ID_212;
		obj[0xA]->objectBase.animate();
		obj[0xB]->objectBase.action = SokuLib::ACTION_USING_SC_ID_212;
		obj[0xB]->objectBase.animate();
		spawned = true;
	}

	puts("Update");
	int ret = (This->*ogBattleOnProcess)();

	puts("A");
	(*(int (__thiscall **)(SokuLib::CharacterManager *))(*(int *)&obj[0xA]->objectBase.vtable + 0x28))(obj[0xA]);
	puts("B");
	(obj[0xA]->objects.*SokuLib::union_cast<void (SokuLib::ObjListManager::*)()>(0x633ce0))();
	puts("C");
	(*(int (__thiscall **)(SokuLib::CharacterManager *))(*(int *)&obj[0xB]->objectBase.vtable + 0x28))(obj[0xB]);
	puts("D");
	(obj[0xB]->objects.*SokuLib::union_cast<void (SokuLib::ObjListManager::*)()>(0x633ce0))();

	if (obj[0xA]->objects.list.size >= 3) {
		if (obj[0xA]->objects.list.head->next->next->next->val->actionBlockId == 0) {
			obj[0xA]->objects.list.head->next->next->next->val->animationCounter = 7;
			obj[0xA]->objects.list.head->next->next->next->val->animationSubFrame = 0;
			obj[0xA]->objects.list.head->next->next->next->val->frameCount = 30;
		}
	}
	if (obj[0xB]->objects.list.size >= 3) {
		if (obj[0xB]->objects.list.head->next->next->next->val->actionBlockId == 0) {
			obj[0xB]->objects.list.head->next->next->next->val->animationCounter = 7;
			obj[0xB]->objects.list.head->next->next->next->val->animationSubFrame = 0;
			obj[0xB]->objects.list.head->next->next->next->val->frameCount = 30;
		}
	}
	if (obj[0xA]->objectBase.action == 0 && obj[0xA]->objectBase.renderInfos.color.a)
		obj[0xA]->objectBase.renderInfos.color.a -= 15;
	if (obj[0xB]->objectBase.action == 0 && obj[0xB]->objectBase.renderInfos.color.a)
		obj[0xB]->objectBase.renderInfos.color.a -= 15;
	puts("END");
	return ret;
}

int __fastcall CSelect_OnProcess(SokuLib::Select *This)
{
	int ret = (This->*ogSelectOnProcess)();

	return ret;
}

int __fastcall CBattle_OnRender(SokuLib::Battle *This)
{
	puts("Render start");
	auto ret = (This->*ogBattleOnRender)();

	if (spawned) {
		//TODO: Add these in SokuLib
		puts("Render 1 -2");
		(obj[0xA]->objects.*SokuLib::union_cast<void (SokuLib::ObjListManager::*)(int)>(0x59be00))(-2);
		puts("Render 1 -1");
		(obj[0xA]->objects.*SokuLib::union_cast<void (SokuLib::ObjListManager::*)(int)>(0x59be00))(-1);
		puts("Render 1");
		(obj[0xA]->*SokuLib::union_cast<void (SokuLib::CharacterManager::*)()>(0x438d20))();
		puts("Render 1 1");
		(obj[0xA]->objects.*SokuLib::union_cast<void (SokuLib::ObjListManager::*)(int)>(0x59be00))(1);
		puts("Render 1 2");
		(obj[0xA]->objects.*SokuLib::union_cast<void (SokuLib::ObjListManager::*)(int)>(0x59be00))(2);

		puts("Render 2 -2");
		(obj[0xB]->objects.*SokuLib::union_cast<void (SokuLib::ObjListManager::*)(int)>(0x59be00))(-2);
		puts("Render 2 -1");
		(obj[0xB]->objects.*SokuLib::union_cast<void (SokuLib::ObjListManager::*)(int)>(0x59be00))(-1);
		puts("Render 2");
		(obj[0xB]->*SokuLib::union_cast<void (SokuLib::CharacterManager::*)()>(0x438d20))();
		puts("Render 2 1");
		(obj[0xB]->objects.*SokuLib::union_cast<void (SokuLib::ObjListManager::*)(int)>(0x59be00))(1);
		puts("Render 2 2");
		(obj[0xB]->objects.*SokuLib::union_cast<void (SokuLib::ObjListManager::*)(int)>(0x59be00))(2);
	}
	puts("Render end");
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

	puts("Hello");
	// DWORD old;
	::VirtualProtect((PVOID)RDATA_SECTION_OFFSET, RDATA_SECTION_SIZE, PAGE_EXECUTE_WRITECOPY, &old);
	ogBattleOnRender  = SokuLib::TamperDword(&SokuLib::VTable_Battle.onRender,  CBattle_OnRender);
	ogBattleOnProcess = SokuLib::TamperDword(&SokuLib::VTable_Battle.onProcess, CBattle_OnProcess);
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