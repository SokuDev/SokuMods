//
// Created by PinkySmile on 15/01/2021.
//

#include "RightChrImgString.hpp"

#include "vars.hpp"
#include <SokuLib.hpp>

std::string RightChrImgString::getString() const {
	return charactersImg[SokuLib::rightChar];
}
