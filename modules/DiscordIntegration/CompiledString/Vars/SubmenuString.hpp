//
// Created by PinkySmile on 15/01/2021.
//

#ifndef SWRSTOYS_SUBMENUSTRING_HPP
#define SWRSTOYS_SUBMENUSTRING_HPP


#include "../CompiledString.hpp"

class SubmenuString : public CompiledString {
public:
	std::string getString() const override;
};


#endif //SWRSTOYS_SUBMENUSTRING_HPP
