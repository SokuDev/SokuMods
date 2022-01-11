//
// Created by PinkySmile on 15/01/2021.
//

#include "SubmenuString.hpp"

#include "vars.hpp"
#include <SokuLib.hpp>

std::string SubmenuString::getString() const {
	auto menu = SokuLib::getCurrentMenu();

	if (menu == SokuLib::MENU_COUNT)
		return "Unknown menu";
	if (menu == SokuLib::MENU_RESULT && (LoadLibraryA("TrialMode") || LoadLibraryA("TrialModeDebug")))
		menu = SokuLib::MENU_COUNT;
	return submenusNames[menu];
}
