//
// Created by PinkySmile on 15/01/2021.
//

#ifndef SWRSTOYS_LEFTWINSSTRING_HPP
#define SWRSTOYS_LEFTWINSSTRING_HPP

#include "../CompiledString.hpp"

class LeftWinsString: public CompiledString {
public:
	std::string getString() const override;
};

#endif // SWRSTOYS_LEFTWINSSTRING_HPP
