//
// Created by PinkySmile on 15/01/2021.
//

#ifndef SWRSTOYS_MYCHRSHORTNAMESTRING_HPP
#define SWRSTOYS_MYCHRSHORTNAMESTRING_HPP


#include "../CompiledString.hpp"

class MyChrShortNameString : public CompiledString {
public:
	std::string getString() const override;
};


#endif //SWRSTOYS_MYCHRSHORTNAMESTRING_HPP
