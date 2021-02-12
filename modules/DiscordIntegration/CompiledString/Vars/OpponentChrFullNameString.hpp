//
// Created by PinkySmile on 15/01/2021.
//

#ifndef SWRSTOYS_OPPONENTCHRFULLNAMESTRING_HPP
#define SWRSTOYS_OPPONENTCHRFULLNAMESTRING_HPP

#include "../CompiledString.hpp"

class OpponentChrFullNameString: public CompiledString {
public:
	std::string getString() const override;
};

#endif // SWRSTOYS_OPPONENTCHRFULLNAMESTRING_HPP
