//
// Created by PinkySmile on 15/01/2021.
//

#include <SokuLib.hpp>
#include "vars.hpp"
#include "LeftChrShortNameString.hpp"

std::string LeftChrShortNameString::getString() const
{
	return charactersNames[SokuLib::leftChar].first;
}
