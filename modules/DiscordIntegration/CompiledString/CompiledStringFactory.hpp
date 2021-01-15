//
// Created by PinkySmile on 15/01/2021.
//

#ifndef SWRSTOYS_COMPILEDSTRINGFACTORY_HPP
#define SWRSTOYS_COMPILEDSTRINGFACTORY_HPP


#include <memory>
#include <map>
#include <functional>
#include "CompiledString.hpp"

class CompiledStringFactory {
private:
	static std::shared_ptr<CompiledString> _compileVar(const char *str);
	static const std::map<std::string, std::function<CompiledString *()>> _varsMakers;

public:
	static std::shared_ptr<CompiledString> compileString(const char *str);
	static std::shared_ptr<CompiledString> compileString(const std::string &str);
};


#endif //SWRSTOYS_COMPILEDSTRINGFACTORY_HPP
