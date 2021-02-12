//
// Created by PinkySmile on 15/01/2021.
//

#include "RightChrFullNameString.hpp"

#include "vars.hpp"
#include <SokuLib.hpp>

std::string RightChrFullNameString::getString() const {
	return charactersNames[SokuLib::rightChar].second;
}
