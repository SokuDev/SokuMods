//
// Created by PinkySmile on 24/07/2021.
//

#include "Actions.hpp"

#define FAKE_ACTION_ORRERIES_REACTIVATE static_cast<SokuLib::Action>(SokuLib::ACTION_AIR_ORRERIES_C + 1)
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
#define FAKE_ACTION_LILYPAD_2B static_cast<SokuLib::Action>(1107)
#define FAKE_ACTION_LILYPAD_6B static_cast<SokuLib::Action>(1108)
#define FAKE_ACTION_LILYPAD_2C static_cast<SokuLib::Action>(1109)
#define FAKE_ACTION_LILYPAD_SPAWN static_cast<SokuLib::Action>(1110)
#define FAKE_ACTION_LILYPAD_DESPAWN static_cast<SokuLib::Action>(1111)
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

std::map<SokuLib::Character, std::vector<std::string>> characterSkills{
	{ SokuLib::CHARACTER_REIMU,     {"236", "214", "421", "623"} },
	{ SokuLib::CHARACTER_MARISA,    {"214", "623", "22",  "236"} },
	{ SokuLib::CHARACTER_SAKUYA,    {"623", "214", "236", "22" } },
	{ SokuLib::CHARACTER_ALICE,     {"236", "623", "214", "22" } },
	{ SokuLib::CHARACTER_PATCHOULI, {"236", "22",  "623", "214", "421"} },
	{ SokuLib::CHARACTER_YOUMU,     {"236", "623", "214", "22" } },
	{ SokuLib::CHARACTER_REMILIA,   {"236", "214", "623", "22" } },
	{ SokuLib::CHARACTER_YUYUKO,    {"214", "236", "421", "623"} },
	{ SokuLib::CHARACTER_YUKARI,    {"236", "623", "214", "421"} },
	{ SokuLib::CHARACTER_SUIKA,     {"236", "623", "214", "22" } },
	{ SokuLib::CHARACTER_REISEN,    {"236", "214", "623", "22" } },
	{ SokuLib::CHARACTER_AYA,       {"236", "214", "22",  "421"} },
	{ SokuLib::CHARACTER_KOMACHI,   {"236", "623", "22",  "214"} },
	{ SokuLib::CHARACTER_IKU,       {"236", "623", "22",  "214"} },
	{ SokuLib::CHARACTER_TENSHI,    {"214", "22",  "236", "623"} },
	{ SokuLib::CHARACTER_SANAE,     {"236", "22",  "623", "214"} },
	{ SokuLib::CHARACTER_CIRNO,     {"236", "214", "22",  "623"} },
	{ SokuLib::CHARACTER_MEILING,   {"214", "623", "22",  "236"} },
	{ SokuLib::CHARACTER_UTSUHO,    {"623", "236", "22",  "214"} },
	{ SokuLib::CHARACTER_SUWAKO,    {"214", "623", "236", "22" } }
};

const std::map<std::string, SokuLib::Action> actionsFromStr{
	{ "66a",        SokuLib::ACTION_66A },
	{ "1a",         SokuLib::ACTION_2A },
	{ "f2a",        SokuLib::ACTION_f2A },
	{ "2a",         SokuLib::ACTION_2A },
	{ "3a",         SokuLib::ACTION_3A },
	{ "s3a",        SokuLib::ACTION_SUWAKO_3A },
	{ "l6a",        SokuLib::ACTION_SUWAKO_LILYPAD_6A },
	{ "4a",         SokuLib::ACTION_4A },
	{ "f5a",        SokuLib::ACTION_f5A },
	{ "5a",         SokuLib::ACTION_5A },
	{ "6a",         SokuLib::ACTION_6A },
	{ "5aa",        SokuLib::ACTION_5AA },
	{ "5aaa",       SokuLib::ACTION_5AAA },
	{ "5aaaa",      SokuLib::ACTION_5AAAA },
	{ "5aaaaa",     SokuLib::ACTION_5AAAAA },
	{ "5aaa6a",     FAKE_ACTION_5AAA6A },
	{ "5aaa3a",     SokuLib::ACTION_5AAA3A },
	{ "j2a",        SokuLib::ACTION_j2A },
	{ "j5a",        SokuLib::ACTION_j5A },
	{ "j5aa",       SokuLib::ACTION_j5AA },
	{ "j6a",        SokuLib::ACTION_j6A },
	{ "j8a",        SokuLib::ACTION_j8A },

	{ "66b",        SokuLib::ACTION_66B },
	{ "2b",         SokuLib::ACTION_2B },
	{ "3b",         SokuLib::ACTION_3B },
	{ "4b",         SokuLib::ACTION_4B },
	{ "5b",         SokuLib::ACTION_5B },
	{ "6b",         SokuLib::ACTION_6B },
	{ "j2b",        SokuLib::ACTION_j2B },
	{ "j4b",        SokuLib::ACTION_j4B },
	{ "j5b",        SokuLib::ACTION_j5B },
	{ "j6b",        SokuLib::ACTION_j6B },
	{ "Ob",         SokuLib::ACTION_ORRERIES_B },
	{ "jOb",        SokuLib::ACTION_AIR_ORRERIES_B },

	{ "66c",        SokuLib::ACTION_66C },
	{ "2c",         SokuLib::ACTION_2C },
	{ "5c",         SokuLib::ACTION_5C },
	{ "6c",         SokuLib::ACTION_6C },
	{ "j1c",        SokuLib::ACTION_j1C },
	{ "j2c",        SokuLib::ACTION_j2C },
	{ "j5c",        SokuLib::ACTION_j5C },
	{ "j6c",        SokuLib::ACTION_j6C },
	{ "Oc",         SokuLib::ACTION_ORRERIES_C },
	{ "jOc",        SokuLib::ACTION_AIR_ORRERIES_C },

	{ "dskill1b",   SokuLib::ACTION_DEFAULT_SKILL1_B },
	{ "dskill1c",   SokuLib::ACTION_DEFAULT_SKILL1_C },
	{ "a1skill1b",  SokuLib::ACTION_ALT1_SKILL1_B },
	{ "a1skill1c",  SokuLib::ACTION_ALT1_SKILL1_C },
	{ "a2skill1b",  SokuLib::ACTION_ALT2_SKILL1_B },
	{ "a2skill1c",  SokuLib::ACTION_ALT2_SKILL1_C },
	{ "jdskill1b",  SokuLib::ACTION_DEFAULT_SKILL1_AIR_B },
	{ "jdskill1c",  SokuLib::ACTION_DEFAULT_SKILL1_AIR_C },
	{ "ja1skill1b", SokuLib::ACTION_ALT1_SKILL1_AIR_B },
	{ "ja1skill1c", SokuLib::ACTION_ALT1_SKILL1_AIR_C },
	{ "ja2skill1b", SokuLib::ACTION_ALT2_SKILL1_AIR_B },
	{ "ja2skill1c", SokuLib::ACTION_ALT2_SKILL1_AIR_C },
	{ "dskill2b",   SokuLib::ACTION_DEFAULT_SKILL2_B },
	{ "dskill2c",   SokuLib::ACTION_DEFAULT_SKILL2_C },
	{ "a1skill2b",  SokuLib::ACTION_ALT1_SKILL2_B },
	{ "a1skill2c",  SokuLib::ACTION_ALT1_SKILL2_C },
	{ "a2skill2b",  SokuLib::ACTION_ALT2_SKILL2_B },
	{ "a2skill2c",  SokuLib::ACTION_ALT2_SKILL2_C },
	{ "jdskill2b",  SokuLib::ACTION_DEFAULT_SKILL2_AIR_B },
	{ "jdskill2c",  SokuLib::ACTION_DEFAULT_SKILL2_AIR_C },
	{ "ja1skill2b", SokuLib::ACTION_ALT1_SKILL2_AIR_B },
	{ "ja1skill2c", SokuLib::ACTION_ALT1_SKILL2_AIR_C },
	{ "ja2skill2b", SokuLib::ACTION_ALT2_SKILL2_AIR_B },
	{ "ja2skill2c", SokuLib::ACTION_ALT2_SKILL2_AIR_C },
	{ "dskill3b",   SokuLib::ACTION_DEFAULT_SKILL3_B },
	{ "dskill3c",   SokuLib::ACTION_DEFAULT_SKILL3_C },
	{ "a1skill3b",  SokuLib::ACTION_ALT1_SKILL3_B },
	{ "a1skill3c",  SokuLib::ACTION_ALT1_SKILL3_C },
	{ "a2skill3b",  SokuLib::ACTION_ALT2_SKILL3_B },
	{ "a2skill3c",  SokuLib::ACTION_ALT2_SKILL3_C },
	{ "jdskill3b",  SokuLib::ACTION_DEFAULT_SKILL3_AIR_B },
	{ "jdskill3c",  SokuLib::ACTION_DEFAULT_SKILL3_AIR_C },
	{ "ja1skill3b", SokuLib::ACTION_ALT1_SKILL3_AIR_B },
	{ "ja1skill3c", SokuLib::ACTION_ALT1_SKILL3_AIR_C },
	{ "ja2skill3b", SokuLib::ACTION_ALT2_SKILL3_AIR_B },
	{ "ja2skill3c", SokuLib::ACTION_ALT2_SKILL3_AIR_C },
	{ "dskill4b",   SokuLib::ACTION_DEFAULT_SKILL4_B },
	{ "dskill4c",   SokuLib::ACTION_DEFAULT_SKILL4_C },
	{ "a1skill4b",  SokuLib::ACTION_ALT1_SKILL4_B },
	{ "a1skill4c",  SokuLib::ACTION_ALT1_SKILL4_C },
	{ "a2skill4b",  SokuLib::ACTION_ALT2_SKILL4_B },
	{ "a2skill4c",  SokuLib::ACTION_ALT2_SKILL4_C },
	{ "jdskill4b",  SokuLib::ACTION_DEFAULT_SKILL4_AIR_B },
	{ "jdskill4c",  SokuLib::ACTION_DEFAULT_SKILL4_AIR_C },
	{ "ja1skill4b", SokuLib::ACTION_ALT1_SKILL4_AIR_B },
	{ "ja1skill4c", SokuLib::ACTION_ALT1_SKILL4_AIR_C },
	{ "ja2skill4b", SokuLib::ACTION_ALT2_SKILL4_AIR_B },
	{ "ja2skill4c", SokuLib::ACTION_ALT2_SKILL4_AIR_C },
	{ "dskill5b",   SokuLib::ACTION_DEFAULT_SKILL5_B },
	{ "dskill5c",   SokuLib::ACTION_DEFAULT_SKILL5_C },
	{ "a1skill5b",  SokuLib::ACTION_ALT1_SKILL5_B },
	{ "a1skill5c",  SokuLib::ACTION_ALT1_SKILL5_C },
	{ "a2skill5b",  SokuLib::ACTION_ALT2_SKILL5_B },
	{ "a2skill5c",  SokuLib::ACTION_ALT2_SKILL5_C },
	{ "jdskill5b",  SokuLib::ACTION_DEFAULT_SKILL5_AIR_B },
	{ "jdskill5c",  SokuLib::ACTION_DEFAULT_SKILL5_AIR_C },
	{ "ja1skill5b", SokuLib::ACTION_ALT1_SKILL5_AIR_B },
	{ "ja1skill5c", SokuLib::ACTION_ALT1_SKILL5_AIR_C },
	{ "ja2skill5b", SokuLib::ACTION_ALT2_SKILL5_AIR_B },
	{ "ja2skill5c", SokuLib::ACTION_ALT2_SKILL5_AIR_C },

	{ "sc200",      SokuLib::ACTION_USING_SC_ID_200 },
	{ "sc201",      SokuLib::ACTION_USING_SC_ID_201 },
	{ "sc202",      SokuLib::ACTION_USING_SC_ID_202 },
	{ "sc203",      SokuLib::ACTION_USING_SC_ID_203 },
	{ "sc204",      SokuLib::ACTION_USING_SC_ID_204 },
	{ "sc205",      SokuLib::ACTION_USING_SC_ID_205 },
	{ "sc206",      SokuLib::ACTION_USING_SC_ID_206 },
	{ "sc207",      SokuLib::ACTION_USING_SC_ID_207 },
	{ "sc208",      SokuLib::ACTION_USING_SC_ID_208 },
	{ "sc209",      SokuLib::ACTION_USING_SC_ID_209 },
	{ "sc210",      SokuLib::ACTION_USING_SC_ID_210 },
	{ "sc211",      SokuLib::ACTION_USING_SC_ID_211 },
	{ "sc212",      SokuLib::ACTION_USING_SC_ID_212 },
	{ "sc213",      SokuLib::ACTION_USING_SC_ID_213 },
	{ "sc214",      SokuLib::ACTION_USING_SC_ID_214 },
	{ "sc215",      SokuLib::ACTION_USING_SC_ID_215 },
	{ "sc216",      SokuLib::ACTION_USING_SC_ID_216 },
	{ "sc217",      SokuLib::ACTION_USING_SC_ID_217 },
	{ "sc218",      SokuLib::ACTION_USING_SC_ID_218 },
	{ "sc219",      SokuLib::ACTION_USING_SC_ID_219 },
	{ "jsc200",     SokuLib::ACTION_SC_ID_200_ALT_EFFECT },
	{ "jsc201",     SokuLib::ACTION_SC_ID_201_ALT_EFFECT },
	{ "jsc202",     SokuLib::ACTION_SC_ID_202_ALT_EFFECT },
	{ "jsc203",     SokuLib::ACTION_SC_ID_203_ALT_EFFECT },
	{ "jsc204",     SokuLib::ACTION_SC_ID_204_ALT_EFFECT },
	{ "jsc205",     SokuLib::ACTION_SC_ID_205_ALT_EFFECT },
	{ "jsc206",     SokuLib::ACTION_SC_ID_206_ALT_EFFECT },
	{ "jsc207",     SokuLib::ACTION_SC_ID_207_ALT_EFFECT },
	{ "jsc208",     SokuLib::ACTION_SC_ID_208_ALT_EFFECT },
	{ "jsc209",     SokuLib::ACTION_SC_ID_209_ALT_EFFECT },
	{ "jsc210",     SokuLib::ACTION_SC_ID_210_ALT_EFFECT },
	{ "jsc211",     SokuLib::ACTION_SC_ID_211_ALT_EFFECT },
	{ "jsc212",     SokuLib::ACTION_SC_ID_212_ALT_EFFECT },
	{ "jsc213",     SokuLib::ACTION_SC_ID_213_ALT_EFFECT },
	{ "jsc214",     SokuLib::ACTION_SC_ID_214_ALT_EFFECT },
	{ "jsc215",     SokuLib::ACTION_SC_ID_215_ALT_EFFECT },
	{ "jsc216",     SokuLib::ACTION_SC_ID_216_ALT_EFFECT },
	{ "jsc217",     SokuLib::ACTION_SC_ID_217_ALT_EFFECT },
	{ "jsc218",     SokuLib::ACTION_SC_ID_218_ALT_EFFECT },
	{ "jsc219",     SokuLib::ACTION_SC_ID_219_ALT_EFFECT },
	{"Osc215",         FAKE_ACTION_ORRERIES_REACTIVATE },

	{ "hjd9",       SokuLib::ACTION_FORWARD_HIGH_JUMP_FROM_GROUND_DASH },
	{ "hjd8",       SokuLib::ACTION_FORWARD_HIGH_JUMP_FROM_GROUND_DASH },
	{ "hj9",        SokuLib::ACTION_FORWARD_HIGH_JUMP },
	{ "hj8",        SokuLib::ACTION_NEUTRAL_HIGH_JUMP },
	{ "hj7",        SokuLib::ACTION_BACKWARD_HIGH_JUMP },
	{ "be1",        SokuLib::ACTION_BE1 },
	{ "be2",        SokuLib::ACTION_BE2 },
	{ "be4",        SokuLib::ACTION_BE4 },
	{ "be6",        SokuLib::ACTION_BE6 },

	{ "j66",        SokuLib::ACTION_FORWARD_AIRDASH },
	{ "j44",        SokuLib::ACTION_BACKWARD_AIRDASH },
	{ "44",         SokuLib::ACTION_BACKDASH },
	{ "66",         SokuLib::ACTION_FORWARD_DASH },
	{ "d4",         SokuLib::ACTION_BACKDASH },
	{ "d6",         SokuLib::ACTION_FORWARD_DASH },
	{ "j1d",        SokuLib::ACTION_FLY },
	{ "j2d",        SokuLib::ACTION_FLY },
	{ "j3d",        SokuLib::ACTION_FLY },
	{ "j4d",        SokuLib::ACTION_FLY },
	{ "j6d",        SokuLib::ACTION_FLY },
	{ "j7d",        SokuLib::ACTION_FLY },
	{ "j8d",        SokuLib::ACTION_FLY },
	{ "j9d",        SokuLib::ACTION_FLY },

	{ "9",          SokuLib::ACTION_FORWARD_JUMP },
	{ "8",          SokuLib::ACTION_NEUTRAL_JUMP },
	{ "7",          SokuLib::ACTION_BACKWARD_JUMP },

	{ "drug",       SokuLib::ACTION_SPELL_BREAKING_DRUG },
	{ "hackoruken", SokuLib::ACTION_FORWARD_DASH },

	{ "sj1d",    FAKE_ACTION_j1D },
	{ "sj3d",    FAKE_ACTION_j3D },
	{ "lhj8",    FAKE_ACTION_LILYPAD_NEUTRAL_HIGHJUMP },
	{ "lhj9",    FAKE_ACTION_LILYPAD_FORWARD_HIGHJUMP },
	{ "lhj7",    FAKE_ACTION_LILYPAD_BACKWARD_HIGHJUMP },
	{ "l5a",     FAKE_ACTION_LILYPAD_A },
	{ "l3a",     FAKE_ACTION_LILYPAD_3A },
	{ "l2b",     FAKE_ACTION_LILYPAD_2B },
	{ "l6b",     FAKE_ACTION_LILYPAD_6B },
	{ "l2c",     FAKE_ACTION_LILYPAD_2C },
	{ "2",       FAKE_ACTION_LILYPAD_SPAWN },
	{ "5",       FAKE_ACTION_LILYPAD_DESPAWN },
	{ "ld22b",   FAKE_ACTION_LILYPAD_DEFAULT_22B },
	{ "ld22c",   FAKE_ACTION_LILYPAD_DEFAULT_22C },
	{ "ua122b",  FAKE_ACTION_UNDERGROUND_ALT1_22B },
	{ "ua122c",  FAKE_ACTION_UNDERGROUND_ALT1_22C },
	{ "ld623b",  FAKE_ACTION_LILYPAD_DEFAULT_623b },
	{ "ud623b",  FAKE_ACTION_UNDERGROUND_DEFAULT_623b },
	{ "ud623c",  FAKE_ACTION_UNDERGROUND_DEFAULT_623c },
	{ "la1623b", FAKE_ACTION_LILYPAD_ALT1_623b },
	{ "la1623c", FAKE_ACTION_LILYPAD_ALT1_623c },
	{ "ud236b",  FAKE_ACTION_UNDERGROUND_DEFAULT_236b },
	{ "ud236c",  FAKE_ACTION_UNDERGROUND_DEFAULT_236c },
	{ "ua1236b", FAKE_ACTION_UNDERGROUND_ALT1_236b },
	{ "ua1236c", FAKE_ACTION_UNDERGROUND_ALT1_236c },
	{ "ua2236b", FAKE_ACTION_UNDERGROUND_ALT2_236b },
	{ "ua2236c", FAKE_ACTION_UNDERGROUND_ALT2_236c },
	{ "ud214b",  FAKE_ACTION_UNDERGROUND_DEFAULT_214b },
	{ "ud214c",  FAKE_ACTION_UNDERGROUND_DEFAULT_214c },
	{ "la1214b", FAKE_ACTION_LILYPAD_ALT1_214b },
	{ "la1214c", FAKE_ACTION_LILYPAD_ALT1_214c },
	{ "la2214b", FAKE_ACTION_LILYPAD_ALT2_214b },
	{ "la2214c", FAKE_ACTION_LILYPAD_ALT2_214c },
	{ "lSC201",  FAKE_ACTION_lSC201 },
	{ "jSC201",  FAKE_ACTION_jSC201 },
	{ "uSC202",  FAKE_ACTION_uSC202 },
	{ "jSC203",  FAKE_ACTION_jSC203 },
	{ "lSC203",  FAKE_ACTION_lSC203 },
	{ "jSC204",  FAKE_ACTION_jSC204 },
	{ "lSC205",  FAKE_ACTION_lSC205 },
	{ "uSC205",  FAKE_ACTION_uSC205 },
	{ "lSC206",  FAKE_ACTION_lSC206 },
	{ "jSC207",  FAKE_ACTION_jSC207 },
	{ "lSC209",  FAKE_ACTION_lSC209 },
	{ "jSC209",  FAKE_ACTION_jSC209 },
	{ "uSC212",  FAKE_ACTION_uSC212 },
};

const std::map<std::string, std::vector<SokuLib::KeyInput>> actionStrToInputs{
	{ "66a",        {{1, 0, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 0}, {1, 0, 0, 0, 0, 0, 0, 0}, {1, 0, 1, 0, 0, 0, 0, 0}} },
	{ "1a",         {{-1, 1, 1, 0, 0, 0, 0, 0}} },
	{ "2a",         {{0, 1, 1, 0, 0, 0, 0, 0}} },
	{ "f2a",        {{0, 1, 1, 0, 0, 0, 0, 0}} },
	{ "3a",         {{1, 1, 1, 0, 0, 0, 0, 0}} },
	{ "s3a",        {{1, 1, 1, 0, 0, 0, 0, 0}} },
	{ "4a",         {{-1, 0, 1, 0, 0, 0, 0, 0}} },
	{ "5a",         {{0, 0, 1, 0, 0, 0, 0, 0}} },
	{ "f5a",        {{0, 0, 1, 0, 0, 0, 0, 0}} },
	{ "6a",         {{1, 0, 1, 0, 0, 0, 0, 0}} },
	{ "l6a",        {{1, 0, 1, 0, 0, 0, 0, 0}} },
	{ "5aa",        {{0, 0, 1, 0, 0, 0, 0, 0}} },
	{ "5aaa",       {{0, 0, 1, 0, 0, 0, 0, 0}} },
	{ "5aaaa",      {{0, 0, 1, 0, 0, 0, 0, 0}} },
	{ "5aaaaa",     {{0, 0, 1, 0, 0, 0, 0, 0}} },
	{ "5aaa6a",     {{0, 0, 1, 0, 0, 0, 0, 0}, {1, 0, 1, 0, 0, 0, 0, 0}} },
	{ "5aaa3a",     {{0, 0, 1, 0, 0, 0, 0, 0}, {1, 1, 1, 0, 0, 0, 0, 0}} },
	{ "j2a",        {{0, 1, 1, 0, 0, 0, 0, 0}} },
	{ "j5a",        {{0, 0, 1, 0, 0, 0, 0, 0}} },
	{ "j5aa",       {{0, 0, 1, 0, 0, 0, 0, 0}} },
	{ "j6a",        {{1, 0, 1, 0, 0, 0, 0, 0}} },
	{ "j8a",        {{0, -1, 1, 0, 0, 0, 0, 0}} },

	{ "66b",        {{1, 0, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 0}, {1, 0, 0, 0, 0, 0, 0, 0}, {1, 0, 0, 1, 0, 0, 0, 0}} },
	{ "2b",         {{0, 1, 0, 1, 0, 0, 0, 0}} },
	{ "3b",         {{1, 1, 0, 1, 0, 0, 0, 0}} },
	{ "4b",         {{-1, 0, 0, 1, 0, 0, 0, 0}} },
	{ "5b",         {{0, 0, 0, 1, 0, 0, 0, 0}} },
	{ "6b",         {{1, 0, 0, 1, 0, 0, 0, 0}} },
	{ "j2b",        {{0, 1, 0, 1, 0, 0, 0, 0}} },
	{ "j4b",        {{1, 0, 0, 1, 0, 0, 0, 0}} },
	{ "j5b",        {{0, 0, 0, 1, 0, 0, 0, 0}} },
	{ "j6b",        {{1, 0, 0, 1, 0, 0, 0, 0}} },
	{ "Ob",         {{0, 0, 0, 1, 0, 0, 0, 0}} },
	{ "jOb",        {{0, 0, 0, 1, 0, 0, 0, 0}} },

	{ "66c",        {{1, 0, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 0}, {1, 0, 0, 0, 0, 0, 0, 0}, {1, 0, 0, 0, 1, 0, 0, 0}} },
	{ "2c",         {{0, 1, 0, 0, 1, 0, 0, 0}} },
	{ "5c",         {{0, 0, 0, 0, 1, 0, 0, 0}} },
	{ "6c",         {{1, 0, 0, 0, 1, 0, 0, 0}} },
	{ "j1c",        {{-1, 1, 0, 0, 1, 0, 0, 0}} },
	{ "j2c",        {{0, 1, 0, 0, 1, 0, 0, 0}} },
	{ "j5c",        {{0, 0, 0, 0, 1, 0, 0, 0}} },
	{ "j6c",        {{1, 0, 0, 0, 1, 0, 0, 0}} },
	{ "Oc",         {{0, 0, 0, 0, 1, 0, 0, 0}} },
	{ "jOc",        {{0, 0, 0, 0, 1, 0, 0, 0}} },

	{ "sc200",      {{0, 0, 0, 0, 0, 0, 0, 1}} },
	{ "sc201",      {{0, 0, 0, 0, 0, 0, 0, 1}} },
	{ "sc202",      {{0, 0, 0, 0, 0, 0, 0, 1}} },
	{ "sc203",      {{0, 0, 0, 0, 0, 0, 0, 1}} },
	{ "sc204",      {{0, 0, 0, 0, 0, 0, 0, 1}} },
	{ "sc205",      {{0, 0, 0, 0, 0, 0, 0, 1}} },
	{ "sc206",      {{0, 0, 0, 0, 0, 0, 0, 1}} },
	{ "sc207",      {{0, 0, 0, 0, 0, 0, 0, 1}} },
	{ "sc208",      {{0, 0, 0, 0, 0, 0, 0, 1}} },
	{ "sc209",      {{0, 0, 0, 0, 0, 0, 0, 1}} },
	{ "sc210",      {{0, 0, 0, 0, 0, 0, 0, 1}} },
	{ "sc211",      {{0, 0, 0, 0, 0, 0, 0, 1}} },
	{ "sc212",      {{0, 0, 0, 0, 0, 0, 0, 1}} },
	{ "sc213",      {{0, 0, 0, 0, 0, 0, 0, 1}} },
	{ "sc214",      {{0, 0, 0, 0, 0, 0, 0, 1}} },
	{ "sc215",      {{0, 0, 0, 0, 0, 0, 0, 1}} },
	{ "sc216",      {{0, 0, 0, 0, 0, 0, 0, 1}} },
	{ "sc217",      {{0, 0, 0, 0, 0, 0, 0, 1}} },
	{ "sc218",      {{0, 0, 0, 0, 0, 0, 0, 1}} },
	{ "sc219",      {{0, 0, 0, 0, 0, 0, 0, 1}} },
	{ "jsc200",     {{0, 0, 0, 0, 0, 0, 0, 1}} },
	{ "jsc201",     {{0, 0, 0, 0, 0, 0, 0, 1}} },
	{ "jsc202",     {{0, 0, 0, 0, 0, 0, 0, 1}} },
	{ "jsc203",     {{0, 0, 0, 0, 0, 0, 0, 1}} },
	{ "jsc204",     {{0, 0, 0, 0, 0, 0, 0, 1}} },
	{ "jsc205",     {{0, 0, 0, 0, 0, 0, 0, 1}} },
	{ "jsc206",     {{0, 0, 0, 0, 0, 0, 0, 1}} },
	{ "jsc207",     {{0, 0, 0, 0, 0, 0, 0, 1}} },
	{ "jsc208",     {{0, 0, 0, 0, 0, 0, 0, 1}} },
	{ "jsc209",     {{0, 0, 0, 0, 0, 0, 0, 1}} },
	{ "jsc210",     {{0, 0, 0, 0, 0, 0, 0, 1}} },
	{ "jsc211",     {{0, 0, 0, 0, 0, 0, 0, 1}} },
	{ "jsc212",     {{0, 0, 0, 0, 0, 0, 0, 1}} },
	{ "jsc213",     {{0, 0, 0, 0, 0, 0, 0, 1}} },
	{ "jsc214",     {{0, 0, 0, 0, 0, 0, 0, 1}} },
	{ "jsc215",     {{0, 0, 0, 0, 0, 0, 0, 1}} },
	{ "jsc216",     {{0, 0, 0, 0, 0, 0, 0, 1}} },
	{ "jsc217",     {{0, 0, 0, 0, 0, 0, 0, 1}} },
	{ "jsc218",     {{0, 0, 0, 0, 0, 0, 0, 1}} },
	{ "jsc219",     {{0, 0, 0, 0, 0, 0, 0, 1}} },
	{ "Osc215",     {{0, 0, 0, 0, 0, 0, 0, 1}} },

	{ "hjd9",       {{1, -1, 0, 0, 0, 1, 0, 0}} },
	{ "hjd8",       {{0, -1, 0, 0, 0, 1, 0, 0}} },
	{ "hj9",        {{1, -1, 0, 0, 0, 1, 0, 0}} },
	{ "hj8",        {{0, -1, 0, 0, 0, 1, 0, 0}} },
	{ "hj7",        {{-1, -1, 0, 0, 0, 1, 0, 0}} },
	{ "be1",        {{-0, 0, 0, 0, 0, 1, 0, 0}, {-1, 1, 0, 0, 0, 0, 0, 0}, {-1, 1, 0, 0, 0, 1, 0, 0}} },
	{ "be2",        {{-0, 0, 0, 0, 0, 1, 0, 0}, {0, 1, 0, 0, 0, 0, 0, 0}, {0, 1, 0, 0, 0, 1, 0, 0}} },
	{ "be4",        {{-0, 0, 0, 0, 0, 1, 0, 0}, {-1, 0, 0, 0, 0, 0, 0, 0}, {-1, 0, 0, 0, 0, 1, 0, 0}} },
	{ "be6",        {{-0, 0, 0, 0, 0, 1, 0, 0}, {1, 0, 0, 0, 0, 0, 0, 0}, {1, 0, 0, 0, 0, 1, 0, 0}} },

	{ "j66",        {{1, 0, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 0}, {1, 0, 0, 0, 0, 0, 0, 0}, {1, 0, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 0}} },
	{ "j44",        {{-1, 0, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 0}, {-1, 0, 0, 0, 0, 0, 0, 0}, {1, 0, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 0}} },
	{ "44",         {{-1, 0, 0, 0, 0, 1, 0, 0}} },
	{ "66",         {{1, 0, 0, 0, 0, 1, 0, 0}} },
	{ "d4",         {{-1, 0, 0, 0, 0, 1, 0, 0}} },
	{ "d6",         {{1, 0, 0, 0, 0, 1, 0, 0}} },
	{ "j1d",        {{-1, 1, 0, 0, 0, 1, 0, 0}} },
	{ "j2d",        {{0, 1, 0, 0, 0, 1, 0, 0}} },
	{ "j3d",        {{1, 1, 0, 0, 0, 1, 0, 0}} },
	{ "j4d",        {{-1, 0, 0, 0, 0, 1, 0, 0}} },
	{ "j6d",        {{1, 0, 0, 0, 0, 1, 0, 0}} },
	{ "j7d",        {{-1, -1, 0, 0, 0, 1, 0, 0}} },
	{ "j8d",        {{0, -1, 0, 0, 0, 1, 0, 0}} },
	{ "j9d",        {{1, -1, 0, 0, 0, 1, 0, 0}} },

	{ "d22b",       {{0, 1, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 0}, {0, 1, 0, 1, 0, 0, 0, 0}} },
	{ "a122b",      {{0, 1, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 0}, {0, 1, 0, 1, 0, 0, 0, 0}} },
	{ "a222b",      {{0, 1, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 0}, {0, 1, 0, 1, 0, 0, 0, 0}} },
	{ "jd22b",      {{0, 1, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 0}, {0, 1, 0, 1, 0, 0, 0, 0}} },
	{ "ja122b",     {{0, 1, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 0}, {0, 1, 0, 1, 0, 0, 0, 0}} },
	{ "ja222b",     {{0, 1, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 0}, {0, 1, 0, 1, 0, 0, 0, 0}} },
	{ "d22c",       {{0, 1, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 0}, {0, 1, 0, 0, 1, 0, 0, 0}} },
	{ "a122c",      {{0, 1, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 0}, {0, 1, 0, 0, 1, 0, 0, 0}} },
	{ "a222c",      {{0, 1, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 0}, {0, 1, 0, 0, 1, 0, 0, 0}} },
	{ "jd22c",      {{0, 1, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 0}, {0, 1, 0, 0, 1, 0, 0, 0}} },
	{ "ja122c",     {{0, 1, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 0}, {0, 1, 0, 0, 1, 0, 0, 0}} },
	{ "ja222c",     {{0, 1, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 0}, {0, 1, 0, 0, 1, 0, 0, 0}} },

	{ "d214b",       {{0, 1, 0, 0, 0, 0, 0, 0}, {-1, 1, 0, 0, 0, 0, 0, 0}, {-1, 0, 0, 1, 0, 0, 0, 0}} },
	{ "a1214b",      {{0, 1, 0, 0, 0, 0, 0, 0}, {-1, 1, 0, 0, 0, 0, 0, 0}, {-1, 0, 0, 1, 0, 0, 0, 0}} },
	{ "a2214b",      {{0, 1, 0, 0, 0, 0, 0, 0}, {-1, 1, 0, 0, 0, 0, 0, 0}, {-1, 0, 0, 1, 0, 0, 0, 0}} },
	{ "jd214b",      {{0, 1, 0, 0, 0, 0, 0, 0}, {-1, 1, 0, 0, 0, 0, 0, 0}, {-1, 0, 0, 1, 0, 0, 0, 0}} },
	{ "ja1214b",     {{0, 1, 0, 0, 0, 0, 0, 0}, {-1, 1, 0, 0, 0, 0, 0, 0}, {-1, 0, 0, 1, 0, 0, 0, 0}} },
	{ "ja2214b",     {{0, 1, 0, 0, 0, 0, 0, 0}, {-1, 1, 0, 0, 0, 0, 0, 0}, {-1, 0, 0, 1, 0, 0, 0, 0}} },
	{ "d214c",       {{0, 1, 0, 0, 0, 0, 0, 0}, {-1, 1, 0, 0, 0, 0, 0, 0}, {-1, 0, 0, 0, 1, 0, 0, 0}} },
	{ "a1214c",      {{0, 1, 0, 0, 0, 0, 0, 0}, {-1, 1, 0, 0, 0, 0, 0, 0}, {-1, 0, 0, 0, 1, 0, 0, 0}} },
	{ "a2214c",      {{0, 1, 0, 0, 0, 0, 0, 0}, {-1, 1, 0, 0, 0, 0, 0, 0}, {-1, 0, 0, 0, 1, 0, 0, 0}} },
	{ "jd214c",      {{0, 1, 0, 0, 0, 0, 0, 0}, {-1, 1, 0, 0, 0, 0, 0, 0}, {-1, 0, 0, 0, 1, 0, 0, 0}} },
	{ "ja1214c",     {{0, 1, 0, 0, 0, 0, 0, 0}, {-1, 1, 0, 0, 0, 0, 0, 0}, {-1, 0, 0, 0, 1, 0, 0, 0}} },
	{ "ja2214c",     {{0, 1, 0, 0, 0, 0, 0, 0}, {-1, 1, 0, 0, 0, 0, 0, 0}, {-1, 0, 0, 0, 1, 0, 0, 0}} },

	{ "d236b",       {{0, 1, 0, 0, 0, 0, 0, 0}, {1, 1, 0, 0, 0, 0, 0, 0}, {1, 0, 0, 1, 0, 0, 0, 0}} },
	{ "a1236b",      {{0, 1, 0, 0, 0, 0, 0, 0}, {1, 1, 0, 0, 0, 0, 0, 0}, {1, 0, 0, 1, 0, 0, 0, 0}} },
	{ "a2236b",      {{0, 1, 0, 0, 0, 0, 0, 0}, {1, 1, 0, 0, 0, 0, 0, 0}, {1, 0, 0, 1, 0, 0, 0, 0}} },
	{ "jd236b",      {{0, 1, 0, 0, 0, 0, 0, 0}, {1, 1, 0, 0, 0, 0, 0, 0}, {1, 0, 0, 1, 0, 0, 0, 0}} },
	{ "ja1236b",     {{0, 1, 0, 0, 0, 0, 0, 0}, {1, 1, 0, 0, 0, 0, 0, 0}, {1, 0, 0, 1, 0, 0, 0, 0}} },
	{ "ja2236b",     {{0, 1, 0, 0, 0, 0, 0, 0}, {1, 1, 0, 0, 0, 0, 0, 0}, {1, 0, 0, 1, 0, 0, 0, 0}} },
	{ "d236c",       {{0, 1, 0, 0, 0, 0, 0, 0}, {1, 1, 0, 0, 0, 0, 0, 0}, {1, 0, 0, 0, 1, 0, 0, 0}} },
	{ "a1236c",      {{0, 1, 0, 0, 0, 0, 0, 0}, {1, 1, 0, 0, 0, 0, 0, 0}, {1, 0, 0, 0, 1, 0, 0, 0}} },
	{ "a2236c",      {{0, 1, 0, 0, 0, 0, 0, 0}, {1, 1, 0, 0, 0, 0, 0, 0}, {1, 0, 0, 0, 1, 0, 0, 0}} },
	{ "jd236c",      {{0, 1, 0, 0, 0, 0, 0, 0}, {1, 1, 0, 0, 0, 0, 0, 0}, {1, 0, 0, 0, 1, 0, 0, 0}} },
	{ "ja1236c",     {{0, 1, 0, 0, 0, 0, 0, 0}, {1, 1, 0, 0, 0, 0, 0, 0}, {1, 0, 0, 0, 1, 0, 0, 0}} },
	{ "ja2236c",     {{0, 1, 0, 0, 0, 0, 0, 0}, {1, 1, 0, 0, 0, 0, 0, 0}, {1, 0, 0, 0, 1, 0, 0, 0}} },

	{ "d421b",       {{-1, 0, 0, 0, 0, 0, 0, 0}, {0, 1, 0, 0, 0, 0, 0, 0}, {-1, 1, 0, 1, 0, 0, 0, 0}} },
	{ "a1421b",      {{-1, 0, 0, 0, 0, 0, 0, 0}, {0, 1, 0, 0, 0, 0, 0, 0}, {-1, 1, 0, 1, 0, 0, 0, 0}} },
	{ "a2421b",      {{-1, 0, 0, 0, 0, 0, 0, 0}, {0, 1, 0, 0, 0, 0, 0, 0}, {-1, 1, 0, 1, 0, 0, 0, 0}} },
	{ "jd421b",      {{-1, 0, 0, 0, 0, 0, 0, 0}, {0, 1, 0, 0, 0, 0, 0, 0}, {-1, 1, 0, 1, 0, 0, 0, 0}} },
	{ "ja1421b",     {{-1, 0, 0, 0, 0, 0, 0, 0}, {0, 1, 0, 0, 0, 0, 0, 0}, {-1, 1, 0, 1, 0, 0, 0, 0}} },
	{ "ja2421b",     {{-1, 0, 0, 0, 0, 0, 0, 0}, {0, 1, 0, 0, 0, 0, 0, 0}, {-1, 1, 0, 1, 0, 0, 0, 0}} },
	{ "d421c",       {{-1, 0, 0, 0, 0, 0, 0, 0}, {0, 1, 0, 0, 0, 0, 0, 0}, {-1, 1, 0, 0, 1, 0, 0, 0}} },
	{ "a1421c",      {{-1, 0, 0, 0, 0, 0, 0, 0}, {0, 1, 0, 0, 0, 0, 0, 0}, {-1, 1, 0, 0, 1, 0, 0, 0}} },
	{ "a2421c",      {{-1, 0, 0, 0, 0, 0, 0, 0}, {0, 1, 0, 0, 0, 0, 0, 0}, {-1, 1, 0, 0, 1, 0, 0, 0}} },
	{ "jd421c",      {{-1, 0, 0, 0, 0, 0, 0, 0}, {0, 1, 0, 0, 0, 0, 0, 0}, {-1, 1, 0, 0, 1, 0, 0, 0}} },
	{ "ja1421c",     {{-1, 0, 0, 0, 0, 0, 0, 0}, {0, 1, 0, 0, 0, 0, 0, 0}, {-1, 1, 0, 0, 1, 0, 0, 0}} },
	{ "ja2421c",     {{-1, 0, 0, 0, 0, 0, 0, 0}, {0, 1, 0, 0, 0, 0, 0, 0}, {-1, 1, 0, 0, 1, 0, 0, 0}} },

	{ "d623b",       {{1, 0, 0, 0, 0, 0, 0, 0}, {0, 1, 0, 0, 0, 0, 0, 0}, {1, 1, 0, 1, 0, 0, 0, 0}} },
	{ "a1623b",      {{1, 0, 0, 0, 0, 0, 0, 0}, {0, 1, 0, 0, 0, 0, 0, 0}, {1, 1, 0, 1, 0, 0, 0, 0}} },
	{ "a2623b",      {{1, 0, 0, 0, 0, 0, 0, 0}, {0, 1, 0, 0, 0, 0, 0, 0}, {1, 1, 0, 1, 0, 0, 0, 0}} },
	{ "jd623b",      {{1, 0, 0, 0, 0, 0, 0, 0}, {0, 1, 0, 0, 0, 0, 0, 0}, {1, 1, 0, 1, 0, 0, 0, 0}} },
	{ "ja1623b",     {{1, 0, 0, 0, 0, 0, 0, 0}, {0, 1, 0, 0, 0, 0, 0, 0}, {1, 1, 0, 1, 0, 0, 0, 0}} },
	{ "ja2623b",     {{1, 0, 0, 0, 0, 0, 0, 0}, {0, 1, 0, 0, 0, 0, 0, 0}, {1, 1, 0, 1, 0, 0, 0, 0}} },
	{ "d623c",       {{1, 0, 0, 0, 0, 0, 0, 0}, {0, 1, 0, 0, 0, 0, 0, 0}, {1, 1, 0, 0, 1, 0, 0, 0}} },
	{ "a1623c",      {{1, 0, 0, 0, 0, 0, 0, 0}, {0, 1, 0, 0, 0, 0, 0, 0}, {1, 1, 0, 0, 1, 0, 0, 0}} },
	{ "a2623c",      {{1, 0, 0, 0, 0, 0, 0, 0}, {0, 1, 0, 0, 0, 0, 0, 0}, {1, 1, 0, 0, 1, 0, 0, 0}} },
	{ "jd623c",      {{1, 0, 0, 0, 0, 0, 0, 0}, {0, 1, 0, 0, 0, 0, 0, 0}, {1, 1, 0, 0, 1, 0, 0, 0}} },
	{ "ja1623c",     {{1, 0, 0, 0, 0, 0, 0, 0}, {0, 1, 0, 0, 0, 0, 0, 0}, {1, 1, 0, 0, 1, 0, 0, 0}} },
	{ "ja2623c",     {{1, 0, 0, 0, 0, 0, 0, 0}, {0, 1, 0, 0, 0, 0, 0, 0}, {1, 1, 0, 0, 1, 0, 0, 0}} },

	{ "9",          {{1, -1, 0, 0, 0, 0, 0, 0}} },
	{ "8",          {{0, -1, 0, 0, 0, 0, 0, 0}} },
	{ "7",          {{-1, -1, 0, 0, 0, 0, 0, 0}} },

	{ "drug",       {{0, 0, 0, 0, 0, 0, 0, 1}} },
	{ "hackoruken", {{0, 0, 0, 0, 0, 0, 0, 1}} },

	{ "sj1d",       {{-1, 1, 0, 0, 0, 1, 0, 0}} },
	{ "sj3d",       {{1, 1, 0, 0, 0, 1, 0, 0}} },
	{ "lhj8",       {{0, -1, 0, 0, 0, 1, 0, 0}} },
	{ "lhj9",       {{1, -1, 0, 0, 0, 1, 0, 0}} },
	{ "lhj7",       {{-1, -1, 0, 0, 0, 1, 0, 0}} },
	{ "l5a",        {{0, 0, 1, 0, 0, 0, 0, 0}} },
	{ "l3a",        {{1, 1, 1, 0, 0, 0, 0, 0}} },
	{ "l2b",        {{0, 1, 0, 1, 0, 0, 0, 0}} },
	{ "l6b",        {{1, 0, 0, 1, 0, 0, 0, 0}} },
	{ "l2c",        {{0, 1, 0, 0, 1, 0, 0, 0}} },
	{ "2",          {{0, 1, 0, 0, 0, 0, 0, 0}} },
	{ "5",          {{0, 0, 0, 0, 0, 0, 0, 0}} },
	{ "ld22b",      {{0, 1, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 0}, {0, 1, 0, 1, 0, 0, 0, 0}} },
	{ "ld22c",      {{0, 1, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 0}, {0, 1, 0, 0, 1, 0, 0, 0}} },
	{ "ua122b",     {{0, 1, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 0}, {0, 1, 0, 1, 0, 0, 0, 0}} },
	{ "ua122c",     {{0, 1, 0, 0, 0, 0, 0, 0}, {0, 0, 0, 0, 0, 0, 0, 0}, {0, 1, 0, 0, 1, 0, 0, 0}} },
	{ "ld623b",     {{1, 0, 0, 0, 0, 0, 0, 0}, {0, 1, 0, 0, 0, 0, 0, 0}, {1, 1, 0, 1, 0, 0, 0, 0}} },
	{ "ud623b",     {{1, 0, 0, 0, 0, 0, 0, 0}, {0, 1, 0, 0, 0, 0, 0, 0}, {1, 1, 0, 1, 0, 0, 0, 0}} },
	{ "ud623c",     {{1, 0, 0, 0, 0, 0, 0, 0}, {0, 1, 0, 0, 0, 0, 0, 0}, {1, 1, 0, 0, 1, 0, 0, 0}} },
	{ "la1623b",    {{1, 0, 0, 0, 0, 0, 0, 0}, {0, 1, 0, 0, 0, 0, 0, 0}, {1, 1, 0, 1, 0, 0, 0, 0}} },
	{ "la1623c",    {{1, 0, 0, 0, 0, 0, 0, 0}, {0, 1, 0, 0, 0, 0, 0, 0}, {1, 1, 0, 0, 1, 0, 0, 0}} },
	{ "ud236b",     {{0, 1, 0, 0, 0, 0, 0, 0}, {1, 1, 0, 0, 0, 0, 0, 0}, {1, 0, 0, 1, 0, 0, 0, 0}} },
	{ "ud236c",     {{0, 1, 0, 0, 0, 0, 0, 0}, {1, 1, 0, 0, 0, 0, 0, 0}, {1, 0, 0, 0, 1, 0, 0, 0}} },
	{ "ua1236b",    {{0, 1, 0, 0, 0, 0, 0, 0}, {1, 1, 0, 0, 0, 0, 0, 0}, {1, 0, 0, 1, 0, 0, 0, 0}} },
	{ "ua1236c",    {{0, 1, 0, 0, 0, 0, 0, 0}, {1, 1, 0, 0, 0, 0, 0, 0}, {1, 0, 0, 0, 1, 0, 0, 0}} },
	{ "ua2236b",    {{0, 1, 0, 0, 0, 0, 0, 0}, {1, 1, 0, 0, 0, 0, 0, 0}, {1, 0, 0, 1, 0, 0, 0, 0}} },
	{ "ua2236c",    {{0, 1, 0, 0, 0, 0, 0, 0}, {1, 1, 0, 0, 0, 0, 0, 0}, {1, 0, 0, 0, 1, 0, 0, 0}} },
	{ "ud214b",     {{0, 1, 0, 0, 0, 0, 0, 0}, {-1, 1, 0, 0, 0, 0, 0, 0}, {-1, 0, 0, 1, 0, 0, 0, 0}} },
	{ "ud214c",     {{0, 1, 0, 0, 0, 0, 0, 0}, {-1, 1, 0, 0, 0, 0, 0, 0}, {-1, 0, 0, 0, 1, 0, 0, 0}} },
	{ "la1214b",    {{0, 1, 0, 0, 0, 0, 0, 0}, {-1, 1, 0, 0, 0, 0, 0, 0}, {-1, 0, 0, 1, 0, 0, 0, 0}} },
	{ "la1214c",    {{0, 1, 0, 0, 0, 0, 0, 0}, {-1, 1, 0, 0, 0, 0, 0, 0}, {-1, 0, 0, 0, 1, 0, 0, 0}} },
	{ "la2214b",    {{0, 1, 0, 0, 0, 0, 0, 0}, {-1, 1, 0, 0, 0, 0, 0, 0}, {-1, 0, 0, 1, 0, 0, 0, 0}} },
	{ "la2214c",    {{0, 1, 0, 0, 0, 0, 0, 0}, {-1, 1, 0, 0, 0, 0, 0, 0}, {-1, 0, 0, 0, 1, 0, 0, 0}} },
	{ "lSC201",     {{0, 0, 0, 0, 0, 0, 0, 1}} },
	{ "jSC201",     {{0, 0, 0, 0, 0, 0, 0, 1}} },
	{ "uSC202",     {{0, 0, 0, 0, 0, 0, 0, 1}} },
	{ "jSC203",     {{0, 0, 0, 0, 0, 0, 0, 1}} },
	{ "lSC203",     {{0, 0, 0, 0, 0, 0, 0, 1}} },
	{ "jSC204",     {{0, 0, 0, 0, 0, 0, 0, 1}} },
	{ "lSC205",     {{0, 0, 0, 0, 0, 0, 0, 1}} },
	{ "uSC205",     {{0, 0, 0, 0, 0, 0, 0, 1}} },
	{ "lSC206",     {{0, 0, 0, 0, 0, 0, 0, 1}} },
	{ "jSC207",     {{0, 0, 0, 0, 0, 0, 0, 1}} },
	{ "lSC209",     {{0, 0, 0, 0, 0, 0, 0, 1}} },
	{ "jSC209",     {{0, 0, 0, 0, 0, 0, 0, 1}} },
	{ "uSC212",     {{0, 0, 0, 0, 0, 0, 0, 1}} },
};


bool isStartOfMove(SokuLib::Action action, const SokuLib::CharacterManager &character, SokuLib::Character characterId)
{
	if (action == FAKE_ACTION_ORRERIES_REACTIVATE)
		return character.objectBase.frameCount == 0 && character.objectBase.actionBlockId == 1;
	if (characterId == SokuLib::CHARACTER_YUKARI && (action == SokuLib::ACTION_4A || action == SokuLib::ACTION_5A))
		return character.objectBase.frameCount == 1 && character.objectBase.actionBlockId == 0;
	if (characterId == SokuLib::CHARACTER_AYA && action == SokuLib::ACTION_66B)
		return character.objectBase.frameCount == 1 && character.objectBase.actionBlockId == 0;

	if (characterId == SokuLib::CHARACTER_REMILIA || characterId == SokuLib::CHARACTER_FLANDRE) {
		if (action == SokuLib::ACTION_ALT1_SKILL1_B || action == SokuLib::ACTION_ALT1_SKILL1_C)
			return character.objectBase.frameCount == 0 && character.objectBase.actionBlockId < 4;
	}

	if (characterId == SokuLib::CHARACTER_YOUMU) {
		if (action == SokuLib::ACTION_DEFAULT_SKILL3_B)
			return character.objectBase.frameCount == 0 && character.objectBase.actionBlockId < 3;
		if (action == SokuLib::ACTION_DEFAULT_SKILL3_C)
			return character.objectBase.frameCount == 0 && (character.objectBase.actionBlockId == 0 || character.objectBase.actionBlockId == 6);
	}

	if (characterId == SokuLib::CHARACTER_REIMU) {
		if (action == SokuLib::ACTION_ALT1_SKILL4_B || action == SokuLib::ACTION_ALT1_SKILL4_C)
			return character.objectBase.frameCount == 0 && (character.objectBase.actionBlockId == 0 || character.objectBase.actionBlockId == 2 || character.objectBase.actionBlockId == 7);
	}

	if (characterId == SokuLib::CHARACTER_SUWAKO) {
		if (action == FAKE_ACTION_LILYPAD_SPAWN)
			return character.objectBase.frameCount == 0 && character.objectBase.animationCounter == 3;
		if (action == FAKE_ACTION_LILYPAD_DESPAWN)
			return character.objectBase.frameCount == 0 && character.objectBase.animationCounter == 3;
		if (action == FAKE_ACTION_UNDERGROUND_DEFAULT_623c || action == FAKE_ACTION_UNDERGROUND_DEFAULT_623b)
			return character.objectBase.frameCount == 0 && character.objectBase.actionBlockId == 6;
		if (action >= FAKE_ACTION_LILYPAD_NEUTRAL_HIGHJUMP && action <= FAKE_ACTION_LILYPAD_BACKWARD_HIGHJUMP)
			return character.objectBase.frameCount == 0 && character.objectBase.actionBlockId == 4;
		if (action == FAKE_ACTION_j1D || action == FAKE_ACTION_j3D)
			return character.objectBase.frameCount == 10 && (character.objectBase.actionBlockId == 3 || character.objectBase.actionBlockId == 0);
		if (action == SokuLib::ACTION_SUWAKO_j2D)
			return character.objectBase.frameCount == 0 && (character.objectBase.actionBlockId == 3 || character.objectBase.actionBlockId == 0);
		if (action == SokuLib::ACTION_j6C)
			return character.objectBase.frameCount == 0 && character.objectBase.actionBlockId == 0 && character.objectBase.animationCounter == 0;
		if (action == SokuLib::ACTION_USING_SC_ID_201 || action == SokuLib::ACTION_SC_ID_201_ALT_EFFECT)
			return character.objectBase.frameCount == 0 && character.objectBase.actionBlockId == 0 && character.objectBase.animationCounter == 0;
	}
	return character.objectBase.frameCount == 0 && character.objectBase.actionBlockId == 0;
}

SokuLib::Action addCustomActions(SokuLib::CharacterManager &character, SokuLib::Character characterId)
{
	if (characterId == SokuLib::CHARACTER_MARISA && character.orreriesTimeLeft && character.objectBase.action == SokuLib::ACTION_USING_SC_ID_215)
		return FAKE_ACTION_ORRERIES_REACTIVATE;
	if (characterId == SokuLib::CHARACTER_IKU && (character.objectBase.action >= SokuLib::ACTION_5AAA && character.objectBase.action <= SokuLib::ACTION_5AAAAA))
		return static_cast<SokuLib::Action>(character.objectBase.action - 1);
	if (characterId == SokuLib::CHARACTER_MEILING && character.objectBase.action == SokuLib::ACTION_5AAAAA)
		return FAKE_ACTION_5AAA6A;
	if (characterId == SokuLib::CHARACTER_YOUMU && character.objectBase.action == SokuLib::ACTION_DEFAULT_SKILL3_B && character.objectBase.actionBlockId == 6)
		return SokuLib::ACTION_DEFAULT_SKILL3_C;
	if ((characterId == SokuLib::CHARACTER_REMILIA || characterId == SokuLib::CHARACTER_FLANDRE) && character.objectBase.action == SokuLib::ACTION_ALT1_SKILL1_B && character.objectBase.actionBlockId == 3)
		return SokuLib::ACTION_ALT1_SKILL1_C;
	//My mom says I am special !
	if (characterId == SokuLib::CHARACTER_SUWAKO)
		switch (character.objectBase.action) {
		case SokuLib::ACTION_NEUTRAL_HIGH_JUMP:
			if (character.objectBase.actionBlockId == 4)
				return FAKE_ACTION_LILYPAD_NEUTRAL_HIGHJUMP;
			break;
		case SokuLib::ACTION_BACKWARD_HIGH_JUMP:
			if (character.objectBase.actionBlockId == 4)
				return FAKE_ACTION_LILYPAD_BACKWARD_HIGHJUMP;
			break;
		case SokuLib::ACTION_FORWARD_HIGH_JUMP:
			if (character.objectBase.actionBlockId == 4)
				return FAKE_ACTION_LILYPAD_FORWARD_HIGHJUMP;
			break;
		case SokuLib::ACTION_SUWAKO_j1D_j3D:
			if (copysign(1, character.objectBase.speed.x) == character.objectBase.direction)
				return FAKE_ACTION_j3D;
			return FAKE_ACTION_j1D;
		case SokuLib::ACTION_2A:
			return FAKE_ACTION_LILYPAD_A;
		case SokuLib::ACTION_3A:
			return FAKE_ACTION_LILYPAD_3A;
		case SokuLib::ACTION_2B:
			return FAKE_ACTION_LILYPAD_2B;
		case SokuLib::ACTION_3B:
			return FAKE_ACTION_LILYPAD_6B;
		case SokuLib::ACTION_2C:
			return FAKE_ACTION_LILYPAD_2C;
		case SokuLib::ACTION_CROUCHING:
			return FAKE_ACTION_LILYPAD_SPAWN;
		case SokuLib::ACTION_STANDING_UP:
			return FAKE_ACTION_LILYPAD_DESPAWN;
		case SokuLib::ACTION_DEFAULT_SKILL4_AIR_B:
			return FAKE_ACTION_LILYPAD_DEFAULT_22B;
		case SokuLib::ACTION_DEFAULT_SKILL4_AIR_C:
			return FAKE_ACTION_LILYPAD_DEFAULT_22C;
		case SokuLib::ACTION_ALT1_SKILL4_AIR_B:
			return FAKE_ACTION_UNDERGROUND_ALT1_22B;
		case SokuLib::ACTION_ALT1_SKILL4_AIR_C:
			return FAKE_ACTION_UNDERGROUND_ALT1_22C;
		case SokuLib::ACTION_DEFAULT_SKILL2_AIR_B:
			if (character.objectBase.actionBlockId == 6)
				return FAKE_ACTION_UNDERGROUND_DEFAULT_623c;
			return FAKE_ACTION_LILYPAD_DEFAULT_623b;
		case SokuLib::ACTION_DEFAULT_SKILL2_B:
			if (character.objectBase.actionBlockId == 6)
				return FAKE_ACTION_UNDERGROUND_DEFAULT_623b;
			break;
		case SokuLib::ACTION_ALT1_SKILL2_B:
			return FAKE_ACTION_LILYPAD_ALT1_623b;
		case SokuLib::ACTION_ALT1_SKILL2_C:
			return FAKE_ACTION_LILYPAD_ALT1_623c;
		case SokuLib::ACTION_DEFAULT_SKILL3_B:
			return FAKE_ACTION_UNDERGROUND_DEFAULT_236b;
		case SokuLib::ACTION_DEFAULT_SKILL3_C:
			return FAKE_ACTION_UNDERGROUND_DEFAULT_236c;
		case SokuLib::ACTION_ALT1_SKILL3_B:
			return FAKE_ACTION_UNDERGROUND_ALT1_236b;
		case SokuLib::ACTION_ALT1_SKILL3_C:
			return FAKE_ACTION_UNDERGROUND_ALT1_236c;
		case SokuLib::ACTION_ALT2_SKILL3_B:
			return FAKE_ACTION_UNDERGROUND_ALT2_236b;
		case SokuLib::ACTION_ALT2_SKILL3_C:
			return FAKE_ACTION_UNDERGROUND_ALT2_236c;
		case SokuLib::ACTION_DEFAULT_SKILL1_B:
			return FAKE_ACTION_UNDERGROUND_DEFAULT_214b;
		case SokuLib::ACTION_DEFAULT_SKILL1_C:
			return FAKE_ACTION_UNDERGROUND_DEFAULT_214c;
		case SokuLib::ACTION_ALT1_SKILL1_AIR_B:
			return FAKE_ACTION_LILYPAD_ALT1_214b;
		case SokuLib::ACTION_ALT1_SKILL1_AIR_C:
			return FAKE_ACTION_LILYPAD_ALT1_214c;
		case SokuLib::ACTION_ALT2_SKILL1_AIR_B:
			return FAKE_ACTION_LILYPAD_ALT2_214b;
		case SokuLib::ACTION_ALT2_SKILL1_AIR_C:
			return FAKE_ACTION_LILYPAD_ALT2_214c;
		case SokuLib::ACTION_USING_SC_ID_201:
			return FAKE_ACTION_lSC201;
		case SokuLib::ACTION_SC_ID_201_ALT_EFFECT:
			return FAKE_ACTION_jSC201;
		case SokuLib::ACTION_SC_ID_202_ALT_EFFECT:
			return FAKE_ACTION_uSC202;
		case SokuLib::ACTION_SC_ID_203_ALT_EFFECT:
			return FAKE_ACTION_jSC203;
		case SokuLib::ACTION_USING_SC_ID_203:
			return FAKE_ACTION_lSC203;
		case SokuLib::ACTION_SC_ID_204_ALT_EFFECT:
			return FAKE_ACTION_jSC204;
		case SokuLib::ACTION_USING_SC_ID_205:
			return FAKE_ACTION_lSC205;
		case SokuLib::ACTION_SC_ID_205_ALT_EFFECT:
			return FAKE_ACTION_uSC205;
		case SokuLib::ACTION_SC_ID_206_ALT_EFFECT:
			return FAKE_ACTION_lSC206;
		case SokuLib::ACTION_SC_ID_207_ALT_EFFECT:
			return FAKE_ACTION_jSC207;
		case SokuLib::ACTION_USING_SC_ID_209:
			return FAKE_ACTION_lSC209;
		case SokuLib::ACTION_SC_ID_209_ALT_EFFECT:
			return FAKE_ACTION_jSC209;
		case SokuLib::ACTION_USING_SC_ID_212:
			return FAKE_ACTION_uSC212;
		default:
			break;
	}
	return character.objectBase.action;
}