//
// Created by PinkySmile on 15/01/2021.
//

#include <SokuLib.hpp>
#include "vars.hpp"

std::map<unsigned int, std::pair<std::string, std::string>> charactersNames{
	{ SokuLib::CHARACTER_REIMU, {"Reimu", "Reimu Hakurei"}},
	{ SokuLib::CHARACTER_MARISA, {"Marisa", "Marisa Kirisame"}},
	{ SokuLib::CHARACTER_SAKUYA, {"Sakuya", "Sakuya Izayoi"}},
	{ SokuLib::CHARACTER_ALICE, {"Alice", "Alice Margatroid"}},
	{ SokuLib::CHARACTER_PATCHOULI, {"Patchouli", "Patchouli Knowledge"}},
	{ SokuLib::CHARACTER_YOUMU, {"Youmu", "Youmu Konpaku"}},
	{ SokuLib::CHARACTER_REMILIA, {"Remilia", "Remilia Scarlet"}},
	{ SokuLib::CHARACTER_YUYUKO, {"Yuyuko", "Yuyuko Saigyouji"}},
	{ SokuLib::CHARACTER_YUKARI, {"Yukari", "Yukari Yakumo"}},
	{ SokuLib::CHARACTER_SUIKA, {"Suika", "Suika Ibuki"}},
	{ SokuLib::CHARACTER_REISEN, {"Reisen", "Reisen Undongein Inaba"}},
	{ SokuLib::CHARACTER_AYA, {"Aya", "Aya Shameimaru"}},
	{ SokuLib::CHARACTER_KOMACHI, {"Komachi", "Komachi Onozuka"}},
	{ SokuLib::CHARACTER_IKU, {"Iku", "Iku Nagae"}},
	{ SokuLib::CHARACTER_TENSHI, {"Tenshi", "Tenshi Hinanawi"}},
	{ SokuLib::CHARACTER_SANAE, {"Sanae", "Sanae Kochiya"}},
	{ SokuLib::CHARACTER_CIRNO, {"Cirno", "Cirno"}},
	{ SokuLib::CHARACTER_MEILING, {"Meiling", "Hong Meiling"}},
	{ SokuLib::CHARACTER_UTSUHO, {"Utsuho", "Utsuho Reiuji"}},
	{ SokuLib::CHARACTER_SUWAKO, {"Suwako", "Suwako Moriya"}},
	{ SokuLib::CHARACTER_RANDOM, {"Random", "Random Select"}},
};

std::vector<std::string> submenusNames{
	"Title screen", "Network screen", "Selecting replay", "Listening to music", "Selecting time trial", "Changing profile", "Editing configs"};

std::vector<std::string> stagesNames{"Hakurei Shrine", "Forest of Magic", "Creek of Genbu", "Youkai Mountain", "Mysterious Sea of Cloud", "Bhava-Agra",
	"Hakurei Shrine", "Kirisame Magic Shop", "Scarlet Devil Mansion Clock Tower", "Forest of Dolls", "Scarlet Devil Mansion Library", "Netherworld",
	"Scarlet Devil Mansion Foyer", "Hakugyokurou Snowy Garden", "Bamboo Forest of the Lost", "Shore of Misty Lake", "Moriya Shrine", "Mouth of Geyser",
	"Catwalk of Geyser", "Fusion Reactor Core"};

std::map<unsigned int, std::string> charactersImg{
	{SokuLib::CHARACTER_REIMU, "reimu"},
	{SokuLib::CHARACTER_MARISA, "marisa"},
	{SokuLib::CHARACTER_SAKUYA, "sakuya"},
	{SokuLib::CHARACTER_ALICE, "alice"},
	{SokuLib::CHARACTER_PATCHOULI, "patchouli"},
	{SokuLib::CHARACTER_YOUMU, "youmu"},
	{SokuLib::CHARACTER_REMILIA, "remilia"},
	{SokuLib::CHARACTER_YUYUKO, "yuyuko"},
	{SokuLib::CHARACTER_YUKARI, "yukari"},
	{SokuLib::CHARACTER_SUIKA, "suika"},
	{SokuLib::CHARACTER_REISEN, "reisen"},
	{SokuLib::CHARACTER_AYA, "aya"},
	{SokuLib::CHARACTER_KOMACHI, "komachi"},
	{SokuLib::CHARACTER_IKU, "iku"},
	{SokuLib::CHARACTER_TENSHI, "tenshi"},
	{SokuLib::CHARACTER_SANAE, "sanae"},
	{SokuLib::CHARACTER_CIRNO, "cirno"},
	{SokuLib::CHARACTER_MEILING, "meiling"},
	{SokuLib::CHARACTER_UTSUHO, "okuu"},
	{SokuLib::CHARACTER_SUWAKO, "suwako"},
	{SokuLib::CHARACTER_RANDOM, "random_select"}
};

std::pair<unsigned, unsigned> wins{0, 0};