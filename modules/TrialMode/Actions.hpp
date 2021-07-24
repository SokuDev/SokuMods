//
// Created by PinkySmile on 24/07/2021.
//

#ifndef SWRSTOYS_ACTIONS_HPP
#define SWRSTOYS_ACTIONS_HPP


#include <map>
#include <SokuLib.hpp>

extern const std::map<std::string, std::vector<SokuLib::KeyInput>> actionStrToInputs;
extern std::map<SokuLib::Character, std::vector<std::string>> characterSkills;
extern const std::map<std::string, SokuLib::Action> actionsFromStr;


#endif //SWRSTOYS_ACTIONS_HPP
