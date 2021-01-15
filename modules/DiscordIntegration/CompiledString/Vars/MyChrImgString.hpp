//
// Created by PinkySmile on 15/01/2021.
//

#ifndef SWRSTOYS_MYCHRIMGSTRING_HPP
#define SWRSTOYS_MYCHRIMGSTRING_HPP


#include "../CompiledString.hpp"

class MyChrImgString : public CompiledString {
public:
	std::string getString() const override;
};


#endif //SWRSTOYS_MYCHRIMGSTRING_HPP
