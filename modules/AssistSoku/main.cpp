//
// Created by Gegel85 on 31/10/2020
//

#include <fstream>
#include <sstream>
#include <optional>
#include <dinput.h>
#include <nlohmann/json.hpp>
#include <SokuLib.hpp>
#include <shlwapi.h>
#include <thread>
#include <iostream>

#define BOXES_ALPHA 0.25
#define ASSIST_BOX_Y 429
#define LEFT_ASSIST_BOX_X 57
#define RIGHT_ASSIST_BOX_X 475
#define LEFT_BAR SokuLib::Vector2i{82, 422}
#define RIGHT_BAR SokuLib::Vector2i{479, 422}
#define LEFT_CROSS SokuLib::Vector2i{108, 417}
#define RIGHT_CROSS SokuLib::Vector2i{505, 417}

struct Coord { float x, y, z; };
struct SpriteEx {
	void *vtable;
	int dxHandle = 0;
	SokuLib::DxVertex vertices[4];
	SokuLib::Vector2f size;
	Coord baseCoords[4];
	Coord transfCoords[4];
	SokuLib::Vector2f size2;

	void setTexture(int texture, int texOffsetX, int texOffsetY, int width, int height, int anchorX, int anchorY);
	void setTexture(int texture, int texOffsetX, int texOffsetY, int width, int height);
	void clearTransform();
	void render();
	void render(float r, float g, float b);
	void render(float a, float r, float g, float b);
};
void SpriteEx::setTexture(int texture, int texOffsetX, int texOffsetY, int width, int height, int anchorX, int anchorY) {
	(this->*SokuLib::union_cast<void(SpriteEx::*)(int, int, int, int, int, int, int)>(0x406c60))(texture, texOffsetX, texOffsetY, width, height, anchorX, anchorY);
}
void SpriteEx::setTexture(int texture, int texOffsetX, int texOffsetY, int width, int height) {
	(this->*SokuLib::union_cast<void(SpriteEx::*)(int, int, int, int, int)>(0x41f7f0))(texture, texOffsetX, texOffsetY, width, height);
}
void SpriteEx::clearTransform() { (this->*SokuLib::union_cast<void(SpriteEx::*)()>(0x406ea0))(); }
void SpriteEx::render() { (this->*SokuLib::union_cast<void(SpriteEx::*)()>(0x4075d0))(); }
void SpriteEx::render(float r, float g, float b) { (this->*SokuLib::union_cast<void(SpriteEx::*)(float, float, float)>(0x7fb080))(r, g, b); }
void SpriteEx::render(float a, float r, float g, float b) { (this->*SokuLib::union_cast<void(SpriteEx::*)(float, float, float, float)>(0x7fb150))(a, r, g, b); }

struct ResetValue {
	unsigned int offset;
	unsigned char value;
	unsigned int size;
};

struct ChrInfo {
	unsigned nb = 0;
	unsigned cd = 0;
	unsigned maxCd = 0;
	unsigned ctr = 0;
	SokuLib::Action action;
	SokuLib::Character chr;
	std::vector<ResetValue> resetValues;
	SokuLib::Vector2<std::optional<int>> pos;
	SokuLib::Vector2<std::optional<int>> speed;
	SokuLib::Vector2<std::optional<int>> offset;
	SokuLib::Vector2<std::optional<int>> gravity;
	bool (*cond)(SokuLib::CharacterManager *mgr, ChrInfo &This) = nullptr;
};

struct ChrData {
	std::map<std::string, std::pair<std::optional<ChrInfo>, std::optional<ChrInfo>>> elems;
	SokuLib::Vector2i size;
	SpriteEx sprite;
};


struct SWRCMDINFO {
	bool enabled;
	int prev; // number representing the previously pressed buttons (masks are applied)
	int now; // number representing the current pressed buttons (masks are applied)

	struct {
		bool enabled;
		int id[10];
		int base; // once len reaches 10 (first cycle), is incremented modulo 10
		int len; // starts at 0, caps at 10
	} record;
};

struct RivControl {
	bool enabled;
	int texID;
	int forwardCount;
	int forwardStep;
	int forwardIndex;
	SWRCMDINFO cmdp1;
	SWRCMDINFO cmdp2;
	bool hitboxes;
	bool untech;
	bool show_debug;
	bool paused;
};

static int (__stdcall *realRecvFrom)(SOCKET s, char * buf, int len, int flags, sockaddr * from, int * fromlen);
static int (__stdcall *realSendTo)(SOCKET s, char * buf, int len, int flags, sockaddr * to, int tolen);
static char modFolder[1024];
static char soku2Dir[MAX_PATH];
static SokuLib::DrawUtils::RectangleShape rectangle;
static int (SokuLib::Select::*ogSelectOnProcess)();
static int (SokuLib::SelectClient::*ogSelectCLOnProcess)();
static int (SokuLib::SelectServer::*ogSelectSVOnProcess)();
static int (SokuLib::BattleManager::*ogBattleMgrOnProcess)();
static void (SokuLib::BattleManager::*ogBattleMgrOnRender)();
static void (__stdcall *s_origLoadDeckData)(char *, void *, SokuLib::DeckInfo &, int, SokuLib::Dequeue<short> &);
static SokuLib::CharacterManager *obj[0xC] = {nullptr};
static SokuLib::DrawUtils::Sprite gagesEffects[3];
static bool spawned = false;
static bool init = false;
static bool disp = false;
static HMODULE myModule;
static std::pair<ChrInfo, ChrInfo> chr;
static std::vector<ChrData> data{22};
static std::pair<SokuLib::Character, SokuLib::Character> assists;

static const std::pair<size_t, size_t> objectOffsets[] = {
	{0x0EC, 0x150}
};
static const std::pair<size_t, size_t> characterObjectOffsets[] = {
	{0x0184, 0x03A0}
};
static const std::pair<size_t, size_t> characterOffsets[] = {
	{0x0184, 0x0358},
	{0x0488, 0x057C},
	{0x05C4, 0x05E8},
	{0x06A4, 0x06F8},
	{0x0710, 0x0714},
	{0x0720, 0x072C},
	{0x0740, 0x0750},
	{0x07D0, sizeof(SokuLib::CharacterManager)},
};

bool condBasic(SokuLib::CharacterManager *mgr, ChrInfo &This)
{
	return mgr->objectBase.action != This.action || mgr->objectBase.offset_0x18C[4];
}

bool waitEnd(SokuLib::CharacterManager *mgr, ChrInfo &This)
{
	return mgr->objectBase.action != This.action;
}

int __stdcall mySendTo(SOCKET s, char * buf, int len, int flags, sockaddr * to, int tolen)
{
	auto &packet = *reinterpret_cast<SokuLib::Packet *>(buf);

	if (packet.type != SokuLib::HOST_GAME && packet.type != SokuLib::CLIENT_GAME && packet.game.event.type != SokuLib::GAME_MATCH_REQUEST && packet.game.event.type != SokuLib::GAME_MATCH)
		return realSendTo(s, buf, len, flags, to, tolen);

	if (packet.game.event.type == SokuLib::GAME_MATCH) {
		if (SokuLib::mainMode == SokuLib::BATTLE_MODE_VSSERVER)
			packet.game.event.match.host.skinId = assists.second << 3;
		else
			packet.game.event.match.host.skinId |= assists.first << 3;
		printf("Send assists %i (%i) | %i %i\n", packet.game.event.match.host.skinId >> 3, packet.game.event.match.host.skinId, assists.first, assists.second);
	}
	return realSendTo(s, buf, len, flags, to, tolen);
}

int __stdcall myRecvFrom(SOCKET s, char * buf, int len, int flags, sockaddr * from, int * fromlen)
{
	int result = realRecvFrom(s, buf, len, flags, from, fromlen);
	auto &packet = *reinterpret_cast<SokuLib::Packet *>(buf);

	if (packet.type != SokuLib::HOST_GAME && packet.type != SokuLib::CLIENT_GAME && packet.game.event.type != SokuLib::GAME_MATCH_REQUEST && packet.game.event.type != SokuLib::GAME_MATCH)
		return result;

	if (packet.game.event.type == SokuLib::GAME_MATCH) {
		if (SokuLib::mainMode == SokuLib::BATTLE_MODE_VSSERVER)
			assists.first = static_cast<SokuLib::Character>(packet.game.event.match.host.skinId >> 3);
		else
			assists.second = static_cast<SokuLib::Character>(packet.game.event.match.host.skinId >> 3);
		printf("Recv assists %i %i (%i)\n", assists.first, assists.second, packet.game.event.match.host.skinId);
		packet.game.event.match.host.skinId &= 7;
	}
	return result;
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

void updateObject(SokuLib::CharacterManager *main, SokuLib::CharacterManager *mgr, ChrInfo &chr)
{
	if (mgr->objectBase.renderInfos.yRotation == 90) {
		if (SokuLib::mainMode == SokuLib::BATTLE_MODE_PRACTICE)
			chr.cd = 0;
		chr.cd -= !!chr.cd;
		if (mgr->objectBase.action < SokuLib::ACTION_STAND_GROUND_HIT_SMALL_HITSTUN)
			mgr->objectBase.position = main->objectBase.position;
		goto update;
	}
	if (chr.nb != 0 && mgr->objectBase.renderInfos.yRotation != 0) {
		mgr->objectBase.renderInfos.yRotation -= 10;
		goto update;
	}
	if (mgr->objectBase.action >= SokuLib::ACTION_STAND_GROUND_HIT_SMALL_HITSTUN && mgr->objectBase.action <= SokuLib::ACTION_NEUTRAL_TECH) {
		chr.nb = 0;
		mgr->objectBase.renderInfos.yRotation += 10;
		chr.cd *= 2;
		chr.maxCd *= 2;
		goto update;
	}
	if (chr.cond(mgr, chr)) {
		if (chr.nb == 0) {
			mgr->objectBase.renderInfos.yRotation += 10;
			goto update;
		}
		chr.nb--;
		mgr->objectBase.action = chr.action;
		mgr->objectBase.hitCount = 1;
		mgr->objectBase.offset_0x18C[4] = 0;
		mgr->objectBase.animate();
	}
update:
	if (mgr->objectBase.hitstop)
		mgr->objectBase.hitstop--;

	if (chr.gravity.x) {
		mgr->objectBase.gravity.x = *chr.gravity.x;
		if (chr.speed.x)
			mgr->objectBase.speed.x = *chr.speed.x;
	}
	if (chr.gravity.y) {
		mgr->objectBase.gravity.y = *chr.gravity.y;
		if (chr.speed.y)
			mgr->objectBase.speed.y = *chr.speed.y;
	}
	(*(int (__thiscall **)(SokuLib::CharacterManager *))(*(int *)&mgr->objectBase.vtable + 0x28))(mgr);
	for (auto o : mgr->objects.list.vector())
		if (o)
			o->owner = o->owner2 = mgr->objectBase.owner;
	if (!mgr->objectBase.hitstop)
		mgr->objectBase.position += SokuLib::Vector2f{mgr->objectBase.speed.x * mgr->objectBase.direction, mgr->objectBase.speed.y};
	//if (mgr->objectBase.position.y <= 0) {
	//	mgr->objectBase.gravity = 0;
	//	mgr->objectBase.speed.y = 0;
	//	mgr->objectBase.position.y = 0;
	//}
	//if (mgr->objectBase.position.y >= 1240)
	//	mgr->objectBase.position.y = 1240;
	if (mgr->objectBase.position.x <= 40 && chr.chr == SokuLib::CHARACTER_REMILIA && chr.action == 560)
		mgr->objectBase.position.x = 40;
	if (mgr->objectBase.position.x >= 1240 && chr.chr == SokuLib::CHARACTER_REMILIA && chr.action == 560)
		mgr->objectBase.position.x = 1240;
	//mgr->objectBase.speed.y -= mgr->objectBase.gravity;
	(mgr->objects.*SokuLib::union_cast<void (SokuLib::ObjListManager::*)()>(0x633ce0))();
}

bool initAttack(SokuLib::CharacterManager *main, SokuLib::CharacterManager *obj, ChrInfo &chr, std::pair<std::optional<ChrInfo>, std::optional<ChrInfo>> &data)
{
	auto &atk = main->objectBase.position.y == 0 ? data.first : data.second;

	if (atk) {
		chr = *atk;
		obj->objectBase.direction = main->objectBase.direction;
		obj->objectBase.speed = main->objectBase.speed;
		if (chr.speed.x)
			obj->objectBase.speed.x = *chr.speed.x;
		if (chr.speed.y)
			obj->objectBase.speed.y = *chr.speed.y;
		obj->objectBase.position = main->objectBase.position;
		if (chr.offset.x)
			obj->objectBase.position.x += *chr.offset.x * obj->objectBase.direction;
		if (chr.offset.y)
			obj->objectBase.position.y += *chr.offset.y;
		if (chr.pos.x)
			obj->objectBase.position.x = *chr.pos.x;
		if (chr.pos.y)
			obj->objectBase.position.y = *chr.pos.y;
		for (auto &r : chr.resetValues)
			memset(&((char *)obj)[r.offset], r.value, r.size);
		obj->objectBase.renderInfos.yRotation -= 10;
		obj->objectBase.action = SokuLib::ACTION_IDLE;
		obj->objectBase.animate();
	}
	return atk.operator bool();
}

void assisterAttacks(SokuLib::CharacterManager *main, SokuLib::CharacterManager *obj, ChrInfo &chr, ChrData &data)
{
	if (chr.cd || obj->objectBase.renderInfos.yRotation != 90)
		return;
	if ((main->keyCombination._6314a || main->keyCombination._6314d) && data.elems.find("624") != data.elems.end() && initAttack(main, obj, chr, data.elems["624"]))
		return;
	if ((main->keyCombination._4136a || main->keyCombination._4136d) && data.elems.find("426") != data.elems.end() && initAttack(main, obj, chr, data.elems["426"]))
		return;
	if ((main->keyCombination._623a  || main->keyCombination._623d)  && data.elems.find("623") != data.elems.end() && initAttack(main, obj, chr, data.elems["623"]))
		return;
	if ((main->keyCombination._421a  || main->keyCombination._421d)  && data.elems.find("421") != data.elems.end() && initAttack(main, obj, chr, data.elems["421"]))
		return;
	if ((main->keyCombination._236a  || main->keyCombination._236d)  && data.elems.find("236") != data.elems.end() && initAttack(main, obj, chr, data.elems["236"]))
		return;
	if ((main->keyCombination._214a  || main->keyCombination._214d)  && data.elems.find("214") != data.elems.end() && initAttack(main, obj, chr, data.elems["214"]))
		return;
	if ((main->keyCombination._22a   || main->keyCombination._22d)   && data.elems.find("22")  != data.elems.end() && initAttack(main, obj, chr, data.elems["22"]))
		return;
}

int __fastcall CBattleManager_OnProcess(SokuLib::BattleManager *This)
{
	if (This->matchState == -1)
		return (This->*ogBattleMgrOnProcess)();
	if (SokuLib::checkKeyOneshot(DIK_F4, false, false, false))
		disp = !disp;
	if (!init) {
		puts("Init assisters");
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

	if (!SokuLib::menuManager.empty() && SokuLib::sceneId == SokuLib::SCENE_BATTLE)
		return ret;

	auto left  = ((SokuLib::CharacterManager **)This)[3];
	auto right = ((SokuLib::CharacterManager **)This)[4];

	updateObject(left,  obj[0xA], chr.first);
	updateObject(right, obj[0xB], chr.second);
	assisterAttacks(left,  obj[0xA], chr.first,  data[chr.first.chr]);
	assisterAttacks(right, obj[0xB], chr.second, data[chr.second.chr]);

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
	auto &scene = SokuLib::currentScene->to<SokuLib::Select>();

	if (spawned) {
		spawned = false;
		init = false;
		(*(void (__thiscall **)(SokuLib::CharacterManager *, char))obj[0xA]->objectBase.vtable)(obj[0xA], 0);
		SokuLib::Delete(obj[0xA]);
		(*(void (__thiscall **)(SokuLib::CharacterManager *, char))obj[0xB]->objectBase.vtable)(obj[0xB], 0);
		SokuLib::Delete(obj[0xB]);
	}

	if (scene.leftSelect.keys && scene.leftSelect.keys != scene.rightSelect.keys && SokuLib::checkKeyOneshot(DIK_F8, false, false, false) && SokuLib::mainMode != SokuLib::BATTLE_MODE_VSSERVER) {
		printf("Changing assist for P1 to %i\n", SokuLib::leftChar);
		assists.first = SokuLib::leftChar;
		SokuLib::playSEWaveBuffer(0x28);
	}
	if (scene.rightSelect.keys && SokuLib::checkKeyOneshot(DIK_F8, false, false, false) && SokuLib::mainMode != SokuLib::BATTLE_MODE_VSCLIENT) {
		printf("Changing assist for P2 to %i\n", SokuLib::rightChar);
		assists.second = SokuLib::rightChar;
		SokuLib::playSEWaveBuffer(0x28);
	}

	//if (scene.leftSelectionStage != 0 && SokuLib::leftChar == assists.first) {
	//	assists.first = static_cast<SokuLib::Character>(assists.first + 1);
	//	assists.first = static_cast<SokuLib::Character>(assists.first + (assists.first == SokuLib::CHARACTER_RANDOM));
	//	assists.first = static_cast<SokuLib::Character>(assists.first + (assists.first == SokuLib::CHARACTER_NAMAZU));
	//	assists.first = static_cast<SokuLib::Character>(assists.first % data.size());
	//}
	//if (scene.rightSelectionStage != 0 && SokuLib::rightChar == assists.second) {
	//	assists.second = static_cast<SokuLib::Character>(assists.second + 1);
	//	assists.second = static_cast<SokuLib::Character>(assists.second + (assists.second == SokuLib::CHARACTER_RANDOM));
	//	assists.second = static_cast<SokuLib::Character>(assists.second + (assists.second == SokuLib::CHARACTER_NAMAZU));
	//	assists.second = static_cast<SokuLib::Character>(assists.second % data.size());
	//}
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

void displayAssistGage(ChrInfo &chr, int x, SokuLib::Vector2i bar, SokuLib::Vector2i cross, bool mirror)
{
	auto &sprite = data[chr.chr];
	auto &s = sprite.sprite;

	s.clearTransform();
	for (auto &c : s.transfCoords) {
		c.x += x;
		c.y += ASSIST_BOX_Y - sprite.size.y;
	}
	if (mirror) {
		auto tmp1 = s.transfCoords[0];
		auto tmp2 = s.transfCoords[2];

		s.transfCoords[0] = s.transfCoords[1];
		s.transfCoords[1] = tmp1;
		s.transfCoords[2] = s.transfCoords[3];
		s.transfCoords[3] = tmp2;
	}
	if (chr.cd) {
		(s.*SokuLib::union_cast<void(SpriteEx::*)(float, float, float)>(0x7fb200))(0.3f,0.587f,0.114f);
		if (chr.maxCd != chr.cd) {
			if (mirror)
				bar.x += 78 - 79 * (chr.maxCd - chr.cd) / chr.maxCd;
			gagesEffects[1].setPosition(bar);
			gagesEffects[1].setSize({79 * (chr.maxCd - chr.cd) / chr.maxCd, 4});
			gagesEffects[1].draw();
		}
		if (chr.ctr / 30 % 2 == 0) {
			gagesEffects[2].setPosition(cross);
			gagesEffects[2].draw();
		}
		chr.ctr++;
	} else {
		s.render();
		gagesEffects[0].setPosition(bar);
		gagesEffects[0].draw();
	}
}

void __fastcall CBattleManager_OnRender(SokuLib::BattleManager *This)
{
	bool hasRIV = LoadLibraryA("ReplayInputView+") != nullptr;
	bool show = hasRIV ? ((RivControl *)((char *)This + sizeof(*This)))->hitboxes : disp;

	(This->*ogBattleMgrOnRender)();
	if (init) {
		//TODO: Add these in SokuLib
		(obj[0xA]->objects.*SokuLib::union_cast<void (SokuLib::ObjListManager::*)(int)>(0x59be00))(-2);
		(obj[0xB]->objects.*SokuLib::union_cast<void (SokuLib::ObjListManager::*)(int)>(0x59be00))(-2);
		(obj[0xA]->objects.*SokuLib::union_cast<void (SokuLib::ObjListManager::*)(int)>(0x59be00))(-1);
		(obj[0xB]->objects.*SokuLib::union_cast<void (SokuLib::ObjListManager::*)(int)>(0x59be00))(-1);
		//(This->leftCharacterManager.*SokuLib::union_cast<void (SokuLib::CharacterManager::*)()>(0x438d20))();
		//(This->rightCharacterManager.*SokuLib::union_cast<void (SokuLib::CharacterManager::*)()>(0x438d20))();
		(obj[0xA]->*SokuLib::union_cast<void (SokuLib::CharacterManager::*)()>(0x438d20))();
		(obj[0xB]->*SokuLib::union_cast<void (SokuLib::CharacterManager::*)()>(0x438d20))();
		displayAssistGage(chr.first,  LEFT_ASSIST_BOX_X,  LEFT_BAR,  LEFT_CROSS,  false);
		displayAssistGage(chr.second, RIGHT_ASSIST_BOX_X, RIGHT_BAR, RIGHT_CROSS, true);
		//(This->leftCharacterManager.objects.*SokuLib::union_cast<void (SokuLib::ObjListManager::*)(int)>(0x59be00))(1);
		//(This->rightCharacterManager.objects.*SokuLib::union_cast<void (SokuLib::ObjListManager::*)(int)>(0x59be00))(1);
		(obj[0xA]->objects.*SokuLib::union_cast<void (SokuLib::ObjListManager::*)(int)>(0x59be00))(1);
		(obj[0xB]->objects.*SokuLib::union_cast<void (SokuLib::ObjListManager::*)(int)>(0x59be00))(1);
		//(This->leftCharacterManager.objects.*SokuLib::union_cast<void (SokuLib::ObjListManager::*)(int)>(0x59be00))(2);
		//(This->rightCharacterManager.objects.*SokuLib::union_cast<void (SokuLib::ObjListManager::*)(int)>(0x59be00))(2);
		(obj[0xA]->objects.*SokuLib::union_cast<void (SokuLib::ObjListManager::*)(int)>(0x59be00))(2);
		(obj[0xB]->objects.*SokuLib::union_cast<void (SokuLib::ObjListManager::*)(int)>(0x59be00))(2);

		float old = This->leftCharacterManager.objectBase.position.y;

		This->leftCharacterManager.objectBase.position.y = 15000;
		(This->leftCharacterManager.*SokuLib::union_cast<void (SokuLib::CharacterManager::*)()>(0x438d20))();
		This->leftCharacterManager.objectBase.position.y = old;
		if (show && SokuLib::mainMode != SokuLib::BATTLE_MODE_VSSERVER && SokuLib::mainMode != SokuLib::BATTLE_MODE_VSCLIENT) {
			if (!hasRIV) {
				drawPlayerBoxes(This->leftCharacterManager);
				drawPlayerBoxes(This->rightCharacterManager);
			}
			drawPlayerBoxes(*obj[0xA], chr.first.nb != 0 || obj[0xA]->objectBase.renderInfos.yRotation == 0);
			drawPlayerBoxes(*obj[0xB], chr.second.nb != 0 || obj[0xB]->objectBase.renderInfos.yRotation == 0);
		}
	}
}

void __stdcall loadDeckData(char *charName, void *csvFile, SokuLib::DeckInfo &deck, int param4, SokuLib::Dequeue<short> &newDeck)
{
	if (!spawned) {
		spawned = true;
		SokuLib::PlayerInfo p;
		const char *c[] = {
			"22",
			"236",
			"214",
			"623",
			"421",
			"624",
			"426"
		};

		puts("Not spawned. Loading both assisters");
		memset(&p, 0, sizeof(p));
		chr.first = ChrInfo();
		chr.first.chr = assists.first;
		p.character = assists.first;
		p.palette = SokuLib::leftPlayerInfo.palette;
		p.palette += SokuLib::leftChar == assists.first;
		p.palette += p.palette == SokuLib::rightPlayerInfo.palette && SokuLib::rightChar == assists.first;
		p.isRight = false;
		puts("Loading character 1");
		((void (__thiscall *)(SokuLib::CharacterManager **, bool, SokuLib::PlayerInfo &))0x46da40)(obj, false, p);
		(*(void (__thiscall **)(SokuLib::CharacterManager *))(*(int *)obj[0xA] + 0x44))(obj[0xA]);

		int oldPal = p.palette;

		chr.second = ChrInfo();
		chr.second.chr = assists.second;
		p.character = assists.second;
		p.palette = SokuLib::rightPlayerInfo.palette;
		p.palette += SokuLib::rightChar == assists.second;
		p.palette += p.palette == SokuLib::leftPlayerInfo.palette && SokuLib::leftChar == assists.second;
		p.palette += p.palette == oldPal && assists.second == assists.first;
		p.isRight = true;
		puts("Loading character 2");
		((void (__thiscall *)(SokuLib::CharacterManager **, bool, SokuLib::PlayerInfo &))0x46da40)(obj, true, p);
		(*(void (__thiscall **)(SokuLib::CharacterManager *))(*(int *)obj[0xB] + 0x44))(obj[0xB]);

		init = false;
		printf("%p %p\n", obj[0xA], obj[0xB]);
		for (unsigned i = 0; i < data.size(); i++) {
			std::string basePath = i > SokuLib::CHARACTER_NAMAZU ? (std::string(soku2Dir) + "/config/tag/") : (std::string(modFolder) + "/assets/");
			const char *chrName = (i == SokuLib::CHARACTER_RANDOM || i == SokuLib::CHARACTER_NAMAZU) ? "Empty" : reinterpret_cast<char *(*)(int)>(0x43f3f0)(i);
			SokuLib::DrawUtils::Texture texture;
			nlohmann::json j;
			std::string path = basePath + chrName + ".json";
			std::ifstream stream;

			if (texture.loadFromFile((basePath + "gage" + chrName + ".png").c_str())) {
				data[i].sprite.setTexture(texture.releaseHandle(), 0, 0, texture.getSize().x, texture.getSize().y);
				data[i].size = texture.getSize().to<int>();
			}
			data[i].elems.clear();
			stream.open(path);
			printf("Loading %s\n", path.c_str());
			if (stream.fail()) {
				printf("%s: %s\n", path.c_str(), strerror(errno));
				continue;
			}
			try {
				stream >> j;
				for (auto &a : j.items()) {
					bool b = false;

					for (auto s : c)
						if (strcmp(s, a.key().c_str()) == 0) {
							b = true;
							break;
						}
					if (!b) {
						printf("Ignored %s\n", a.key().c_str());
						continue;
					}

					auto &elem = data[i].elems[a.key()];
					auto &val = a.value();

					if (val.contains("ground") && val["ground"].is_object()) {
						ChrInfo e;
						auto &gr = val["ground"];

						e.cd = gr["cd"];
						e.ctr = 0;
						e.maxCd = e.cd;
						e.nb = gr["nb"];
						e.action = gr["action"];
						e.chr = static_cast<SokuLib::Character>(i);
						for (auto &v : gr["reset"]) {
							e.resetValues.push_back({
								v["offset"].is_number() ? v["offset"].get<unsigned>() : std::stoul(v["offset"].get<std::string>(), nullptr, 16),
								static_cast<unsigned char>(v.contains("value") && v["value"].is_number() ? v["value"].get<unsigned>() : 0),
								v.contains("size") && v["size"].is_number() ? v["size"].get<unsigned>() : 1
							});
						}
						e.cond = gr["cond"] == "end" ? waitEnd : condBasic;
						if (gr.contains("posX") && gr["posX"].is_number())
							e.pos.x = gr["posX"];
						if (gr.contains("posY") && gr["posY"].is_number())
							e.pos.y = gr["posY"];
						if (gr.contains("speedX") && gr["speedX"].is_number())
							e.speed.x = gr["speedX"];
						if (gr.contains("speedY") && gr["speedY"].is_number())
							e.speed.y = gr["speedY"];
						if (gr.contains("gravityX") && gr["gravityX"].is_number())
							e.gravity.x = gr["gravityX"];
						if (gr.contains("gravityY") && gr["gravityY"].is_number())
							e.gravity.y = gr["gravityY"];
						if (gr.contains("offsetX") && gr["offsetX"].is_number())
							e.offset.x = gr["offsetX"];
						if (gr.contains("offsetY") && gr["offsetY"].is_number())
							e.offset.y = gr["offsetY"];
						elem.first = e;
					}
					if (val.contains("air") && val["air"].is_object()) {
						ChrInfo e;
						auto &air = val["air"];

						e.cd = air["cd"];
						e.ctr = 0;
						e.maxCd = e.cd;
						e.nb = air["nb"];
						e.action = air["action"];
						e.chr = static_cast<SokuLib::Character>(i);
						for (auto &v : air["reset"]) {
							e.resetValues.push_back({
								v["offset"].is_number() ? v["offset"].get<unsigned>() : std::stoul(v["offset"].get<std::string>(), nullptr, 16),
								static_cast<unsigned char>(v.contains("value") && v["value"].is_number() ? v["value"].get<unsigned>() : 0),
								v.contains("size") && v["size"].is_number() ? v["size"].get<unsigned>() : 1
							});
						}
						e.cond = air["cond"] == "end" ? waitEnd : condBasic;
						if (air.contains("posX") && air["posX"].is_number())
							e.pos.x = air["posX"];
						if (air.contains("posY") && air["posY"].is_number())
							e.pos.y = air["posY"];
						if (air.contains("speedX") && air["speedX"].is_number())
							e.speed.x = air["speedX"];
						if (air.contains("speedY") && air["speedY"].is_number())
							e.speed.y = air["speedY"];
						if (air.contains("gravityX") && air["gravityX"].is_number())
							e.gravity.x = air["gravityX"];
						if (air.contains("gravityY") && air["gravityY"].is_number())
							e.gravity.y = air["gravityY"];
						if (air.contains("offsetX") && air["offsetX"].is_number())
							e.offset.x = air["offsetX"];
						if (air.contains("offsetY") && air["offsetY"].is_number())
							e.offset.y = air["offsetY"];
						elem.second = e;
					}
				}
			} catch (std::exception &e) {
				puts(e.what());
				MessageBox(SokuLib::window, e.what(), "Loading error", MB_ICONERROR);
			}
			puts("Loading completed");
		}
		gagesEffects[0].texture.loadFromGame("data/character/sanae/gageBa000.cv0");
		gagesEffects[0].setSize(gagesEffects[0].texture.getSize());
		gagesEffects[0].rect.width = gagesEffects[0].texture.getSize().x;
		gagesEffects[0].rect.height = gagesEffects[0].texture.getSize().y;

		gagesEffects[1].texture.loadFromGame("data/character/sanae/gageBb000.cv0");
		gagesEffects[1].rect.width = gagesEffects[1].texture.getSize().x;
		gagesEffects[1].rect.height = gagesEffects[1].texture.getSize().y;

		gagesEffects[2].texture.loadFromGame("data/character/sanae/gageCa000.cv0");
		gagesEffects[2].setSize(gagesEffects[2].texture.getSize());
		gagesEffects[2].rect.width = gagesEffects[2].texture.getSize().x;
		gagesEffects[2].rect.height = gagesEffects[2].texture.getSize().y;
	}
	s_origLoadDeckData(charName, csvFile, deck, param4, newDeck);
}

void loadSoku2CSV(LPWSTR path)
{
	std::ifstream stream{path};
	std::string line;

	if (stream.fail()) {
		printf("%S: %s\n", path, strerror(errno));
		return;
	}
	while (std::getline(stream, line)) {
		std::stringstream str{line};
		unsigned id;
		std::string idStr;
		std::string codeName;
		std::string shortName;
		std::string fullName;
		std::string skillInputs;

		std::getline(str, idStr, ';');
		std::getline(str, codeName, ';');
		std::getline(str, shortName, ';');
		std::getline(str, fullName, ';');
		std::getline(str, skillInputs, ';');
		if (str.fail()) {
			printf("Skipping line %s: Stream failed\n", line.c_str());
			continue;
		}
		try {
			id = std::stoi(idStr);
		} catch (...) {
			printf("Skipping line %s: Invalid id\n", line.c_str());
			continue;
		}
		while (id > data.size())
			data.emplace_back();
	}
}

void loadSoku2Config()
{
	puts("Looking for Soku2 config...");

	int argc;
	wchar_t app_path[MAX_PATH];
	wchar_t setting_path[MAX_PATH];
	wchar_t **arg_list = CommandLineToArgvW(GetCommandLineW(), &argc);

	wcsncpy(app_path, arg_list[0], MAX_PATH);
	PathRemoveFileSpecW(app_path);
	if (GetEnvironmentVariableW(L"SWRSTOYS", setting_path, sizeof(setting_path)) <= 0) {
		if (arg_list && argc > 1 && StrStrIW(arg_list[1], L"ini")) {
			wcscpy(setting_path, arg_list[1]);
			LocalFree(arg_list);
		} else {
			wcscpy(setting_path, app_path);
			PathAppendW(setting_path, L"\\SWRSToys.ini");
		}
		if (arg_list) {
			LocalFree(arg_list);
		}
	}
	printf("Config file is %S\n", setting_path);

	wchar_t moduleKeys[1024];
	wchar_t moduleValue[MAX_PATH];
	GetPrivateProfileStringW(L"Module", nullptr, nullptr, moduleKeys, sizeof(moduleKeys), setting_path);
	for (wchar_t *key = moduleKeys; *key; key += wcslen(key) + 1) {
		wchar_t module_path[MAX_PATH];

		GetPrivateProfileStringW(L"Module", key, nullptr, moduleValue, sizeof(moduleValue), setting_path);

		wchar_t *filename = wcsrchr(moduleValue, '/');

		printf("Check %S\n", moduleValue);
		if (!filename)
			filename = app_path;
		else
			filename++;
		for (int i = 0; filename[i]; i++)
			filename[i] = tolower(filename[i]);
		if (wcscmp(filename, L"soku2.dll") != 0)
			continue;

		wcscpy(module_path, app_path);
		PathAppendW(module_path, moduleValue);
		while (auto result = wcschr(module_path, '/'))
			*result = '\\';
		PathRemoveFileSpecW(module_path);
		printf("Found Soku2 module folder at %S\n", module_path);
		memset(soku2Dir, 0, sizeof(soku2Dir));
		for (int i = 0; module_path[i]; i++)
			soku2Dir[i] = module_path[i];
		PathAppendW(module_path, L"\\config\\info\\characters.csv");
		loadSoku2CSV(module_path);
		return;
	}
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
	GetModuleFileName(hMyModule, modFolder, 1024);
	PathRemoveFileSpec(modFolder);
	loadSoku2Config();
	puts("Hello");
	// DWORD old;
	::VirtualProtect((PVOID)RDATA_SECTION_OFFSET, RDATA_SECTION_SIZE, PAGE_EXECUTE_WRITECOPY, &old);
	ogBattleMgrOnRender  = SokuLib::TamperDword(&SokuLib::VTable_BattleManager.onRender,  CBattleManager_OnRender);
	ogBattleMgrOnProcess = SokuLib::TamperDword(&SokuLib::VTable_BattleManager.onProcess, CBattleManager_OnProcess);
	ogSelectOnProcess = SokuLib::TamperDword(&SokuLib::VTable_Select.onProcess, CSelect_OnProcess);
	ogSelectCLOnProcess = SokuLib::TamperDword(&SokuLib::VTable_SelectClient.onProcess, CSelectCL_OnProcess);
	ogSelectSVOnProcess = SokuLib::TamperDword(&SokuLib::VTable_SelectServer.onProcess, CSelectSV_OnProcess);
	realRecvFrom = SokuLib::TamperDword(&SokuLib::DLL::ws2_32.recvfrom, myRecvFrom);
	realSendTo = SokuLib::TamperDword(&SokuLib::DLL::ws2_32.sendto, mySendTo);
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