//
// Created by PinkySmile on 15/01/2021.
//

#include "RightChrShortNameString.hpp"

#include "vars.hpp"
#include <SokuLib.hpp>

std::string RightChrShortNameString::getString() const {
	return charactersNames[SokuLib::rightChar].first;
}
