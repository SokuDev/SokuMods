//
// Created by PinkySmile on 23/07/2021.
//

#include "Trial.hpp"
#include "ComboTrial.hpp"
#include "Menu.hpp"

static void (SokuLib::KeymapManager::*s_origKeymapManager_SetInputs)();
const std::map<std::string, std::function<Trial *(SokuLib::Character player, const nlohmann::json &json)>> Trial::_factory{
	{ "combo", [](SokuLib::Character player, const nlohmann::json &json){ return new ComboTrial(player, json); } }
};

static void __fastcall KeymapManagerSetInputs(SokuLib::KeymapManager *This)
{
	(This->*s_origKeymapManager_SetInputs)();
	if (This == *(SokuLib::KeymapManager **)0x0089918c)
		This->input = loadedTrial->getDummyInputs();
	else if (This == *(SokuLib::KeymapManager **)0x008989A0)
		loadedTrial->editPlayerInputs(This->input);
}

Trial::Trial()
{
	DWORD old;

	puts("Hook !");
	VirtualProtect((PVOID)TEXT_SECTION_OFFSET, TEXT_SECTION_SIZE, PAGE_EXECUTE_WRITECOPY, &old);
	s_origKeymapManager_SetInputs = SokuLib::union_cast<void (SokuLib::KeymapManager::*)()>(SokuLib::TamperNearJmpOpr(0x40A45D, KeymapManagerSetInputs));
	VirtualProtect((PVOID)TEXT_SECTION_OFFSET, TEXT_SECTION_SIZE, old, &old);
}

Trial::~Trial()
{
	DWORD old;

	puts("No hook !");
	VirtualProtect((PVOID)TEXT_SECTION_OFFSET, TEXT_SECTION_SIZE, PAGE_EXECUTE_WRITECOPY, &old);
	SokuLib::TamperNearJmpOpr(0x40A45D, s_origKeymapManager_SetInputs);
	VirtualProtect((PVOID)TEXT_SECTION_OFFSET, TEXT_SECTION_SIZE, old, &old);
}

Trial *Trial::create(SokuLib::Character player, const nlohmann::json &json)
{
	if (!json.contains("type") || !json["type"].is_string())
		return nullptr;
	return Trial::_factory.at(json["type"])(player, json);
}
