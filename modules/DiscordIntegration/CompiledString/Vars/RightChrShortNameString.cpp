//
// Created by PinkySmile on 15/01/2021.
//

#include <SokuLib.hpp>
#include "vars.hpp"
#include "RightChrShortNameString.hpp"

std::string RightChrShortNameString::getString() const
{
	return charactersNames[SokuLib::rightChar].first;
}
