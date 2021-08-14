//
// Created by PinkySmile on 12/08/2021.
//

#include <SokuLib.hpp>
#include <lua.hpp>
#include <sol/sol.hpp>
#include <map>
#include "LuaBattleAnimation.hpp"

struct FakeCharacterManager {
	// 0x000
	char offset_0x000[0xEC];

	//  ADDR_POINTXOFS          float             (4) 0x0EC
	//  ADDR_POINTYOFS          float             (4) 0x0F0
	SokuLib::Vector2f position;

	//  ADDR_SPEEDXOFS          float             (4) 0x0F4
	//  ADDR_SPEEDYOFS          float             (4) 0x0F8
	SokuLib::Vector2f speed;

	// 0x0FC
	char offset_0x0FC[0x4];

	// 0x100
	float gravity;

	//  ADDR_DIRECTIONOFS       enum Direction    (1) 0x104
	int8_t direction;

	// 0x105
	char offset_0x105[0xB];

	// 0x110
	SokuLib::RenderInfo renderInfos;

	// 0x128
	char offset_0x128[0x14];

	//  ADDR_ACTIONIDOFS        enum Action       (2) 0x13C
	SokuLib::Action action;

	//  ADDR_ACTIONBLOCKIDOFS   unsigned short    (2) 0x13E
	unsigned short actionBlockId;

	//  unsigned short    (2) 0x140
	unsigned short animationCounter;

	//  unsigned short    (2) 0x142
	unsigned short animationSubFrame;

	//  ADDR_FRAMECOUNTOFS      unsigned int      (4) 0x144
	unsigned int frameCount;

	// 0x148
	unsigned short animationCounterMax;

	// 0x14A
	char offset_0x14A[0x2];

	// 0x14C
	unsigned short animationSubFrameMax;

	// 0x14E
	char offset_0x14E[0x2];

	//  ADDR_IMAGESTRUCTOFS     ImageStruct &     (4) 0x150
	SokuLib::FrameData *image;

	// 0x154
	char offset_0x154[0x4];

	// 0x158
	SokuLib::FrameData *frameData;

	// 0x15C
	SokuLib::FrameDataReader *frameDataReader;

	// 0x160
	void *offset_0x160;

	//  int *[256]         (4) 0x164
	int *soundTable;

	// 0x168
	char offset_0x168[0x1C];

	//  ADDR_HPOFS              short    (2) 0x184
	short hp;

	// 0x186
	char offset_0x186[0x2];

	// 0x188
	unsigned int superarmorDamageTaken;

	// 0x18C
	char offset_0x18C[0x8];

	//CF_HIT_COUNT 0x194 // char
	char hitCount;

	// 0x195
	char offset_0x195;

	//  ADDR_HITSTOPOFS         unsigned short    (2) 0x196
	unsigned short hitstop;

	// 0x198
	char offset_0x198[0x33];

	//  ADDR_ATTACKAREACOUNTOFS unsigned char     (1) 0x1CB
	unsigned char hitBoxCount;

	//  ADDR_HITAREACOUNTOFS    unsigned char     (1) 0x1CC
	unsigned char hurtBoxCount;

	// 0x1CD
	char offset_0x1CD[0x3];

	//  ADDR_HITAREA2OFS        Box[5]           (80) 0x1D0
	SokuLib::Box hurtBoxes[5];

	//  ADDR_ATTACKAREA2OFS     Box[5]           (80) 0x220
	SokuLib::Box hitBoxes[5];

	// 0x270
	char offset_0x270[0xB0];

	// CF_ATTACK_BOXES_ROT 0x320 // altbox[5]
	SokuLib::RotationBox *hitBoxesRotation[5];

	//  ADDR_HITAREAFLAGOFS     Box *[5]         (20) 0x334
	SokuLib::RotationBox *hurtBoxesRotation[5];

	// 0x348
	char offset_0x348[0x2];

	// 0x34A
	unsigned char characterIndex;

	// 0x34B
	unsigned char offset_0x34B[3];

	// 0x34E
	unsigned char playerIndex;

	// 0x34F
	char offset_0x34F;

	// 0x350
	bool isRightPlayer;

	// 0x351
	char offset_0x351[0x14A];

	//  ADDR_AIRDASHCOUNTOFS    unsigned char     (1) 0x49B
	unsigned char airdashCount;

	char offset_0x49C[2];

	//  ADDR_REIPOWEROFS        unsigned short    (2) 0x49E
	unsigned short currentSpirit;

	//  ADDR_MAXREIPOWEROFS     unsigned short    (2) 0x4A0
	unsigned short maxSpirit;

	//  ADDR_REISTOPROFS        unsigned short    (2) 0x4A2
	unsigned short spiritRegenDelay;

	// 0x4A4
	unsigned short timeWithBrokenOrb;

	// 0x4A6
	char offset_0x4A6[0x2];

	//  ADDR_TIMESTOPOFS        short             (2) 0x4A8
	unsigned short timeStop;

	// 0x4AA
	char offset_0x4AA[0x3];

	//  ADDR_CORRECTIONOFS      char              (1) 0x4AD
	char correction;

	// 0x4AE
	char offset_0x4AE[0x2];

	//  ADDR_COMBORATEOFS       float             (4) 0x4B0
	//  ADDR_COMBOCOUNTOFS      unsigned short    (2) 0x4B4
	//  ADDR_COMBODAMAGEOFS     unsigned short    (2) 0x4B6
	//  ADDR_COMBOLIMITOFS      unsigned short    (2) 0x4B8
	SokuLib::Combo combo;

	// 0x4BA
	unsigned short untech;

	// 0x4BC
	char offset_0x4BC[0x2];

	// 0x4BE
	unsigned short realLimit;

	// 0x4C0
	char offset_0x4C0[0x10];

	//  ADDR_SPEEDPOWEROFS      float             (4) 0x4D0
	float speedPower;

	// 0x4D4
	char offset_0x4D4[0x48];

	// 0x51C
	unsigned short meleeInvulTimer;

	// 0x51E
	unsigned short grabInvulTimer;

	// 0x520
	unsigned short projectileInvulTimer;

	// 0x522
	char offset_0x522[0x4];

	//  ADDR_HIHISOUOFS         unsigned short    (2) 0x526
	unsigned short swordOfRaptureDebuffTimeLeft;

	//  ADDR_LIFERECOVERYOFS    unsigned short    (2) 0x528
	unsigned short healingCharmTimeLeft;

	// 0x52A
	char offset_0x52A[0x6];

	//  ADDR_ATTACKPOWEROFS     float             (4) 0x530
	float attackPower;

	//  ADDR_DEFENSEPOWEROFS    float             (4) 0x534
	float defensePower;

	// 0x538
	float noSuperArmor;

	// 0x53C
	char offset_0x53C[0x24];

	// 0x560
	unsigned short grimoires;

	// 0x562
	char offset_0x562[0x11];

	// 0x573
	char score;

	// 0x574
	char offset_0x574[8];

	// 0x57C
	SokuLib::DeckInfo deckInfo;

	// 0x5C4
	char offset_0x5C4[0x20];

	// 0x5E4
	unsigned short cardGauge;

	// 0x5E6
	unsigned char cardCount;

	// 0x5E7
	unsigned char cardSlotCount;

	// 0x5E8 (40, 0x28)
	SokuLib::HandContainer hand;

	// 0x610
	char offset_0x610[0x94];

	// 0x6A4
	unsigned char skillLevels[16];

	// 0x6B4
	char offset_0x6B4[0x10];

	// 0x6C4
	SokuLib::Skill skillMap[16];

	// 0x6D4
	char offset_0x6D4[0x24];

	//  ADDR_OBJLISTMGR = 0x6F8
	SokuLib::ObjListManager *objects;

	// 0x6FC
	char offset_0x6FC[0x54];

	//  ADDR_KEYMGROFS          KeyManager &      (4) 0x750
	SokuLib::KeyManager *keyManager;

	// 0x754
	SokuLib::KeyInput keyMap;

	// 0x774
	char offset_0x774[0x54];

	// CF_PRESSED_COMBINATION 0x7C8 // KeyCombination
	KeyCombination keyCombination;

	// 0x7CC
	bool nameHidden;

	// 0x7CD
	char offset_0x7CD[0x27];

	// CF_CHARGE_ATTACK 0x7F4 // char
	char chargedAttack;

	// 0x7F5
	char offset_0x7F5[2];

	// CF_DAMAGE_LIMITED 0x7F7 // bool
	bool damageLimited;

	// 0x7F8
	char offset_0x7F8[0x3C];

	// 0x834
	unsigned short tenguFans;

	// 0x836
	char offset_0x836[0xA];

	// 0x840
	float sacrificialDolls;

	// 0x844
	float controlRod;

	// 0x848
	float magicPotionTimeLeft;

	// 0x84C
	char offset_0x84C[2];

	//  ADDR_RYUUSEIOFS         unsigned short    (2) 0x84E
	unsigned short dragonStarTimeLeft;

	// 0x850
	unsigned short drops;

	//  ADDR_DROPWATERTIMEOFS   unsigned short    (2) 0x852
	unsigned short dropInvulTimeLeft;

	// 0x854
	char offset_0x854[0x3C];

	union {
		//  ADDR_MILLENIUMOFS       unsigned short    (2) 0x890 (REMILIA Millenium Vampire time left (in frame) 600 - 0)
		unsigned short milleniumVampireTime;
		//  ADDR_PHILOSOPHEROFS     unsigned short    (2) 0x890 (PATCHOULI Philosopher's Stone time left (in frame) 1200 - 0)
		unsigned short philosophersStoneTime;
		//  ADDR_SAKUYAWORLDOFS     unsigned short    (2) 0x890 (SAKUYA Sakuya's World time left (in frame) 300 - 0)
		unsigned short sakuyasWorldTime;
	};

	union {
		//  ADDR_PRIVATESQOFS       unsigned short    (2) 0x892 (SAKUYA Private Square time left (in frame) 300 - 0)
		unsigned short privateSquare;
		//  ADDR_ORRERYOFS          unsigned short    (2) 0x892 (MARISA Orreries time left (in frames) 600 - 0)
		unsigned short orreriesTimeLeft;
		//  ADDR_MPPOFS             unsigned short    (2) 0x892 (SUIKA Missing Purple Power time left (in frames) 480 - 0)
		unsigned short missingPurplePowerTimeLeft;
	};

	// 0x894
	char offset_0x894[0x4];

	//  ADDR_FIELDREDOFS        unsigned short    (2) 0x898 (REISEN d623 ?)
	unsigned short reisenD632;

	//  ADDR_FIELDPURPLEOFS     unsigned short    (2) 0x89A (REISEN a623 3 when the opponent is in the circle)
	unsigned short reisenA632;

	//  ADDR_KANAKOOFS          int               (4) 0x89C (SANAE Time left (in frame) before getting Kanako back)
	int kanakoTimeLeft;

	union {
		//  ADDR_SUWAKOOFS          int               (4) 0x8A0 (SANAE Time left (in frame) before getting Suwako back)
		int suwakoTimeLeft;

		//  ADDR_KOKUSHIOFS         unsigned int      (4) 0x8A0 (REISEN Number of Elixir used 0 - 3)
		unsigned short elixirUsed;

		//  ADDR_DIAHARDOFS         unsigned short    (2) 0x8A0 (PATCHOULI Time left (in frame) in the effect of Diamond Hard 720 - 0)
		unsigned short diamondHardEffectLeft;

		//  ADDR_RESBUTTEROFS       unsigned short    (2) 0x8A0 (YUYUKO Number of Resurrection Butterflies used 0 - 4)
		unsigned short resurrectionButterfliesUsed;
	};

	// 0x8A4
	char offset_0x8A4[0xE];

	//  ADDR_FIELDRED2OFS       unsigned short    (2) 0x8B2 (REISEN Infrared Moon time left (in frame) 420 - 0)
	unsigned short infraredMoonTimeLeft;

	//  ADDR_TENSEITIMEOFS      unsigned short    (2) 0x8B4 (REIMU Time left (in frame) before the end of Fantasy Heaven 900 - 0)
	unsigned short fantasyHeavenTimeLeft;

	//  ADDR_TENSEINUMOFS       unsigned short    (2) 0x8B6 (REIMU Number of stacks completed for Fantasy Heaven 0 - 7)
	unsigned short fantasyHeavenStacks;

	// 0x8B8
	char offset_0x8B8[0x1E];

	//  ADDR_TEKETENOFS         unsigned short    (2) 0x8D6 (YOUMU Time left (in frame) before the clone disappear 600 - 0)
	unsigned short youmuCloneTimeLeft;

	// 0x8D8
	char offset_0x8D8[0x4C];

	//  ADDR_MUNENOFS           unsigned short    (2) 0x924 (TENSHI State of Enlightenment time left (in frame) 900 - 0)
	unsigned short stateOfEnlightenmentTimeLeft;

	// 0x926
	char offset_0x926[0];

	void animate() { reinterpret_cast<SokuLib::ObjectManager *>(this)->animate(); };
	void animate2() { reinterpret_cast<SokuLib::ObjectManager *>(this)->animate2(); };
	void doAnimation() { reinterpret_cast<SokuLib::ObjectManager *>(this)->doAnimation(); };
	bool generateCard(unsigned short id, SokuLib::Card &buffer) { return reinterpret_cast<SokuLib::CharacterManager *>(this)->generateCard(id, buffer); };
	SokuLib::Card *addCard(unsigned short id) { return reinterpret_cast<SokuLib::CharacterManager *>(this)->addCard(id); };
	void playSE(int id) { reinterpret_cast<SokuLib::CharacterManager *>(this)->playSE(id); };
};

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
		"StandDialog", sol::constructors<void(std::vector<std::string>)>(),
		"render", &SokuStand::render,
		"update", &SokuStand::update,
		"onKeyPress", &SokuStand::onKeyPress,
		"hidden", sol::property(&SokuStand::isHidden, &SokuStand::setHidden),
		"getCurrentDialog", &SokuStand::getCurrentDialog,
		"isAnimationFinished", &SokuStand::isAnimationFinished,
		"finishAnimations", &SokuStand::finishAnimations,
		sol::meta_function::length, &SokuStand::getCurrentDialog
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
	auto type = this->_lua->new_usertype<FakeCharacterManager>("Character");
	type["position"] = &FakeCharacterManager::position;
	type["speed"] = &FakeCharacterManager::speed;
	type["gravity"] = &FakeCharacterManager::gravity;
	type["direction"] = &FakeCharacterManager::direction;
	type["renderInfos"] = &FakeCharacterManager::renderInfos;
	type["action"] = &FakeCharacterManager::action;
	type["actionBlockId"] = &FakeCharacterManager::actionBlockId;
	type["animationCounter"] = &FakeCharacterManager::animationCounter;
	type["animationSubFrame"] = &FakeCharacterManager::animationSubFrame;
	type["frameCount"] = &FakeCharacterManager::frameCount;
	type["animationCounterMax"] = &FakeCharacterManager::animationCounterMax;
	type["animationSubFrameMax"] = &FakeCharacterManager::animationSubFrameMax;
	type["image"] = &FakeCharacterManager::image;
	type["frameData"] = &FakeCharacterManager::frameData;
	type["frameDataReader"] = &FakeCharacterManager::frameDataReader;
	type["soundTable"] = &FakeCharacterManager::soundTable;
	type["hp"] = &FakeCharacterManager::hp;
	type["superarmorDamageTaken"] = &FakeCharacterManager::superarmorDamageTaken;
	type["hitCount"] = &FakeCharacterManager::hitCount;
	type["hitstop"] = &FakeCharacterManager::hitstop;
	type["hitBoxCount"] = &FakeCharacterManager::hitBoxCount;
	type["hurtBoxCount"] = &FakeCharacterManager::hurtBoxCount;
	type["hurtBoxes"] = &FakeCharacterManager::hurtBoxes;
	type["hitBoxes"] = &FakeCharacterManager::hitBoxes;
	type["position"] = &FakeCharacterManager::position;
	type["speed"] = &FakeCharacterManager::speed;
	type["gravity"] = &FakeCharacterManager::gravity;
	type["direction"] = &FakeCharacterManager::direction;
	type["renderInfos"] = &FakeCharacterManager::renderInfos;
	type["action"] = &FakeCharacterManager::action;
	type["actionBlockId"] = &FakeCharacterManager::actionBlockId;
	type["animationCounter"] = &FakeCharacterManager::animationCounter;
	type["animationSubFrame"] = &FakeCharacterManager::animationSubFrame;
	type["frameCount"] = &FakeCharacterManager::frameCount;
	type["animationCounterMax"] = &FakeCharacterManager::animationCounterMax;
	type["animationSubFrameMax"] = &FakeCharacterManager::animationSubFrameMax;
	type["image"] = &FakeCharacterManager::image;
	type["frameData"] = &FakeCharacterManager::frameData;
	type["frameDataReader"] = &FakeCharacterManager::frameDataReader;
	type["soundTable"] = &FakeCharacterManager::soundTable;
	type["hp"] = &FakeCharacterManager::hp;
	type["superarmorDamageTaken"] = &FakeCharacterManager::superarmorDamageTaken;
	type["hitCount"] = &FakeCharacterManager::hitCount;
	type["hitstop"] = &FakeCharacterManager::hitstop;
	type["hitBoxCount"] = &FakeCharacterManager::hitBoxCount;
	type["hurtBoxCount"] = &FakeCharacterManager::hurtBoxCount;
	type["hurtBoxes"] = &FakeCharacterManager::hurtBoxes;
	type["hitBoxes"] = &FakeCharacterManager::hitBoxes;
	//type["hitBoxesRotation"] = &FakeCharacterManager::hitBoxesRotation;
	//type["hurtBoxesRotation"] = &FakeCharacterManager::hurtBoxesRotation;
	type["characterIndex"] = &FakeCharacterManager::characterIndex;
	type["playerIndex"] = &FakeCharacterManager::playerIndex;
	type["isRightPlayer"] = &FakeCharacterManager::isRightPlayer;
	type["airdashCount"] = &FakeCharacterManager::airdashCount;
	type["currentSpirit"] = &FakeCharacterManager::currentSpirit;
	type["maxSpirit"] = &FakeCharacterManager::maxSpirit;
	type["spiritRegenDelay"] = &FakeCharacterManager::spiritRegenDelay;
	type["timeWithBrokenOrb"] = &FakeCharacterManager::timeWithBrokenOrb;
	type["timeStop"] = &FakeCharacterManager::timeStop;
	type["correction"] = &FakeCharacterManager::correction;
	type["combo"] = &FakeCharacterManager::combo;
	type["untech"] = &FakeCharacterManager::untech;
	type["realLimit"] = &FakeCharacterManager::realLimit;
	type["speedPower"] = &FakeCharacterManager::speedPower;
	type["meleeInvulTimer"] = &FakeCharacterManager::meleeInvulTimer;
	type["grabInvulTimer"] = &FakeCharacterManager::grabInvulTimer;
	type["projectileInvulTimer"] = &FakeCharacterManager::projectileInvulTimer;
	type["swordOfRaptureDebuffTimeLeft"] = &FakeCharacterManager::swordOfRaptureDebuffTimeLeft;
	type["healingCharmTimeLeft"] = &FakeCharacterManager::healingCharmTimeLeft;
	type["attackPower"] = &FakeCharacterManager::attackPower;
	type["defensePower"] = &FakeCharacterManager::defensePower;
	type["noSuperArmor"] = &FakeCharacterManager::noSuperArmor;
	type["grimoires"] = &FakeCharacterManager::grimoires;
	type["score"] = &FakeCharacterManager::score;
	type["deckInfo"] = &FakeCharacterManager::deckInfo;
	type["cardGauge"] = &FakeCharacterManager::cardGauge;
	type["cardCount"] = &FakeCharacterManager::cardCount;
	type["cardSlotCount"] = &FakeCharacterManager::cardSlotCount;
	type["hand"] = &FakeCharacterManager::hand;
	type["skillLevels"] = &FakeCharacterManager::skillLevels;
	type["skillMap"] = &FakeCharacterManager::skillMap;
	type["objects"] = &FakeCharacterManager::objects;
	type["keyManager"] = &FakeCharacterManager::keyManager;
	type["keyMap"] = &FakeCharacterManager::keyMap;
	type["keyCombination"] = &FakeCharacterManager::keyCombination;
	type["nameHidden"] = &FakeCharacterManager::nameHidden;
	type["chargedAttack"] = &FakeCharacterManager::chargedAttack;
	type["damageLimited"] = &FakeCharacterManager::damageLimited;
	type["tenguFans"] = &FakeCharacterManager::tenguFans;
	type["sacrificialDolls"] = &FakeCharacterManager::sacrificialDolls;
	type["controlRod"] = &FakeCharacterManager::controlRod;
	type["magicPotionTimeLeft"] = &FakeCharacterManager::magicPotionTimeLeft;
	type["dragonStarTimeLeft"] = &FakeCharacterManager::dragonStarTimeLeft;
	type["drops"] = &FakeCharacterManager::drops;
	type["dropInvulTimeLeft"] = &FakeCharacterManager::dropInvulTimeLeft;
	type["milleniumVampireTime"] = &FakeCharacterManager::milleniumVampireTime;
	type["philosophersStoneTime"] = &FakeCharacterManager::philosophersStoneTime;
	type["sakuyasWorldTime"] = &FakeCharacterManager::sakuyasWorldTime;
	type["privateSquare"] = &FakeCharacterManager::privateSquare;
	type["orreriesTimeLeft"] = &FakeCharacterManager::orreriesTimeLeft;
	type["missingPurplePowerTimeLeft"] = &FakeCharacterManager::missingPurplePowerTimeLeft;
	type["reisenD632"] = &FakeCharacterManager::reisenD632;
	type["reisenA632"] = &FakeCharacterManager::reisenA632;
	type["kanakoTimeLeft"] = &FakeCharacterManager::kanakoTimeLeft;
	type["suwakoTimeLeft"] = &FakeCharacterManager::suwakoTimeLeft;
	type["elixirUsed"] = &FakeCharacterManager::elixirUsed;
	type["diamondHardEffectLeft"] = &FakeCharacterManager::diamondHardEffectLeft;
	type["resurrectionButterfliesUsed"] = &FakeCharacterManager::resurrectionButterfliesUsed;
	type["infraredMoonTimeLeft"] = &FakeCharacterManager::infraredMoonTimeLeft;
	type["fantasyHeavenTimeLeft"] = &FakeCharacterManager::fantasyHeavenTimeLeft;
	type["fantasyHeavenStacks"] = &FakeCharacterManager::fantasyHeavenStacks;
	type["youmuCloneTimeLeft"] = &FakeCharacterManager::youmuCloneTimeLeft;
	type["stateOfEnlightenmentTimeLeft"] = &FakeCharacterManager::stateOfEnlightenmentTimeLeft;
	type["initAnimation"] = &FakeCharacterManager::animate;
	type["animate"] = &FakeCharacterManager::animate2;
	type["updateAnimation"] = &FakeCharacterManager::doAnimation;
	type["generateCard"] = &FakeCharacterManager::generateCard;
	type["addCard"] = &FakeCharacterManager::addCard;
	type["playSfx"] = &FakeCharacterManager::playSE;
	(*this->_lua)[sol::create_if_nil]["enums"]["directions"] = std::map<std::string, int>{
		{ "LEFT", SokuLib::LEFT },
		{ "RIGHT", SokuLib::RIGHT }
	};
	(*this->_lua)[sol::create_if_nil]["enums"]["sfxs"] = std::map<std::string, int>{
		{ "smallHit", 1 },
		{ "mediumHit", 2 },
		{ "bigHit", 3 },
		{ "hugeHit", 4 },
		{ "Remi j2a hit", 5 },
		{ "sharkHit", 6 },
		{ "Remi fork hit", 7 },
		{ "Remi claw hit", 8 },
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

	(*this->_lua)["playBGM"] = (void (*)(const char *))0x43ff10;
	(*this->_lua)["camera"] = std::ref(SokuLib::camera);
	(*this->_lua)["packPath"] = packPath;
	(*this->_lua)["playSfx"] = SokuLib::playSEWaveBuffer;

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

			std::cerr << "update error: " << err.what() << std::endl;
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

			std::cerr << "render error: " << err.what() << std::endl;
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

			std::cerr << "onKeyPressed error: " << err.what() << std::endl;
			MessageBox(SokuLib::window, err.what(), "function onKeyPressed() error", MB_ICONERROR);
			this->_hasError = true;
		}
	} catch (std::exception &e) {
		MessageBox(SokuLib::window, e.what(), "Cannot fetch onKeyPressed function", MB_ICONERROR);
		this->_hasError = true;
	}
}