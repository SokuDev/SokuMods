//
// Created by PinkySmile on 23/02/2021.
//

#ifndef SWRSTOYS_DUMMY_HPP
#define SWRSTOYS_DUMMY_HPP


#include <SokuLib.hpp>

namespace Practice
{
	std::vector<SokuLib::KeyInput> moveNameToSequence(const std::vector<std::string> &input);
	std::vector<SokuLib::KeyInput> moveNameToSequence(const std::string &input);
	std::vector<std::string> moveNameToSequenceStrs(const std::string &input);
	SokuLib::KeyInput moveNameToInput(const std::string &input);
	void addInputSequence(const std::vector<SokuLib::KeyInput> &inputs);
	void addInputSequence(const std::vector<std::string> &inputs);
	void addInputSequence(const std::string &inputs);
	void addNextInput(const SokuLib::KeyInput &input);
	void addNextInput(const std::string &input);
	void moveDummy(SokuLib::KeymapManager &manager);
}


#endif //SWRSTOYS_DUMMY_HPP
