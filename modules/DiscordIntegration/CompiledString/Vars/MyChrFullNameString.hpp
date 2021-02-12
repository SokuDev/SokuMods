//
// Created by PinkySmile on 15/01/2021.
//

#ifndef SWRSTOYS_MYCHRFULLNAMESTRING_HPP
#define SWRSTOYS_MYCHRFULLNAMESTRING_HPP

#include "../CompiledString.hpp"

class MyChrFullNameString: public CompiledString {
public:
	std::string getString() const override;
};

#endif // SWRSTOYS_MYCHRFULLNAMESTRING_HPP
