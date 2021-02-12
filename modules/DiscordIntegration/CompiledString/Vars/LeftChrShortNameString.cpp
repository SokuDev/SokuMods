//
// Created by PinkySmile on 15/01/2021.
//

#include "LeftChrShortNameString.hpp"

#include "vars.hpp"
#include <SokuLib.hpp>

std::string LeftChrShortNameString::getString() const {
	return charactersNames[SokuLib::leftChar].first;
}
