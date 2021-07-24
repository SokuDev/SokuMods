//
// Created by PinkySmile on 23/07/2021.
//

#include "ComboTrial.hpp"
#include "Actions.hpp"

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

	this->_counterHit = json.contains("counter_hit") && json["counter_hit"].is_boolean() ? json["counter_hit"].get<bool>() : false;
	this->_disableLimit = json.contains("disable_limit") && json["disable_limit"].is_boolean() ? json["disable_limit"].get<bool>() : false;
	this->_uniformCardCost = json.contains("uniform_card_cost") && json["uniform_card_cost"].is_number() ? json["uniform_card_cost"].get<int>() : -1;
	this->_playComboAfterIntro = json.contains("play_combo_after_intro") && json["play_combo_after_intro"].is_boolean() ? json["play_combo_after_intro"].get<bool>() : false;
	this->_playerStartPos = json["player"]["pos"];
	this->_dummyStartPos.x = json["dummy"]["pos"]["x"];
	this->_dummyStartPos.y = json["dummy"]["pos"]["y"];
}

bool ComboTrial::update(bool &canHaveNextFrame)
{
	auto &battleMgr = SokuLib::getBattleMgr();

	if (this->_isStart)
		this->_initGameStart();
	if (this->_disableLimit) {
		battleMgr.leftCharacterManager.combo.limit = 0;
		battleMgr.rightCharacterManager.combo.limit = 0;
		battleMgr.leftCharacterManager.realLimit = 0;
		battleMgr.rightCharacterManager.realLimit = 0;
	}

	auto hit = battleMgr.rightCharacterManager.objectBase.action >= SokuLib::ACTION_STAND_GROUND_HIT_SMALL_HITSTUN &&
	           battleMgr.rightCharacterManager.objectBase.action <= SokuLib::ACTION_FORWARD_DASH;

	if (this->_dummyHit && !hit)
		this->_isStart = true;
	this->_dummyHit |= hit;
	if (!this->_dummyHit) {
		battleMgr.rightCharacterManager.objectBase.position.x = this->_dummyStartPos.x;
		battleMgr.rightCharacterManager.objectBase.position.y = this->_dummyStartPos.y;
		battleMgr.rightCharacterManager.objectBase.action = this->_dummyStartPos.y == 0 ? SokuLib::ACTION_IDLE : SokuLib::ACTION_FALLING;
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

	this->_isStart = false;
	this->_dummyHit = false;

	battleMgr.leftCharacterManager.objectBase.hp = 10000;
	battleMgr.leftCharacterManager.objectBase.action = SokuLib::ACTION_FALLING;
	battleMgr.leftCharacterManager.objectBase.position.x = this->_playerStartPos;
	battleMgr.leftCharacterManager.objectBase.position.y = 0;
	memcpy(&battleMgr.leftCharacterManager.skillMap, &this->_skills, sizeof(this->_skills));

	battleMgr.rightCharacterManager.objectBase.hp = 10000;
	battleMgr.rightCharacterManager.objectBase.action = SokuLib::ACTION_FALLING;
	battleMgr.rightCharacterManager.objectBase.position.x = this->_dummyStartPos.x;
	battleMgr.rightCharacterManager.objectBase.position.y = this->_dummyStartPos.y;
}
