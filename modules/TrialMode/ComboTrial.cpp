//
// Created by PinkySmile on 23/07/2021.
//

#include "ComboTrial.hpp"
#include "Actions.hpp"

#ifndef _DEBUG
#define puts(...)
#define printf(...)
#endif

static SokuLib::KeyInput empty{0, 0, 0, 0, 0, 0, 0, 0};

ComboTrial::ComboTrial(SokuLib::Character player, const nlohmann::json &json)
{
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

	memset(&this->_skills, 0xFF, sizeof(this->_skills));
	if (json.contains("skills") && json["skills"].is_array() && json["skills"].size() == characterSkills[player].size()) {
		for (int i = 0; i < json["skills"].size(); i++) {
			auto &arr = json["skills"][i];

			if (!arr.is_array() || arr.size() != 2 || arr[0].get<int>() < 0 || arr[0].get<int>() > 2) {
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
	} catch (...) {}

	this->_weather = json.contains("weather") && json["weather"].is_number() ? static_cast<SokuLib::Weather>(json["weather"].get<int>()) : SokuLib::WEATHER_CLEAR;
	this->_disableLimit = json.contains("disable_limit") && json["disable_limit"].is_boolean() ? json["disable_limit"].get<bool>() : false;
	this->_uniformCardCost = json.contains("uniform_card_cost") && json["uniform_card_cost"].is_number() ? json["uniform_card_cost"].get<int>() : -1;
	this->_playComboAfterIntro = json.contains("play_combo_after_intro") && json["play_combo_after_intro"].is_boolean() ? json["play_combo_after_intro"].get<bool>() : false;
	this->_playerStartPos = json["player"]["pos"];
	this->_dummyStartPos.x = json["dummy"]["pos"]["x"];
	this->_dummyStartPos.y = json["dummy"]["pos"]["y"];
	this->_loadExpected(json["expected"]);
}

bool ComboTrial::update(bool &canHaveNextFrame)
{
	auto &battleMgr = SokuLib::getBattleMgr();

	if (this->_isStart) {
		this->_initGameStart();
		return false;
	}
	if (this->_waitCounter) {
		this->_waitCounter--;
	} else if (this->_playingIntro)
		this->_playIntro();

	if (this->_disableLimit) {
		battleMgr.leftCharacterManager.combo.limit = 0;
		battleMgr.rightCharacterManager.combo.limit = 0;
		battleMgr.leftCharacterManager.realLimit = 0;
		battleMgr.rightCharacterManager.realLimit = 0;
	}

	auto hit = battleMgr.rightCharacterManager.objectBase.action >= SokuLib::ACTION_STAND_GROUND_HIT_SMALL_HITSTUN &&
	           battleMgr.rightCharacterManager.objectBase.action <= SokuLib::ACTION_FORWARD_DASH;

	SokuLib::weatherCounter = this->_weather == SokuLib::WEATHER_CLEAR ? 0 : 999;
	if (this->_dummyHit && !hit)
		this->_isStart = true;
	this->_dummyHit |= hit;
	if (!this->_dummyHit) {
		battleMgr.rightCharacterManager.objectBase.position.x = this->_dummyStartPos.x;
		battleMgr.rightCharacterManager.objectBase.position.y = this->_dummyStartPos.y;
		if (battleMgr.rightCharacterManager.objectBase.action != SokuLib::ACTION_IDLE && battleMgr.rightCharacterManager.objectBase.action != SokuLib::ACTION_LANDING)
			battleMgr.rightCharacterManager.objectBase.action = SokuLib::ACTION_FALLING;
	}
	return false;
}

void ComboTrial::render()
{

}

int ComboTrial::getScore()
{
	return -1;
}

void ComboTrial::_initGameStart()
{
	auto &battleMgr = SokuLib::getBattleMgr();

	if (this->_first) {
		SokuLib::displayedWeather = this->_weather;
		SokuLib::activeWeather = SokuLib::WEATHER_CLEAR;
		SokuLib::weatherCounter = this->_weather == SokuLib::WEATHER_CLEAR ? 0 : 999;
		this->_waitCounter = 180;
	}

	this->_isStart = false;
	this->_dummyHit = false;
	this->_playingIntro = this->_playComboAfterIntro;
	this->_playComboAfterIntro = false;
	this->_actionCounter = 0;
	this->_first = false;
	if (this->_playingIntro)
		this->_waitCounter += 30;


	battleMgr.leftCharacterManager.objectBase.hp = 10000;
	battleMgr.leftCharacterManager.objectBase.action = SokuLib::ACTION_FALLING;
	battleMgr.leftCharacterManager.objectBase.position.x = this->_playerStartPos;
	battleMgr.leftCharacterManager.objectBase.position.y = 0;
	memcpy(&battleMgr.leftCharacterManager.skillMap, &this->_skills, sizeof(this->_skills));

	battleMgr.rightCharacterManager.objectBase.hp = 10000;
	battleMgr.rightCharacterManager.objectBase.action = SokuLib::ACTION_USING_SC_ID_200;
	battleMgr.rightCharacterManager.objectBase.actionBlockId = 0;
	battleMgr.rightCharacterManager.objectBase.frameCount = 0;
	battleMgr.rightCharacterManager.objectBase.animationSubFrame = 0;
	battleMgr.rightCharacterManager.objectBase.position.x = this->_dummyStartPos.x;
	battleMgr.rightCharacterManager.objectBase.position.y = this->_dummyStartPos.y;
	battleMgr.rightCharacterManager.objectBase.direction =
		battleMgr.rightCharacterManager.objectBase.position.x > battleMgr.leftCharacterManager.objectBase.position.x ?
		SokuLib::LEFT : SokuLib::RIGHT;
}

void ComboTrial::_loadExpected(const std::string &expected)
{
	bool par = false;
	char last = ' ';

	this->_exceptedActions.clear();
	this->_exceptedActions.emplace_back();
	for (auto c : expected) {
		par |= c == '(';
		par &= c != ')';
		if (!par && c == ' ') {
			if (last != ' ')
				this->_exceptedActions.emplace_back();
		} else
			this->_exceptedActions.back().name += c;
		last = c;
	}
	for (auto &action : this->_exceptedActions)
		action.parse();
}

void ComboTrial::_playIntro()
{
	if (this->_actionCounter == this->_exceptedActions.size())
		return;

	auto &battleMgr = SokuLib::getBattleMgr();
	auto &arr = this->_exceptedActions[this->_actionCounter];

	if (this->_actionWaitCounter < arr.delay) {
		this->_actionWaitCounter++;
		return;
	}
	arr.counter = (arr.counter + 1) % arr.inputs.size();
	if (battleMgr.leftCharacterManager.objectBase.action == arr.action) {
		arr.counter = 0;
		this->_actionWaitCounter = 0;
		this->_actionCounter++;
	}
}

void ComboTrial::editPlayerInputs(SokuLib::KeyInput &originalInputs)
{
	if (this->_playingIntro) {
		if (this->_actionCounter == this->_exceptedActions.size())
			return static_cast<void>(memset(&originalInputs, 0, sizeof(originalInputs)));
		if (this->_waitCounter)
			return static_cast<void>(memset(&originalInputs, 0, sizeof(originalInputs)));
		if (this->_exceptedActions[this->_actionCounter].delay > this->_actionWaitCounter)
			return static_cast<void>(memset(&originalInputs, 0, sizeof(originalInputs)));
		originalInputs = this->_exceptedActions[this->_actionCounter].inputs[this->_exceptedActions[this->_actionCounter].counter];
		return;
	}
}

SokuLib::KeyInput ComboTrial::getDummyInputs()
{
	return {0, 0, 0, 0, 0, 0, 0, 0};
}

void ComboTrial::SpecialAction::parse()
{
	std::string moveName;
	std::string hitsStr;
	std::string delayStr;
	bool d = false;
	bool p = false;

	for (auto c : this->name) {
		if (c == ':' && !p) {
			d = !d;
			if (d && !delayStr.empty())
				throw std::invalid_argument("Multiple delays found for move " + this->name);
		}
		if (c == '(' && !p && !d) {
			p = true;
			if (!hitsStr.empty())
				throw std::invalid_argument("Multiple hit counts found for move " + this->name);
		}
		p &= c != ')' || !d;
		if (c == '(' || c == ')' || c == ':')
			continue;
		if (d)
			delayStr += c;
		else if (p)
			hitsStr += c;
		else
			moveName += std::tolower(c);
	}
	printf("Move %s -> %s (%s) :%s: -> ", this->name.c_str(), moveName.c_str(), hitsStr.c_str(), delayStr.c_str());
	try {
		this->action = actionsFromStr.at(moveName);
		printf("%i ", this->action);
	} catch (std::exception &) {
		printf("INVALID\n");
		throw std::invalid_argument(moveName + " is not a recognized move name");
	}

	try {
		if (!hitsStr.empty())
			this->nbHits = std::stoul(hitsStr);
		else
			this->nbHits = 0;
		printf("%i ", this->nbHits);
	} catch (std::exception &) {
		printf("INVALID\n");
		throw std::invalid_argument(hitsStr + " is not a valid hit count");
	}

	try {
		if (!delayStr.empty())
			this->delay = std::stoul(delayStr);
		else
			this->delay = 0;
		printf("%i\n", this->delay);
	} catch (std::exception &) {
		printf("INVALID\n");
		throw std::invalid_argument(delayStr + " is not a valid delay");
	}

	try {
		this->inputs = actionStrToInputs.at(moveName);
	} catch (...) {
		throw std::invalid_argument(moveName + " is not yet implemented");
	}
}
