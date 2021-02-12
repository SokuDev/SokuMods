//
// Created by PinkySmile on 15/01/2021.
//

#include "LeftChrFullNameString.hpp"

#include "vars.hpp"
#include <SokuLib.hpp>

std::string LeftChrFullNameString::getString() const {
	return charactersNames[SokuLib::leftChar].second;
}
