//
// Created by PinkySmile on 15/01/2021.
//

#include <SokuLib.hpp>
#include "vars.hpp"
#include "RightChrImgString.hpp"

std::string RightChrImgString::getString() const
{
	return charactersImg[SokuLib::rightChar];
}
