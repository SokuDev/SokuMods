//
// Created by PinkySmile on 15/01/2021.
//

#include "CompiledStringFactory.hpp"

#include "BasicCompiledString.hpp"
#include "ComplexCompiledString.hpp"
#include "Exceptions.hpp"
#include "Vars/Strings.hpp"
#include <stdexcept>

const std::map<std::string, std::function<CompiledString *()>> CompiledStringFactory::_varsMakers{
	{"lChrImg", [] { return new LeftChrImgString(); }},
	{"lChrFullName", [] { return new LeftChrFullNameString(); }},
	{"lChrShortName", [] { return new LeftChrShortNameString(); }},
	{"lProfileName", [] { return new LeftProfileNameString; }},
	{"lWins", [] { return new LeftWinsString(); }},
	{"rChrImg", [] { return new RightChrImgString(); }},
	{"rChrFullName", [] { return new RightChrFullNameString(); }},
	{"rChrShortName", [] { return new RightChrShortNameString(); }},
	{"rProfileName", [] { return new RightProfileNameString(); }},
	{"rWins", [] { return new RightWinsString(); }},
	{"myChrImg", [] { return new MyChrImgString(); }},
	{"myChrFullName", [] { return new MyChrFullNameString(); }},
	{"myChrShortName", [] { return new MyChrShortNameString(); }},
	{"myProfileName", [] { return new MyProfileNameString; }},
	{"myWins", [] { return new LeftWinsString(); }},
	{"myWr", [] { return new MyWRString(); }},
	{"opChrImg", [] { return new OpponentChrImgString(); }},
	{"opChrFullName", [] { return new OpponentChrFullNameString(); }},
	{"opChrShortName", [] { return new OpponentChrShortNameString(); }},
	{"opProfileName", [] { return new OpponentProfileNameString(); }},
	{"opProfileName", [] { return new OpponentProfileNameString(); }},
	{"opWins", [] { return new RightWinsString(); }},
	{"opWr", [] { return new OpponentWRString(); }},
	{"submenu", [] { return new SubmenuString(); }},
	{"stageImg", [] { return new StageImgString(); }},
	{"stageName", [] { return new StageNameString(); }},
	{"trialName", [] { return new TrialName(); }},
	{"trialPack", [] { return new TrialPack(); }},
	{"trialAttempt", [] { return new TrialAttempt(); }},
};

std::shared_ptr<CompiledString> CompiledStringFactory::compileString(const char *str) {
	const char *var = strstr(str, "{{");

	if (!var)
		return std::make_shared<BasicCompiledString>(str);
	if (var != str)
		return std::make_shared<ComplexCompiledString>(std::make_shared<BasicCompiledString>(std::string(str, var)), CompiledStringFactory::compileString(var));
	return CompiledStringFactory::_compileVar(var + 2);
}

std::shared_ptr<CompiledString> CompiledStringFactory::compileString(const std::string &str) {
	return CompiledStringFactory::compileString(str.c_str());
}

std::shared_ptr<CompiledString> CompiledStringFactory::_compileVar(const char *str) {
	const char *end = strstr(str, "}}");

	if (!end)
		throw InvalidString("Cannot find closing variable marker (Near \"" + std::string(str) + "\").");

	try {
		auto compiled = std::shared_ptr<CompiledString>(CompiledStringFactory::_varsMakers.at({str, end})());

		if (end[2])
			return std::make_shared<ComplexCompiledString>(compiled, CompiledStringFactory::compileString(end + 2));
		return compiled;
	} catch (std::out_of_range &) {
		throw InvalidString("\"" + std::string{str, end} + "\" is not a valid variable name.");
	}
}
