//
// Created by PinkySmile on 15/01/2021.
//

#ifndef SWRSTOYS_LEFTCHRIMGSTRING_HPP
#define SWRSTOYS_LEFTCHRIMGSTRING_HPP

#include "../CompiledString.hpp"

class LeftChrImgString: public CompiledString {
public:
	std::string getString() const override;
};

#endif // SWRSTOYS_LEFTCHRIMGSTRING_HPP
