//
// Created by PinkySmile on 23/07/2021.
//

#include "Patches.hpp"
#include "Trial.hpp"
#include "ComboTrial.hpp"

#ifndef _DEBUG
#define puts(...)
#define printf(...)
#endif

const std::map<std::string, std::function<Trial *(const char *folder, SokuLib::Character player, const nlohmann::json &json)>> Trial::_factory{
	{ "combo", [](const char *folder, SokuLib::Character player, const nlohmann::json &json){ return new ComboTrial(folder, player, json); } }
};

Trial::Trial(const char *folder, const nlohmann::json &json)
{
	applyFileSystemPatch();
	this->_folder = folder;
	if (json.contains("intro") && json["intro"].is_string())
		this->_introPath = folder + json["intro"].get<std::string>();
	if (json.contains("outro") && json["outro"].is_string())
		this->_outroPath = folder + json["outro"].get<std::string>();
	if (json["music"].is_number()) {
		unsigned t = json["music"];

		this->_music = (t < 10 ? "data/bgm/st0" : "data/bgm/st") + std::to_string(t) + ".ogg";
	} else
		this->_music = json["music"];
	for (auto pos = this->_music.find("{{pack_path}}"); pos != std::string::npos; pos = this->_music.find("{{pack_path}}"))
		this->_music.replace(pos, strlen("{{pack_path}}"), folder);
	if (json.contains("counter_hit") && json["counter_hit"].is_boolean() && json["counter_hit"].get<bool>())
		applyCounterHitOnlyPatch();
	this->_initAnimations();
}

Trial::~Trial()
{
	removeFileSystemPatch();
	removeCounterHitOnlyPatch();
}

TrialBase *Trial::create(const char *folder, SokuLib::Character player, const nlohmann::json &json)
{
	if (!json.contains("type") || !json["type"].is_string())
		return nullptr;
	return Trial::_factory.at(json["type"])(folder, player, json);
}

void Trial::_introOnRender() const
{
	if (!this->_intro)
		return;
	this->_intro->render();
}

void Trial::_outroOnRender() const
{
	if (!this->_outro)
		return;
	this->_outro->render();
}

void Trial::_introOnUpdate()
{
	(*reinterpret_cast<char **>(0x8985E8))[0x494] = 22; // Remove HUD
	if (!this->_intro) {
		this->_introPlayed = true;
		SokuLib::playBGM(this->_music.c_str());
		return;
	}

	auto result = this->_intro->update();

	if (!result && !this->_introPlayed)
		SokuLib::playBGM(this->_music.c_str());
	this->_introPlayed |= !result;
	if (SokuLib::inputMgrs.input.a == 1 || SokuLib::inputMgrs.input.b)
		this->_intro->onKeyPressed();
	if (SokuLib::inputMgrs.pause == 1) {
		SokuLib::playSEWaveBuffer(0x29);
		this->_introPlayed = true;
		SokuLib::playBGM(this->_music.c_str());
	};
}

void Trial::_outroOnUpdate()
{
	if (!this->_outro) {
		this->_outroPlayed = true;
		return;
	}
	this->_outroPlayed |= !this->_outro->update();
	if (SokuLib::inputMgrs.input.a == 1 || SokuLib::inputMgrs.input.b)
		this->_outro->onKeyPressed();
	if (SokuLib::inputMgrs.pause == 1) {
		SokuLib::playSEWaveBuffer(0x29);
		this->_outroPlayed = true;
	};
}

void Trial::_initAnimations(bool intro, bool outro)
{
	if (!this->_introPath.empty() && intro)
		try {
			this->_intro.reset(loadBattleAnimation(this->_folder.c_str(), this->_introPath.c_str(), true));
		} catch (std::exception &e) {
			throw std::invalid_argument("Cannot load intro file \"" + this->_introPath + "\":" + e.what());
		}
	if (!this->_outroPath.empty() && outro)
		try {
			this->_outro.reset(loadBattleAnimation(this->_folder.c_str(), this->_outroPath.c_str(), false));
		} catch (std::exception &e) {
			throw std::invalid_argument("Cannot load outro file \"" + this->_outroPath + "\":" + e.what());
		}
}
