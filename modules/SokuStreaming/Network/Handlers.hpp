//
// Created by PinkySmile on 08/12/2020.
//

#ifndef SWRSTOYS_HANDLERS_HPP
#define SWRSTOYS_HANDLERS_HPP


#include "WebServer.hpp"

Socket::HttpResponse root(const Socket::HttpRequest &requ);
Socket::HttpResponse state(const Socket::HttpRequest &requ);
void onNewWebSocket(WebSocket &s);


#endif //SWRSTOYS_HANDLERS_HPP
