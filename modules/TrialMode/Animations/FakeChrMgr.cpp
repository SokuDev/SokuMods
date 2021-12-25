//
// Created by PinkySmile on 13/09/2021.
//

#include <sol/sol.hpp>
#include "FakeChrMgr.hpp"

void pushFakeChrMgrLuaTable(sol::state &state, std::vector<FakeCharacterManager *> &_created)
{
	auto type = state.new_usertype<FakeCharacterManager>("Character");

	type[sol::meta_function::construct] = [&_created](SokuLib::PlayerInfo &playerInfo){
		FakeCharacterManager *obj[0xC];
		FakeCharacterManager *chr;

		if (SokuLib::sceneId != SokuLib::SCENE_BATTLE)
			throw sol::error("You need to init the character when in battle.");
		((void (__thiscall *)(FakeCharacterManager **, bool, SokuLib::PlayerInfo &))0x46da40)(obj, playerInfo.isRight, playerInfo);
		chr = obj[0xA + playerInfo.isRight];
		(*(void (__thiscall **)(FakeCharacterManager *))(*(int *)chr + 0x44))(chr);
		chr->opponent = playerInfo.isRight ? (FakeCharacterManager *)&SokuLib::getBattleMgr().leftCharacterManager : (FakeCharacterManager *)&SokuLib::getBattleMgr().rightCharacterManager;
		_created.push_back(chr);
		printf("Allocated character %p\n", chr);
		return chr;
	};
	type["createSubObject"] = [](FakeCharacterManager *chr, int id, float x, float y){
		float something[3];

		memset(something, 0, sizeof(something));
		something[2] = 1;
		((void (__thiscall *)(FakeCharacterManager *, int, float, float, char, int, float *, int))0x46EB30)(chr, id, x, y, 1, 1, something, 3);
		return chr->objects->list.vector().back();
	};
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
	type["draw"] = SokuLib::union_cast<void (FakeCharacterManager::*)()>(0x438d20);


	auto type2 = state.new_usertype<SokuLib::ObjectManager>("Object");

	type2["position"] = &SokuLib::ObjectManager::position;
	type2["speed"] = &SokuLib::ObjectManager::speed;
	type2["gravity"] = &SokuLib::ObjectManager::gravity;
	type2["direction"] = &SokuLib::ObjectManager::direction;
	type2["renderInfos"] = &SokuLib::ObjectManager::renderInfos;
	type2["action"] = &SokuLib::ObjectManager::action;
	type2["actionBlockId"] = &SokuLib::ObjectManager::actionBlockId;
	type2["animationCounter"] = &SokuLib::ObjectManager::animationCounter;
	type2["animationSubFrame"] = &SokuLib::ObjectManager::animationSubFrame;
	type2["frameCount"] = &SokuLib::ObjectManager::frameCount;
	type2["animationCounterMax"] = &SokuLib::ObjectManager::animationCounterMax;
	type2["animationSubFrameMax"] = &SokuLib::ObjectManager::animationSubFrameMax;
	type2["image"] = &SokuLib::ObjectManager::image;
	type2["frameData"] = &SokuLib::ObjectManager::frameData;
	type2["frameDataReader"] = &SokuLib::ObjectManager::frameDataReader;
	//type2["soundTable"] = &SokuLib::ObjectManager::soundTable;
	type2["hp"] = &SokuLib::ObjectManager::hp;
	type2["superarmorDamageTaken"] = &SokuLib::ObjectManager::superarmorDamageTaken;
	type2["hitCount"] = &SokuLib::ObjectManager::hitCount;
	type2["hitstop"] = &SokuLib::ObjectManager::hitstop;
	type2["hitBoxCount"] = &SokuLib::ObjectManager::hitBoxCount;
	type2["hurtBoxCount"] = &SokuLib::ObjectManager::hurtBoxCount;
	type2["hurtBoxes"] = &SokuLib::ObjectManager::hurtBoxes;
	type2["hitBoxes"] = &SokuLib::ObjectManager::hitBoxes;
	//ty2pe["hitBoxesRotation"] = &SokuLib::ObjectManager::hitBoxesRotation;
	//ty2pe["hurtBoxesRotation"] = &SokuLib::ObjectManager::hurtBoxesRotation;
	type2["initAnimation"] = &SokuLib::ObjectManager::animate;
	type2["animate"] = &SokuLib::ObjectManager::animate2;
	type2["updateAnimation"] = &SokuLib::ObjectManager::doAnimation;
	type2["draw"] = SokuLib::union_cast<void (SokuLib::ObjectManager::*)()>(0x438d20);
}