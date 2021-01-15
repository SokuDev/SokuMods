//
// Created by PinkySmile on 15/01/2021.
//

#ifndef SWRSTOYS_BASICCOMPILEDSTRING_HPP
#define SWRSTOYS_BASICCOMPILEDSTRING_HPP


#include "CompiledString.hpp"

class BasicCompiledString : public CompiledString {
private:
	std::string _value;

public:
	BasicCompiledString(const std::string &&str);
	std::string getString() const override;
};


#endif //SWRSTOYS_BASICCOMPILEDSTRING_HPP
