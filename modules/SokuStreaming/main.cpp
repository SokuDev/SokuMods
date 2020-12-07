//
// Created by Gegel85 on 04/12/2020
//

#include <algorithm>
#include <SokuLib.hpp>
#include <Windows.h>
#include <Shlwapi.h>
#include "ShiftJISDecoder.hpp"
#include "Network/WebServer.hpp"
#include "Exceptions.hpp"

static bool enabled;
static unsigned short port;
static WebServer webServer;
static struct CachedMatchData {
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
struct Title{};
struct BattleCL{};
struct BattleSV{};
struct BattleWatch{};
static bool needReset;
static bool needDeckRefresh;
static int (__thiscall BattleWatch::*s_origCBattleWatch_Render)();
static int (__thiscall Title::*s_origCTitle_Render)();

/*static int (__thiscall C::*array[16])();

#define TEST(i)int __fastcall fun##i(C *This)\
{                                            \
        printf(#i "\n");                  \
	return (This->*array[i])();\
}                                            \

TEST(0)
TEST(1)
TEST(2)
TEST(3)
TEST(4)
TEST(5)
TEST(6)
TEST(7)
TEST(8)
TEST(9)
TEST(10)
TEST(11)
TEST(12)
TEST(13)
TEST(14)
TEST(15)

int (__fastcall *funs[16])(C *This) = {
	fun0,
	fun1,
	fun2,
	fun3,
	fun4,
	fun5,
	fun6,
	fun7,
	fun8,
	fun9,
	fun10,
	fun11,
	fun12,
	fun13,
	fun14,
	fun15,
};*/

void onNewWebSocket(WebSocket &s)
{
	s.send("Hello my friend. I hope you enjoy your stay here.");
}

Socket::HttpResponse root(const Socket::HttpRequest &requ)
{
	Socket::HttpResponse response;

	if (requ.method != "GET")
		throw AbortConnectionException(405);
	response.header["Location"] = "http://" + requ.host + "/static/html/overlay.html";
	response.returnCode = 301;
	return response;
}

std::string serializeUShortArray(const std::vector<unsigned short> &arr, int maxSize)
{
	std::string result;

	result.reserve(maxSize);
	result += "[";
	if (!arr.empty())
		result += std::to_string(arr[0]);
	for (size_t i = 1; i < arr.size(); i++) {
		result += ",";
		result += std::to_string(arr[i]);
	}
	result += "]";
	return result;
}

std::string generateMtVpDeck(size_t size)
{
	std::string result;

	result.reserve(2 + size * 2 + size - 1);
	result += "[";
	if (size)
		result += "21";
	for (size_t i = 1; i < size; i++)
		result += ",21";
	result += "]";
	return result;
}

Socket::HttpResponse state(const Socket::HttpRequest &requ)
{
	CachedMatchData cache = _cache;
	Socket::HttpResponse response;
	std::string sanitizedLeftName;
	std::string sanitizedRightName;
	std::string leftDeck;
	std::string rightDeck;
	std::string leftHand;
	std::string rightHand;

	if (requ.method != "GET")
		throw AbortConnectionException(405);
	sanitizedLeftName.reserve(cache.leftName.size() * 3);
	for (char c : convertShiftJisToUTF8(cache.leftName.c_str()))
		switch (c) {
		case '"':
		case '\\':
			sanitizedLeftName.push_back('\\');
		default:
			sanitizedLeftName.push_back(c);
		}

	sanitizedRightName.reserve(cache.rightName.size() * 3);
	for (char c : convertShiftJisToUTF8(cache.rightName.c_str()))
		switch (c) {
		case '"':
		case '\\':
			sanitizedRightName.push_back('\\');
		default:
			sanitizedRightName.push_back(c);
		}

	if (cache.weather == SokuLib::WEATHER_MOUNTAIN_VAPOR) {
		leftDeck = generateMtVpDeck(cache.leftCards.size() + cache.leftHand.size());
		rightDeck = generateMtVpDeck(cache.rightCards.size() + cache.rightHand.size());
		leftHand = "[]";
		rightHand = "[]";
	} else {
		leftDeck = serializeUShortArray(cache.leftCards, 2 + 20 * 3 + 19);
		rightDeck = serializeUShortArray(cache.rightCards, 2 + 20 * 3 + 19);
		leftHand = serializeUShortArray(cache.leftHand, 2 + 20 * 3 + 19);
		rightHand = serializeUShortArray(cache.rightHand, 2 + 20 * 3 + 19);
	}

	response.returnCode = 200;
	response.header["content-type"] = "application/json";
	response.body = "{"
		 "\"left\":{"
			"\"character\":" + std::to_string(cache.left) + ","
			"\"score\":" + std::to_string(cache.leftScore) + ","
			R"("name":")" + sanitizedLeftName + "\","
			"\"deck\":" + leftDeck + ","
			"\"used\":" + serializeUShortArray(cache.leftUsed, 2 + 20 * 3 + 19) + ","
			"\"hand\":" + leftHand +
		"},"
		"\"right\":{"
			"\"character\":" + std::to_string(cache.right) + ","
			"\"score\":" + std::to_string(cache.rightScore) + ","
			R"("name":")" + sanitizedRightName + "\","
			"\"deck\":" + rightDeck + ","
			"\"used\":" + serializeUShortArray(cache.rightUsed, 2 + 20 * 3 + 19) + ","
			"\"hand\":" + rightHand +
		"}"
	"}";
	return response;
}

void updateCache(int ret)
{
	auto &battleMgr = SokuLib::getBattleMgr();
	auto &netObj = SokuLib::getNetObject();

	if (needReset) {
		_cache.leftScore = 0;
		_cache.rightScore = 0;
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

int __fastcall CTitle_OnRender(Title *This) {
	// super
	int ret = (This->*s_origCTitle_Render)();

	needReset = true;
	needDeckRefresh = true;
	return ret;
}

int __fastcall CBattleWatch_OnRender(BattleWatch *This) {
	// super
	int ret = (This->*s_origCBattleWatch_Render)();

	updateCache(ret);
	return ret;
}

// �ݒ胍�[�h
void LoadSettings(LPCSTR profilePath, LPCSTR parentPath)
{
	// �����V���b�g�_�E��
	enabled = GetPrivateProfileInt("SokuStreaming", "Enabled", 1, profilePath) != 0;
	port = GetPrivateProfileInt("Server", "Enabled", 80, profilePath);
	webServer.addRoute("/", root);
	webServer.addRoute("/state", state);
	webServer.addStaticFolder("/static", std::string(parentPath) + "/static");
	webServer.start(port);
	webServer.onWebSocketConnect(onNewWebSocket);
}

extern "C"
__declspec(dllexport) bool CheckVersion(const BYTE hash[16])
{
	return true;
}

extern "C"
__declspec(dllexport) bool Initialize(HMODULE hMyModule, HMODULE hParentModule)
{
	char profilePath[1024 + MAX_PATH];
	char profileParent[1024 + MAX_PATH];

	GetModuleFileName(hMyModule, profilePath, 1024);
	PathRemoveFileSpec(profilePath);
	strcpy(profileParent, profilePath);
	PathAppend(profilePath, "SokuStreaming.ini");
	LoadSettings(profilePath, profileParent);
	DWORD old;
	::VirtualProtect((PVOID)RDATA_SECTION_OFFSET, RDATA_SECTION_SIZE, PAGE_EXECUTE_WRITECOPY, &old);
	s_origCTitle_Render = SokuLib::union_cast<int (Title::*)()>(SokuLib::TamperDword(SokuLib::vtbl_CTitle + 8, reinterpret_cast<DWORD>(CTitle_OnRender)));
	s_origCBattleWatch_Render = SokuLib::union_cast<int (BattleWatch::*)()>(SokuLib::TamperDword(SokuLib::vtbl_CBattleWatch + 0x08, reinterpret_cast<DWORD>(CBattleWatch_OnRender)));
	::VirtualProtect((PVOID)RDATA_SECTION_OFFSET, RDATA_SECTION_SIZE, old, &old);

	/*for (int i = 0; i < 16; i++) {
		DWORD addr = SokuLib::ADDR_VTBL_BATTLE_CL + 0x08 + 60 + 4 * i + 0x40 * 3;

		printf("%i: 0x%X\n", i, addr);
		array[i] = SokuLib::union_cast<int (C::*)()>(SokuLib::TamperDword(addr, reinterpret_cast<DWORD>(funs[i])));
	}*/

	::FlushInstructionCache(GetCurrentProcess(), NULL, 0);

	return true;
}

extern "C"
int APIENTRY DllMain(HMODULE hModule, DWORD fdwReason, LPVOID lpReserved)
{
	return TRUE;
}