//
// Created by PinkySmile on 15/01/2021.
//

#include "RightChrShortNameString.hpp"

#include "vars.hpp"
#include <SokuLib.hpp>

std::string RightChrShortNameString::getString() const {
	auto it = charactersNames.find(SokuLib::rightChar);

	return (it == charactersNames.end() ? "Unknown" : it->second.first);
}
