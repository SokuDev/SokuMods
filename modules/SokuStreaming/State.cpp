//
// Created by PinkySmile on 08/12/2020.
//

#include <SokuLib.hpp>
#include <dinput.h>
#include <iostream>
#include "Network/Handlers.hpp"
#include "nlohmann/json.hpp"
#include "Utils/ShiftJISDecoder.hpp"
#include "Utils/InputBox.hpp"
#include "State.hpp"

#define checkKey(key) SokuLib::checkKeyOneshot(keys[key], true, false, false)

bool enabled;
unsigned short port;
std::vector<unsigned> keys(TOTAL_NB_OF_KEYS);
std::unique_ptr<WebServer> webServer;
struct CachedMatchData _cache;
bool needReset;
bool needRefresh;
int (__thiscall LoadingWatch::*s_origCLoadingWatch_Process)();
int (__thiscall BattleWatch::*s_origCBattleWatch_Process)();
int (__thiscall Loading::*s_origCLoading_Process)();
int (__thiscall Battle::*s_origCBattle_Process)();
int (__thiscall Title::*s_origCTitle_Process)();

bool threadUsed = false;
std::thread thread;

static void checkKeyInputs()
{
	if (!threadUsed && thread.joinable())
		thread.join();

	/*for (int i = 0; i < 0xFF; i++) {
		auto val = SokuLib::checkKeyOneshot(i, 0, 0, 0);

		if (val)
			std::cout << std::hex << i << " pressed !" << std::endl;
	}*/

	if (checkKey(KEY_DECREASE_L_SCORE)) {
		_cache.leftScore--;
		broadcastOpcode(L_SCORE_UPDATE, std::to_string(_cache.leftScore));
	}
	if (checkKey(KEY_DECREASE_R_SCORE)) {
		_cache.rightScore--;
		broadcastOpcode(R_SCORE_UPDATE, std::to_string(_cache.rightScore));
	}
	if (checkKey(KEY_INCREASE_L_SCORE)) {
		_cache.leftScore++;
		broadcastOpcode(L_SCORE_UPDATE, std::to_string(_cache.leftScore));
	}
	if (checkKey(KEY_INCREASE_R_SCORE)) {
		_cache.rightScore++;
		broadcastOpcode(R_SCORE_UPDATE, std::to_string(_cache.rightScore));
	}
	if (checkKey(KEY_CHANGE_L_NAME)) {
		if (!threadUsed) {
			threadUsed = true;
			if (thread.joinable())
				thread.join();
			thread = std::thread{[]{
				auto answer = InputBox("Answer", "Are you ok ?", "Yes");

				if (answer.empty()) {
					threadUsed = false;
					return;
				}
				SokuLib::player1Profile = answer;
				threadUsed = false;
			}};
		}
	}
}

nlohmann::json statsToJson(const Stats &stats)
{
	nlohmann::json result = {
		{"doll",     stats.doll},
		{"rod",      stats.rod},
		{"grimoire", stats.grimoire},
		{"fan",      stats.fan},
		{"drops",    stats.drops}
	};
	std::map<std::string, unsigned char> skillMap;

	for (int i = 0; i < 16; i++) {
		if (!stats.skillMap[i].notUsed)
			skillMap[std::to_string(i)] = stats.skillMap[i].level;
	}

	result["skills"] = skillMap;
	return result;
}

std::string statsToString(const Stats &stats)
{
	return statsToJson(stats).dump(-1, ' ', true);
}

void updateCache(bool isMultiplayer)
{
	auto &battleMgr = SokuLib::getBattleMgr();

	if (needReset) {
		if (isMultiplayer) {
			auto &netObj = SokuLib::getNetObject();

			if (_cache.leftName != netObj.profile1name || _cache.rightName != netObj.profile2name) {
				_cache.leftScore = 0;
				_cache.rightScore = 0;
			}
			_cache.leftName = netObj.profile1name;
			_cache.rightName = netObj.profile2name;
		} else {
			_cache.leftName = SokuLib::player1Profile;
			_cache.rightName = SokuLib::player2Profile;
		}
		needReset = false;
	}

	auto oldLeftHand = _cache.leftHand;
	auto oldRightHand = _cache.rightHand;

	_cache.leftCards.clear();
	_cache.rightCards.clear();
	_cache.leftHand.clear();
	_cache.rightHand.clear();

	auto &leftDeck = battleMgr.leftCharacterManager->deckInfos;
	auto &rightDeck = battleMgr.rightCharacterManager->deckInfos;

	// Left remaining cards
	if (leftDeck.deck.size == 20)
		_cache.leftUsed.clear();
	for (int i = 0; i < leftDeck.deck.size; i++)
		_cache.leftCards.push_back(leftDeck.deck[i]);
	std::sort(_cache.leftCards.begin(), _cache.leftCards.end());

	//Right remaining cards
	if (rightDeck.deck.size == 20)
		_cache.rightUsed.clear();
	for (int i = 0; i < rightDeck.deck.size; i++)
		_cache.rightCards.push_back(rightDeck.deck[i]);
	std::sort(_cache.rightCards.begin(), _cache.rightCards.end());

	//Hands
	for (int i = 0; i < leftDeck.cardCount; i++)
		_cache.leftHand.push_back(leftDeck.handCardBase[(i + leftDeck.selectedCard) % leftDeck.handCardMax]->id);
	for (int i = 0; i < rightDeck.cardCount; i++)
		_cache.rightHand.push_back(rightDeck.handCardBase[(i + rightDeck.selectedCard) % rightDeck.handCardMax]->id);
	std::sort(_cache.leftHand.begin(), _cache.leftHand.end());
	std::sort(_cache.rightHand.begin(), _cache.rightHand.end());

	auto leftOldSize = oldLeftHand.size();
	auto rightOldSize = oldRightHand.size();

	//Used cards
	if (oldLeftHand.size() < _cache.leftHand.size())
		oldLeftHand.clear();
	if (oldRightHand.size() < _cache.rightHand.size())
		oldRightHand.clear();
	for (auto id : _cache.leftHand) {
		auto it = std::find(oldLeftHand.begin(), oldLeftHand.end(), id);

		if (it != oldLeftHand.end())
			oldLeftHand.erase(it);
	}
	for (auto id : _cache.rightHand) {
		auto it = std::find(oldRightHand.begin(), oldRightHand.end(), id);

		if (it != oldRightHand.end())
			oldRightHand.erase(it);
	}
	for (auto id : oldLeftHand)
		_cache.leftUsed.push_back(id);
	for (auto id : oldRightHand)
		_cache.rightUsed.push_back(id);
	std::sort(_cache.leftUsed.begin(), _cache.leftUsed.end());
	std::sort(_cache.rightUsed.begin(), _cache.rightUsed.end());

	if (_cache.leftHand.size() != leftOldSize && !needRefresh)
		broadcastOpcode(L_CARDS_UPDATE, generateLeftCardsJson(_cache));
	if (_cache.rightHand.size() != rightOldSize && !needRefresh)
		broadcastOpcode(R_CARDS_UPDATE, generateRightCardsJson(_cache));

	if (
		_cache.oldLeftScore != battleMgr.leftCharacterManager->score ||
		_cache.oldRightScore != battleMgr.rightCharacterManager->score
	) {
		if (battleMgr.leftCharacterManager->score == 2) {
			_cache.leftScore++;
			if (!needRefresh)
				broadcastOpcode(L_SCORE_UPDATE, std::to_string(_cache.leftScore));
		} else if (battleMgr.rightCharacterManager->score == 2) {
			_cache.rightScore++;
			if (!needRefresh)
				broadcastOpcode(R_SCORE_UPDATE, std::to_string(_cache.rightScore));
		}
		_cache.oldLeftScore = battleMgr.leftCharacterManager->score;
		_cache.oldRightScore = battleMgr.rightCharacterManager->score;
	}

	if (_cache.weather != SokuLib::activeWeather) {
		auto old = _cache.weather;

		_cache.weather = SokuLib::activeWeather;
		if (old == SokuLib::WEATHER_MOUNTAIN_VAPOR || SokuLib::activeWeather == SokuLib::WEATHER_MOUNTAIN_VAPOR)
			if (!needRefresh)
				broadcastOpcode(CARDS_UPDATE, generateCardsJson(_cache));
	}

	Stats newStats;

	newStats.doll =     battleMgr.leftCharacterManager->sacrificialDolls;
	newStats.drops =    battleMgr.leftCharacterManager->drops;
	newStats.rod =      battleMgr.leftCharacterManager->controlRod;
	newStats.fan =      battleMgr.leftCharacterManager->tenguFans;
	newStats.grimoire = battleMgr.leftCharacterManager->grimoires;
	std::memcpy(newStats.skillMap, battleMgr.leftCharacterManager->skillMap, sizeof(newStats.skillMap));
	if (memcmp(&newStats, &_cache.leftStats, sizeof(newStats)) != 0) {
		std::memcpy(&_cache.leftStats, &newStats, sizeof(newStats));
		if (!needRefresh)
			broadcastOpcode(L_STATS_UPDATE, statsToString(newStats));
	}

	newStats.doll =     battleMgr.rightCharacterManager->sacrificialDolls;
	newStats.drops =    battleMgr.rightCharacterManager->drops;
	newStats.rod =      battleMgr.rightCharacterManager->controlRod;
	newStats.fan =      battleMgr.rightCharacterManager->tenguFans;
	newStats.grimoire = battleMgr.rightCharacterManager->grimoires;
	std::memcpy(newStats.skillMap, battleMgr.rightCharacterManager->skillMap, sizeof(newStats.skillMap));
	if (memcmp(&newStats, &_cache.rightStats, sizeof(newStats)) != 0) {
		std::memcpy(&_cache.rightStats, &newStats, sizeof(newStats));
		if (!needRefresh)
			broadcastOpcode(R_STATS_UPDATE, statsToString(newStats));
	}

	if (needRefresh) {
		_cache.left = SokuLib::leftChar;
		_cache.right = SokuLib::rightChar;
		needRefresh = false;
		broadcastOpcode(STATE_UPDATE, cacheToJson(_cache));
	}
	checkKeyInputs();
}

std::string cacheToJson(CachedMatchData cache)
{
	nlohmann::json result;
	std::vector<unsigned short> leftDeck;
	std::vector<unsigned short> rightDeck;
	std::vector<unsigned short> leftHand;
	std::vector<unsigned short> rightHand;

	if (cache.weather == SokuLib::WEATHER_MOUNTAIN_VAPOR) {
		leftDeck.resize(cache.leftCards.size() + cache.leftHand.size(), 21);
		rightDeck.resize(cache.rightCards.size() + cache.rightHand.size(), 21);
	} else {
		leftDeck = cache.leftCards;
		rightDeck = cache.rightCards;
		leftHand = cache.leftHand;
		rightHand = cache.rightHand;
	}

	result["left"] = {
		{ "character", cache.left },
		{ "score",     cache.leftScore },
		{ "name",      convertShiftJisToUTF8(cache.leftName.c_str()) },
		{ "used",      cache.leftUsed },
		{ "deck",      leftDeck },
		{ "hand",      leftHand },
		{ "stats",     statsToJson(cache.leftStats) }
	};
	result["right"] = {
		{ "character", cache.right },
		{ "score",     cache.rightScore },
		{ "name",      convertShiftJisToUTF8(cache.rightName.c_str()) },
		{ "used",      cache.rightUsed },
		{ "deck",      rightDeck },
		{ "hand",      rightHand },
		{ "stats",     statsToJson(cache.rightStats) }
	};
	return result.dump(-1, ' ', true);
}

std::string generateCardsJson(CachedMatchData cache)
{
	nlohmann::json result;
	std::vector<unsigned short> leftDeck;
	std::vector<unsigned short> rightDeck;
	std::vector<unsigned short> leftHand;
	std::vector<unsigned short> rightHand;

	if (cache.weather == SokuLib::WEATHER_MOUNTAIN_VAPOR) {
		leftDeck.resize(cache.leftCards.size() + cache.leftHand.size(), 21);
		rightDeck.resize(cache.rightCards.size() + cache.rightHand.size(), 21);
	} else {
		leftDeck = cache.leftCards;
		rightDeck = cache.rightCards;
		leftHand = cache.leftHand;
		rightHand = cache.rightHand;
	}

	result["left"] = {
		{ "used", cache.leftUsed },
		{ "deck", leftDeck },
		{ "hand", leftHand },
	};
	result["right"] = {
		{ "used", cache.rightUsed },
		{ "deck", rightDeck },
		{ "hand", rightHand },
	};
	return result.dump(-1, ' ', true);
}

std::string generateRightCardsJson(CachedMatchData cache)
{
	nlohmann::json result;
	std::vector<unsigned short> rightDeck;
	std::vector<unsigned short> rightHand;

	if (cache.weather == SokuLib::WEATHER_MOUNTAIN_VAPOR)
		rightDeck.resize(cache.rightCards.size() + cache.rightHand.size(), 21);
	else {
		rightDeck = cache.rightCards;
		rightHand = cache.rightHand;
	}

	result = {
		{ "used", cache.rightUsed },
		{ "deck", rightDeck },
		{ "hand", rightHand },
	};
	return result.dump(-1, ' ', true);
}

std::string generateLeftCardsJson(CachedMatchData cache)
{
	nlohmann::json result;
	std::vector<unsigned short> leftDeck;
	std::vector<unsigned short> leftHand;

	if (cache.weather == SokuLib::WEATHER_MOUNTAIN_VAPOR)
		leftDeck.resize(cache.leftCards.size() + cache.leftHand.size(), 21);
	else {
		leftDeck = cache.leftCards;
		leftHand = cache.leftHand;
	}

	result = {
		{ "used", cache.leftUsed },
		{ "deck", leftDeck },
		{ "hand", leftHand },
	};
	return result.dump(-1, ' ', true);
}
