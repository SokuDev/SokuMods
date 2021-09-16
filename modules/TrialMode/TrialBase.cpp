//
// Created by PinkySmile on 16/09/2021.
//

#include "TrialBase.hpp"
#include "Menu.hpp"

static void (SokuLib::KeymapManager::*s_origKeymapManager_SetInputs)();

const std::array<std::string, TrialBase::NB_MENU_ACTION> TrialBase::menuActionText{
	"Go to next trial",
	"Retry",
	"Return to trial select",
	"Return to title screen"
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

void TrialBase::hook()
{
	DWORD old;

	puts("Hooked !");
	VirtualProtect((PVOID)TEXT_SECTION_OFFSET, TEXT_SECTION_SIZE, PAGE_EXECUTE_WRITECOPY, &old);
	s_origKeymapManager_SetInputs = SokuLib::union_cast<void (SokuLib::KeymapManager::*)()>(SokuLib::TamperNearJmpOpr(0x40A45D, KeymapManagerSetInputs));
	VirtualProtect((PVOID)TEXT_SECTION_OFFSET, TEXT_SECTION_SIZE, old, &old);
	::FlushInstructionCache(GetCurrentProcess(), nullptr, 0);
}