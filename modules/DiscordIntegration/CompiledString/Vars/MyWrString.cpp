//
// Created by PinkySmile on 15/01/2021.
//

#include "MyWrString.hpp"

#include "vars.hpp"

std::string MyWRString::getString() const {
	if (!wins.second)
		return wins.first ? "Perfect" : "N/A";
	return std::to_string(wins.first * 100 / (wins.first + wins.second)) + "%";
}
