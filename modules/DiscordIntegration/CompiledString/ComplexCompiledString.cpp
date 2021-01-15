//
// Created by PinkySmile on 15/01/2021.
//

#include <utility>
#include "ComplexCompiledString.hpp"

ComplexCompiledString::ComplexCompiledString(std::shared_ptr<CompiledString> left, std::shared_ptr<CompiledString> right) :
	_left(std::move(left)),
	_right(std::move(right))
{
}

std::string ComplexCompiledString::getString() const
{
	return this->_left->getString() + this->_right->getString();
}
