//
// Created by PinkySmile on 15/01/2021.
//

#include "OpponentWrString.hpp"

#include "vars.hpp"

std::string OpponentWRString::getString() const {
	if (!wins.first)
		return wins.second ? "Perfect" : "N/A";
	return std::to_string(wins.second * 100 / (wins.second + wins.first)) + "%";
}
