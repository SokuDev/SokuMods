//
// Created by PinkySmile on 08/12/2020.
//

#include <SokuLib.hpp>
#include "nlohmann/json.hpp"
#include "ShiftJISDecoder.hpp"
#include "State.hpp"

bool enabled;
unsigned short port;
WebServer webServer;
struct CachedMatchData _cache;
bool needReset;
int (__thiscall BattleWatch::*s_origCBattleWatch_Render)();
int (__thiscall Title::*s_origCTitle_Render)();

void updateCache()
{
	auto &battleMgr = SokuLib::getBattleMgr();
	auto &netObj = SokuLib::getNetObject();

	if (needReset) {
		if (_cache.leftName != netObj.profile1name || _cache.rightName != netObj.profile2name) {
			_cache.leftScore = 0;
			_cache.rightScore = 0;
		}
		needReset = false;
	}

	auto oldLeftHand = _cache.leftHand;
	auto oldRightHand = _cache.rightHand;

	_cache.leftCards.clear();
	_cache.rightCards.clear();
	_cache.leftHand.clear();
	_cache.rightHand.clear();

	auto &leftDeck = battleMgr.leftCharacterManager->mDeckInfo2Obj;
	auto &rightDeck = battleMgr.rightCharacterManager->mDeckInfo2Obj;

	// Left remaining cards
	if (leftDeck.deck.size == 20)
		_cache.leftUsed.clear();
	if (leftDeck.deck.size <= 20)
		for (int i = 0; i < leftDeck.deck.size; i++) {
			auto card = leftDeck.deck[i];

			_cache.leftCards.push_back(*card);
		}
	std::sort(_cache.leftCards.begin(), _cache.leftCards.end());
	//Right remaining cards
	if (rightDeck.deck.size == 20)
		_cache.rightUsed.clear();
	if (rightDeck.deck.size <= 20)
		for (int i = 0; i < rightDeck.deck.size; i++) {
			auto card = rightDeck.deck[i];

			_cache.rightCards.push_back(*card);
		}
	std::sort(_cache.rightCards.begin(), _cache.rightCards.end());

	//Hands
	for (int i = 0; i < leftDeck.cardCount; i++)
		_cache.leftHand.push_back(leftDeck.handCardBase[(i + leftDeck.selectedCard) % leftDeck.handCardMax]->id);
	for (int i = 0; i < rightDeck.cardCount; i++)
		_cache.rightHand.push_back(rightDeck.handCardBase[(i + rightDeck.selectedCard) % rightDeck.handCardMax]->id);
	std::sort(_cache.leftHand.begin(), _cache.leftHand.end());
	std::sort(_cache.rightHand.begin(), _cache.rightHand.end());

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

	if (
		_cache.oldLeftScore != battleMgr.leftCharacterManager->score ||
		_cache.oldRightScore != battleMgr.rightCharacterManager->score
		) {
		_cache.leftScore += battleMgr.leftCharacterManager->score == 2;
		_cache.rightScore += battleMgr.rightCharacterManager->score == 2;
	}
	_cache.oldLeftScore = battleMgr.leftCharacterManager->score;
	_cache.oldRightScore = battleMgr.rightCharacterManager->score;
	_cache.left = SokuLib::leftChar;
	_cache.right = SokuLib::rightChar;
	_cache.leftName = netObj.profile1name;
	_cache.rightName = netObj.profile2name;
	_cache.weather = SokuLib::activeWeather;
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
		{ "score", cache.leftScore },
		{ "name", convertShiftJisToUTF8(cache.leftName.c_str()) },
		{ "used", cache.leftUsed },
		{ "deck", leftDeck },
		{ "hand", leftHand },
	};
	result["right"] = {
		{ "character", cache.right },
		{ "score", cache.rightScore },
		{ "name", convertShiftJisToUTF8(cache.rightName.c_str()) },
		{ "used", cache.rightUsed },
		{ "deck", rightDeck },
		{ "hand", rightHand },
	};
	return result.dump(-1, ' ', true);
}
