//
// Created by PinkySmile on 08/12/2020.
//

#include <nlohmann/json.hpp>
#include "Handlers.hpp"
#include "../State.hpp"
#include "../Exceptions.hpp"

Socket::HttpResponse connect(const Socket::HttpRequest &requ)
{
	if (requ.ip != 0x0100007F)
		throw AbortConnectionException(403);
	if (requ.method != "POST")
		throw AbortConnectionException(405);

	Socket::HttpResponse response;
	auto menuObj = SokuLib::getMenuObj<SokuLib::MenuConnect>();
	std::string ip;
	unsigned short hport;
	bool isSpec;

	try {
		auto json = nlohmann::json::parse(requ.body);

		ip = json["ip"];
		hport = json["port"];
		isSpec = json["spec"];
		if (inet_addr(ip.c_str()) == -1)
			throw std::exception();
	} catch (...) {
		throw AbortConnectionException(400);
	}

	if (!SokuLib::isInNetworkMenu()) {
		SokuLib::moveToConnectMenu();
		menuObj = SokuLib::getMenuObj<SokuLib::MenuConnect>();
	}

	if (
		menuObj->choice >= SokuLib::MenuConnect::CHOICE_ASSIGN_IP_CONNECT &&
		menuObj->choice < SokuLib::MenuConnect::CHOICE_SELECT_PROFILE &&
		menuObj->subchoice == 3
	)
		throw AbortConnectionException(503);
	if (
		menuObj->choice >= SokuLib::MenuConnect::CHOICE_HOST &&
		menuObj->choice < SokuLib::MenuConnect::CHOICE_SELECT_PROFILE &&
		menuObj->subchoice == 255
	)
		throw AbortConnectionException(503);
	if (
		menuObj->choice == SokuLib::MenuConnect::CHOICE_HOST &&
		menuObj->subchoice == 2
	)
		throw AbortConnectionException(503);

	SokuLib::joinHost(
		ip.c_str(),
		hport,
		isSpec
	);
	response.returnCode = 202;
	return response;
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

static void setState(const Socket::HttpRequest &requ)
{
	if (requ.ip != 0x0100007F)
		throw AbortConnectionException(403);
	try {
		auto result = nlohmann::json::parse(requ.body);

		if (result.contains("left")) {
			auto &chr = result["left"];

			if (chr.contains("name"))
				_cache.leftName = chr["name"];
			if (chr.contains("score"))
				_cache.leftScore = chr["score"];
			/*result["left"] = {
				{"character", cache.left},
				{"score",     cache.leftScore},
				{"name",      convertShiftJisToUTF8(cache.leftName.c_str())},
				{"used",      cache.leftUsed},
				{"deck",      leftDeck},
				{"hand",      leftHand},
				{"stats",     statsToJson(cache.leftStats)}
			};*/
		}
		if (result.contains("right")) {
			auto &chr = result["right"];

			if (chr.contains("name"))
				_cache.rightName = chr["name"];
			if (chr.contains("score"))
				_cache.rightScore = chr["score"];
		}
		if (result.contains("round"))
			_cache.round = result["round"];
	} catch (nlohmann::detail::exception &) {
		throw AbortConnectionException(400);
	}
	broadcastOpcode(STATE_UPDATE, cacheToJson(_cache));
	_cache.noReset = true;
}

Socket::HttpResponse state(const Socket::HttpRequest &requ)
{
	Socket::HttpResponse response;

	response.returnCode = 200;
	if (requ.method == "POST")
		return setState(requ), response;

	if (requ.method != "GET")
		throw AbortConnectionException(405);

	response.header["content-type"] = "application/json";
	response.body = cacheToJson(_cache);
	return response;
}

void onNewWebSocket(WebSocket &s)
{
	sendOpcode(s, STATE_UPDATE, cacheToJson(_cache));
}

void sendOpcode(WebSocket &s, Opcodes op, const std::string &data)
{
	std::string json = "{"
		"\"o\": " + std::to_string(op) + ","
		"\"d\": " + data +
	"}";

	s.send(json);
}

void broadcastOpcode(Opcodes op, const std::string &data)
{
	std::string json = "{"
		"\"o\": " + std::to_string(op) + ","
		"\"d\": " + data +
	"}";

	webServer->broadcast(json);
}
