//
// Created by PinkySmile on 15/01/2021.
//

#ifndef SWRSTOYS_MYPROFILENAMESTRING_HPP
#define SWRSTOYS_MYPROFILENAMESTRING_HPP


#include "../CompiledString.hpp"

class MyProfileNameString : public CompiledString {
public:
	std::string getString() const override;
};


#endif //SWRSTOYS_MYPROFILENAMESTRING_HPP
