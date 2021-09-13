//
// Created by PinkySmile on 13/09/2021.
//

#include <sol/sol.hpp>
#include "FakeChrMgr.hpp"

static std::vector<FakeCharacterManager *> _created;

void pushFakeChrMgrLuaTable(sol::state &state)
{
	auto type = state.new_usertype<FakeCharacterManager>("Character");

	type["new"] = [](SokuLib::PlayerInfo &playerInfo){
		FakeCharacterManager *obj[0xB];

		if (SokuLib::sceneId != SokuLib::SCENE_BATTLE)
			throw sol::error("You need to init the character when in battle.");
		((void (__thiscall *)(FakeCharacterManager **, bool, SokuLib::PlayerInfo &))0x46da40)(obj, false, playerInfo);
		(*(void (__thiscall **)(FakeCharacterManager *))(*(int *)obj[0xA] + 0x44))(obj[0xA]);
		obj[0xA]->opponent = (FakeCharacterManager *)&SokuLib::getBattleMgr().rightCharacterManager;
		_created.push_back(obj[0xA]);
		return obj[0xA];
	};
	type[sol::meta_function::garbage_collect] = [](FakeCharacterManager *obj){
		auto it = std::find(_created.begin(), _created.end(), obj);

		if (it == _created.end())
			return;
		printf("Deleting %p\n", obj);
		_created.erase(it);
		(*(void (__thiscall **)(FakeCharacterManager *, char))obj->offset_0x000)(obj, 0);
		SokuLib::Delete(obj);
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
	type["opponent"] = &FakeCharacterManager::opponent;
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
	type["draw"] = SokuLib::union_cast<void (FakeCharacterManager::*)()>(0x438d20);
}