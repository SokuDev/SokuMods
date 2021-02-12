//
// Created by PinkySmile on 15/01/2021.
//

#ifndef SWRSTOYS_RIGHTPROFILENAMESTRING_HPP
#define SWRSTOYS_RIGHTPROFILENAMESTRING_HPP

#include "../CompiledString.hpp"

class RightProfileNameString: public CompiledString {
public:
	std::string getString() const override;
};

#endif // SWRSTOYS_RIGHTPROFILENAMESTRING_HPP
