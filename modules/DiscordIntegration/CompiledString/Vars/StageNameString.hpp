//
// Created by PinkySmile on 15/01/2021.
//

#ifndef SWRSTOYS_STAGENAMESTRING_HPP
#define SWRSTOYS_STAGENAMESTRING_HPP

#include "../CompiledString.hpp"

class StageNameString: public CompiledString {
public:
	std::string getString() const override;
};

#endif // SWRSTOYS_STAGENAMESTRING_HPP
