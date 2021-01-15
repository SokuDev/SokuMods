//
// Created by PinkySmile on 15/01/2021.
//

#ifndef SWRSTOYS_OPPONENTWRSTRING_HPP
#define SWRSTOYS_OPPONENTWRSTRING_HPP


#include "../CompiledString.hpp"

class OpponentWRString : public CompiledString {
public:
	std::string getString() const override;
};


#endif //SWRSTOYS_OPPONENTWRSTRING_HPP
