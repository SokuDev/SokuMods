//
// Created by PinkySmile on 15/01/2021.
//

#include <SokuLib.hpp>
#include "vars.hpp"
#include "RightChrFullNameString.hpp"

std::string RightChrFullNameString::getString() const
{
	return charactersNames[SokuLib::rightChar].second;
}
