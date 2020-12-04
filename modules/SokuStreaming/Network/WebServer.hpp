//
// Created by PinkySmile on 04/12/2020.
//

#ifndef SWRSTOYS_WEBSERVER_HPP
#define SWRSTOYS_WEBSERVER_HPP


#include <functional>
#include <thread>
#include "Socket.hpp"

class WebServer {
private:
	bool _closed = false;
	Socket _sock;
	std::thread _thread;
	std::map<std::string, std::string> _folders;
	std::map<std::string, std::function<Socket::HttpResponse (const Socket::HttpRequest &request)>> _routes;

	void _serverLoop();
	Socket::HttpResponse _checkFolders(const Socket::HttpRequest &request);
	static std::string _getContentType(const std::string &path);
	static Socket::HttpResponse _makeGenericPage(unsigned short code);
	static Socket::HttpResponse _makeGenericPage(unsigned short code, const std::string &extra);

public:
	static const std::map<std::string, std::string> types;
	static const std::map<unsigned short, std::string> codes;

	WebServer() = default;
	~WebServer();
	void addRoute(const std::string &&route, std::function<Socket::HttpResponse (const Socket::HttpRequest &request)> &&fct);
	void addStaticFolder(const std::string &&route, const std::string &&path);
	void start(unsigned short port);
	void stop();
};


#endif //SWRSTOYS_WEBSERVER_HPP
