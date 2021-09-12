//
// Created by PinkySmile on 23/07/2021.
//

#include "Trial.hpp"
#include "ComboTrial.hpp"
#include "ComboTrialEditor.hpp"

#ifndef _DEBUG
#define puts(...)
#define printf(...)
#endif

static unsigned char buffer[11];
static int hooks = 0;
static bool activated = false;
static void (SokuLib::KeymapManager::*s_origKeymapManager_SetInputs)();
const std::map<std::string, std::function<Trial *(const char *folder, SokuLib::Character player, const nlohmann::json &json)>> Trial::_factory{
	{ "combo", [](const char *folder, SokuLib::Character player, const nlohmann::json &json){ return new ComboTrial(folder, player, json); } }
};
const std::map<std::string, std::function<Trial *(const char *folder, SokuLib::Character player, const nlohmann::json &json)>> Trial::_editorFactory{
	{ "combo", [](const char *folder, SokuLib::Character player, const nlohmann::json &json){ return new ComboTrialEditor(folder, player, json); } }
};

const std::map<std::string, SokuLib::Weather> weathers{
	{ "sunny",          SokuLib::WEATHER_SUNNY },
	{ "drizzle",        SokuLib::WEATHER_DRIZZLE },
	{ "cloudy",         SokuLib::WEATHER_CLOUDY },
	{ "blue sky",       SokuLib::WEATHER_BLUE_SKY },
	{ "hail",           SokuLib::WEATHER_HAIL },
	{ "spring haze",    SokuLib::WEATHER_SPRING_HAZE },
	{ "heavy fog",      SokuLib::WEATHER_HEAVY_FOG },
	{ "snow",           SokuLib::WEATHER_SNOW },
	{ "sun shower",     SokuLib::WEATHER_SUN_SHOWER },
	{ "sprinkle",       SokuLib::WEATHER_SPRINKLE },
	{ "tempest",        SokuLib::WEATHER_TEMPEST },
	{ "mountain vapor", SokuLib::WEATHER_MOUNTAIN_VAPOR },
	{ "mm",             SokuLib::WEATHER_MOUNTAIN_VAPOR },
	{ "river mist",     SokuLib::WEATHER_RIVER_MIST },
	{ "rm",             SokuLib::WEATHER_RIVER_MIST },
	{ "typhoon",        SokuLib::WEATHER_TYPHOON },
	{ "calm",           SokuLib::WEATHER_CALM },
	{ "diamond dust",   SokuLib::WEATHER_DIAMOND_DUST },
	{ "dd",             SokuLib::WEATHER_DIAMOND_DUST },
	{ "dust storm",     SokuLib::WEATHER_DUST_STORM },
	{ "ds",             SokuLib::WEATHER_DUST_STORM },
	{ "scorching sun",  SokuLib::WEATHER_SCORCHING_SUN },
	{ "ss",             SokuLib::WEATHER_SCORCHING_SUN },
	{ "monsoon",        SokuLib::WEATHER_MONSOON },
	{ "aurora",         SokuLib::WEATHER_AURORA },
	{ "random",         SokuLib::WEATHER_AURORA },
	{ "twilight",       SokuLib::WEATHER_TWILIGHT },
	{ "clear",          SokuLib::WEATHER_CLEAR },
	{ "none",           SokuLib::WEATHER_CLEAR },
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
	DWORD old;

	buffer[0] = *(unsigned char *)0x40D1FB;
	buffer[1] = *(unsigned char *)0x40D245;
	buffer[2] = *(unsigned char *)0x40D27A;
	buffer[3] = *(unsigned char *)0x40D27B;
	buffer[4] = *(unsigned char *)0x40D27C;
	buffer[5] = *(unsigned char *)0x40D27D;
	buffer[6] = *(unsigned char *)0x40D27E;
	buffer[7] = *(unsigned char *)0x40D27F;
	buffer[8] = *(unsigned char *)0x40D280;
	buffer[9] = *(unsigned char *)0x40D281;
	buffer[10]= *(unsigned char *)0x40D282;

	// This applies the filesystem first patch from Slen
	::VirtualProtect((PVOID)TEXT_SECTION_OFFSET, TEXT_SECTION_SIZE, PAGE_EXECUTE_READWRITE, &old);
	*(unsigned char *)0x40D1FB = 0xEB;
	*(unsigned char *)0x40D245 = 0x1C;
	*(unsigned char *)0x40D27A = 0x74;
	*(unsigned char *)0x40D27B = 0x91;
	for (int i = 0; i < 7; i++)
		((unsigned char *)0x40D27C)[i] = 0x90;
	::VirtualProtect((PVOID)TEXT_SECTION_OFFSET, TEXT_SECTION_SIZE, old, &old);
	::FlushInstructionCache(GetCurrentProcess(), nullptr, 0);

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
	for (auto pos = this->music.find("{{pack_path}}"); pos != std::string::npos; pos = this->music.find("{{pack_path}}"))
		this->music.replace(pos, strlen("{{pack_path}}"), folder);
	if (json.contains("counter_hit") && json["counter_hit"].is_boolean() && json["counter_hit"].get<bool>()) {
		DWORD old;

		puts("Moves always counter hit");
		::VirtualProtect((PVOID)TEXT_SECTION_OFFSET, TEXT_SECTION_SIZE, PAGE_EXECUTE_READWRITE, &old);
		// This removes the check for counter hits so moves will always counter hit if they can
		*(unsigned short *)0x47abb7 = 0x9090;
		*(unsigned short *)0x47abc3 = 0x9090;
		::VirtualProtect((PVOID)TEXT_SECTION_OFFSET, TEXT_SECTION_SIZE, old, &old);
		::FlushInstructionCache(GetCurrentProcess(), nullptr, 0);
	}
}

Trial::~Trial()
{
	DWORD old;

	puts("Restore counter hit check");
	::VirtualProtect((PVOID)TEXT_SECTION_OFFSET, TEXT_SECTION_SIZE, PAGE_EXECUTE_READWRITE, &old);
	// We remove the always counter hit patch
	*(unsigned short *)0x47abb7 = 0x1e75; //           JNZ        LAB_0047abd7
	*(unsigned short *)0x47abc3 = 0x1275; //           JNZ        LAB_0047abd7

	// We remove the filesystem first patch if it was there
	*(unsigned char *)0x40D1FB = buffer[0];
	*(unsigned char *)0x40D245 = buffer[1];
	for (int i = 2; i < 11; i++)
		((unsigned char *)0x40D27A)[i - 2] = buffer[i];
	::VirtualProtect((PVOID)TEXT_SECTION_OFFSET, TEXT_SECTION_SIZE, old, &old);
	::FlushInstructionCache(GetCurrentProcess(), nullptr, 0);
}

Trial *Trial::create(const char *folder, SokuLib::Character player, const nlohmann::json &json)
{
	if (!json.contains("type") || !json["type"].is_string())
		return nullptr;
	return (editorMode ? Trial::_editorFactory : Trial::_factory).at(json["type"])(folder, player, json);
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
	::FlushInstructionCache(GetCurrentProcess(), nullptr, 0);
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
		SokuLib::playBGM(this->music.c_str());
		return;
	}

	auto result = this->_intro->update();

	if (!result && !this->_introPlayed)
		SokuLib::playBGM(this->music.c_str());
	this->_introPlayed |= !result;
	if (SokuLib::inputMgrs.input.a == 1 || SokuLib::inputMgrs.input.b)
		this->_intro->onKeyPressed();
	if (SokuLib::inputMgrs.pause == 1) {
		SokuLib::playSEWaveBuffer(0x29);
		this->_introPlayed = true;
		SokuLib::playBGM(this->music.c_str());
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
