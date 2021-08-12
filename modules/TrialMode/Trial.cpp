//
// Created by PinkySmile on 23/07/2021.
//

#include "Trial.hpp"
#include "ComboTrial.hpp"
#include "Menu.hpp"

static int hooks = 0;
static bool activated = false;
static void (SokuLib::KeymapManager::*s_origKeymapManager_SetInputs)();
const std::map<std::string, std::function<Trial *(const char *folder, SokuLib::Character player, const nlohmann::json &json)>> Trial::_factory{
	{ "combo", [](const char *folder, SokuLib::Character player, const nlohmann::json &json){ return new ComboTrial(folder, player, json); } }
};

static void __fastcall KeymapManagerSetInputs(SokuLib::KeymapManager *This)
{
	(This->*s_origKeymapManager_SetInputs)();
	if (!loadedTrial)
		return;
	if (This == *(SokuLib::KeymapManager **)0x0089918c)
		This->input = loadedTrial->getDummyInputs();
	else if (This == *(SokuLib::KeymapManager **)0x008989A0)
		loadedTrial->editPlayerInputs(This->input);
}

Trial::Trial(const char *folder, const nlohmann::json &json)
{
	if (json.contains("intro") && json["intro"].is_string())
		try {
			this->_intro.reset(loadBattleAnimation(folder, (folder + json["intro"].get<std::string>()).c_str(), true));
		} catch (std::exception &e) {
			throw std::invalid_argument("Cannot load intro file \"" + (folder + json["intro"].get<std::string>()) + "\":" + e.what());
		}
	if (json.contains("outro") && json["outro"].is_string())
		try {
			this->_outro.reset(loadBattleAnimation(folder, (folder + json["outro"].get<std::string>()).c_str(), false));
		} catch (std::exception &e) {
			throw std::invalid_argument("Cannot load outro file \"" + (folder + json["outro"].get<std::string>()) + "\":" + e.what());
		}
	if (json["music"].is_number()) {
		unsigned t = json["music"];

		this->music = (t < 10 ? "data/bgm/st0" : "data/bgm/st") + std::to_string(t) + ".ogg";
	} else
		this->music = json["music"];
}

Trial::~Trial()
{
}

Trial *Trial::create(const char *folder, SokuLib::Character player, const nlohmann::json &json)
{
	if (!json.contains("type") || !json["type"].is_string())
		return nullptr;
	return Trial::_factory.at(json["type"])(folder, player, json);
}

const std::array<std::string, Trial::NB_MENU_ACTION> Trial::menuActionText{
	"Go to next trial",
	"Retry",
	"Return to trial select",
	"Return to title screen"
};

void Trial::hook()
{
	DWORD old;

	puts("Hooked !");
	VirtualProtect((PVOID)TEXT_SECTION_OFFSET, TEXT_SECTION_SIZE, PAGE_EXECUTE_WRITECOPY, &old);
	s_origKeymapManager_SetInputs = SokuLib::union_cast<void (SokuLib::KeymapManager::*)()>(SokuLib::TamperNearJmpOpr(0x40A45D, KeymapManagerSetInputs));
	VirtualProtect((PVOID)TEXT_SECTION_OFFSET, TEXT_SECTION_SIZE, old, &old);
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
	auto keys = reinterpret_cast<SokuLib::KeyManager *>(0x89A394);

	(*reinterpret_cast<char **>(0x8985E8))[0x494] = 22; // Remove HUD
	if (!this->_intro) {
		this->_introPlayed = true;
		((void (*)(const char *))0x43ff10)(this->music.c_str());
		return;
	}

	auto result = this->_intro->update();

	if (!result && !this->_introPlayed)
		((void (*)(const char *))0x43ff10)(this->music.c_str());
	this->_introPlayed |= !result;
	if (keys->keymapManager->input.a == 1 || keys->keymapManager->input.b)
		this->_intro->onKeyPressed();
}

void Trial::_outroOnUpdate()
{
	auto keys = reinterpret_cast<SokuLib::KeyManager *>(0x89A394);

	if (!this->_outro) {
		this->_outroPlayed = true;
		return;
	}
	this->_outroPlayed |= !this->_outro->update();
	if (keys->keymapManager->input.a == 1 || keys->keymapManager->input.b)
		this->_outro->onKeyPressed();
}
