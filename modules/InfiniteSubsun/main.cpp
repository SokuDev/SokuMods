//
// Created by Gegel85 on 31/10/2020
//

#include <SokuLib.hpp>
#include <shlwapi.h>
#include <thread>

static int (SokuLib::Select::*ogSelectOnProcess)();
static int (SokuLib::SelectClient::*ogSelectCLOnProcess)();
static int (SokuLib::SelectServer::*ogSelectSVOnProcess)();
static int (SokuLib::BattleManager::*ogBattleMgrOnProcess)();
static void (SokuLib::BattleManager::*ogBattleMgrOnRender)();
static void (__stdcall *s_origLoadDeckData)(char *, void *, SokuLib::DeckInfo &, int, SokuLib::Dequeue<short> &);
static SokuLib::CharacterManager *obj[0xC] = {nullptr};
static bool spawned = false;
static bool init = false;

int __fastcall CBattleManager_OnProcess(SokuLib::BattleManager *This)
{
	if (This->matchState == -1) {
		puts("Premature process");
		return (This->*ogBattleMgrOnProcess)();
	}
	if (!init) {
		puts("Init okuus");
		obj[0xA]->objectBase.action = SokuLib::ACTION_USING_SC_ID_212;
		obj[0xA]->objectBase.animate();
		obj[0xA]->objectBase.opponent = &This->rightCharacterManager.objectBase;
		obj[0xB]->objectBase.action = SokuLib::ACTION_USING_SC_ID_212;
		obj[0xB]->objectBase.animate();
		obj[0xB]->objectBase.opponent = &This->leftCharacterManager.objectBase;
		init = true;
	}

	puts("Mgr process");
	int ret = (This->*ogBattleMgrOnProcess)();
	puts("Calc d1");
	float d1 = (This->leftCharacterManager.objectBase.position.x - 640.f);
	puts("Calc d2");
	float d2 = (This->rightCharacterManager.objectBase.position.x - 640.f);
	puts("Calc v1");
	float v1 = log(std::abs(d1)) / 2;
	puts("Calc v2");
	float v2 = log(std::abs(d2)) / 2;

	printf("%f %f / %f %f\n", d1, v1, d2, v2);
	if (std::abs(d1) > std::abs(v1) && d1 != 0)
		This->leftCharacterManager.objectBase.position.x -= std::copysign(v1, d1);
	else
		This->leftCharacterManager.objectBase.position.x = 640;
	if (std::abs(d2) > std::abs(v2) && d2 != 0)
		This->rightCharacterManager.objectBase.position.x -= std::copysign(v2, d2);
	else
		This->rightCharacterManager.objectBase.position.x = 640;
	puts("OBJ1 process");
	(*(int (__thiscall **)(SokuLib::CharacterManager *))(*(int *)&obj[0xA]->objectBase.vtable + 0x28))(obj[0xA]);
	puts("OBJ1 objs process");
	(obj[0xA]->objects.*SokuLib::union_cast<void (SokuLib::ObjListManager::*)()>(0x633ce0))();
	puts("OBJ2 process");
	(*(int (__thiscall **)(SokuLib::CharacterManager *))(*(int *)&obj[0xB]->objectBase.vtable + 0x28))(obj[0xB]);
	puts("OBJ2 objs process");
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

	auto left  = ((SokuLib::CharacterManager **)This)[3];
	auto right = ((SokuLib::CharacterManager **)This)[4];

	if (This->matchState == 2) {
		((SokuLib::CharacterManager **)This)[3] = obj[0xA];
		puts("OBJ1 objs cols");
		reinterpret_cast<void (__thiscall *)(SokuLib::BattleManager *)>(0x47d0d0)(This);
		((SokuLib::CharacterManager **)This)[3] = left;
		((SokuLib::CharacterManager **)This)[4] = obj[0xB];
		puts("OBJ2 objs cols");
		reinterpret_cast<void (__thiscall *)(SokuLib::BattleManager *)>(0x47d0d0)(This);
		((SokuLib::CharacterManager **)This)[4] = right;
	}
	puts("Done!");
	return ret;
}

void selectCommon()
{
	if (spawned) {
		spawned = false;
		init = false;
		(*(void (__thiscall **)(SokuLib::CharacterManager *, char))obj[0xA]->objectBase.vtable)(obj[0xA], 0);
		SokuLib::Delete(obj[0xA]);
		(*(void (__thiscall **)(SokuLib::CharacterManager *, char))obj[0xB]->objectBase.vtable)(obj[0xB], 0);
		SokuLib::Delete(obj[0xB]);
	}
}

int __fastcall CSelect_OnProcess(SokuLib::Select *This)
{
	int ret = (This->*ogSelectOnProcess)();

	selectCommon();
	return ret;
}

int __fastcall CSelectSV_OnProcess(SokuLib::SelectServer *This)
{
	int ret = (This->*ogSelectSVOnProcess)();

	selectCommon();
	return ret;
}

int __fastcall CSelectCL_OnProcess(SokuLib::SelectClient *This)
{
	int ret = (This->*ogSelectCLOnProcess)();

	selectCommon();
	return ret;
}

void __fastcall CBattleManager_OnRender(SokuLib::BattleManager *This)
{
	puts("Render mgr");
	(This->*ogBattleMgrOnRender)();
	if (init) {
		//TODO: Add these in SokuLib
		puts("Render p1");
		(obj[0xA]->*SokuLib::union_cast<void (SokuLib::CharacterManager::*)()>(0x438d20))();
		puts("Render obj1");
		(obj[0xA]->objects.*SokuLib::union_cast<void (SokuLib::ObjListManager::*)(int)>(0x59be00))(1);

		puts("Render p2");
		(obj[0xB]->*SokuLib::union_cast<void (SokuLib::CharacterManager::*)()>(0x438d20))();
	}
	puts("Gone");
}

void __stdcall loadDeckData(char *charName, void *csvFile, SokuLib::DeckInfo &deck, int param4, SokuLib::Dequeue<short> &newDeck)
{
	if (!spawned) {
		spawned = true;
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
		init = false;
	}
	s_origLoadDeckData(charName, csvFile, deck, param4, newDeck);
}

extern "C" __declspec(dllexport) bool CheckVersion(const BYTE hash[16]) {
	return true;
}

#define PAYLOAD_ADDRESS_DECK_INFOS 0x437D24
#define PAYLOAD_NEXT_INSTR_DECK_INFOS (PAYLOAD_ADDRESS_DECK_INFOS + 4)

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
	ogSelectCLOnProcess = SokuLib::TamperDword(&SokuLib::VTable_SelectClient.onProcess, CSelectCL_OnProcess);
	ogSelectSVOnProcess = SokuLib::TamperDword(&SokuLib::VTable_SelectServer.onProcess, CSelectSV_OnProcess);
	// s_origCLogo_OnProcess   = TamperDword(vtbl_CLogo   + 4, (DWORD)CLogo_OnProcess);
	// s_origCBattle_OnProcess = TamperDword(vtbl_CBattle + 4, (DWORD)CBattle_OnProcess);
	// s_origCBattleSV_OnProcess = TamperDword(vtbl_CBattleSV + 4, (DWORD)CBattleSV_OnProcess);
	// s_origCBattleCL_OnProcess = TamperDword(vtbl_CBattleCL + 4, (DWORD)CBattleCL_OnProcess);
	// s_origCTitle_OnProcess  = TamperDword(vtbl_CTitle  + 4, (DWORD)CTitle_OnProcess);
	// s_origCSelect_OnProcess = TamperDword(vtbl_CSelect + 4, (DWORD)CSelect_OnProcess);
	::VirtualProtect((PVOID)RDATA_SECTION_OFFSET, RDATA_SECTION_SIZE, old, &old);
	::VirtualProtect((PVOID)TEXT_SECTION_OFFSET, TEXT_SECTION_SIZE, PAGE_EXECUTE_WRITECOPY, &old);
	int newOffset = reinterpret_cast<int>(loadDeckData) - PAYLOAD_NEXT_INSTR_DECK_INFOS;
	s_origLoadDeckData = SokuLib::union_cast<void (__stdcall *)(char *, void *, SokuLib::DeckInfo &, int, SokuLib::Dequeue<short> &)>(*(int *)PAYLOAD_ADDRESS_DECK_INFOS + PAYLOAD_NEXT_INSTR_DECK_INFOS);
	*(int *)PAYLOAD_ADDRESS_DECK_INFOS = newOffset;
	::VirtualProtect((PVOID)TEXT_SECTION_OFFSET, TEXT_SECTION_SIZE, old, &old);

	::FlushInstructionCache(GetCurrentProcess(), nullptr, 0);
	return true;
}

extern "C" int APIENTRY DllMain(HMODULE hModule, DWORD fdwReason, LPVOID lpReserved) {
	return TRUE;
}