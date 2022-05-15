//
// Created by PinkySmile on 23/07/2021.
//

#include <dinput.h>
#include "ComboTrial.hpp"
#include "Actions.hpp"
#include "Menu.hpp"
#include "Pack.hpp"

#ifndef _DEBUG
#define puts(...)
#define printf(...)
#endif

#define BOTTOM_POS_PAUSE 320
#define BOTTOM_POS 332
#define SIZE 29
#define PAUSE_NB_BUTTONS 6

#define dxLockRect(texture, ...) (*((int (__stdcall**)(void*, int, D3DLOCKED_RECT*, int, int))(*(int*)texture + 0x4c)))(texture, __VA_ARGS__)
#define dxUnlockRect(texture, ...) (*((int (__stdcall**)(void*, int))(*(int*)texture + 0x50)))(texture, __VA_ARGS__)

static SokuLib::KeyInput empty{0, 0, 0, 0, 0, 0, 0, 0};

ComboTrial::ComboTrial(const char *folder, SokuLib::Character player, const nlohmann::json &json) :
	Trial(folder, json)
{
	int text;

	if (!json.contains("score") || !json["score"].is_array())
		throw std::invalid_argument("The \"score\" field is not present or invalid.");
	if (json["score"].size() != 4)
		throw std::invalid_argument("The \"score\" field doesn't have exactly 4 elements.");
	if (!json.contains("expected") || !json["expected"].is_string())
		throw std::invalid_argument("The \"expected\" field is not present or invalid.");
	if (json.contains("hint") && !json["hint"].is_array())
		throw std::invalid_argument("The \"hint\" field is not valid.");
	if (!json["player"].contains("pos") || !json["player"]["pos"].is_number())
		throw std::invalid_argument(R"(The field "pos" in the "player" field is not present or invalid.)");
	if (!json["dummy"].contains("pos") || !json["dummy"]["pos"].is_object())
		throw std::invalid_argument(R"(The field "pos" in the "dummy" field is not present or invalid.)");
	if (!json["dummy"]["pos"].contains("x") || !json["dummy"]["pos"]["x"].is_number())
		throw std::invalid_argument(R"(The field "x" of the field "pos" in the "dummy" field is not present or invalid.)");
	if (!json["dummy"]["pos"].contains("y") || !json["dummy"]["pos"]["y"].is_number())
		throw std::invalid_argument(R"(The field "y" of the field "pos" in the "dummy" field is not present or invalid.)");

	if (json.contains("inputs") && json["inputs"].is_array()) {
		int i = 1;

		for (auto &input : json["inputs"]) {
			if (!input.is_array())
				throw std::invalid_argument("Input macro item #" + std::to_string(i) + " is not an array");
			if (input.size() < 9)
				throw std::invalid_argument("Input macro item #" + std::to_string(i) + " does not contain 9 elements");
			if (!input[0].is_number())
				throw std::invalid_argument("Input macro item #" + std::to_string(i) + ": Elem #1 is not a number");
			if (!input[1].is_number())
				throw std::invalid_argument("Input macro item #" + std::to_string(i) + ": Elem #2 is not a number");
			if (!input[2].is_number())
				throw std::invalid_argument("Input macro item #" + std::to_string(i) + ": Elem #3 is not a number");
			if (!input[3].is_number())
				throw std::invalid_argument("Input macro item #" + std::to_string(i) + ": Elem #4 is not a number");
			if (!input[4].is_number())
				throw std::invalid_argument("Input macro item #" + std::to_string(i) + ": Elem #5 is not a number");
			if (!input[5].is_number())
				throw std::invalid_argument("Input macro item #" + std::to_string(i) + ": Elem #6 is not a number");
			if (!input[6].is_number())
				throw std::invalid_argument("Input macro item #" + std::to_string(i) + ": Elem #7 is not a number");
			if (!input[7].is_number())
				throw std::invalid_argument("Input macro item #" + std::to_string(i) + ": Elem #8 is not a number");
			if (!input[8].is_number())
				throw std::invalid_argument("Input macro item #" + std::to_string(i) + ": Elem #9 is not a number");
			this->_previewInputs.emplace_back(SokuLib::KeyInput{
				input[0],
				input[1],
				input[2],
				input[3],
				input[4],
				input[5],
				input[6],
				input[7],
			}, input[8].get<int>());
		}
	}

	if (json.contains("dolls") && json["dolls"].is_array()) {
		if (player != SokuLib::CHARACTER_ALICE)
			throw std::invalid_argument("Can only specify doll placement for alice");
		for (int i = 0; i < json["dolls"].size(); i++) {
			auto &obj = json["dolls"][i];

			if (!obj.is_object())
				throw std::invalid_argument("Error in doll #" + std::to_string(i) + ": Value is not an object");
			if (!obj.contains("x") && obj["x"].is_number())
				throw std::invalid_argument("Error in doll #" + std::to_string(i) + ": x is not a number");
			if (!obj.contains("y") && obj["y"].is_number())
				throw std::invalid_argument("Error in doll #" + std::to_string(i) + ": y is not a number");
			if (!obj.contains("dir") && obj["dir"].is_number())
				throw std::invalid_argument("Error in doll #" + std::to_string(i) + ": Direction is not a number");
			this->_dolls.push_back({SokuLib::Vector2f{obj["x"], obj["y"]}, obj["dir"].get<SokuLib::Direction>()});
		}
	}

	this->_jump = json["dummy"].contains("jump") && json["dummy"]["jump"].is_boolean() && json["dummy"]["jump"].get<bool>();
	this->_failTimer = json.contains("fail_timer") && json["fail_timer"].is_number() ? json["fail_timer"].get<int>() : 60;
	this->_crouching = json["dummy"].contains("crouch") && json["dummy"]["crouch"].is_boolean() && json["dummy"]["crouch"].get<bool>();
	this->_leftWeather = !json["player"].contains("affected_by_weather") || !json["player"]["affected_by_weather"].is_boolean() || json["player"]["affected_by_weather"].get<bool>();
	this->_rightWeather = !json["dummy"].contains("affected_by_weather") || !json["dummy"]["affected_by_weather"].is_boolean() || json["dummy"]["affected_by_weather"].get<bool>();
	memset(&this->_skills, 0xFF, sizeof(this->_skills));
	if (json.contains("skills") && json["skills"].is_array() && json["skills"].size() == characterSkills[player].size()) {
		for (int i = 0; i < json["skills"].size(); i++) {
			auto &arr = json["skills"][i];

			if (!arr.is_array() || arr.size() != 2 || !arr[0].is_number() || arr[0].get<int>() < 0 || arr[0].get<int>() > 2 || !arr[1].is_number()) {
				MessageBox(
					SokuLib::window,
					("Element #" + std::to_string(i) + " in the skill array was not valid and will be discarded").c_str(),
					"Invalid skill array",
					MB_ICONWARNING
				);
				this->_skills[i].notUsed = false;
				this->_skills[i].level = 0;
			} else {
				this->_skills[i + characterSkills[player].size() * arr[0].get<int>()].notUsed = false;
				this->_skills[i + characterSkills[player].size() * arr[0].get<int>()].level = arr[1];
			}
		}
	} else
		for (int i = 0; i < characterSkills[player].size(); i++) {
			this->_skills[i].notUsed = false;
			this->_skills[i].level = 0;
		}

	try {
		this->_hand = json.contains("hand") && json["hand"].is_array() ? json["hand"].get<std::vector<unsigned short>>() : std::vector<unsigned short>{};
	} catch (...) {
		throw std::invalid_argument("Hand contains invalid values");
	}

	for (auto card : this->_hand) {
		if (card <= 20)
			continue;
		for (int i = 0; i < SokuLib::leftPlayerInfo.effectiveDeck.size; i++)
			if (card == SokuLib::leftPlayerInfo.effectiveDeck[i])
				goto ok;
		throw std::invalid_argument("Player deck doesn't have card " + std::to_string(card));
	ok:
		continue;
	}

	this->_weather = json.contains("weather") && json["weather"].is_number() ? static_cast<SokuLib::Weather>(json["weather"].get<int>()) : SokuLib::WEATHER_CLEAR;
	if (json.contains("weather") && json["weather"].is_string()) {
		std::string weather = json["weather"];

		std::for_each(weather.begin(), weather.end(), [](char &c){ c = tolower(c); });

		auto it = weathers.find(weather);

		if (it == weathers.end())
			throw std::invalid_argument(weather + " is not a valid weather name");
		this->_weather = it->second;
	}

	this->_mpp           = json["player"].contains("mpp")            && json["player"]["mpp"].is_boolean()            && json["player"]["mpp"].get<bool>();
	this->_stones        = json["player"].contains("stones")         && json["player"]["stones"].is_boolean()         && json["player"]["stones"].get<bool>();
	this->_clones        = json["player"].contains("clones")         && json["player"]["clones"].is_boolean()         && json["player"]["clones"].get<bool>();
	this->_orerries      = json["player"].contains("orreries")       && json["player"]["orreries"].is_boolean()       && json["player"]["orreries"].get<bool>();
	this->_tickTimer     = json["player"].contains("tick_timer")     && json["player"]["tick_timer"].is_boolean()     && json["player"]["tick_timer"].get<bool>();
	this->_privateSquare = json["player"].contains("private_square") && json["player"]["private_square"].is_boolean() && json["player"]["private_square"].get<bool>();

	if (this->_mpp && player != SokuLib::CHARACTER_SUIKA)
		throw std::invalid_argument("Missing Purple Power is only allowed for Suika");
	if (this->_stones && player != SokuLib::CHARACTER_PATCHOULI)
		throw std::invalid_argument("Philosopher's Stones is only allowed for Patchouli");
	if (this->_orerries && player != SokuLib::CHARACTER_MARISA)
		throw std::invalid_argument("Orreries Sun is only allowed for Marisa");
	if (this->_privateSquare && player != SokuLib::CHARACTER_SAKUYA)
		throw std::invalid_argument("Private Square is only allowed for Sakuya");
	if (this->_clones && player != SokuLib::CHARACTER_YOUMU)
		throw std::invalid_argument("Spirit Clone is only allowed for Youmu");

	this->_disableLimit = json.contains("disable_limit") && json["disable_limit"].is_boolean() && json["disable_limit"].get<bool>();
	this->_uniformCardCost = json.contains("uniform_card_cost") && json["uniform_card_cost"].is_number() ? json["uniform_card_cost"].get<int>() : 0;
	this->_playerStartPos = json["player"]["pos"];
	this->_dummyStartPos.x = json["dummy"]["pos"]["x"];
	this->_dummyStartPos.y = json["dummy"]["pos"]["y"];
	this->_loadExpected(json["expected"]);
	if (this->_jump && this->_dummyStartPos.y != 0)
		throw std::invalid_argument("Cannot specify a non zero y if the dummy is jumping");
	if (this->_jump && this->_crouching)
		throw std::invalid_argument("The dummy cannot crouch and jump at the time!");

	this->_gear.texture.loadFromResource(myModule, MAKEINTRESOURCE(12));
	this->_gear.setPosition({559, 70});
	this->_gear.setSize({64, 64});
	this->_gear.rect.left = 0;
	this->_gear.rect.top = 0;
	this->_gear.rect.width = this->_gear.texture.getSize().x;
	this->_gear.rect.height = this->_gear.texture.getSize().y;

	this->_gearShadow.texture.loadFromResource(myModule, MAKEINTRESOURCE(16));
	this->_gearShadow.setPosition({561, 72});
	this->_gearShadow.setSize({64, 64});
	this->_gearShadow.rect.left = 0;
	this->_gearShadow.rect.top = 0;
	this->_gearShadow.rect.width = this->_gearShadow.texture.getSize().x;
	this->_gearShadow.rect.height = this->_gearShadow.texture.getSize().y;

	this->_doll.texture.loadFromResource(myModule, MAKEINTRESOURCE(20));
	this->_doll.setPosition({577, 80});
	this->_doll.setSize({33, 46});
	this->_doll.setMirroring(true, false);
	this->_doll.rect.left = 0;
	this->_doll.rect.top = 0;
	this->_doll.rect.width = this->_doll.texture.getSize().x / 4;
	this->_doll.rect.height = this->_doll.texture.getSize().y;

	this->_attemptText.texture.createFromText("214a -> Review demo<br>Attempt #1", defaultFont10, {116, 24});
	this->_attemptText.setPosition({4, 58});
	this->_attemptText.setSize(this->_attemptText.texture.getSize());
	this->_attemptText.rect.left = 0;
	this->_attemptText.rect.top = 0;
	this->_attemptText.rect.width = this->_attemptText.texture.getSize().x;
	this->_attemptText.rect.height = this->_attemptText.texture.getSize().y;

	SokuLib::Vector2i realSize;

	this->_name.texture.createFromText(loadedPacks[currentPack]->scenarios[currentEntry]->nameStr.c_str(), defaultFont16, {412, 27}, &realSize);
	this->_name.setPosition({static_cast<int>(392 + 212 / 2 - realSize.x / 2), 100});
	this->_name.setSize(realSize.to<unsigned>());
	this->_name.rect.left = 0;
	this->_name.rect.top = 0;
	this->_name.rect.width = realSize.x;
	this->_name.rect.height = realSize.y;

	ScorePrerequisites *old = nullptr;

	if (!json.contains("score") || !json["score"].is_array() || json["score"].size() != 4)
		for (int i = 0; i < 4; i++) {
			this->_scores.emplace_back(nlohmann::json{}, old);
			old = &this->_scores.back();
		}
	else
		for (auto &j : json["score"])
			try {
				this->_scores.emplace_back(j, old);
				old = &this->_scores.back();
			} catch (std::exception &e) {
				throw std::invalid_argument("Score element #" + std::to_string(this->_scores.size()) + " is invalid : " + e.what());
			}

	for (int i = 0; i < 4; i++) {
		auto &prerequ = this->_scores[i];

		this->_parts[i]._score.texture.loadFromGame("data/infoeffect/result/rankFont.bmp");
		this->_parts[i]._score.setPosition({92 + 141 * i, BOTTOM_POS_PAUSE});
		this->_parts[i]._score.setSize({32, 32});
		this->_parts[i]._score.rect.left = i * this->_parts[i]._score.texture.getSize().x / 4;
		this->_parts[i]._score.rect.width = this->_parts[i]._score.texture.getSize().x / 4;
		this->_parts[i]._score.rect.height = this->_parts[i]._score.texture.getSize().y;

		this->_parts[i]._attempts.texture.createFromText(
			prerequ.attempts == -1 ? "" : ("At most " + std::to_string(prerequ.attempts) + " attempt" + (prerequ.attempts == 1 ? "" : "s")).c_str(),
			defaultFont12,
			{130, 20},
			&realSize
		);
		this->_parts[i]._attempts.setPosition({76 + 141 * i - realSize.x / 2 + 32, BOTTOM_POS_PAUSE + SIZE + 14 * 3});
		this->_parts[i]._attempts.setSize(this->_parts[i]._attempts.texture.getSize());
		this->_parts[i]._attempts.rect.width = this->_parts[i]._attempts.texture.getSize().x;
		this->_parts[i]._attempts.rect.height = this->_parts[i]._attempts.texture.getSize().y;

		this->_parts[i]._hits.texture.createFromText(
			("At least " + std::to_string(prerequ.hits) + " hit" + (prerequ.hits == 1 ? "" : "s")).c_str(),
			defaultFont12,
			{130, 20},
			&realSize
		);
		this->_parts[i]._hits.setPosition({76 + 141 * i - realSize.x / 2 + 32, BOTTOM_POS_PAUSE + SIZE + 14 * 0});
		this->_parts[i]._hits.setSize(this->_parts[i]._hits.texture.getSize());
		this->_parts[i]._hits.rect.width = this->_parts[i]._hits.texture.getSize().x;
		this->_parts[i]._hits.rect.height = this->_parts[i]._hits.texture.getSize().y;

		this->_parts[i]._damages.texture.createFromText(
			("At least " + std::to_string(prerequ.damage) + " damage" + (prerequ.damage <= 1 ? "" : "s")).c_str(),
			defaultFont12,
			{130, 20},
			&realSize
		);
		this->_parts[i]._damages.setPosition({76 + 141 * i - realSize.x / 2 + 32, BOTTOM_POS_PAUSE + SIZE + 14 * 1});
		this->_parts[i]._damages.setSize(this->_parts[i]._damages.texture.getSize());
		this->_parts[i]._damages.rect.width = this->_parts[i]._damages.texture.getSize().x;
		this->_parts[i]._damages.rect.height = this->_parts[i]._damages.texture.getSize().y;

		this->_parts[i]._limit.texture.createFromText(
			("At least " + std::to_string(prerequ.minLimit) + "% limit").c_str(),
			defaultFont12,
			{130, 20},
			&realSize
		);
		this->_parts[i]._limit.setPosition({76 + 141 * i - realSize.x / 2 + 32, BOTTOM_POS_PAUSE + SIZE + 14 * 2});
		this->_parts[i]._limit.setSize(this->_parts[i]._limit.texture.getSize());
		this->_parts[i]._limit.rect.width = this->_parts[i]._limit.texture.getSize().x;
		this->_parts[i]._limit.rect.height = this->_parts[i]._limit.texture.getSize().y;
	}
	if (this->_crouching && this->_dummyStartPos.y)
		MessageBox(
			SokuLib::window,
			"Warning: The field \"crouch\" from the dummy is set to true but the dummy is airborne so it cannot crouch.",
			"Incompatible parameters",
			MB_ICONWARNING
		);
	this->_loadPauseAssets();
}

void ____(int) {}

bool ComboTrial::update(bool &canHaveNextFrame)
{
	auto &battleMgr = SokuLib::getBattleMgr();
	//00439770
	if (this->_quit) {
		canHaveNextFrame = false;
		return true;
	}
	if (*(char*)0x89a88c == 2)
		return true;
	battleMgr.rightCharacterManager.nameHidden = true;
	if (!this->_introPlayed) {
		//SokuLib::displayedWeather = this->_weather;
		//SokuLib::activeWeather = this->_weather;
		canHaveNextFrame = false;//this->_firstFirst == 1;
		if (this->_firstFirst) {
			DWORD dwOldProtect;
			::VirtualProtect((void*)0x439789, 5, PAGE_EXECUTE_READWRITE, &dwOldProtect);
			auto oldFct = SokuLib::TamperNearJmpOpr(0x439789, ____);

			SokuLib::activateWeather(this->_weather, 1);
			SokuLib::TamperNearJmpOpr(0x439789, oldFct);
			::VirtualProtect((void*)0x439789, 5, dwOldProtect, &dwOldProtect);
			this->_firstFirst--;
		} else
			this->_introOnUpdate();
		if (this->_introPlayed)
			SokuLib::activeWeather = SokuLib::WEATHER_CLEAR;
		return false;
	}

	this->_rotation += 0.025;
	this->_dollAnim++;
	this->_dollAnim &= 0b11111;
	if (this->_freezeCounter) {
		canHaveNextFrame = (this->_freezeCounter % max((5 - this->_freezeCounter / 30), 1) == 0);
		this->_freezeCounter--;
		if (!this->_freezeCounter && this->_outroPlayed)
			SokuLib::activateMenu(new ComboTrialResult(*this));
		return !this->_freezeCounter && this->_outroPlayed;
	}

	if (!this->_leftWeather)
		battleMgr.leftCharacterManager.swordOfRaptureDebuffTimeLeft = 3;
	if (!this->_rightWeather)
		battleMgr.rightCharacterManager.swordOfRaptureDebuffTimeLeft = 3;
	if (!this->_outroPlayed && this->_finished && !this->_playingIntro) {
		auto ptr = *(unsigned *)(0x8985E8) + 0x16C + 0x18;
		auto ptr2 = *(unsigned *)(0x8985E8) + 0x190 + 0x18;

		// Hide the combo counter
		*(int *)ptr = 0x00;
		*(int *)ptr2 = 0x00;
		if (!this->_dummyHit) {
			if ((*reinterpret_cast<char **>(0x8985E8))[0x494] < 22) {
				(*reinterpret_cast<char **>(0x8985E8))[0x494]++;
				return false;
			}
			this->_outroOnUpdate();
			canHaveNextFrame = false;
			if (this->_outroPlayed)
				SokuLib::activateMenu(new ComboTrialResult(*this));
			return false;
		}
	} else if (this->_finished && !this->_playingIntro) {
		canHaveNextFrame = false;
		return true;
	}

	if ((*reinterpret_cast<char **>(0x8985E8))[0x494])
		(*reinterpret_cast<char **>(0x8985E8))[0x494]--;

	if (this->_isStart) {
		this->_initGameStart();
		return false;
	}

	if (this->_tickTimer);
	else if (this->_mpp)
		battleMgr.leftCharacterManager.missingPurplePowerTimeLeft = 480;
	else if (this->_stones)
		battleMgr.leftCharacterManager.philosophersStoneTime = 900;
	else if (this->_orerries)
		battleMgr.leftCharacterManager.orreriesTimeLeft = 600;
	else if (this->_privateSquare)
		battleMgr.leftCharacterManager.privateSquare = 300 - (battleMgr.leftCharacterManager.privateSquare & 1);
	else if (this->_clones)
		battleMgr.leftCharacterManager.youmuCloneTimeLeft = 600;

	if (this->_waitCounter)
		this->_waitCounter--;
	else if (this->_playingIntro && this->_previewInputs.empty())
		this->_playIntro();
	else if (this->_actionCounter != this->_expectedActions.size()) {
		auto i = this->_actionCounter;

		while (i == this->_actionCounter || this->_expectedActions[i - 1]->optional) {
			if (i >= this->_expectedActions.size())
				break;
			for (auto act : this->_expectedActions[i]->actions)
				if (
					addCustomActions(battleMgr.leftCharacterManager, SokuLib::leftChar) == act &&
					isStartOfMove(act, battleMgr.leftCharacterManager, SokuLib::leftChar)
				) {
					this->_actionCounter = i + 1;
					goto checkFinish;
				}
			i++;
		}
	}

checkFinish:
	if (!this->_finished && this->_scores.front().met(this->_attempts)) {
		auto i = this->_actionCounter;

		while (i < this->_expectedActions.size()) {
			if (!this->_expectedActions[i]->optional)
				goto disableLimit;
			i++;
		}
		SokuLib::playSEWaveBuffer(44);
		if (!this->_playingIntro)
			this->_freezeCounter = 120;
		this->_finished = true;
		return false;
	}

disableLimit:
	if (this->_disableLimit) {
		battleMgr.leftCharacterManager.combo.limit = 0;
		battleMgr.rightCharacterManager.combo.limit = 0;
		battleMgr.leftCharacterManager.realLimit = 0;
		battleMgr.rightCharacterManager.realLimit = 0;
	}

	auto hit = battleMgr.rightCharacterManager.objectBase.action >= SokuLib::ACTION_STAND_GROUND_HIT_SMALL_HITSTUN &&
	           battleMgr.rightCharacterManager.objectBase.action <= SokuLib::ACTION_FORWARD_DASH;

	if (this->_playComboAfterIntro && !hit && battleMgr.leftCharacterManager.objectBase.action <= SokuLib::ACTION_STAND_GROUND_HIT_SMALL_HITSTUN) {
		this->_attempts--;
		this->_initGameStart();
		return false;
	}

	if (this->_actionCounter && !this->_dummyHit && !battleMgr.leftCharacterManager.timeStop && !battleMgr.rightCharacterManager.timeStop)
		this->_timer++;
	else
		this->_timer = 0;
	this->_isStart = this->_timer >= this->_failTimer;
	battleMgr.currentRound = 3;
	battleMgr.leftCharacterManager.score = 0;
	if (
		(SokuLib::activeWeather != this->_weather && this->_weather != SokuLib::WEATHER_AURORA) ||
		(this->_weather == SokuLib::WEATHER_AURORA && (SokuLib::displayedWeather != this->_weather || SokuLib::activeWeather == SokuLib::WEATHER_CLEAR))
	) {
		//if (SokuLib::activeWeather == SokuLib::WEATHER_CLEAR) {
		//	SokuLib::weatherCounter = this->_weather == SokuLib::WEATHER_CLEAR ? 0 : 999;
		//	SokuLib::displayedWeather = this->_weather;
		//} else
		//	SokuLib::weatherCounter = 0;
		SokuLib::activateWeather(this->_weather, 1);
	} else
		SokuLib::weatherCounter = weatherTimes[this->_weather];
	if (this->_dummyHit && !hit && (!this->_finished || this->_playingIntro))
		this->_isStart = true;
	else if (this->_dummyHit && !hit)
		this->_dummyHit = false;
	this->_dummyHit |= hit;
	if (!this->_dummyHit && !this->_finished) {
		battleMgr.rightCharacterManager.objectBase.position.x = (this->_mirror ? 1280 - this->_dummyStartPos.x : this->_dummyStartPos.x);
		if (!this->_jump) {
			battleMgr.rightCharacterManager.objectBase.position.y = this->_dummyStartPos.y;
			battleMgr.rightCharacterManager.objectBase.speed.y = 0;
		}
		if (battleMgr.leftCharacterManager.keyCombination._214a && !this->_playingIntro)
			this->_playComboAfterIntro = true;
	}
	return false;
}

void ComboTrial::render() const
{
	if (!this->_introPlayed)
		return this->_introOnRender();

	if (this->_finished && !this->_outroPlayed && !this->_dummyHit)
		return this->_outroOnRender();

	if (this->_finished && !this->_playingIntro)
		return;

	SokuLib::Vector2i pos = {120, 60};

	if (this->_playingIntro) {
		this->_gearShadow.setRotation(this->_rotation);
		this->_gearShadow.draw();

		this->_gear.setRotation(this->_rotation);
		this->_gear.draw();

		this->_doll.rect.left = (this->_dollAnim >> 3 & 0b11) * this->_doll.texture.getSize().x / 4;
		this->_doll.draw();
	} else
		this->_attemptText.draw();

	auto last = 0;

	for (int i = 0; i < this->_expectedActions.size(); i++) {
		auto &elem = this->_expectedActions[i];

		if (this->_actionCounter == i)
			elem->sprite.tint = SokuLib::DrawUtils::DxSokuColor{0x60, 0xFF, 0x60};
		else if (this->_actionCounter > i)
			elem->sprite.tint = SokuLib::DrawUtils::DxSokuColor{0x60, 0x60, 0x60};
		else if (elem->optional)
			elem->sprite.tint = SokuLib::DrawUtils::DxSokuColor{0xFF, 0xFF, 0x60};
		else {
			bool good = false;

			for (int j = i; j > 0 && this->_expectedActions[j - 1]->optional; j--)
				good |= (j - 1) == this->_actionCounter;

			if (good)
				elem->sprite.tint = SokuLib::DrawUtils::DxSokuColor{0x60, 0xFF, 0x60};
			else
				elem->sprite.tint = SokuLib::DrawUtils::DxSokuColor::White;
		}
		elem->sprite.setPosition(pos);
		elem->sprite.draw();
		pos.y += elem->sprite.getSize().y;
	}
}

int ComboTrial::getScore()
{
	int index = 0;

	while (index < this->_scores.size() && this->_scores[index].met(this->_attempts))
		index++;
	return index - 1;
}

void ComboTrial::_initGameStart()
{
	auto &battleMgr = SokuLib::getBattleMgr();
	float scale = this->_mirror ? -1.f : 1.f;
	float off = this->_mirror ? 1280.f : 0.f;

	if (this->_currentDoll) {
		auto obj = battleMgr.leftCharacterManager.objects.list.vector()[battleMgr.leftCharacterManager.objects.list.size - 2];

		obj->action = static_cast<SokuLib::Action>(805);
		obj->animate();
		while (obj->actionBlockId != 8)
			obj->animate2();
		obj->position = this->_dolls[this->_currentDoll - 1].pos;
		obj->direction = this->_dolls[this->_currentDoll - 1].dir;
		obj->position.x *= scale;
		obj->position.x += off;
		obj->direction = static_cast<SokuLib::Direction>(static_cast<int>(obj->direction * scale));
		battleMgr.leftCharacterManager.aliceDollCount = 0;
	} else if (SokuLib::leftChar == SokuLib::CHARACTER_ALICE) {
		for (auto &obj : battleMgr.leftCharacterManager.objects.list.vector()) {
			if (
				(obj->action == 805 && obj->image->number >= 304 && obj->image->number <= 313) || //This is Alice's doll (C)
				(obj->action == 825 && obj->image->number >= 322 && obj->image->number <= 325)    //This is Alice's doll (d22)
			) {
				obj->action = static_cast<SokuLib::Action>(805);
				obj->animate();
				obj->actionBlockId = 7;
			}
		}
	}
	if (this->_currentDoll != this->_dolls.size()) {
		battleMgr.leftCharacterManager.objectBase.action = SokuLib::ACTION_5C;
		battleMgr.leftCharacterManager.objectBase.animate();
		while (battleMgr.leftCharacterManager.objectBase.frameCount != 9)
			battleMgr.leftCharacterManager.objectBase.doAnimation();
		this->_lastSize = battleMgr.leftCharacterManager.objects.list.size;
		this->_currentDoll++;
		return;
	}
	this->_waitCounter = 0;
	if (this->_first)
		this->_waitCounter = 180;
	else if (!this->_playingIntro)
		this->_attempts++;

	this->_currentDoll = 0;
	this->_attemptText.texture.createFromText(("214a -> Review demo<br>Attempt #" + std::to_string(this->_attempts + 1)).c_str(), defaultFont10, {116, 30});
	this->_isStart = false;
	this->_dummyHit = false;
	this->_finished = false;
	this->_playingIntro = this->_playComboAfterIntro;
	this->_playComboAfterIntro = false;
	this->_actionCounter = 0;
	this->_first = false;
	if (this->_playingIntro)
		this->_waitCounter += 30;

	this->_firstFirst = 0;
	battleMgr.leftCharacterManager.combo.limit = 0;
	battleMgr.leftCharacterManager.combo.damages = 0;
	battleMgr.leftCharacterManager.combo.nbHits = 0;
	battleMgr.leftCharacterManager.combo.rate = 0;

	auto ptr = *(unsigned *)(0x8985E8) + 0x16C + 0x18;
	auto ptr2 = *(unsigned *)(0x8985E8) + 0x190 + 0x18;

	// Hide the combo counter
	*(int *)ptr = 0x00;
	*(int *)ptr2 = 0x00;
	battleMgr.leftCharacterManager.cardGauge = 0;
	battleMgr.leftCharacterManager.hand.size = 0;
	battleMgr.leftCharacterManager.cardCount = 0;
	for (auto card : this->_hand) {
		auto obj = battleMgr.leftCharacterManager.addCard(card);

		if (this->_uniformCardCost)
			obj->cost = this->_uniformCardCost;
	}

	battleMgr.leftCharacterManager.objectBase.hp = 10000;
	battleMgr.leftCharacterManager.currentSpirit = 1000;
	battleMgr.leftCharacterManager.maxSpirit = 1000;

	if (!this->_mpp && SokuLib::leftChar == SokuLib::CHARACTER_SUIKA)
		battleMgr.leftCharacterManager.missingPurplePowerTimeLeft = !!battleMgr.leftCharacterManager.missingPurplePowerTimeLeft;
	else if (!this->_stones && SokuLib::leftChar == SokuLib::CHARACTER_PATCHOULI)
		battleMgr.leftCharacterManager.philosophersStoneTime = !!battleMgr.leftCharacterManager.philosophersStoneTime;
	else if (!this->_orerries && SokuLib::leftChar == SokuLib::CHARACTER_MARISA)
		battleMgr.leftCharacterManager.orreriesTimeLeft = !!battleMgr.leftCharacterManager.orreriesTimeLeft;
	else if (!this->_privateSquare && SokuLib::leftChar == SokuLib::CHARACTER_SAKUYA)
		battleMgr.leftCharacterManager.privateSquare = !!battleMgr.leftCharacterManager.privateSquare;
	else if (!this->_clones && SokuLib::leftChar == SokuLib::CHARACTER_YOUMU)
		battleMgr.leftCharacterManager.youmuCloneTimeLeft = !!battleMgr.leftCharacterManager.youmuCloneTimeLeft;
	if (this->_mpp && SokuLib::leftChar == SokuLib::CHARACTER_SUIKA) {
		puts("Init MPP");
		battleMgr.leftCharacterManager.objectBase.action = SokuLib::ACTION_USING_SC_ID_205;
		battleMgr.leftCharacterManager.objectBase.animate();
		while (battleMgr.leftCharacterManager.objectBase.frameCount != 80)
			battleMgr.leftCharacterManager.objectBase.doAnimation();
	} else if (this->_stones && SokuLib::leftChar == SokuLib::CHARACTER_PATCHOULI) {
		puts("Init Philosophers' Stone");
		if (battleMgr.leftCharacterManager.philosophersStoneTime <= 1 || this->_tickTimer)
			battleMgr.leftCharacterManager.objectBase.action = SokuLib::ACTION_USING_SC_ID_205;
		else
			battleMgr.leftCharacterManager.objectBase.action = SokuLib::ACTION_IDLE;
		battleMgr.leftCharacterManager.objectBase.animate();
		battleMgr.leftCharacterManager.objectBase.doAnimation();
	} else if (this->_orerries && SokuLib::leftChar == SokuLib::CHARACTER_MARISA) {
		puts("Init Orreries");
		if (battleMgr.leftCharacterManager.orreriesTimeLeft <= 1 || this->_tickTimer)
			battleMgr.leftCharacterManager.objectBase.action = SokuLib::ACTION_USING_SC_ID_215;
		else
			battleMgr.leftCharacterManager.objectBase.action = SokuLib::ACTION_IDLE;
		battleMgr.leftCharacterManager.objectBase.animate();
		battleMgr.leftCharacterManager.objectBase.doAnimation();
	} else if (this->_privateSquare && SokuLib::leftChar == SokuLib::CHARACTER_SAKUYA) {
		puts("Init Private Square");
		if (battleMgr.leftCharacterManager.privateSquare <= 1 || this->_tickTimer)
			battleMgr.leftCharacterManager.objectBase.action = SokuLib::ACTION_USING_SC_ID_201;
		else
			battleMgr.leftCharacterManager.objectBase.action = SokuLib::ACTION_IDLE;
		battleMgr.leftCharacterManager.objectBase.animate();
		battleMgr.leftCharacterManager.objectBase.doAnimation();
	} else if (this->_clones && SokuLib::leftChar == SokuLib::CHARACTER_YOUMU) {
		puts("Init Clones");
		if (battleMgr.leftCharacterManager.youmuCloneTimeLeft <= 1 || this->_tickTimer)
			battleMgr.leftCharacterManager.objectBase.action = SokuLib::ACTION_USING_SC_ID_205;
		else
			battleMgr.leftCharacterManager.objectBase.action = SokuLib::ACTION_IDLE;
		battleMgr.leftCharacterManager.objectBase.animate();
		battleMgr.leftCharacterManager.objectBase.doAnimation();
	} else {
		battleMgr.leftCharacterManager.objectBase.action = SokuLib::ACTION_IDLE;
		battleMgr.leftCharacterManager.objectBase.animate();
	}
	battleMgr.leftCharacterManager.objectBase.position.x = this->_playerStartPos * scale + off;
	battleMgr.leftCharacterManager.objectBase.position.y = 0;
	battleMgr.leftCharacterManager.objectBase.speed.x = 0;
	battleMgr.leftCharacterManager.objectBase.speed.y = 0;
	battleMgr.leftCharacterManager.objectBase.renderInfos.xRotation = 0;
	battleMgr.leftCharacterManager.objectBase.renderInfos.yRotation = 0;
	battleMgr.leftCharacterManager.objectBase.renderInfos.zRotation = 0;
	battleMgr.leftCharacterManager.objectBase.renderInfos.scale.x = 1;
	battleMgr.leftCharacterManager.objectBase.renderInfos.scale.y = 1;
	if (SokuLib::leftChar == SokuLib::CHARACTER_SANAE) {
		battleMgr.leftCharacterManager.suwakoTimeLeft = 0;
		battleMgr.leftCharacterManager.kanakoTimeLeft = 0;
	}
	//(*(void (__thiscall **)(SokuLib::ObjListManager &, int))&*battleMgr.leftCharacterManager.objects.offset_0x00)(battleMgr.leftCharacterManager.objects, 0);
	//battleMgr.leftCharacterManager.objects;
	memcpy(&battleMgr.leftCharacterManager.skillMap, &this->_skills, sizeof(this->_skills));
	battleMgr.leftCharacterManager.aliceDollCount = this->_dolls.size();

	battleMgr.rightCharacterManager.objectBase.hp = 10000;
	battleMgr.rightCharacterManager.currentSpirit = 1000;
	battleMgr.rightCharacterManager.maxSpirit = 1000;
	battleMgr.rightCharacterManager.objectBase.renderInfos.xRotation = 0;
	battleMgr.rightCharacterManager.objectBase.renderInfos.yRotation = 0;
	battleMgr.rightCharacterManager.objectBase.renderInfos.zRotation = 0;
	if (this->_dummyStartPos.y == 0)
		battleMgr.rightCharacterManager.objectBase.action = this->_crouching ? SokuLib::ACTION_CROUCHED : SokuLib::ACTION_IDLE;
	else
		battleMgr.rightCharacterManager.objectBase.action = SokuLib::ACTION_FALLING;
	battleMgr.rightCharacterManager.objectBase.animate();
	battleMgr.rightCharacterManager.objectBase.position.x = this->_dummyStartPos.x * scale + off;
	battleMgr.rightCharacterManager.objectBase.position.y = this->_dummyStartPos.y;
	battleMgr.rightCharacterManager.objectBase.speed.x = 0;
	battleMgr.rightCharacterManager.objectBase.speed.y = 0;
	battleMgr.rightCharacterManager.objectBase.animate();

	battleMgr.leftCharacterManager.objectBase.direction =
		battleMgr.rightCharacterManager.objectBase.position.x > battleMgr.leftCharacterManager.objectBase.position.x ?
		SokuLib::RIGHT : SokuLib::LEFT;
	battleMgr.rightCharacterManager.objectBase.direction =
		battleMgr.rightCharacterManager.objectBase.position.x > battleMgr.leftCharacterManager.objectBase.position.x ?
		SokuLib::LEFT : SokuLib::RIGHT;
}

void ComboTrial::_loadExpected(const std::string &expected)
{
	bool par = false;
	char last = ' ';

	this->_expectedActions.clear();
	this->_expectedActions.emplace_back(new SpecialAction());
	for (auto c : expected) {
		par |= c == '(';
		par &= c != ')';
		if (!par && c == ' ') {
			if (last != ' ')
				this->_expectedActions.emplace_back(new SpecialAction());
		} else
			this->_expectedActions.back()->name += c;
		last = c;
	}
	for (auto &action : this->_expectedActions)
		action->parse();
}

void ComboTrial::_playIntro()
{
	if (this->_actionCounter == this->_expectedActions.size())
		return;

	auto &battleMgr = SokuLib::getBattleMgr();
	auto &arr = this->_expectedActions[this->_actionCounter];

	if (this->_actionWaitCounter < arr->delay) {
		this->_actionWaitCounter++;
		return;
	}
	arr->counter = (arr->counter + 1) % arr->inputs.size();
	if (
		addCustomActions(battleMgr.leftCharacterManager, SokuLib::leftChar) == arr->actions[0] &&
		isStartOfMove(arr->actions[0], battleMgr.leftCharacterManager, SokuLib::leftChar)
	) {
		if (arr->chargeTime) {
			arr->chargeCounter++;
		} else {
			arr->counter = 0;
			this->_actionWaitCounter = 0;
			this->_actionCounter++;
		}
	}
	if (arr->chargeCounter) {
		if (arr->chargeCounter == arr->chargeTime) {
			arr->counter = 0;
			arr->chargeCounter = 0;
			this->_actionWaitCounter = 0;
			this->_actionCounter++;
		} else
			arr->chargeCounter++;
	}
}

void ComboTrial::editPlayerInputs(SokuLib::KeyInput &originalInputs)
{
	if (this->_playingIntro) {
		if (this->_waitCounter)
			return static_cast<void>(memset(&originalInputs, 0, sizeof(originalInputs)));

		if (this->_previewInputs.empty()) {
			if (this->_actionCounter == this->_expectedActions.size())
				return static_cast<void>(memset(&originalInputs, 0, sizeof(originalInputs)));

			auto &arr = this->_expectedActions[this->_actionCounter];

			if (arr->delay > this->_actionWaitCounter)
				return static_cast<void>(memset(&originalInputs, 0, sizeof(originalInputs)));
			if (arr->chargeCounter == 0)
				originalInputs = arr->inputs[arr->counter];
			else {
				originalInputs = arr->inputs.back();
				originalInputs.a *= 2;
				originalInputs.b *= 2;
				originalInputs.c *= 2;
				originalInputs.d *= 2;
				originalInputs.horizontalAxis *= 2;
				originalInputs.verticalAxis *= 2;
				originalInputs.changeCard *= 2;
				originalInputs.spellcard *= 2;
			}
			originalInputs.horizontalAxis *= SokuLib::getBattleMgr().leftCharacterManager.objectBase.direction;
		} else if (this->_inputPos < this->_previewInputs.size()) {
			memcpy(&originalInputs, &this->_previewInputs[this->_inputPos].first, sizeof(originalInputs));
			if (++this->_inputCurrent > this->_previewInputs[this->_inputPos].second) {
				this->_inputPos++;
				this->_inputCurrent = 0;
			}
		}
		return;
	}
	if (this->_playComboAfterIntro || this->_finished || this->_firstFirst) {
		memset(&originalInputs, 0, sizeof(originalInputs));
		return;
	}
}

SokuLib::KeyInput ComboTrial::getDummyInputs()
{
	return {0, this->_crouching - (this->_jump && this->_waitCounter < 30), 0, 0, 0, 0, 0, 0};
}

void ComboTrial::onMenuClosed(MenuAction action)
{
	switch (action) {
	case RETRY:
		this->_attempts = 0;
		this->_playingIntro = true;
		this->_actionCounter = this->_expectedActions.size();
		break;
	case GO_TO_NEXT_TRIAL:
	case RETURN_TO_TRIAL_SELECT:
		this->_next = SokuLib::SCENE_SELECT;
		break;
	case RETURN_TO_TITLE_SCREEN:
		this->_next = SokuLib::SCENE_TITLE;
		break;
	}
}

SokuLib::Scene ComboTrial::getNextScene()
{
	return this->_next;
}

SokuLib::Action ComboTrial::_getMoveAction(SokuLib::Character chr, std::string &myMove, std::string &name)
{
	auto error = false;

	try {
		auto act = actionsFromStr.at(myMove);
		auto pos = myMove.find("sc2");

		if (act >= SokuLib::ACTION_DEFAULT_SKILL1_B && act <= SokuLib::ACTION_ALT2_SKILL5_AIR_C) {
			auto input = characterSkills[chr].at(myMove[myMove.size() - 2] - '1');
			auto move = ((act - 500) / 20) * 3 + ((act - 500) % 20 / 5);
			auto skillName = characterCards[chr][100 + (move % 3) * characterSkills[chr].size() + move / 3].first;

			myMove.replace(myMove.find("skill"), 6, input);
			name = input + myMove.back() + " (" + (skillName.empty() ? "Unknown skill" : skillName) + ")";
			return act;
		}
		try {
			if (pos != std::string::npos) {
				auto scId = std::stoul(myMove.substr(pos + 2, 3));
				auto &entry = characterCards[chr].at(scId);

				name = myMove.substr(0, pos) + std::to_string(entry.second) + "SC (" + entry.first + ")";
				return act;
			}
		} catch (...) {
			printf("%u %s %s\n", myMove.size(), myMove.c_str(), myMove.substr(pos + 2, 3).c_str());
			error = true;
			if (error) {
				assert(false);
				throw;
			}
		}
		name = myMove;
		return act;
	} catch (...) {
		if (error)
			throw;
	}

	int start = myMove[0] == 'j';
	int realStart = (myMove[start] == 'a' ? 2 : 1) + start;
	auto move = myMove.substr(realStart, myMove.size() - realStart - 1);
	auto &inputs = characterSkills[chr];
	auto it = std::find(inputs.begin(), inputs.end(), move);

	if (it == inputs.end())
		throw std::exception();
	try {
		auto act = actionsFromStr.at(myMove.substr(0, realStart) + "skill" + std::to_string(it - inputs.begin() + 1) + myMove[myMove.size() - 1]);
		auto moveId = ((act - 500) / 20) * 3 + ((act - 500) % 20 / 5);
		auto skillName = characterCards[chr][100 + (moveId % 3) * characterSkills[chr].size() + moveId / 3].first;

		name = myMove.substr(realStart, myMove.size() - realStart - 1) + myMove.back() + " (" + (skillName.empty() ? "Unknown skill" : skillName) + ")";
		return act;
	} catch (std::exception &e) {
		printf("%s\n", (myMove.substr(0, realStart) + "skill" + std::to_string(it - inputs.begin() + 1) + myMove.back()).c_str());
		assert(false);
		throw;
	}
}

unsigned ComboTrial::getAttempt()
{
	return this->_attempts;
}

void ComboTrial::_loadPauseAssets()
{
	this->_title.texture.loadFromGame("data/menu/battle/100_Pause.bmp");
	this->_title.setSize(this->_title.texture.getSize());
	this->_title.rect.width = this->_title.texture.getSize().x;
	this->_title.rect.height = this->_title.texture.getSize().y;

	this->_pause.texture.loadFromResource(myModule, MAKEINTRESOURCE(448));
	this->_pause.setSize(this->_pause.texture.getSize());
	this->_pause.rect.width = this->_pause.getSize().x;
	this->_pause.rect.height = this->_pause.getSize().y;

	this->_tick.texture.loadFromResource(myModule, MAKEINTRESOURCE(68));
	this->_tick.setSize(this->_tick.texture.getSize());
	this->_tick.rect.width = this->_tick.getSize().x;
	this->_tick.rect.height = this->_tick.getSize().y;
}

int ComboTrial::pauseOnUpdate()
{
	if (this->_finished)
		return false;
	if (this->_playingIntro)
		return this->_initGameStart(), false;

	auto c = SokuLib::checkKeyOneshot(DIK_ESCAPE, false, false, false);

	if (!loadedPacks[currentPack]->scenarios[currentEntry]->loading && loadedPacks[currentPack]->scenarios[currentEntry]->preview)
		loadedPacks[currentPack]->scenarios[currentEntry]->preview->update();
	updateNoiseTexture();
	updateBandTexture();
	if (c || SokuLib::inputMgrs.input.b == 1)
		return SokuLib::playSEWaveBuffer(0x29), this->_scoreShown = this->_cursorPos = 0;
	if (std::abs(SokuLib::inputMgrs.input.verticalAxis) == 1 || (std::abs(SokuLib::inputMgrs.input.verticalAxis) > 36 && SokuLib::inputMgrs.input.verticalAxis % 6 == 0)) {
		this->_cursorPos += PAUSE_NB_BUTTONS + std::copysign(1, SokuLib::inputMgrs.input.verticalAxis);
		this->_cursorPos %= PAUSE_NB_BUTTONS;
		SokuLib::playSEWaveBuffer(0x27);
	}
	if (SokuLib::inputMgrs.input.a == 1)
		return this->_pauseOnKeyPressed();
	return true;
}

int ComboTrial::pauseOnRender() const
{
	if (this->_finished)
		return true;
	if (this->_playingIntro)
		return true;

	displaySokuCursor({
		61,
		static_cast<int>(129 + 32 * this->_cursorPos)
	}, {256, 16});
	this->_title.draw();
	this->_pause.draw();
	if (this->_mirror) {
		this->_tick.setPosition({42, 191});
		this->_tick.draw();
	}
	lockedNoise.draw();
	if (
		!loadedPacks[currentPack]->scenarios[currentEntry]->loading &&
		loadedPacks[currentPack]->scenarios[currentEntry]->preview &&
		loadedPacks[currentPack]->scenarios[currentEntry]->preview->isValid()
	)
		loadedPacks[currentPack]->scenarios[currentEntry]->preview->render();
	CRTBands.draw();
	frame.draw();
	if (!this->_scoreShown)
		loadedPacks[currentPack]->scenarios[currentEntry]->description.draw();
	else
		for (int i = 0; i < 4; i++) {
			this->_parts[i]._score.draw();
			this->_parts[i]._attempts.draw();
			this->_parts[i]._damages.draw();
			this->_parts[i]._hits.draw();
			this->_parts[i]._limit.draw();
		}
	this->_name.draw();
	return true;
}

bool ComboTrial::_pauseOnKeyPressed()
{
	switch (this->_cursorPos) {
	case 0:
		this->_scoreShown = false;
		SokuLib::playSEWaveBuffer(0x28);
		return false;
	case 1:
		if (this->_waitCounter)
			return SokuLib::playSEWaveBuffer(0x29), false;
		SokuLib::playSEWaveBuffer(0x28);
		this->_playComboAfterIntro = true;
		return false;
	case 2:
		if (this->_waitCounter)
			return SokuLib::playSEWaveBuffer(0x29), false;
		SokuLib::playSEWaveBuffer(0x28);
		this->_mirror = !this->_mirror;
		this->_attempts--;
		this->_initGameStart();
		return true;
	case 3:
		SokuLib::playSEWaveBuffer(0x28);
		this->_scoreShown = !this->_scoreShown;
		return true;
	case 4:
		SokuLib::playSEWaveBuffer(0x28);
		this->_quit = true;
		this->_next = SokuLib::SCENE_SELECT;
		return false;
	case 5:
		SokuLib::playSEWaveBuffer(0x28);
		this->_quit = true;
		this->_next = SokuLib::SCENE_TITLE;
		return false;
	}
	return true;
}

void ComboTrial::SpecialAction::parse()
{
	SokuLib::Vector2i realSize;
	std::string firstMove;
	std::string chargeStr;
	std::string delayStr;
	std::string move;
	std::string name;
	bool d = false;
	bool p = false;

	this->moveName.clear();
	for (auto c : this->name) {
		if (c == ':' && !p) {
			d = !d;
			if (d && !delayStr.empty())
				throw std::invalid_argument("Multiple delays found for move " + this->name);
		}
		if (c == '[' && !p && !d) {
			p = true;
			if (!chargeStr.empty())
				throw std::invalid_argument("Multiple charge timers found for move " + this->name);
		}
		if (c == '!') {
			this->optional = true;
			continue;
		}
		p &= c != ']' && !d;
		if (c == '[' || c == ']' || c == ':')
			continue;
		if (d)
			delayStr += c;
		else if (p)
			chargeStr += c;
		else
			this->moveName += std::tolower(c);
	}
	printf("Move %s -> %s [%s] :%s: -> ", this->name.c_str(), this->moveName.c_str(), chargeStr.c_str(), delayStr.c_str());

	this->actions.clear();
	try {
		size_t pos;
		std::string str = this->moveName;
		std::string real;

		do {
			pos = str.find('/');
			move = str.substr(0, pos);
			this->actions.push_back(_getMoveAction(SokuLib::leftChar, move, name));
			if (firstMove.empty())
				firstMove = move;
			if (pos != std::string::npos) {
				str = str.substr(pos + 1);
				printf("%i/", this->actions.back());
				real += name + "/";
			} else {
				printf("%i ", this->actions.back());
				real += name;
			}
		} while (pos != std::string::npos);
		this->moveName = real;
	} catch (std::exception &) {
		printf("INVALID\n");
		throw std::invalid_argument(move + " is not a recognized move name");
	}

	try {
		if (!chargeStr.empty())
			this->chargeTime = std::stoul(chargeStr);
		else
			this->chargeTime = 0;
		printf("%i ", this->chargeTime);
	} catch (std::exception &) {
		printf("INVALID\n");
		throw std::invalid_argument(chargeStr + " is not a valid charge timer");
	}

	try {
		if (!delayStr.empty())
			this->delay = std::stoul(delayStr);
		else
			this->delay = 0;
		printf("%i ", this->delay);
	} catch (std::exception &) {
		printf("INVALID ");
		throw std::invalid_argument(delayStr + " is not a valid delay");
	}

	try {
		this->inputs = actionStrToInputs.at(firstMove);
	} catch (...) {
		throw std::invalid_argument(firstMove + " is not yet implemented");
	}

	if (this->chargeTime) {
		int dig = std::isdigit(this->moveName.back());
		int index = this->moveName.size() - 2;

		while (index >= 0 && !dig && !std::isdigit(this->moveName[index]))
			index--;
		this->moveName = this->moveName.substr(0, index + 1) + "[" + this->moveName.substr(index + 1) + "]";
	}
	puts(this->moveName.c_str());
	this->sprite.texture.createFromText(this->moveName.c_str(), defaultFont16, {400, 20}, &realSize);
	this->sprite.setSize(realSize.to<unsigned>());
	this->sprite.rect.width = realSize.x;
	this->sprite.rect.height = realSize.y;
}

ComboTrial::ScorePrerequisites::ScorePrerequisites(const nlohmann::json &json, const ComboTrial::ScorePrerequisites *other)
{
	if (!other) {
		if (json.contains("max_attempts"))
			throw std::invalid_argument("First score element shouldn't have the field \"max_attempts\"");
		if (!json.contains("min_hits"))
			throw std::invalid_argument("First score element is missing the field \"min_hits\"");
		if (!json.contains("min_damage"))
			throw std::invalid_argument("First score element is missing the field \"min_damage\"");
		if (!json.contains("min_limit"))
			throw std::invalid_argument("First score element is missing the field \"min_limit\"");
	} else
		*this = *other;

	if (json.contains("max_attempts")) {
		if (!json["max_attempts"].is_number())
			throw std::invalid_argument("Field \"max_attempts\" is specified but not a number");
		this->attempts = json["max_attempts"];
		if (!this->attempts)
			throw std::invalid_argument("It's impossible to win without trying! THINK MARK! THINK!");
	}
	if (json.contains("min_hits")) {
		if (!json["min_hits"].is_number())
			throw std::invalid_argument("Field \"min_hits\" is specified but not a number");
		this->hits = json["min_hits"];
	}
	if (json.contains("min_damage")) {
		if (!json["min_damage"].is_number())
			throw std::invalid_argument("Field \"min_damage\" is specified but not a number");
		this->damage = json["min_damage"];
	}
	if (json.contains("min_limit")) {
		if (!json["min_limit"].is_number())
			throw std::invalid_argument("Field \"min_limit\" is specified but not a number");
		this->minLimit = json["min_limit"];
	}
}

bool ComboTrial::ScorePrerequisites::met(unsigned currentAttempts) const
{
	auto &battle = SokuLib::getBattleMgr();

	if (this->attempts <= currentAttempts)
		return false;
	if (this->hits > battle.leftCharacterManager.combo.nbHits)
		return false;
	if (this->damage > battle.leftCharacterManager.combo.damages)
		return false;
	if (this->minLimit > battle.leftCharacterManager.combo.limit)
		return false;
	//if (this->maxLimit < battle.leftCharacterManager.combo.limit)
	//	return false;
	return true;
}

ComboTrialResult::ComboTrialResult(ComboTrial &trial) :
	ResultMenu(trial.getScore()),
	_parent(trial)
{
	for (int i = 0; i < this->_parts.size(); i++)
		this->_parts[i].load(trial._attempts, trial._scores[i], i);
}

int ComboTrialResult::onRender()
{
	auto ret = ResultMenu::onRender();

	for (auto &part : this->_parts)
		part.draw(1);
	return ret;
}

void ComboTrialResult::ScorePart::load(int ttlattempts, const ComboTrial::ScorePrerequisites &prerequ, int index)
{
	SokuLib::Vector2i size;
	int ogIndex = index;

	this->_ttlAttempts = ttlattempts;
	this->_prerequ = prerequ;

	this->_score.texture.loadFromGame("data/infoeffect/result/rankFont.bmp");
	this->_score.setPosition({92 + 141 * index, BOTTOM_POS});
	this->_score.setSize({32, 32});
	this->_score.rect.left = ogIndex * this->_score.texture.getSize().x / 4;
	this->_score.rect.width = this->_score.texture.getSize().x / 4;
	this->_score.rect.height = this->_score.texture.getSize().y;

	this->_attempts.texture.createFromText(
		("At most " + std::to_string(prerequ.attempts) + " attempt" + (prerequ.attempts == 1 ? "" : "s")).c_str(),
		defaultFont12,
		{130, 20},
		&size
	);
	this->_attempts.setPosition({76 + 141 * index - size.x / 2 + 32, BOTTOM_POS + SIZE + 14 * 3});
	this->_attempts.setSize(this->_attempts.texture.getSize());
	this->_attempts.rect.width = this->_attempts.texture.getSize().x;
	this->_attempts.rect.height = this->_attempts.texture.getSize().y;

	this->_hits.texture.createFromText(
		("At least " + std::to_string(prerequ.hits) + " hit" + (prerequ.hits == 1 ? "" : "s")).c_str(),
		defaultFont12,
		{130, 20},
		&size
	);
	this->_hits.setPosition({76 + 141 * index - size.x / 2 + 32, BOTTOM_POS + SIZE + 14 * 0});
	this->_hits.setSize(this->_hits.texture.getSize());
	this->_hits.rect.width = this->_hits.texture.getSize().x;
	this->_hits.rect.height = this->_hits.texture.getSize().y;

	this->_damages.texture.createFromText(
		("At least " + std::to_string(prerequ.damage) + " damage" + (prerequ.damage <= 1 ? "" : "s")).c_str(),
		defaultFont12,
		{130, 20},
		&size
	);
	this->_damages.setPosition({76 + 141 * index - size.x / 2 + 32, BOTTOM_POS + SIZE + 14 * 1});
	this->_damages.setSize(this->_damages.texture.getSize());
	this->_damages.rect.width = this->_damages.texture.getSize().x;
	this->_damages.rect.height = this->_damages.texture.getSize().y;

	this->_limit.texture.createFromText(
		("At least " + std::to_string(prerequ.minLimit) + "% limit").c_str(),
		defaultFont12,
		{130, 20},
		&size
	);
	//if (prerequ.minLimit == prerequ.maxLimit)
	//	this->_limit.texture.createFromText(
	//		("Exactly " + std::to_string(prerequ.minLimit) + "% limit").c_str(),
	//		defaultFont12,
	//		{130, 20},
	//		&size
	//	);
	//else
	//	this->_limit.texture.createFromText(
	//		("Between " + std::to_string(prerequ.minLimit) + "% and " + std::to_string(prerequ.maxLimit) + "% limit").c_str(),
	//		defaultFont12,
	//		{130, 20},
	//		&size
	//	);
	this->_limit.setPosition({76 + 141 * index - size.x / 2 + 32, BOTTOM_POS + SIZE + 14 * 2});
	this->_limit.setSize(this->_limit.texture.getSize());
	this->_limit.rect.width = this->_limit.texture.getSize().x;
	this->_limit.rect.height = this->_limit.texture.getSize().y;
}

void ComboTrialResult::ScorePart::draw(float alpha)
{
	auto &battle= SokuLib::getBattleMgr();
	auto white  = SokuLib::DrawUtils::DxSokuColor::White * alpha;
	auto green  = SokuLib::DrawUtils::DxSokuColor::Green * alpha;
	auto red    = SokuLib::DrawUtils::DxSokuColor::Red   * alpha;

	this->_score.tint = white;
	this->_score.draw();
	this->_attempts.tint = this->_prerequ.attempts == -1 ? SokuLib::DrawUtils::DxSokuColor::Transparent : (this->_ttlAttempts < this->_prerequ.attempts ? green : red);
	this->_attempts.draw();
	this->_damages.tint = battle.leftCharacterManager.combo.damages >= this->_prerequ.damage ? green : red;
	this->_damages.draw();
	this->_hits.tint = battle.leftCharacterManager.combo.nbHits >= this->_prerequ.hits ? green : red;
	this->_hits.draw();
	//this->_limit.tint = battle.leftCharacterManager.combo.limit >= this->_prerequ.minLimit && battle.leftCharacterManager.combo.limit <= this->_prerequ.maxLimit ? green : red;
	this->_limit.tint = battle.leftCharacterManager.combo.limit >= this->_prerequ.minLimit ? green : red;
	this->_limit.draw();
}
