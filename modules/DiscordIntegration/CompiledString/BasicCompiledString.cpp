//
// Created by PinkySmile on 15/01/2021.
//

#include "BasicCompiledString.hpp"

BasicCompiledString::BasicCompiledString(const std::string &&str) :
	_value(str)
{
}

std::string BasicCompiledString::getString() const
{
	return this->_value;
}
