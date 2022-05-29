//
// Created by Gegel85 on 31/10/2020
//

#include <SokuLib.hpp>
#include <shlwapi.h>
#include <thread>

#define BOXES_ALPHA 0.25

static SokuLib::DrawUtils::RectangleShape rectangle;
static int (SokuLib::Select::*ogSelectOnProcess)();
static int (SokuLib::SelectClient::*ogSelectCLOnProcess)();
static int (SokuLib::SelectServer::*ogSelectSVOnProcess)();
static int (SokuLib::BattleManager::*ogBattleMgrOnProcess)();
static void (SokuLib::BattleManager::*ogBattleMgrOnRender)();
static void (__stdcall *s_origLoadDeckData)(char *, void *, SokuLib::DeckInfo &, int, SokuLib::Dequeue<short> &);
static SokuLib::CharacterManager *obj[0xC] = {nullptr};
static SokuLib::DrawUtils::Sprite sprites[35];
static SokuLib::DrawUtils::Sprite gagesEffects[3];
static bool spawned = false;
static bool init = false;
static HMODULE myModule;

#define ASSIST_BOX_Y 428
#define LEFT_ASSIST_BOX_X 57
#define RIGHT_ASSIST_BOX_X 475

struct ChrInfo {
	unsigned nb = 0;
	unsigned cd = 0;
	unsigned maxCd = 0;
	SokuLib::Action action;
	SokuLib::Character chr;
	bool (*cond)(SokuLib::CharacterManager *mgr, ChrInfo &This) = nullptr;
};

static std::pair<ChrInfo, ChrInfo> chr;

bool condBasic(SokuLib::CharacterManager *mgr, ChrInfo &This)
{
	return mgr->objectBase.action != This.action || mgr->objectBase.offset_0x18C[4];
}

bool waitEnd(SokuLib::CharacterManager *mgr, ChrInfo &This)
{
	return mgr->objectBase.action != This.action;
}

static void drawBox(const SokuLib::Box &box, const SokuLib::RotationBox *rotation, SokuLib::Color borderColor, SokuLib::Color fillColor)
{
	if (!rotation) {
		SokuLib::DrawUtils::FloatRect rect{};

		rect.x1 = SokuLib::camera.scale * (SokuLib::camera.translate.x + box.left);
		rect.x2 = SokuLib::camera.scale * (SokuLib::camera.translate.x + box.right);
		rect.y1 = SokuLib::camera.scale * (SokuLib::camera.translate.y + box.top);
		rect.y2 = SokuLib::camera.scale * (SokuLib::camera.translate.y + box.bottom);
		rectangle.setRect(rect);
	} else {
		SokuLib::DrawUtils::Rect<SokuLib::Vector2f> rect;

		rect.x1.x = SokuLib::camera.scale * (SokuLib::camera.translate.x + box.left);
		rect.x1.y = SokuLib::camera.scale * (SokuLib::camera.translate.y + box.top);

		rect.y1.x = SokuLib::camera.scale * (SokuLib::camera.translate.x + box.left + rotation->pt1.x);
		rect.y1.y = SokuLib::camera.scale * (SokuLib::camera.translate.y + box.top + rotation->pt1.y);

		rect.x2.x = SokuLib::camera.scale * (SokuLib::camera.translate.x + box.left + rotation->pt1.x + rotation->pt2.x);
		rect.x2.y = SokuLib::camera.scale * (SokuLib::camera.translate.y + box.top + rotation->pt1.y + rotation->pt2.y);

		rect.y2.x = SokuLib::camera.scale * (SokuLib::camera.translate.x + box.left + rotation->pt2.x);
		rect.y2.y = SokuLib::camera.scale * (SokuLib::camera.translate.y + box.top + rotation->pt2.y);
		rectangle.rawSetRect(rect);
	}

	rectangle.setFillColor(fillColor);
	rectangle.setBorderColor(borderColor);
	rectangle.draw();
}

static void drawCollisionBox(const SokuLib::ObjectManager &manager)
{
	SokuLib::DrawUtils::FloatRect rect{};
	const SokuLib::Box &box = *manager.frameData->collisionBox;

	if (!manager.frameData->collisionBox)
		return;

	rect.x1 = SokuLib::camera.scale * (std::ceil(manager.position.x) + manager.direction * box.left + SokuLib::camera.translate.x);
	rect.x2 = SokuLib::camera.scale * (std::ceil(manager.position.x) + manager.direction * box.right + SokuLib::camera.translate.x);
	rect.y1 = SokuLib::camera.scale * (box.top - std::ceil(manager.position.y) + SokuLib::camera.translate.y);
	rect.y2 = SokuLib::camera.scale * (box.bottom - std::ceil(manager.position.y) + SokuLib::camera.translate.y);

	rectangle.setRect(rect);
	rectangle.setFillColor(SokuLib::Color::Yellow * BOXES_ALPHA);
	rectangle.setBorderColor(SokuLib::Color::Yellow);
	rectangle.draw();
}

static void drawPositionBox(const SokuLib::ObjectManager &manager)
{
	rectangle.setPosition({
		static_cast<int>(SokuLib::camera.scale * (manager.position.x - 2 + SokuLib::camera.translate.x)),
		static_cast<int>(SokuLib::camera.scale * (-manager.position.y - 2 + SokuLib::camera.translate.y))
	});
	rectangle.setSize({
		static_cast<unsigned int>(SokuLib::camera.scale * 5),
		static_cast<unsigned int>(SokuLib::camera.scale * 5)
	});
	rectangle.setFillColor(SokuLib::Color::White);
	rectangle.setBorderColor(SokuLib::Color::White + SokuLib::Color::Black);
	rectangle.draw();
}

static void drawHurtBoxes(const SokuLib::ObjectManager &manager)
{
	if (manager.hurtBoxCount > 5)
		return;

	for (int i = 0; i < manager.hurtBoxCount; i++)
		drawBox(
			manager.hurtBoxes[i],
			manager.hurtBoxesRotation[i],
			SokuLib::Color::Green,
			(manager.frameData->frameFlags.chOnHit ? SokuLib::Color::Cyan : SokuLib::Color::Green) * BOXES_ALPHA
		);
}

static void drawHitBoxes(const SokuLib::ObjectManager &manager)
{
	if (manager.hitBoxCount > 5)
		return;

	for (int i = 0; i < manager.hitBoxCount; i++)
		drawBox(manager.hitBoxes[i], manager.hitBoxesRotation[i], SokuLib::Color::Red, SokuLib::Color::Red * BOXES_ALPHA);
}

static void drawPlayerBoxes(const SokuLib::CharacterManager &manager, bool playerBoxes = true)
{
	if (playerBoxes) {
		drawCollisionBox(manager.objectBase);
		drawHurtBoxes(manager.objectBase);
		drawHitBoxes(manager.objectBase);
		drawPositionBox(manager.objectBase);
	}

	auto array = manager.objects.list.vector();

	for (const auto _elem : array) {
		auto elem = reinterpret_cast<const SokuLib::ProjectileManager *>(_elem);

		if ((elem->isActive && elem->objectBase.hitCount > 0) || elem->objectBase.frameData->attackFlags.value > 0) {
			drawHurtBoxes(elem->objectBase);
			drawHitBoxes(elem->objectBase);
			drawPositionBox(elem->objectBase);
		}
	}
}

void updateObject(SokuLib::CharacterManager *mgr, ChrInfo &chr)
{
	if (chr.cd)
		chr.cd--;
	if (mgr->objectBase.renderInfos.yRotation == 90)
		return;
	if (chr.nb != 0 && mgr->objectBase.renderInfos.yRotation != 0) {
		mgr->objectBase.renderInfos.yRotation -= 10;
		return;
	}
	if (chr.cond(mgr, chr)) {
		if (chr.nb == 0) {
			mgr->objectBase.renderInfos.yRotation += 10;
			return;
		}
		chr.nb--;
		mgr->objectBase.action = chr.action;
		mgr->objectBase.hitCount = 1;
		mgr->objectBase.offset_0x18C[4] = 0;
		mgr->objectBase.animate();
	}
	if (mgr->objectBase.hitstop)
		mgr->objectBase.hitstop--;
	(*(int (__thiscall **)(SokuLib::CharacterManager *))(*(int *)&mgr->objectBase.vtable + 0x28))(mgr);
	(mgr->objects.*SokuLib::union_cast<void (SokuLib::ObjListManager::*)()>(0x633ce0))();
	mgr->objectBase.position += mgr->objectBase.speed;
}

void remiliaAttacks(SokuLib::CharacterManager *main, SokuLib::CharacterManager *obj, ChrInfo &chr)
{
	if (main->keyCombination._22a) {
		chr.cd = 90;
		chr.nb = 4;
		chr.maxCd = 90;
		chr.action = SokuLib::ACTION_4A;
		chr.cond = condBasic;
		obj->objectBase.direction = main->objectBase.direction;
		obj->objectBase.speed = {0, 0};
		obj->objectBase.position.x = main->objectBase.position.x;
		obj->objectBase.position.y = 0;
		obj->objectBase.renderInfos.yRotation -= 10;
		return;
	}
	if (main->keyCombination._236a) {
		chr.cd = 360;
		chr.nb = 1;
		chr.maxCd = 360;
		chr.action = SokuLib::ACTION_ALT2_SKILL3_C;
		chr.cond = waitEnd;
		obj->objectBase.direction = main->objectBase.direction;
		obj->objectBase.speed = {0, 0};
		obj->objectBase.position.x = main->objectBase.position.x;
		obj->objectBase.position.y = 0;
		obj->objectBase.renderInfos.yRotation -= 10;
		return;
	}
}

void assisterAttacks(SokuLib::CharacterManager *main, SokuLib::CharacterManager *obj, ChrInfo &chr)
{
	if (chr.cd || obj->objectBase.renderInfos.yRotation != 90)
		return;
	remiliaAttacks(main, obj, chr);
}

int __fastcall CBattleManager_OnProcess(SokuLib::BattleManager *This)
{
	if (This->matchState == -1)
		return (This->*ogBattleMgrOnProcess)();
	if (!init) {
		puts("Init assisters");
		chr.first = ChrInfo();
		chr.first.chr = SokuLib::CHARACTER_REMILIA;
		chr.second = ChrInfo();
		chr.second.chr = SokuLib::CHARACTER_REMILIA;
		obj[0xA]->objectBase.owner = &This->leftCharacterManager;
		obj[0xA]->objectBase.owner2 = &This->leftCharacterManager;
		obj[0xB]->objectBase.owner = &This->rightCharacterManager;
		obj[0xB]->objectBase.owner2 = &This->rightCharacterManager;
		obj[0xA]->objectBase.opponent = &This->rightCharacterManager;
		obj[0xB]->objectBase.opponent = &This->leftCharacterManager;
		obj[0xA]->objectBase.renderInfos.yRotation = 90;
		obj[0xB]->objectBase.renderInfos.yRotation = 90;
		init = true;
	}

	int ret = (This->*ogBattleMgrOnProcess)();

	if (SokuLib::menuManager.isInMenu)
		return ret;

	updateObject(obj[0xA], chr.first);
	updateObject(obj[0xB], chr.second);

	auto left  = ((SokuLib::CharacterManager **)This)[3];
	auto right = ((SokuLib::CharacterManager **)This)[4];

	assisterAttacks(left,  obj[0xA], chr.first);
	assisterAttacks(right, obj[0xB], chr.second);

	if (This->matchState == 2) {
		((SokuLib::CharacterManager **)This)[3] = obj[0xA];
		if (chr.first.nb || obj[0xA]->objectBase.renderInfos.yRotation == 0)
			((SokuLib::CharacterManager **)This)[4]->objectBase.opponent = obj[0xA];
		reinterpret_cast<void (__thiscall *)(SokuLib::BattleManager *)>(0x47d0d0)(This);
		((SokuLib::CharacterManager **)This)[3] = left;
		((SokuLib::CharacterManager **)This)[4]->objectBase.opponent = left;

		((SokuLib::CharacterManager **)This)[4] = obj[0xB];
		if (chr.second.nb || obj[0xB]->objectBase.renderInfos.yRotation == 0)
			((SokuLib::CharacterManager **)This)[3]->objectBase.opponent = obj[0xB];
		reinterpret_cast<void (__thiscall *)(SokuLib::BattleManager *)>(0x47d0d0)(This);
		((SokuLib::CharacterManager **)This)[4] = right;
		((SokuLib::CharacterManager **)This)[3]->objectBase.opponent = right;
	}
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

void displayAssistGage(ChrInfo &chr, int x, bool mirror)
{
	auto &sprite = sprites[chr.chr];

	sprite.setPosition({x, static_cast<int>(ASSIST_BOX_Y - sprite.getSize().y)});
	sprite.setMirroring(mirror, false);
	sprite.draw();
}

void __fastcall CBattleManager_OnRender(SokuLib::BattleManager *This)
{
	(This->*ogBattleMgrOnRender)();
	if (init) {
		//TODO: Add these in SokuLib
		(obj[0xA]->objects.*SokuLib::union_cast<void (SokuLib::ObjListManager::*)(int)>(0x59be00))(-2);
		(obj[0xB]->objects.*SokuLib::union_cast<void (SokuLib::ObjListManager::*)(int)>(0x59be00))(-2);
		(obj[0xA]->objects.*SokuLib::union_cast<void (SokuLib::ObjListManager::*)(int)>(0x59be00))(-1);
		(obj[0xB]->objects.*SokuLib::union_cast<void (SokuLib::ObjListManager::*)(int)>(0x59be00))(-1);
		(obj[0xA]->*SokuLib::union_cast<void (SokuLib::CharacterManager::*)()>(0x438d20))();
		(obj[0xB]->*SokuLib::union_cast<void (SokuLib::CharacterManager::*)()>(0x438d20))();
		(obj[0xA]->objects.*SokuLib::union_cast<void (SokuLib::ObjListManager::*)(int)>(0x59be00))(1);
		(obj[0xB]->objects.*SokuLib::union_cast<void (SokuLib::ObjListManager::*)(int)>(0x59be00))(1);
		(obj[0xA]->objects.*SokuLib::union_cast<void (SokuLib::ObjListManager::*)(int)>(0x59be00))(2);
		(obj[0xB]->objects.*SokuLib::union_cast<void (SokuLib::ObjListManager::*)(int)>(0x59be00))(2);
		drawPlayerBoxes(This->leftCharacterManager);
		drawPlayerBoxes(This->rightCharacterManager);
		drawPlayerBoxes(*obj[0xA], chr.first.nb != 0 || obj[0xA]->objectBase.renderInfos.yRotation == 0);
		drawPlayerBoxes(*obj[0xB], chr.second.nb != 0 || obj[0xB]->objectBase.renderInfos.yRotation == 0);
		displayAssistGage(chr.first,  LEFT_ASSIST_BOX_X, false);
		displayAssistGage(chr.second, RIGHT_ASSIST_BOX_X, true);
	}
}

void __stdcall loadDeckData(char *charName, void *csvFile, SokuLib::DeckInfo &deck, int param4, SokuLib::Dequeue<short> &newDeck)
{
	if (!spawned) {
		spawned = true;
		SokuLib::PlayerInfo p;

		puts("Not spawned. Loading both assisters");
		memset(&p, 0, sizeof(p));
		p.character = SokuLib::CHARACTER_REMILIA;
		p.palette = 0;
		p.isRight = false;
		puts("Loading character 1");
		((void (__thiscall *)(SokuLib::CharacterManager **, bool, SokuLib::PlayerInfo &))0x46da40)(obj, false, p);
		(*(void (__thiscall **)(SokuLib::CharacterManager *))(*(int *)obj[0xA] + 0x44))(obj[0xA]);
		p.isRight = true;
		puts("Loading character 2");
		((void (__thiscall *)(SokuLib::CharacterManager **, bool, SokuLib::PlayerInfo &))0x46da40)(obj, true, p);
		(*(void (__thiscall **)(SokuLib::CharacterManager *))(*(int *)obj[0xB] + 0x44))(obj[0xB]);
		init = false;
		printf("%p %p\n", obj[0xA], obj[0xB]);
		for (int i = 0; i < sizeof(sprites) / sizeof(*sprites); i++) {
			sprites[i].texture.loadFromResource(myModule, MAKEINTRESOURCE((i + 1) * 4));
			sprites[i].setSize(sprites[i].texture.getSize());
			sprites[i].rect.width = sprites[i].getSize().x;
			sprites[i].rect.height = sprites[i].getSize().y;
		}
		gagesEffects[0].texture.loadFromGame("data/character/sanae/gageBa000.cv0");
		gagesEffects[0].setSize(gagesEffects[0].texture.getSize());
		gagesEffects[0].rect.width = gagesEffects[0].getSize().x;
		gagesEffects[0].rect.height = gagesEffects[0].getSize().y;

		gagesEffects[1].texture.loadFromGame("data/character/sanae/gageBb000.cv0");
		gagesEffects[1].setSize(gagesEffects[1].texture.getSize());
		gagesEffects[1].rect.width = gagesEffects[1].getSize().x;
		gagesEffects[1].rect.height = gagesEffects[1].getSize().y;

		gagesEffects[2].texture.loadFromGame("data/character/sanae/gageCa000.cv0");
		gagesEffects[2].setSize(gagesEffects[2].texture.getSize());
		gagesEffects[2].rect.width = gagesEffects[2].getSize().x;
		gagesEffects[2].rect.height = gagesEffects[2].getSize().y;
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

	myModule = hMyModule;
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