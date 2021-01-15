//
// Created by PinkySmile on 15/01/2021.
//

#include <SokuLib.hpp>
#include "vars.hpp"
#include "LeftChrFullNameString.hpp"

std::string LeftChrFullNameString::getString() const
{
	return charactersNames[SokuLib::leftChar].second;
}
