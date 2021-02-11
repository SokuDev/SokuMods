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
	KEY_RESET_SCORES,
	KEY_RESET_STATE,
	TOTAL_NB_OF_KEYS
};

struct Stats {
	float rod;
	float doll;
	unsigned short grimoire;
	unsigned short fan;
	unsigned short drops;
	SokuLib::Skill skillMap[16];
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
	std::string realLeftName;
	std::string realRightName;
	std::string round;
	unsigned int oldLeftScore;
	unsigned int oldRightScore;
	unsigned int leftScore;
	unsigned int rightScore;
	Stats leftStats;
	Stats rightStats;
	bool noReset;
} _cache;
extern bool needReset;
extern bool needRefresh;
extern int (__thiscall SokuLib::BattleManager::*s_origCBattleManager_Render)();
extern int (__thiscall SokuLib::BattleManager::*s_origCBattleManager_Start)();
extern int (__thiscall SokuLib::BattleManager::*s_origCBattleManager_KO)();
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
void checkKeyInputs();
void onRoundStart();
void onKO();


#endif //SWRSTOYS_STATE_HPP
