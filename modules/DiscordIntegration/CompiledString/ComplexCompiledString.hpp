//
// Created by PinkySmile on 15/01/2021.
//

#ifndef SWRSTOYS_COMPLEXCOMPILEDSTRING_HPP
#define SWRSTOYS_COMPLEXCOMPILEDSTRING_HPP


#include <memory>
#include "CompiledString.hpp"

class ComplexCompiledString : public CompiledString {
private:
	std::shared_ptr<CompiledString> _left;
	std::shared_ptr<CompiledString> _right;

public:
	ComplexCompiledString(std::shared_ptr<CompiledString> left, std::shared_ptr<CompiledString> right);

	std::string getString() const override;
};


#endif //SWRSTOYS_COMPLEXCOMPILEDSTRING_HPP
