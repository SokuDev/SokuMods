//
// Created by PinkySmile on 12/08/2021.
//

#include <SokuLib.hpp>
#include <lua.hpp>
#include <sol/sol.hpp>
#include <map>
#include "FakeChrMgr.hpp"
#include "LuaBattleAnimation.hpp"

#ifndef _DEBUG
#define puts(...)
#define printf(...)
#define fprintf(...)
#endif

static DWORD old;
static double _loopStart, _loopEnd;
static void (__stdcall *og)(int);

static void __stdcall editLoop(int ptr) {
	//int samplePerSec = *reinterpret_cast<int*>(ptr+0x12fc);
	*reinterpret_cast<double*>(ptr+0x12e8) = _loopStart;
	*reinterpret_cast<double*>(ptr+0x12f0) = _loopEnd;
	SokuLib::TamperNearJmpOpr(0x418cc5, og);
	VirtualProtect((PVOID)0x418cc5, 5, old, &old);
}

struct FakeBattleManager {
	// 0x000
	char unknown[0xC];
	// 0x00C
	FakeCharacterManager *leftCharacterManager;
	// 0x010
	FakeCharacterManager *rightCharacterManager;
	// 0x014
	char offset_0x014[0x8F0];
	// 0x904
	char currentRound;
};

template<typename T>
inline void addVector(sol::state &lua, const char *name) {
	auto type = lua.new_usertype<SokuLib::Vector2<T>>(name);

	type["new"] = [](T x, T y){ return SokuLib::Vector2<T>{x, y}; };
	type["x"] = &SokuLib::Vector2<T>::x;
	type["y"] = &SokuLib::Vector2<T>::y;
	type["rotate"] = &SokuLib::Vector2<T>::rotate;
	type[sol::meta_function::addition] = [](const SokuLib::Vector2<T> &a, const SokuLib::Vector2<T> &b){ return a + b; };
	type[sol::meta_function::subtraction] = [](const SokuLib::Vector2<T> &a, const SokuLib::Vector2<T> &b){ return a - b; };
	type[sol::meta_function::multiplication] = [](const SokuLib::Vector2<T> &a, float b){ return a * b; };
	type[sol::meta_function::equal_to] = [](const SokuLib::Vector2<T> &a, const SokuLib::Vector2<T> &b){ return a == b; };
}

LuaBattleAnimation::LuaBattleAnimation(const char *packPath, const char *script)
{
	this->_lua.reset(new sol::state());
	this->_lua->open_libraries(
		sol::lib::base,
		sol::lib::package,
		sol::lib::coroutine,
		sol::lib::string,
		sol::lib::table,
		sol::lib::math,
		sol::lib::debug,
		sol::lib::utf8
	);

	this->_lua->new_usertype<SokuLib::DrawUtils::RectangleShape>(
		"RectangleShape", sol::constructors<void(), void(const SokuLib::Camera &)>(),
		"draw", &SokuLib::DrawUtils::RectangleShape::draw,
		"size", sol::property(&SokuLib::DrawUtils::RectangleShape::getSize, &SokuLib::DrawUtils::RectangleShape::setSize),
		"position", sol::property(&SokuLib::DrawUtils::RectangleShape::getPosition, &SokuLib::DrawUtils::RectangleShape::setPosition),
		"fillColor", sol::property(&SokuLib::DrawUtils::RectangleShape::getFillColor, &SokuLib::DrawUtils::RectangleShape::setFillColor),
		"borderColor", sol::property(&SokuLib::DrawUtils::RectangleShape::getBorderColor, &SokuLib::DrawUtils::RectangleShape::setBorderColor),
		"setRect", &SokuLib::DrawUtils::RectangleShape::setRect,
		"rawSetRect", &SokuLib::DrawUtils::RectangleShape::rawSetRect
	);
	this->_lua->new_usertype<SokuLib::DrawUtils::Sprite>(
		"Sprite", sol::constructors<void(), void(const SokuLib::Camera &)>(),
		"texture", &SokuLib::DrawUtils::Sprite::texture,
		"fillColors", &SokuLib::DrawUtils::Sprite::fillColors,
		"rect", &SokuLib::DrawUtils::Sprite::rect,
		"tint", &SokuLib::DrawUtils::Sprite::tint,
		"draw", &SokuLib::DrawUtils::Sprite::draw,
		"size", sol::property(&SokuLib::DrawUtils::Sprite::getSize, &SokuLib::DrawUtils::Sprite::setSize),
		"position", sol::property(&SokuLib::DrawUtils::Sprite::getPosition, &SokuLib::DrawUtils::Sprite::setPosition),
		"setRect", &SokuLib::DrawUtils::Sprite::setRect,
		"rawSetRect", &SokuLib::DrawUtils::Sprite::rawSetRect
	);
	this->_lua->new_usertype<SokuLib::DrawUtils::Texture>(
		"Texture", sol::constructors<void()>(),
		"hasTexture", &SokuLib::DrawUtils::Texture::hasTexture,
		"activate", &SokuLib::DrawUtils::Texture::activate,
		"size", sol::readonly_property(&SokuLib::DrawUtils::Texture::getSize),
		"setHandle", &SokuLib::DrawUtils::Texture::setHandle,
		"swap", &SokuLib::DrawUtils::Texture::swap,
		"destroy", &SokuLib::DrawUtils::Texture::destroy,
		"releaseHandle", &SokuLib::DrawUtils::Texture::releaseHandle,
		"loadFromFile", &SokuLib::DrawUtils::Texture::loadFromFile,
		"loadFromGame", &SokuLib::DrawUtils::Texture::loadFromGame,
		"loadFromResource", &SokuLib::DrawUtils::Texture::loadFromResource,
		"createFromText", &SokuLib::DrawUtils::Texture::createFromText
	);

	addVector<float>(*this->_lua, "Vector2f");
	addVector<int>(*this->_lua, "Vector2i");
	addVector<unsigned>(*this->_lua, "Vector2u");
	addVector<long>(*this->_lua, "Vector2l");
	addVector<unsigned long>(*this->_lua, "Vector2ul");
	addVector<long long>(*this->_lua, "Vector2ll");
	addVector<unsigned long long>(*this->_lua, "Vector2ull");
	addVector<double>(*this->_lua, "Vector2d");
	addVector<bool>(*this->_lua, "Vector2b");

	this->_lua->new_usertype<SokuStand>(
		"StandDialog", sol::constructors<void(std::vector<std::string>), void(std::vector<std::string>, SokuLib::Character, SokuLib::Character)>(),
		"render", &SokuStand::render,
		"update", &SokuStand::update,
		"onKeyPress", &SokuStand::onKeyPress,
		"hidden", sol::property(&SokuStand::isHidden, &SokuStand::setHidden),
		"getCurrentDialog", &SokuStand::getCurrentDialog,
		"isAnimationFinished", &SokuStand::isAnimationFinished,
		"finishAnimations", &SokuStand::finishAnimations,
		sol::meta_function::length, &SokuStand::getCurrentDialog
	);
	this->_lua->new_usertype<SokuLib::RenderInfo>(
		"RenderInfo",
		"new", [](){ return SokuLib::RenderInfo{}; },
		"color", &SokuLib::RenderInfo::color,
		"shaderType", &SokuLib::RenderInfo::shaderType,
		"shaderColor", &SokuLib::RenderInfo::shaderColor,
		"scale", &SokuLib::RenderInfo::scale,
		"xRotation", &SokuLib::RenderInfo::xRotation,
		"yRotation", &SokuLib::RenderInfo::yRotation,
		"zRotation", &SokuLib::RenderInfo::zRotation
	);
	this->_lua->new_usertype<SokuLib::DrawUtils::TextureRect>(
		"TextureRect",
		"new", [](int l, int r, int w, int h){ return SokuLib::DrawUtils::TextureRect{l, r, w, h}; },
		"left", &SokuLib::DrawUtils::TextureRect::left,
		"top", &SokuLib::DrawUtils::TextureRect::top,
		"width", &SokuLib::DrawUtils::TextureRect::width,
		"height", &SokuLib::DrawUtils::TextureRect::height
	);
	this->_lua->new_usertype<SokuLib::DrawUtils::DxSokuColor>(
		"Color",
		"new", [](unsigned char r, unsigned char g, unsigned char b, std::optional<unsigned char> a){
			return SokuLib::DrawUtils::DxSokuColor{r, g, b, static_cast<unsigned char>(a ? *a : 255)};
		},
		"r", &SokuLib::DrawUtils::DxSokuColor::r,
		"g", &SokuLib::DrawUtils::DxSokuColor::g,
		"b", &SokuLib::DrawUtils::DxSokuColor::b,
		"a", &SokuLib::DrawUtils::DxSokuColor::a
	);

	this->_lua->new_usertype<SokuLib::Camera>(
		"Camera",
		"backgroundTranslate", &SokuLib::Camera::backgroundTranslate,
		"translate", &SokuLib::Camera::translate,
		"scale", &SokuLib::Camera::scale
	);
	this->_lua->new_usertype<FakeBattleManager>(
		"BattleManager",
		"leftCharacterManager", &FakeBattleManager::leftCharacterManager,
		"rightCharacterManager", &FakeBattleManager::rightCharacterManager,
		"leftChr", &FakeBattleManager::leftCharacterManager,
		"rightChr", &FakeBattleManager::rightCharacterManager
	);
	this->_lua->new_usertype<SokuLib::ObjListManager>(
		"ObjListManager",
		sol::meta_function::pairs, [](SokuLib::ObjListManager &object) {
			std::vector<FakeCharacterManager *> objs;

			for (auto &obj : object.list.vector())
				objs.push_back(reinterpret_cast<FakeCharacterManager *>(obj));
			return objs;
		},
		sol::meta_function::length, [](SokuLib::ObjListManager &object) {
			return object.list.size;
		},
		sol::meta_function::index, [](SokuLib::ObjListManager &object, int index) {
			if (index < 1 || index > object.list.size)
				throw sol::error("Out of bound access to a ObjListManager");
			return object.list.vector()[index - 1];
		},
		"draw", SokuLib::union_cast<void (SokuLib::ObjListManager::*)(int)>(0x59be00),
		"update", SokuLib::union_cast<void (SokuLib::ObjListManager::*)()>(0x633ce0)
	);
	this->_lua->new_usertype<SokuLib::PlayerInfo>(
		"PlayerInfo",
		"new", [](SokuLib::Character chr, unsigned int palette, bool rightChr){
			SokuLib::PlayerInfo p;

			memset(&p, 0, sizeof(p));
			p.character = chr;
			p.palette = palette;
			p.isRight = rightChr;
			return p;
		},
		"character", &SokuLib::PlayerInfo::character,
		"isRight", &SokuLib::PlayerInfo::isRight,
		"palette", &SokuLib::PlayerInfo::palette,
		"deck", &SokuLib::PlayerInfo::deck,
		"effectiveDeck", &SokuLib::PlayerInfo::effectiveDeck//,
		//"keyManager", &SokuLib::PlayerInfo::keyManager
	);
	pushFakeChrMgrLuaTable(*this->_lua, this->_created);
	(*this->_lua)[sol::create_if_nil]["enums"]["directions"] = std::map<std::string, int>{
		{ "LEFT", SokuLib::LEFT },
		{ "RIGHT", SokuLib::RIGHT }
	};
	(*this->_lua)[sol::create_if_nil]["enums"]["sfxs"] = std::map<std::string, int>{
		{ "smallHit", 1 },
		{ "mediumHit", 2 },
		{ "bigHit", 3 },
		{ "hugeHit", 4 },
		{ "RemiJ2aHit", 5 },
		{ "sharkHit", 6 },
		{ "RemiForkHit", 7 },
		{ "RemiFlawHit", 8 },
		{ "bulletHit", 9 },
		{ "electricHit", 10 },
		{ "bulletHit2", 11 },
		{ "mishagujiSamaHit", 12 },
		{ "block", 20 },
		{ "wrongBlock", 21 },
		{ "knockdown", 22 },
		{ "weatherActivate", 23 },
		{ "strangeSound", 24 },
		{ "counterHit", 25 },
		{ "weatherCrystalCollect", 26 },
		{ "shortMelee", 27 },
		{ "mediumMelee", 28 },
		{ "longMelee", 29 },
		{ "land", 30 },
		{ "dash", 31 },
		{ "strangeSound2", 32 },
		{ "strangeSound3", 33 },
		{ "earthquake", 34 },
		{ "borderEscape", 35 },
		{ "cardDrawn", 36 },
		{ "graze", 37 },
		{ "orbBroken", 38 },
		{ "menuMove", 39 },
		{ "menuConfirm", 40 },
		{ "menuCancel", 41 },
		{ "highJump", 43 },
		{ "knockOut", 44 },
		{ "charging", 45 },
		{ "weatherCrystalCollect2", 46 },
		{ "brokenOrbRecover", 47 },
		{ "TH11_12pause", 48 },
		{ "pop", 49 },
		{ "weatherActivate2", 53 },
		{ "skillCardUse", 54 },
		{ "bombUse", 55 },
		{ "magicPotionUse", 56 },
		{ "netbell", 57 },
		{ "cardDiscarded", 58 },
		{ "titleScreenMove", 59 },
		{ "menuConfirm", 61 },
		{ "catfishFall", 70 },
		{ "catfishLand", 71 },
		{ "leftHandedFoldingFanCharge", 72 },
		{ "timeStopped", 73 },
		{ "bzzzzz", 74 },
		{ "quietRain", 100 },
		{ "loudRain", 101 },
		{ "wind", 102 },
		{ "wind2", 103 },
		{ "wind3", 104 },
		{ "storm", 105 }
	};
	(*this->_lua)[sol::create_if_nil]["enums"]["colors"] = std::map<std::string, SokuLib::DrawUtils::DxSokuColor>{
		{ "White", SokuLib::DrawUtils::DxSokuColor::White },
		{ "Yellow", SokuLib::DrawUtils::DxSokuColor::Yellow },
		{ "Red", SokuLib::DrawUtils::DxSokuColor::Red },
		{ "Blue", SokuLib::DrawUtils::DxSokuColor::Blue },
		{ "Green", SokuLib::DrawUtils::DxSokuColor::Green },
		{ "Magenta", SokuLib::DrawUtils::DxSokuColor::Magenta },
		{ "Cyan", SokuLib::DrawUtils::DxSokuColor::Cyan },
		{ "Black", SokuLib::DrawUtils::DxSokuColor::Black },
		{ "Transparent", SokuLib::DrawUtils::DxSokuColor::Transparent }
	};
	(*this->_lua)[sol::create_if_nil]["enums"]["characters"] = std::map<std::string, SokuLib::Character>{
		{ "reimu",      SokuLib::CHARACTER_REIMU },
		{ "marisa",     SokuLib::CHARACTER_MARISA },
		{ "sakuya",     SokuLib::CHARACTER_SAKUYA },
		{ "alice",      SokuLib::CHARACTER_ALICE },
		{ "patchouli",  SokuLib::CHARACTER_PATCHOULI },
		{ "youmu",      SokuLib::CHARACTER_YOUMU },
		{ "remilia",    SokuLib::CHARACTER_REMILIA },
		{ "yuyuko",     SokuLib::CHARACTER_YUYUKO },
		{ "yukari",     SokuLib::CHARACTER_YUKARI },
		{ "suika",      SokuLib::CHARACTER_SUIKA },
		{ "reisen",     SokuLib::CHARACTER_REISEN },
		{ "udonge",     SokuLib::CHARACTER_UDONGE },
		{ "aya",        SokuLib::CHARACTER_AYA },
		{ "komachi",    SokuLib::CHARACTER_KOMACHI },
		{ "iku",        SokuLib::CHARACTER_IKU },
		{ "tenshi",     SokuLib::CHARACTER_TENSHI },
		{ "sanae",      SokuLib::CHARACTER_SANAE },
		{ "cirno",      SokuLib::CHARACTER_CIRNO },
		{ "chirno",     SokuLib::CHARACTER_CHIRNO },
		{ "meiling",    SokuLib::CHARACTER_MEILING },
		{ "meirin",     SokuLib::CHARACTER_MEIRIN },
		{ "utsuho",     SokuLib::CHARACTER_UTSUHO },
		{ "suwako",     SokuLib::CHARACTER_SUWAKO },
		{ "random",     SokuLib::CHARACTER_RANDOM },
		{ "namazu",     SokuLib::CHARACTER_NAMAZU },
		{ "momiji",     SokuLib::CHARACTER_MOMIJI },
		{ "clownpiece", SokuLib::CHARACTER_CLOWNPIECE },
		{ "flandre",    SokuLib::CHARACTER_FLANDRE },
		{ "orin",       SokuLib::CHARACTER_ORIN },
		{ "yuuka",      SokuLib::CHARACTER_YUUKA },
		{ "kaguya",     SokuLib::CHARACTER_KAGUYA },
		{ "mokou",      SokuLib::CHARACTER_MOKOU },
		{ "mima",       SokuLib::CHARACTER_MIMA },
		{ "shou",       SokuLib::CHARACTER_SHOU },
		{ "murasa",     SokuLib::CHARACTER_MURASA },
		{ "sekibanki",  SokuLib::CHARACTER_SEKIBANKI },
		{ "satori",     SokuLib::CHARACTER_SATORI }
	};
	(*this->_lua)[sol::create_if_nil]["enums"]["actions"] = std::map<std::string, SokuLib::Action>{
		{ "ACTION_IDLE", SokuLib::ACTION_IDLE },
		{ "ACTION_CROUCHING", SokuLib::ACTION_CROUCHING },
		{ "ACTION_CROUCHED", SokuLib::ACTION_CROUCHED },
		{ "ACTION_STANDING_UP", SokuLib::ACTION_STANDING_UP },
		{ "ACTION_WALK_FORWARD", SokuLib::ACTION_WALK_FORWARD },
		{ "ACTION_WALK_BACKWARD", SokuLib::ACTION_WALK_BACKWARD },
		{ "ACTION_NEUTRAL_JUMP", SokuLib::ACTION_NEUTRAL_JUMP },
		{ "ACTION_FORWARD_JUMP", SokuLib::ACTION_FORWARD_JUMP },
		{ "ACTION_BACKWARD_JUMP", SokuLib::ACTION_BACKWARD_JUMP },
		{ "ACTION_FALLING", SokuLib::ACTION_FALLING },
		{ "ACTION_LANDING", SokuLib::ACTION_LANDING },
		{ "ACTION_STAND_GROUND_HIT_SMALL_HITSTUN", SokuLib::ACTION_STAND_GROUND_HIT_SMALL_HITSTUN },
		{ "ACTION_STAND_GROUND_HIT_MEDIUM_HITSTUN", SokuLib::ACTION_STAND_GROUND_HIT_MEDIUM_HITSTUN },
		{ "ACTION_STAND_GROUND_HIT_BIG_HITSTUN", SokuLib::ACTION_STAND_GROUND_HIT_BIG_HITSTUN },
		{ "ACTION_STAND_GROUND_HIT_HUGE_HITSTUN", SokuLib::ACTION_STAND_GROUND_HIT_HUGE_HITSTUN },
		{ "ACTION_STAND_GROUND_HIT_SMALL_HITSTUN_2", SokuLib::ACTION_STAND_GROUND_HIT_SMALL_HITSTUN_2 },
		{ "ACTION_STAND_GROUND_HIT_MEDIUM_HITSTUN_2", SokuLib::ACTION_STAND_GROUND_HIT_MEDIUM_HITSTUN_2 },
		{ "ACTION_STAND_GROUND_HIT_BIG_HITSTUN_2", SokuLib::ACTION_STAND_GROUND_HIT_BIG_HITSTUN_2 },
		{ "ACTION_CROUCH_GROUND_HIT_SMALL_HITSTUN", SokuLib::ACTION_CROUCH_GROUND_HIT_SMALL_HITSTUN },
		{ "ACTION_CROUCH_GROUND_HIT_MEDIUM_HITSTUN", SokuLib::ACTION_CROUCH_GROUND_HIT_MEDIUM_HITSTUN },
		{ "ACTION_CROUCH_GROUND_HIT_BIG_HITSTUN", SokuLib::ACTION_CROUCH_GROUND_HIT_BIG_HITSTUN },
		{ "ACTION_AIR_HIT_SMALL_HITSTUN", SokuLib::ACTION_AIR_HIT_SMALL_HITSTUN },
		{ "ACTION_AIR_HIT_MEDIUM_HITSTUN", SokuLib::ACTION_AIR_HIT_MEDIUM_HITSTUN },
		{ "ACTION_AIR_HIT_BIG_HITSTUN", SokuLib::ACTION_AIR_HIT_BIG_HITSTUN },
		{ "ACTION_AIR_HIT_DIRECT_KNOCKDOWN", SokuLib::ACTION_AIR_HIT_DIRECT_KNOCKDOWN },
		{ "ACTION_AIR_HIT_CAN_WALL_SLAM", SokuLib::ACTION_AIR_HIT_CAN_WALL_SLAM },
		{ "ACTION_AIR_HIT_BIG_HITSTUN4", SokuLib::ACTION_AIR_HIT_BIG_HITSTUN4 },
		{ "ACTION_AIR_HIT_WALL_SLAMMED", SokuLib::ACTION_AIR_HIT_WALL_SLAMMED },
		{ "ACTION_AIR_HIT_HUGE_HITSTUN", SokuLib::ACTION_AIR_HIT_HUGE_HITSTUN },
		{ "ACTION_AIR_HIT_WALL_SLAMMED2", SokuLib::ACTION_AIR_HIT_WALL_SLAMMED2 },
		{ "ACTION_AIR_HIT_WILL_GROUND_SLAM", SokuLib::ACTION_AIR_HIT_WILL_GROUND_SLAM },
		{ "ACTION_AIR_HIT_GROUND_SLAMMED", SokuLib::ACTION_AIR_HIT_GROUND_SLAMMED },
		{ "ACTION_KNOCKED_DOWN", SokuLib::ACTION_KNOCKED_DOWN },
		{ "ACTION_KNOCKED_DOWN_STATIC", SokuLib::ACTION_KNOCKED_DOWN_STATIC },
		{ "ACTION_GRABBED", SokuLib::ACTION_GRABBED },
		{ "ACTION_GROUND_CRUSHED", SokuLib::ACTION_GROUND_CRUSHED },
		{ "ACTION_AIR_CRUSHED", SokuLib::ACTION_AIR_CRUSHED },
		{ "ACTION_RIGHTBLOCK_HIGH_SMALL_BLOCKSTUN", SokuLib::ACTION_RIGHTBLOCK_HIGH_SMALL_BLOCKSTUN },
		{ "ACTION_RIGHTBLOCK_HIGH_MEDIUM_BLOCKSTUN", SokuLib::ACTION_RIGHTBLOCK_HIGH_MEDIUM_BLOCKSTUN },
		{ "ACTION_RIGHTBLOCK_HIGH_BIG_BLOCKSTUN", SokuLib::ACTION_RIGHTBLOCK_HIGH_BIG_BLOCKSTUN },
		{ "ACTION_RIGHTBLOCK_HIGH_HUGE_BLOCKSTUN", SokuLib::ACTION_RIGHTBLOCK_HIGH_HUGE_BLOCKSTUN },
		{ "ACTION_RIGHTBLOCK_LOW_SMALL_BLOCKSTUN", SokuLib::ACTION_RIGHTBLOCK_LOW_SMALL_BLOCKSTUN },
		{ "ACTION_RIGHTBLOCK_LOW_MEDIUM_BLOCKSTUN", SokuLib::ACTION_RIGHTBLOCK_LOW_MEDIUM_BLOCKSTUN },
		{ "ACTION_RIGHTBLOCK_LOW_BIG_BLOCKSTUN", SokuLib::ACTION_RIGHTBLOCK_LOW_BIG_BLOCKSTUN },
		{ "ACTION_RIGHTBLOCK_LOW_HUGE_BLOCKSTUN", SokuLib::ACTION_RIGHTBLOCK_LOW_HUGE_BLOCKSTUN },
		{ "ACTION_AIR_GUARD", SokuLib::ACTION_AIR_GUARD },
		{ "ACTION_WRONGBLOCK_HIGH_SMALL_BLOCKSTUN", SokuLib::ACTION_WRONGBLOCK_HIGH_SMALL_BLOCKSTUN },
		{ "ACTION_WRONGBLOCK_HIGH_MEDIUM_BLOCKSTUN", SokuLib::ACTION_WRONGBLOCK_HIGH_MEDIUM_BLOCKSTUN },
		{ "ACTION_WRONGBLOCK_HIGH_BIG_BLOCKSTUN", SokuLib::ACTION_WRONGBLOCK_HIGH_BIG_BLOCKSTUN },
		{ "ACTION_WRONGBLOCK_HIGH_HUGE_BLOCKSTUN", SokuLib::ACTION_WRONGBLOCK_HIGH_HUGE_BLOCKSTUN },
		{ "ACTION_WRONGBLOCK_LOW_SMALL_BLOCKSTUN", SokuLib::ACTION_WRONGBLOCK_LOW_SMALL_BLOCKSTUN },
		{ "ACTION_WRONGBLOCK_LOW_MEDIUM_BLOCKSTUN", SokuLib::ACTION_WRONGBLOCK_LOW_MEDIUM_BLOCKSTUN },
		{ "ACTION_WRONGBLOCK_LOW_BIG_BLOCKSTUN", SokuLib::ACTION_WRONGBLOCK_LOW_BIG_BLOCKSTUN },
		{ "ACTION_WRONGBLOCK_LOW_HUGE_BLOCKSTUN", SokuLib::ACTION_WRONGBLOCK_LOW_HUGE_BLOCKSTUN },
		{ "ACTION_FORWARD_AIRTECH", SokuLib::ACTION_FORWARD_AIRTECH },
		{ "ACTION_BACKWARD_AIRTECH", SokuLib::ACTION_BACKWARD_AIRTECH },
		{ "ACTION_FORWARD_TECH", SokuLib::ACTION_FORWARD_TECH },
		{ "ACTION_BACKWARD_TECH", SokuLib::ACTION_BACKWARD_TECH },
		{ "ACTION_NEUTRAL_TECH", SokuLib::ACTION_NEUTRAL_TECH },
		{ "ACTION_FORWARD_DASH", SokuLib::ACTION_FORWARD_DASH },
		{ "ACTION_BACKDASH", SokuLib::ACTION_BACKDASH },
		{ "ACTION_FORWARD_AIRDASH", SokuLib::ACTION_FORWARD_AIRDASH },
		{ "ACTION_BACKWARD_AIRDASH", SokuLib::ACTION_BACKWARD_AIRDASH },
		{ "ACTION_GROUNDDASH_RECOVERY", SokuLib::ACTION_GROUNDDASH_RECOVERY },
		{ "ACTION_LILYPAD_FORWARD_DASH", SokuLib::ACTION_LILYPAD_FORWARD_DASH },
		{ "ACTION_LILYPAD_BACKDASH", SokuLib::ACTION_LILYPAD_BACKDASH },
		{ "ACTION_NEUTRAL_HIGH_JUMP", SokuLib::ACTION_NEUTRAL_HIGH_JUMP },
		{ "ACTION_FORWARD_HIGH_JUMP", SokuLib::ACTION_FORWARD_HIGH_JUMP },
		{ "ACTION_BACKWARD_HIGH_JUMP", SokuLib::ACTION_BACKWARD_HIGH_JUMP },
		{ "ACTION_NEUTRAL_HIGH_JUMP_FROM_GROUND_DASH", SokuLib::ACTION_NEUTRAL_HIGH_JUMP_FROM_GROUND_DASH },
		{ "ACTION_FORWARD_HIGH_JUMP_FROM_GROUND_DASH", SokuLib::ACTION_FORWARD_HIGH_JUMP_FROM_GROUND_DASH },
		{ "ACTION_FLY", SokuLib::ACTION_FLY },
		{ "ACTION_SUWAKO_j2D", SokuLib::ACTION_SUWAKO_j2D },
		{ "ACTION_SUWAKO_j1D_j3D", SokuLib::ACTION_SUWAKO_j1D_j3D },
		{ "ACTION_BE2", SokuLib::ACTION_BE2 },
		{ "ACTION_BE1", SokuLib::ACTION_BE1 },
		{ "ACTION_BE6", SokuLib::ACTION_BE6 },
		{ "ACTION_BE4", SokuLib::ACTION_BE4 },
		{ "ACTION_jBE4", SokuLib::ACTION_jBE4 },
		{ "ACTION_jBE6", SokuLib::ACTION_jBE6 },
		{ "ACTION_5A", SokuLib::ACTION_5A },
		{ "ACTION_f5A", SokuLib::ACTION_f5A },
		{ "ACTION_6A", SokuLib::ACTION_6A },
		{ "ACTION_2A", SokuLib::ACTION_2A },
		{ "ACTION_3A", SokuLib::ACTION_3A },
		{ "ACTION_66A", SokuLib::ACTION_66A },
		{ "ACTION_j5A", SokuLib::ACTION_j5A },
		{ "ACTION_j6A", SokuLib::ACTION_j6A },
		{ "ACTION_j2A", SokuLib::ACTION_j2A },
		{ "ACTION_j8A", SokuLib::ACTION_j8A },
		{ "ACTION_f2A", SokuLib::ACTION_f2A },
		{ "ACTION_5AA", SokuLib::ACTION_5AA },
		{ "ACTION_5AAA", SokuLib::ACTION_5AAA },
		{ "ACTION_5AAAA", SokuLib::ACTION_5AAAA },
		{ "ACTION_5AAAAA", SokuLib::ACTION_5AAAAA },
		{ "ACTION_5AAA3A", SokuLib::ACTION_5AAA3A },
		{ "ACTION_j5AA", SokuLib::ACTION_j5AA },
		{ "ACTION_4A", SokuLib::ACTION_4A },
		{ "ACTION_SUWAKO_3A", SokuLib::ACTION_SUWAKO_3A },
		{ "ACTION_SUWAKO_LILYPAD_6A", SokuLib::ACTION_SUWAKO_LILYPAD_6A },
		{ "ACTION_5B", SokuLib::ACTION_5B },
		{ "ACTION_6B", SokuLib::ACTION_6B },
		{ "ACTION_2B", SokuLib::ACTION_2B },
		{ "ACTION_3B", SokuLib::ACTION_3B },
		{ "ACTION_j5B", SokuLib::ACTION_j5B },
		{ "ACTION_j6B", SokuLib::ACTION_j6B },
		{ "ACTION_j2B", SokuLib::ACTION_j2B },
		{ "ACTION_4B", SokuLib::ACTION_4B },
		{ "ACTION_66B", SokuLib::ACTION_66B },
		{ "ACTION_j4B", SokuLib::ACTION_j4B },
		{ "ACTION_5C", SokuLib::ACTION_5C },
		{ "ACTION_6C", SokuLib::ACTION_6C },
		{ "ACTION_2C", SokuLib::ACTION_2C },
		{ "ACTION_j5C", SokuLib::ACTION_j5C },
		{ "ACTION_j6C", SokuLib::ACTION_j6C },
		{ "ACTION_j2C", SokuLib::ACTION_j2C },
		{ "ACTION_j1C", SokuLib::ACTION_j1C },
		{ "ACTION_66C", SokuLib::ACTION_66C },
		{ "ACTION_ORRERIES_B", SokuLib::ACTION_ORRERIES_B },
		{ "ACTION_AIR_ORRERIES_B", SokuLib::ACTION_AIR_ORRERIES_B },
		{ "ACTION_ORRERIES_C", SokuLib::ACTION_ORRERIES_C },
		{ "ACTION_AIR_ORRERIES_C", SokuLib::ACTION_AIR_ORRERIES_C },
		{ "ACTION_DEFAULT_SKILL1_B", SokuLib::ACTION_DEFAULT_SKILL1_B },
		{ "ACTION_DEFAULT_SKILL1_C", SokuLib::ACTION_DEFAULT_SKILL1_C },
		{ "ACTION_DEFAULT_SKILL1_AIR_B", SokuLib::ACTION_DEFAULT_SKILL1_AIR_B },
		{ "ACTION_DEFAULT_SKILL1_AIR_C", SokuLib::ACTION_DEFAULT_SKILL1_AIR_C },
		{ "ACTION_ALT1_SKILL1_B", SokuLib::ACTION_ALT1_SKILL1_B },
		{ "ACTION_ALT1_SKILL1_C", SokuLib::ACTION_ALT1_SKILL1_C },
		{ "ACTION_ALT1_SKILL1_AIR_B", SokuLib::ACTION_ALT1_SKILL1_AIR_B },
		{ "ACTION_ALT1_SKILL1_AIR_C", SokuLib::ACTION_ALT1_SKILL1_AIR_C },
		{ "ACTION_ALT2_SKILL1_B", SokuLib::ACTION_ALT2_SKILL1_B },
		{ "ACTION_ALT2_SKILL1_C", SokuLib::ACTION_ALT2_SKILL1_C },
		{ "ACTION_ALT2_SKILL1_AIR_B", SokuLib::ACTION_ALT2_SKILL1_AIR_B },
		{ "ACTION_ALT2_SKILL1_AIR_C", SokuLib::ACTION_ALT2_SKILL1_AIR_C },
		{ "ACTION_DEFAULT_SKILL2_B", SokuLib::ACTION_DEFAULT_SKILL2_B },
		{ "ACTION_DEFAULT_SKILL2_C", SokuLib::ACTION_DEFAULT_SKILL2_C },
		{ "ACTION_DEFAULT_SKILL2_AIR_B", SokuLib::ACTION_DEFAULT_SKILL2_AIR_B },
		{ "ACTION_DEFAULT_SKILL2_AIR_C", SokuLib::ACTION_DEFAULT_SKILL2_AIR_C },
		{ "ACTION_ALT1_SKILL2_B", SokuLib::ACTION_ALT1_SKILL2_B },
		{ "ACTION_ALT1_SKILL2_C", SokuLib::ACTION_ALT1_SKILL2_C },
		{ "ACTION_ALT1_SKILL2_AIR_B", SokuLib::ACTION_ALT1_SKILL2_AIR_B },
		{ "ACTION_ALT1_SKILL2_AIR_C", SokuLib::ACTION_ALT1_SKILL2_AIR_C },
		{ "ACTION_ALT2_SKILL2_B", SokuLib::ACTION_ALT2_SKILL2_B },
		{ "ACTION_ALT2_SKILL2_C", SokuLib::ACTION_ALT2_SKILL2_C },
		{ "ACTION_ALT2_SKILL2_AIR_B", SokuLib::ACTION_ALT2_SKILL2_AIR_B },
		{ "ACTION_ALT2_SKILL2_AIR_C", SokuLib::ACTION_ALT2_SKILL2_AIR_C },
		{ "ACTION_DEFAULT_SKILL3_B", SokuLib::ACTION_DEFAULT_SKILL3_B },
		{ "ACTION_DEFAULT_SKILL3_C", SokuLib::ACTION_DEFAULT_SKILL3_C },
		{ "ACTION_DEFAULT_SKILL3_AIR_B", SokuLib::ACTION_DEFAULT_SKILL3_AIR_B },
		{ "ACTION_DEFAULT_SKILL3_AIR_C", SokuLib::ACTION_DEFAULT_SKILL3_AIR_C },
		{ "ACTION_ALT1_SKILL3_B", SokuLib::ACTION_ALT1_SKILL3_B },
		{ "ACTION_ALT1_SKILL3_C", SokuLib::ACTION_ALT1_SKILL3_C },
		{ "ACTION_ALT1_SKILL3_AIR_B", SokuLib::ACTION_ALT1_SKILL3_AIR_B },
		{ "ACTION_ALT1_SKILL3_AIR_C", SokuLib::ACTION_ALT1_SKILL3_AIR_C },
		{ "ACTION_ALT2_SKILL3_B", SokuLib::ACTION_ALT2_SKILL3_B },
		{ "ACTION_ALT2_SKILL3_C", SokuLib::ACTION_ALT2_SKILL3_C },
		{ "ACTION_ALT2_SKILL3_AIR_B", SokuLib::ACTION_ALT2_SKILL3_AIR_B },
		{ "ACTION_ALT2_SKILL3_AIR_C", SokuLib::ACTION_ALT2_SKILL3_AIR_C },
		{ "ACTION_DEFAULT_SKILL4_B", SokuLib::ACTION_DEFAULT_SKILL4_B },
		{ "ACTION_DEFAULT_SKILL4_C", SokuLib::ACTION_DEFAULT_SKILL4_C },
		{ "ACTION_DEFAULT_SKILL4_AIR_B", SokuLib::ACTION_DEFAULT_SKILL4_AIR_B },
		{ "ACTION_DEFAULT_SKILL4_AIR_C", SokuLib::ACTION_DEFAULT_SKILL4_AIR_C },
		{ "ACTION_ALT1_SKILL4_B", SokuLib::ACTION_ALT1_SKILL4_B },
		{ "ACTION_ALT1_SKILL4_C", SokuLib::ACTION_ALT1_SKILL4_C },
		{ "ACTION_ALT1_SKILL4_AIR_B", SokuLib::ACTION_ALT1_SKILL4_AIR_B },
		{ "ACTION_ALT1_SKILL4_AIR_C", SokuLib::ACTION_ALT1_SKILL4_AIR_C },
		{ "ACTION_ALT2_SKILL4_B", SokuLib::ACTION_ALT2_SKILL4_B },
		{ "ACTION_ALT2_SKILL4_C", SokuLib::ACTION_ALT2_SKILL4_C },
		{ "ACTION_ALT2_SKILL4_AIR_B", SokuLib::ACTION_ALT2_SKILL4_AIR_B },
		{ "ACTION_ALT2_SKILL4_AIR_C", SokuLib::ACTION_ALT2_SKILL4_AIR_C },
		{ "ACTION_DEFAULT_SKILL5_B", SokuLib::ACTION_DEFAULT_SKILL5_B },
		{ "ACTION_DEFAULT_SKILL5_C", SokuLib::ACTION_DEFAULT_SKILL5_C },
		{ "ACTION_DEFAULT_SKILL5_AIR_B", SokuLib::ACTION_DEFAULT_SKILL5_AIR_B },
		{ "ACTION_DEFAULT_SKILL5_AIR_C", SokuLib::ACTION_DEFAULT_SKILL5_AIR_C },
		{ "ACTION_ALT1_SKILL5_B", SokuLib::ACTION_ALT1_SKILL5_B },
		{ "ACTION_ALT1_SKILL5_C", SokuLib::ACTION_ALT1_SKILL5_C },
		{ "ACTION_ALT1_SKILL5_AIR_B", SokuLib::ACTION_ALT1_SKILL5_AIR_B },
		{ "ACTION_ALT1_SKILL5_AIR_C", SokuLib::ACTION_ALT1_SKILL5_AIR_C },
		{ "ACTION_ALT2_SKILL5_B", SokuLib::ACTION_ALT2_SKILL5_B },
		{ "ACTION_ALT2_SKILL5_C", SokuLib::ACTION_ALT2_SKILL5_C },
		{ "ACTION_ALT2_SKILL5_AIR_B", SokuLib::ACTION_ALT2_SKILL5_AIR_B },
		{ "ACTION_ALT2_SKILL5_AIR_C", SokuLib::ACTION_ALT2_SKILL5_AIR_C },
		{ "ACTION_USING_SC_ID_200", SokuLib::ACTION_USING_SC_ID_200 },
		{ "ACTION_USING_SC_ID_201", SokuLib::ACTION_USING_SC_ID_201 },
		{ "ACTION_USING_SC_ID_202", SokuLib::ACTION_USING_SC_ID_202 },
		{ "ACTION_USING_SC_ID_203", SokuLib::ACTION_USING_SC_ID_203 },
		{ "ACTION_USING_SC_ID_204", SokuLib::ACTION_USING_SC_ID_204 },
		{ "ACTION_USING_SC_ID_205", SokuLib::ACTION_USING_SC_ID_205 },
		{ "ACTION_USING_SC_ID_206", SokuLib::ACTION_USING_SC_ID_206 },
		{ "ACTION_USING_SC_ID_207", SokuLib::ACTION_USING_SC_ID_207 },
		{ "ACTION_USING_SC_ID_208", SokuLib::ACTION_USING_SC_ID_208 },
		{ "ACTION_USING_SC_ID_209", SokuLib::ACTION_USING_SC_ID_209 },
		{ "ACTION_USING_SC_ID_210", SokuLib::ACTION_USING_SC_ID_210 },
		{ "ACTION_USING_SC_ID_211", SokuLib::ACTION_USING_SC_ID_211 },
		{ "ACTION_USING_SC_ID_212", SokuLib::ACTION_USING_SC_ID_212 },
		{ "ACTION_USING_SC_ID_213", SokuLib::ACTION_USING_SC_ID_213 },
		{ "ACTION_USING_SC_ID_214", SokuLib::ACTION_USING_SC_ID_214 },
		{ "ACTION_USING_SC_ID_215", SokuLib::ACTION_USING_SC_ID_215 },
		{ "ACTION_USING_SC_ID_216", SokuLib::ACTION_USING_SC_ID_216 },
		{ "ACTION_USING_SC_ID_217", SokuLib::ACTION_USING_SC_ID_217 },
		{ "ACTION_USING_SC_ID_218", SokuLib::ACTION_USING_SC_ID_218 },
		{ "ACTION_USING_SC_ID_219", SokuLib::ACTION_USING_SC_ID_219 },
		{ "ACTION_SC_ID_200_ALT_EFFECT", SokuLib::ACTION_SC_ID_200_ALT_EFFECT },
		{ "ACTION_SC_ID_201_ALT_EFFECT", SokuLib::ACTION_SC_ID_201_ALT_EFFECT },
		{ "ACTION_SC_ID_202_ALT_EFFECT", SokuLib::ACTION_SC_ID_202_ALT_EFFECT },
		{ "ACTION_SC_ID_203_ALT_EFFECT", SokuLib::ACTION_SC_ID_203_ALT_EFFECT },
		{ "ACTION_SC_ID_204_ALT_EFFECT", SokuLib::ACTION_SC_ID_204_ALT_EFFECT },
		{ "ACTION_SC_ID_205_ALT_EFFECT", SokuLib::ACTION_SC_ID_205_ALT_EFFECT },
		{ "ACTION_SC_ID_206_ALT_EFFECT", SokuLib::ACTION_SC_ID_206_ALT_EFFECT },
		{ "ACTION_SC_ID_207_ALT_EFFECT", SokuLib::ACTION_SC_ID_207_ALT_EFFECT },
		{ "ACTION_SC_ID_208_ALT_EFFECT", SokuLib::ACTION_SC_ID_208_ALT_EFFECT },
		{ "ACTION_SC_ID_209_ALT_EFFECT", SokuLib::ACTION_SC_ID_209_ALT_EFFECT },
		{ "ACTION_SC_ID_210_ALT_EFFECT", SokuLib::ACTION_SC_ID_210_ALT_EFFECT },
		{ "ACTION_SC_ID_211_ALT_EFFECT", SokuLib::ACTION_SC_ID_211_ALT_EFFECT },
		{ "ACTION_SC_ID_212_ALT_EFFECT", SokuLib::ACTION_SC_ID_212_ALT_EFFECT },
		{ "ACTION_SC_ID_213_ALT_EFFECT", SokuLib::ACTION_SC_ID_213_ALT_EFFECT },
		{ "ACTION_SC_ID_214_ALT_EFFECT", SokuLib::ACTION_SC_ID_214_ALT_EFFECT },
		{ "ACTION_SC_ID_215_ALT_EFFECT", SokuLib::ACTION_SC_ID_215_ALT_EFFECT },
		{ "ACTION_SC_ID_216_ALT_EFFECT", SokuLib::ACTION_SC_ID_216_ALT_EFFECT },
		{ "ACTION_SC_ID_217_ALT_EFFECT", SokuLib::ACTION_SC_ID_217_ALT_EFFECT },
		{ "ACTION_SC_ID_218_ALT_EFFECT", SokuLib::ACTION_SC_ID_218_ALT_EFFECT },
		{ "ACTION_SC_ID_219_ALT_EFFECT", SokuLib::ACTION_SC_ID_219_ALT_EFFECT },
		{ "ACTION_FANTASY_HEAVEN_ACTIVATE", SokuLib::ACTION_FANTASY_HEAVEN_ACTIVATE },
		{ "ACTION_SKILL_CARD", SokuLib::ACTION_SKILL_CARD },
		{ "ACTION_SYSTEM_CARD", SokuLib::ACTION_SYSTEM_CARD },
		{ "ACTION_IBUKI_GOURD", SokuLib::ACTION_IBUKI_GOURD },
		{ "ACTION_RECOVERY_CHARM", SokuLib::ACTION_RECOVERY_CHARM },
		{ "ACTION_MAGIC_POTION", SokuLib::ACTION_MAGIC_POTION },
		{ "ACTION_TALISMAN", SokuLib::ACTION_TALISMAN },
		{ "ACTION_BOMB", SokuLib::ACTION_BOMB },
		{ "ACTION_HANGEKI", SokuLib::ACTION_HANGEKI },
		{ "ACTION_DIVINE_RAIMENT_OF_THE_DRAGON_FISH", SokuLib::ACTION_DIVINE_RAIMENT_OF_THE_DRAGON_FISH },
		{ "ACTION_LEFT_HANDED_FOLDING_FAN", SokuLib::ACTION_LEFT_HANDED_FOLDING_FAN },
		{ "ACTION_SPELL_BREAKING_DRUG", SokuLib::ACTION_SPELL_BREAKING_DRUG },
	};

	(*this->_lua)["playBGM"] = sol::overload(
		SokuLib::playBGM,
		[](const std::string &path, double loopStart, double loopEnd){
			_loopStart = loopStart;
			_loopEnd = loopEnd;
			VirtualProtect((PVOID)0x418cc5, 5, PAGE_EXECUTE_WRITECOPY, &old);
			og = SokuLib::TamperNearJmpOpr(0x418cc5, editLoop);
			SokuLib::playBGM(path.c_str());
		}
	);
	(*this->_lua)["playBgm"] = (*this->_lua)["playBGM"];
	(*this->_lua)["camera"] = std::ref(SokuLib::camera);
	(*this->_lua)["packPath"] = packPath;
	(*this->_lua)["playSFX"] = SokuLib::playSEWaveBuffer;
	(*this->_lua)["playSfx"] = SokuLib::playSEWaveBuffer;
	(*this->_lua)["math"]["randomseed"](time(nullptr));

	this->_lua->script_file(script);
}

bool LuaBattleAnimation::update()
{
	if (this->_hasError)
		return false;
	(*this->_lua)["battleMgr"] = (FakeBattleManager *)&SokuLib::getBattleMgr();
	try {
		auto fct = this->_lua->get<sol::protected_function>("update");
		auto result = fct();

		if (result.valid())
			return result;
		else {
			auto err = result.get<sol::error>();

			fprintf(stderr, "update() error: %s\n", err.what());
			MessageBox(SokuLib::window, err.what(), "function update() error", MB_ICONERROR);
			this->_hasError = true;
			return false;
		}
	} catch (std::exception &e) {
		MessageBox(SokuLib::window, e.what(), "Cannot fetch update function", MB_ICONERROR);
		this->_hasError = true;
		return false;
	}
}

void LuaBattleAnimation::render() const
{
	(*this->_lua)["battleMgr"] = (FakeBattleManager *)&SokuLib::getBattleMgr();
	try {
		auto fct = this->_lua->get<sol::protected_function>("render");
		auto result = fct();

		if (!result.valid()) {
			auto err = result.get<sol::error>();

			fprintf(stderr, "render() error: %s\n", err.what());
			MessageBox(SokuLib::window, err.what(), "function render() error", MB_ICONERROR);
			this->_hasError = true;
		}
	} catch (std::exception &e) {
		MessageBox(SokuLib::window, e.what(), "Cannot fetch render function", MB_ICONERROR);
		this->_hasError = true;
	}
}

void LuaBattleAnimation::onKeyPressed()
{
	(*this->_lua)["battleMgr"] = (FakeBattleManager *)&SokuLib::getBattleMgr();
	try {
		auto fct = this->_lua->get<sol::function>("onKeyPressed");
		auto result = fct();

		if (!result.valid()) {
			auto err = result.get<sol::error>();

			fprintf(stderr, "onKeyPressed() error: %s\n", err.what());
			MessageBox(SokuLib::window, err.what(), "function onKeyPressed() error", MB_ICONERROR);
			this->_hasError = true;
		}
	} catch (std::exception &e) {
		MessageBox(SokuLib::window, e.what(), "Cannot fetch onKeyPressed function", MB_ICONERROR);
		this->_hasError = true;
	}
}

LuaBattleAnimation::~LuaBattleAnimation()
{
	puts("Delete Lua state");
	this->_lua.reset();
	for (auto &obj : this->_created) {
		printf("Deleting character %p\n", obj);
		(*(void (__thiscall **)(FakeCharacterManager *, char))obj->vtable)(obj, 0);
		SokuLib::Delete(obj);
	}
}
