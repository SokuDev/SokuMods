//
// Created by PinkySmile on 15/01/2021.
//

#ifndef SWRSTOYS_OPPONENTPROFILENAMESTRING_HPP
#define SWRSTOYS_OPPONENTPROFILENAMESTRING_HPP


#include "../CompiledString.hpp"

class OpponentProfileNameString : public CompiledString {
public:
	std::string getString() const override;
};


#endif //SWRSTOYS_OPPONENTPROFILENAMESTRING_HPP
