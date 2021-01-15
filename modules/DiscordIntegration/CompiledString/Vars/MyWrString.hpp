//
// Created by PinkySmile on 15/01/2021.
//

#ifndef SWRSTOYS_MYWRSTRING_HPP
#define SWRSTOYS_MYWRSTRING_HPP


#include "../CompiledString.hpp"

class MyWRString : public CompiledString {
public:

	std::string getString() const override;
};


#endif //SWRSTOYS_MYWRSTRING_HPP
