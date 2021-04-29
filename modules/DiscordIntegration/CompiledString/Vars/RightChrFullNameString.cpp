//
// Created by PinkySmile on 15/01/2021.
//

#include "RightChrFullNameString.hpp"

#include "vars.hpp"
#include <SokuLib.hpp>

std::string RightChrFullNameString::getString() const {
	auto it = charactersNames.find(SokuLib::rightChar);

	return (it == charactersNames.end() ? "Unknown character" : it->second.second);
}
