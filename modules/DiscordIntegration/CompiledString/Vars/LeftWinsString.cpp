//
// Created by PinkySmile on 15/01/2021.
//

#include "vars.hpp"
#include "LeftWinsString.hpp"

std::string LeftWinsString::getString() const
{
	return std::to_string(wins.first);
}
