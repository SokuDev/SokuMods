//
// Created by PinkySmile on 15/01/2021.
//

#ifndef SWRSTOYS_LEFTCHRFULLNAMESTRING_HPP
#define SWRSTOYS_LEFTCHRFULLNAMESTRING_HPP

#include "../CompiledString.hpp"

class LeftChrFullNameString: public CompiledString {
public:
	std::string getString() const override;
};

#endif // SWRSTOYS_LEFTCHRFULLNAMESTRING_HPP
