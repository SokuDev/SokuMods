//
// Created by PinkySmile on 15/01/2021.
//

#ifndef SWRSTOYS_RIGHTWINSSTRING_HPP
#define SWRSTOYS_RIGHTWINSSTRING_HPP


#include "../CompiledString.hpp"

class RightWinsString : public CompiledString {

public:
	std::string getString() const override;
};


#endif //SWRSTOYS_RIGHTWINSSTRING_HPP
