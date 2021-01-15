//
// Created by PinkySmile on 15/01/2021.
//

#include <SokuLib.hpp>
#include "vars.hpp"
#include "SubmenuString.hpp"

std::string SubmenuString::getString() const
{
	return submenusNames[SokuLib::getCurrentMenu()];
}
