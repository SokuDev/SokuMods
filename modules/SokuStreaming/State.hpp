//
// Created by PinkySmile on 08/12/2020.
//

#ifndef SWRSTOYS_STATE_HPP
#define SWRSTOYS_STATE_HPP


#include <SokuLib.hpp>
#include "Network/WebServer.hpp"

struct Title{};
struct Battle{};
struct Loading{};
struct BattleWatch{};
struct LoadingWatch{};

enum Keys {
	KEY_DECREASE_L_SCORE,
	KEY_DECREASE_R_SCORE,
	KEY_INCREASE_L_SCORE,
	KEY_INCREASE_R_SCORE,
	KEY_CHANGE_L_NAME,
	KEY_CHANGE_R_NAME,
	TOTAL_NB_OF_KEYS
};

extern bool enabled;
extern unsigned short port;
extern std::vector<unsigned> keys;
extern std::unique_ptr<WebServer> webServer;
extern struct CachedMatchData {
	SokuLib::Weather weather;
	SokuLib::Character left;
	SokuLib::Character right;
	std::vector<unsigned short> leftCards;
	std::vector<unsigned short> rightCards;
	std::vector<unsigned short> leftHand;
	std::vector<unsigned short> rightHand;
	std::vector<unsigned short> leftUsed;
	std::vector<unsigned short> rightUsed;
	std::string leftName;
	std::string rightName;
	unsigned int oldLeftScore;
	unsigned int oldRightScore;
	unsigned int leftScore;
	unsigned int rightScore;
} _cache;
extern bool needReset;
extern bool needRefresh;
extern int (__thiscall LoadingWatch::*s_origCLoadingWatch_Process)();
extern int (__thiscall BattleWatch::*s_origCBattleWatch_Process)();
extern int (__thiscall Loading::*s_origCLoading_Process)();
extern int (__thiscall Battle::*s_origCBattle_Process)();
extern int (__thiscall Title::*s_origCTitle_Process)();

void updateCache(bool isMultiplayer);
std::string generateLeftCardsJson(CachedMatchData cache);
std::string generateRightCardsJson(CachedMatchData cache);
std::string generateCardsJson(CachedMatchData cache);
std::string cacheToJson(CachedMatchData cache);


#endif //SWRSTOYS_STATE_HPP
