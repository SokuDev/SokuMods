//
// Created by PinkySmile on 08/12/2020.
//

#include "Handlers.hpp"
#include "../State.hpp"
#include "../Exceptions.hpp"


Socket::HttpResponse root(const Socket::HttpRequest &requ)
{
	Socket::HttpResponse response;

	if (requ.method != "GET")
		throw AbortConnectionException(405);
	response.header["Location"] = "http://" + requ.host + "/static/html/overlay.html";
	response.returnCode = 301;
	return response;
}

Socket::HttpResponse state(const Socket::HttpRequest &requ)
{
	Socket::HttpResponse response;

	if (requ.method != "GET")
		throw AbortConnectionException(405);

	response.returnCode = 200;
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

	webServer.broadcast(json);
}
