//
// Created by PinkySmile on 15/01/2021.
//

#include <SokuLib.hpp>
#include "vars.hpp"
#include "SubmenuString.hpp"

std::string SubmenuString::getString() const
{
	auto menu = SokuLib::getCurrentMenu();

	if (menu == SokuLib::MENU_COUNT)
		return "Unknown menu";
	return submenusNames[menu];
}
