//
// Created by PinkySmile on 04/12/2020.
//

#ifndef SWRSTOYS_WEBSERVER_HPP
#define SWRSTOYS_WEBSERVER_HPP

#include "Socket.hpp"
#include "WebSocket.hpp"
#include <functional>
#include <memory>
#include <thread>
#include <vector>

class WebServer {
private:
	struct WebSocketConnection {
		WebSocket wsock;
		std::thread thread;
		bool isThreadFinished;

		WebSocketConnection(const Socket &sock): wsock(sock){};
	};

	std::function<void(WebSocket &sock)> _onConnect;
	std::function<void(WebSocket &sock, const std::string &msg)> _onMessage;
	std::function<void(WebSocket &sock, const std::exception &e)> _onError;
	uintptr_t _thread = -1;
	bool _closed = false;
	Socket _sock;
	std::vector<std::shared_ptr<WebSocketConnection>> _webSocks;
	std::map<std::string, std::string> _folders;
	std::map<std::string, std::function<Socket::HttpResponse(const Socket::HttpRequest &request)>> _routes;

	static void _threadFct(WebServer *server);
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
	void onWebSocketConnect(const std::function<void(WebSocket &sock)> &fct);
	void onWebSocketMessage(const std::function<void(WebSocket &sock, const std::string &msg)> &fct);
	void onWebSocketError(const std::function<void(WebSocket &sock, const std::exception &e)> &fct);
	void addRoute(const std::string &&route, std::function<Socket::HttpResponse(const Socket::HttpRequest &request)> &&fct);
	void addStaticFolder(const std::string &&route, const std::string &&path);
	void start(unsigned short port);
	void stop();
};

#endif // SWRSTOYS_WEBSERVER_HPP
