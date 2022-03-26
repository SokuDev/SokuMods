//
// Created by PinkySmile on 24/07/2021.
//

#ifndef SWRSTOYS_ACTIONS_HPP
#define SWRSTOYS_ACTIONS_HPP


#include <map>
#include <SokuLib.hpp>

#define FAKE_ACTION_ORRERIES_REACTIVATE static_cast<SokuLib::Action>(SokuLib::ACTION_AIR_ORRERIES_C + 1)
#define FAKE_ACTION_MPP_ACTIVATE        static_cast<SokuLib::Action>(SokuLib::ACTION_AIR_ORRERIES_C + 2)
#define FAKE_ACTION_MPP_IDLE            static_cast<SokuLib::Action>(SokuLib::ACTION_AIR_ORRERIES_C + 3)
#define FAKE_ACTION_MPP_WALK            static_cast<SokuLib::Action>(SokuLib::ACTION_AIR_ORRERIES_C + 4)
#define FAKE_ACTION_MPP_JUMP            static_cast<SokuLib::Action>(SokuLib::ACTION_AIR_ORRERIES_C + 5)
#define FAKE_ACTION_MPP_FALL            static_cast<SokuLib::Action>(SokuLib::ACTION_AIR_ORRERIES_C + 6)
#define FAKE_ACTION_MPP_SOMETHING       static_cast<SokuLib::Action>(SokuLib::ACTION_AIR_ORRERIES_C + 7)
#define FAKE_ACTION_MPP_LANDING         static_cast<SokuLib::Action>(SokuLib::ACTION_AIR_ORRERIES_C + 8)
#define FAKE_ACTION_MPP_5A              static_cast<SokuLib::Action>(SokuLib::ACTION_AIR_ORRERIES_C + 9)
#define FAKE_ACTION_MPP_5B              static_cast<SokuLib::Action>(SokuLib::ACTION_AIR_ORRERIES_C + 10)
#define FAKE_ACTION_MPP_5C              static_cast<SokuLib::Action>(SokuLib::ACTION_AIR_ORRERIES_C + 11)
#define FAKE_ACTION_MPP_J5A             static_cast<SokuLib::Action>(SokuLib::ACTION_AIR_ORRERIES_C + 12)
#define FAKE_ACTION_MPP_J5A_LANDING     static_cast<SokuLib::Action>(SokuLib::ACTION_AIR_ORRERIES_C + 13)
#define FAKE_ACTION_MPP_J5B             static_cast<SokuLib::Action>(SokuLib::ACTION_AIR_ORRERIES_C + 14)
#define FAKE_ACTION_MPP_J5B_LANDING     static_cast<SokuLib::Action>(SokuLib::ACTION_AIR_ORRERIES_C + 15)
#define FAKE_ACTION_MPP_DEACTIVATE      static_cast<SokuLib::Action>(SokuLib::ACTION_AIR_ORRERIES_C + 16)
#define FAKE_ACTION_MPP_2x              static_cast<SokuLib::Action>(SokuLib::ACTION_AIR_ORRERIES_C + 17)
#define FAKE_ACTION_MPP_J5C             static_cast<SokuLib::Action>(SokuLib::ACTION_AIR_ORRERIES_C + 18)
#define FAKE_ACTION_MPP_J5C_FALLING     static_cast<SokuLib::Action>(SokuLib::ACTION_AIR_ORRERIES_C + 19)

#define FAKE_ACTION_5AAA6A static_cast<SokuLib::Action>(1000)
#define FAKE_ACTION_FLY1 static_cast<SokuLib::Action>(1001)
#define FAKE_ACTION_FLY2 static_cast<SokuLib::Action>(1002)
#define FAKE_ACTION_FLY3 static_cast<SokuLib::Action>(1003)
#define FAKE_ACTION_FLY4 static_cast<SokuLib::Action>(1004)
#define FAKE_ACTION_FLY6 static_cast<SokuLib::Action>(1005)
#define FAKE_ACTION_FLY7 static_cast<SokuLib::Action>(1006)
#define FAKE_ACTION_FLY8 static_cast<SokuLib::Action>(1007)
#define FAKE_ACTION_FLY9 static_cast<SokuLib::Action>(1008)

//Suwako stuff
#define FAKE_ACTION_j1D static_cast<SokuLib::Action>(1100)
#define FAKE_ACTION_j3D static_cast<SokuLib::Action>(1101)
#define FAKE_ACTION_LILYPAD_NEUTRAL_HIGHJUMP static_cast<SokuLib::Action>(1102)
#define FAKE_ACTION_LILYPAD_FORWARD_HIGHJUMP static_cast<SokuLib::Action>(1103)
#define FAKE_ACTION_LILYPAD_BACKWARD_HIGHJUMP static_cast<SokuLib::Action>(1104)
#define FAKE_ACTION_LILYPAD_A static_cast<SokuLib::Action>(1105)
#define FAKE_ACTION_LILYPAD_3A static_cast<SokuLib::Action>(1106)
#define FAKE_ACTION_LILYPAD_5B static_cast<SokuLib::Action>(1107)
#define FAKE_ACTION_LILYPAD_6B static_cast<SokuLib::Action>(1108)
#define FAKE_ACTION_LILYPAD_2C static_cast<SokuLib::Action>(1109)
#define FAKE_ACTION_LILYPAD_DEFAULT_22B static_cast<SokuLib::Action>(1112)
#define FAKE_ACTION_LILYPAD_DEFAULT_22C static_cast<SokuLib::Action>(1113)
#define FAKE_ACTION_UNDERGROUND_ALT1_22B static_cast<SokuLib::Action>(1114)
#define FAKE_ACTION_UNDERGROUND_ALT1_22C static_cast<SokuLib::Action>(1115)
#define FAKE_ACTION_LILYPAD_DEFAULT_623b static_cast<SokuLib::Action>(1116)
#define FAKE_ACTION_UNDERGROUND_DEFAULT_623b static_cast<SokuLib::Action>(1117)
#define FAKE_ACTION_UNDERGROUND_DEFAULT_623c static_cast<SokuLib::Action>(1118)
#define FAKE_ACTION_LILYPAD_ALT1_623b static_cast<SokuLib::Action>(1119)
#define FAKE_ACTION_LILYPAD_ALT1_623c static_cast<SokuLib::Action>(1120)
#define FAKE_ACTION_UNDERGROUND_DEFAULT_236b static_cast<SokuLib::Action>(1121)
#define FAKE_ACTION_UNDERGROUND_DEFAULT_236c static_cast<SokuLib::Action>(1122)
#define FAKE_ACTION_UNDERGROUND_ALT1_236b static_cast<SokuLib::Action>(1123)
#define FAKE_ACTION_UNDERGROUND_ALT1_236c static_cast<SokuLib::Action>(1124)
#define FAKE_ACTION_UNDERGROUND_ALT2_236b static_cast<SokuLib::Action>(1125)
#define FAKE_ACTION_UNDERGROUND_ALT2_236c static_cast<SokuLib::Action>(1126)
#define FAKE_ACTION_UNDERGROUND_DEFAULT_214b static_cast<SokuLib::Action>(1127)
#define FAKE_ACTION_UNDERGROUND_DEFAULT_214c static_cast<SokuLib::Action>(1128)
#define FAKE_ACTION_LILYPAD_ALT1_214b static_cast<SokuLib::Action>(1129)
#define FAKE_ACTION_LILYPAD_ALT1_214c static_cast<SokuLib::Action>(1130)
#define FAKE_ACTION_LILYPAD_ALT2_214b static_cast<SokuLib::Action>(1131)
#define FAKE_ACTION_LILYPAD_ALT2_214c static_cast<SokuLib::Action>(1132)
#define FAKE_ACTION_lSC201 static_cast<SokuLib::Action>(1133)
#define FAKE_ACTION_jSC201 static_cast<SokuLib::Action>(1134)
#define FAKE_ACTION_uSC202 static_cast<SokuLib::Action>(1135)
#define FAKE_ACTION_jSC203 static_cast<SokuLib::Action>(1136)
#define FAKE_ACTION_lSC203 static_cast<SokuLib::Action>(1137)
#define FAKE_ACTION_jSC204 static_cast<SokuLib::Action>(1138)
#define FAKE_ACTION_lSC205 static_cast<SokuLib::Action>(1139)
#define FAKE_ACTION_uSC205 static_cast<SokuLib::Action>(1140)
#define FAKE_ACTION_lSC206 static_cast<SokuLib::Action>(1141)
#define FAKE_ACTION_jSC207 static_cast<SokuLib::Action>(1142)
#define FAKE_ACTION_lSC209 static_cast<SokuLib::Action>(1143)
#define FAKE_ACTION_jSC209 static_cast<SokuLib::Action>(1144)
#define FAKE_ACTION_uSC212 static_cast<SokuLib::Action>(1145)

SokuLib::Action addCustomActions(const SokuLib::CharacterManager &character, SokuLib::Character characterId);
bool isStartOfMove(SokuLib::Action action, const SokuLib::CharacterManager &character, SokuLib::Character characterId);

extern const std::map<std::string, std::vector<SokuLib::KeyInput>> actionStrToInputs;
extern std::map<SokuLib::Character, std::vector<std::string>> characterSkills;
extern const std::map<std::string, SokuLib::Action> actionsFromStr;


#endif //SWRSTOYS_ACTIONS_HPP
