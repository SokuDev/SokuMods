//
// Created by PinkySmile on 08/12/2020.
//

#ifndef SWRSTOYS_HANDLERS_HPP
#define SWRSTOYS_HANDLERS_HPP


#include "WebServer.hpp"

enum Opcodes {
	STATE_UPDATE,
	CARDS_UPDATE,
	L_SCORE_UPDATE,
	R_SCORE_UPDATE,
	L_CARDS_UPDATE,
	R_CARDS_UPDATE,
	L_NAME_UPDATE,
	R_NAME_UPDATE,
};

Socket::HttpResponse root(const Socket::HttpRequest &requ);
Socket::HttpResponse state(const Socket::HttpRequest &requ);
void onNewWebSocket(WebSocket &s);
void sendOpcode(WebSocket &s, Opcodes op, const std::string &data);
void broadcastOpcode(Opcodes op, const std::string &data);


#endif //SWRSTOYS_HANDLERS_HPP
