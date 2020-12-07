//
// Created by PinkySmile on 04/12/2020.
//

#ifndef SWRSTOYS_WEBSERVER_HPP
#define SWRSTOYS_WEBSERVER_HPP


#include <functional>
#include <thread>
#include <vector>
#include "Socket.hpp"
#include "WebSocket.hpp"

class WebServer {
private:
	std::function<void (WebSocket &sock)> _onConnect;
	std::function<void (WebSocket &sock, const std::string &msg)> _onMessage;
	std::function<void (WebSocket &sock, const std::exception &e)> _onError;
	bool _closed = false;
	Socket _sock;
	std::thread _thread;
	std::vector<WebSocket> _webSocks;
	std::vector<std::thread> _webSocksThreads;
	std::map<std::string, std::string> _folders;
	std::map<std::string, std::function<Socket::HttpResponse (const Socket::HttpRequest &request)>> _routes;

	void _serverLoop();
	void _addWebSocket(Socket &sock, const Socket::HttpRequest &requ);
	Socket::HttpResponse _checkFolders(const Socket::HttpRequest &request);
	static std::string _getContentType(const std::string &path);
	static Socket::HttpResponse _makeGenericPage(unsigned short code);
	static Socket::HttpResponse _makeGenericPage(unsigned short code, const std::string &extra);

public:
	static const std::map<std::string, std::string> types;
	static const std::map<unsigned short, std::string> codes;

	WebServer() = default;
	~WebServer();
	void broadcast(const std::string &msg);
	void onWebSocketConnect(const std::function<void (WebSocket &sock)> &fct);
	void onWebSocketMessage(const std::function<void (WebSocket &sock, const std::string &msg)> &fct);
	void onWebSocketError(const std::function<void (WebSocket &sock, const std::exception &e)> &fct);
	void addRoute(const std::string &&route, std::function<Socket::HttpResponse (const Socket::HttpRequest &request)> &&fct);
	void addStaticFolder(const std::string &&route, const std::string &&path);
	void start(unsigned short port);
	void stop();
};


#endif //SWRSTOYS_WEBSERVER_HPP
