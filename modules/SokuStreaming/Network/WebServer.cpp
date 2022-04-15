//
// Created by PinkySmile on 04/12/2020.
//

#include "WebServer.hpp"

#include "../Exceptions.hpp"
#include <csignal>
#include <fstream>
#include <iostream>
#include <process.h>
#include <nlohmann/json.hpp>

const std::map<std::string, std::string> WebServer::types{
	{"txt", "text/plain"},
	{"js", "text/javascript"},
	{"css", "text/css"},
	{"html", "text/html"},
	{"png", "image/png"},
	{"apng", "image/apng"},
	{"bmp", "image/bmp"},
	{"gif", "image/gif"},
	{"ico", "image/x-icon"},
	{"cur", "image/x-icon"},
	{"jpeg", "image/jpeg"},
	{"jpg", "image/jpeg"},
	{"jfif", "image/jpeg"},
	{"pjpeg", "image/jpeg"},
	{"pjg", "image/jpeg"},
	{"tif", "image/tiff"},
	{"tiff", "image/tiff"},
	{"svg", "image/svg+xml"},
	{"webp", "image/webp"},
};

const std::map<unsigned short, std::string> WebServer::codes{
	{100, "Continue"},
	{101, "Switching Protocols"},
	{102, "Processing"},
	{103, "Early Hints"},
	{200, "OK"},
	{201, "Created"},
	{202, "Accepted"},
	{203, "Non-Authoritative Information"},
	{204, "No Content"},
	{205, "Reset Content"},
	{206, "Partial Content"},
	{207, "Multi-Status"},
	{208, "Already Reported"},
	{210, "Content Different"},
	{226, "IM Used"},
	{300, "Multiple Choices"},
	{301, "Moved Permanently"},
	{302, "Found"},
	{303, "See Other"},
	{304, "Not Modified"},
	{305, "Use Proxy"},
	{306, "Switch Proxy"},
	{307, "Temporary Redirect"},
	{308, "Permanent Redirect"},
	{310, "Too many Redirects"},
	{400, "Bad Request"},
	{401, "Unauthorized"},
	{402, "Payment Required"},
	{403, "Forbidden"},
	{404, "Not Found"},
	{405, "Method Not Allowed"},
	{406, "Not Acceptable"},
	{407, "Proxy Authentication Required"},
	{408, "Request Time-out"},
	{409, "Conflict"},
	{410, "Gone"},
	{411, "Length Required"},
	{412, "Precondition Failed"},
	{413, "Request Entity Too Large"},
	{414, "Request-URI Too Long"},
	{415, "Unsupported Media Type"},
	{416, "Requested range unsatisfiable"},
	{417, "Expectation failed"},
	{418, "I’m a teapot"},
	{421, "Misdirected Request"},
	{425, "Too Early"},
	{426, "Upgrade Required"},
	{428, "Precondition Required"},
	{429, "Too Many Requests"},
	{431, "Request Header Fields Too Large"},
	{451, "Unavailable For Legal Reasons"},
	{500, "Internal Server Error"},
	{501, "Not Implemented"},
	{502, "Bad Gateway"},
	{503, "Service Unavailable"},
	{504, "Gateway Time-out"},
	{505, "HTTP Version not supported"},
	{506, "Variant Also Negotiates"},
	{509, "Bandwidth Limit Exceeded"},
	{510, "Not extended"},
	{511, "Network authentication required"},
};

void WebServer::addRoute(const std::string &&route, std::function<Socket::HttpResponse(const Socket::HttpRequest &)> &&fct) {
	this->_routes[route] = fct;
}

void WebServer::addStaticFolder(const std::string &&route, const std::string &&path) {
	this->_folders[route] = path;
}

void WebServer::_threadFct(WebServer *server)
{
	while (!server->_closed)
		server->_serverLoop();
}

void WebServer::start(unsigned short port) {
	if (this->_thread != -1)
		return;
	this->_sock.bind(port);
	std::cout << "Started server on port " << port << std::endl;
	this->_closed = false;
	this->_thread = _beginthread(reinterpret_cast<_beginthread_proc_type>(&_threadFct), 0, this);
}

void ___(int) {}

void WebServer::stop() {
	if (this->_thread == -1)
		return;

	this->_closed = true;
	auto old = signal(SIGINT, ___);

	std::for_each(this->_webSocks.begin(), this->_webSocks.end(), [](std::shared_ptr<WebSocketConnection> &s) {
		s->wsock.disconnect();
		if (s->thread.joinable())
			s->thread.join();
	});
	raise(SIGINT); // Interrupt accept
	signal(SIGINT, old);
	_endthreadex(this->_thread);
	this->_thread = -1;
}

WebServer::~WebServer() {
	this->stop();
}

void WebServer::_serverLoop() {
	Socket newConnection = this->_sock.accept();
	Socket::HttpResponse response;
	Socket::HttpRequest requ;
	std::string s;
	int times = 10;

#ifdef _DEBUG
	std::cerr << "New connection from " << inet_ntoa(newConnection.getRemote().sin_addr) << ":" << newConnection.getRemote().sin_port << std::endl;
#endif
	try {
		try {
			requ.ip = newConnection.getRemote().sin_addr.s_addr;
			while (s.empty() && times--)
				s = newConnection.readUntilEOF();
			requ = Socket::parseHttpRequest(s);
			requ.ip = newConnection.getRemote().sin_addr.s_addr;
			if (requ.httpVer != "HTTP/1.1")
				throw AbortConnectionException(505);
			if (requ.path == "/chat")
				return this->_addWebSocket(newConnection, requ);
			else {
				auto it = this->_routes.find(requ.path);

				response.request = requ;
				if (it != this->_routes.end())
					response = it->second(requ);
				else
					response = this->_checkFolders(requ);
			}
		} catch (InvalidHTTPAnswerException &) {
			response.returnCode = 400;
			response.codeName = WebServer::codes.at(400);
			response.header["content-type"] = "application/json";
			response.body = nlohmann::json({
				{"error", "Invalid HTTP request"},
				{"request", s}
			}).dump();
		} catch (NotImplementedException &) {
			response = WebServer::_makeGenericPage(501);
		} catch (AbortConnectionException &e) {
			if (*e.getBody()) {
				response.returnCode = e.getCode();
				response.codeName = WebServer::codes.at(response.returnCode);
				response.header["content-type"] = e.getType();
				response.body = e.getBody();
			} else
				response = WebServer::_makeGenericPage(e.getCode());
		}
		response.codeName = WebServer::codes.at(response.returnCode);
	} catch (std::exception &e) {
		std::cerr << e.what() << std::endl;
		response = WebServer::_makeGenericPage(500, e.what());
	}
	response.httpVer = "HTTP/1.1";
	response.header["content-length"] = std::to_string(response.body.length());
#ifdef _DEBUG
	std::cout << inet_ntoa(newConnection.getRemote().sin_addr) << ":" << newConnection.getRemote().sin_port << " ";
	if (!requ.httpVer.empty())
		std::cout << requ.path;
	else {
		std::cout << "<Malformed HTTP request>";
		std::cerr << "Parsing error of request \"" << s << "\"" << std::endl;
	}
	std::cout << ": " << response.returnCode << std::endl;
#endif
	try {
		newConnection.send(Socket::generateHttpResponse(response));
	} catch (...) {
	}
	newConnection.disconnect();
}

Socket::HttpResponse WebServer::_makeGenericPage(unsigned short code) {
	Socket::HttpResponse response;

	response.returnCode = code;
	response.codeName = WebServer::codes.at(response.returnCode);
	response.header["content-type"] = "text/html";
	response.body = "<html>"
		"<head>"
			"<title>" + response.codeName + "</title>"
		"</head>"
		"<body>"
			"<h1>" + std::to_string(response.returnCode) + ": " + response.codeName + "</h1>"
		"</body>"
	"</html>";
	return response;
}

Socket::HttpResponse WebServer::_makeGenericPage(unsigned short code, const std::string &extra) {
	Socket::HttpResponse response;

	response.returnCode = code;
	response.codeName = WebServer::codes.at(response.returnCode);
	response.header["content-type"] = "text/html";
	response.body = "<html>"
		"<head>"
			"<title>" + response.codeName + "</title>"
		"</head>"
		"<body>"
			"<h1>" + std::to_string(response.returnCode) + ": " + response.codeName + " (" + extra + ")</h1>"
		"</body>"
	"</html>";
	return response;
}

Socket::HttpResponse WebServer::_checkFolders(const Socket::HttpRequest &request) {
	Socket::HttpResponse response;

	if (request.method != "GET")
		throw AbortConnectionException(405);
	// TODO: Fix vulnerability if using .. in URL
	for (auto &folder : this->_folders) {
		if (request.path.substr(0, folder.first.length()) == folder.first) {
			std::string realPath = folder.second + request.path.substr(folder.first.length());
			std::string type = WebServer::_getContentType(request.path);
			int i = std::ifstream::in;

			if (type.substr(0, 5) != "text/")
				i |= std::ifstream::binary;

			std::ifstream stream{realPath, i};

			if (stream.fail())
				throw AbortConnectionException(404);
			response.returnCode = 200;
			response.header["content-byte"] = type;
			response.body = {std::istreambuf_iterator<char>(stream), std::istreambuf_iterator<char>()};
			return response;
		}
	}
	throw AbortConnectionException(404);
}

std::string WebServer::_getContentType(const std::string &path) {
	// TODO: Fix bug if URL contains a .
	size_t pos = path.find_last_of('.');
	size_t pos2 = path.find_last_of('/');

	if (pos2 != std::string::npos && path.substr(0, pos2).find('.') != std::string::npos)
		throw AbortConnectionException(401);

	if (pos == std::string::npos)
		return "application/octet-stream";

	auto it = WebServer::types.find(path.substr(pos + 1));

	if (it == WebServer::types.end())
		return "application/" + path.substr(pos + 1);
	return it->second;
}

void WebServer::_addWebSocket(Socket &sock, const Socket::HttpRequest &requ) {
	auto response = WebSocket::solveHandshake(requ);
	std::shared_ptr<WebSocketConnection> wsock;
	std::weak_ptr<WebSocketConnection> wsock_weak;

	this->_webSocks.push_back(std::make_shared<WebSocketConnection>(sock));
	wsock_weak = wsock = this->_webSocks.back();
	wsock->wsock.needsMask(false);
	response.httpVer = "HTTP/1.1";
	response.codeName = WebServer::codes.at(response.returnCode);
	sock.send(Socket::generateHttpResponse(response));
	if (this->_onConnect)
		this->_onConnect(wsock->wsock);
	wsock->isThreadFinished = false;
	wsock->thread = std::thread([this, wsock_weak] {
		try {
			while (wsock_weak.lock()->wsock.isOpen()) {
				std::string msg = wsock_weak.lock()->wsock.getAnswer();

				if (this->_onMessage)
					this->_onMessage(wsock_weak.lock()->wsock, msg);
			}
		} catch (const std::exception &e) {
			puts(e.what());
			wsock_weak.lock()->wsock.disconnect();
			if (this->_onError)
				this->_onError(wsock_weak.lock()->wsock, e);
		}
		wsock_weak.lock()->isThreadFinished = true;
	});
#ifdef _DEBUG
	std::cout << inet_ntoa(sock.getRemote().sin_addr) << ":" << sock.getRemote().sin_port << " " << requ.path << ": " << response.returnCode << std::endl;
#endif
}

void WebServer::broadcast(const std::string &msg) {
	this->_webSocks.erase(std::remove_if(this->_webSocks.begin(), this->_webSocks.end(),
													[](std::shared_ptr<WebSocketConnection> &s) {
														if (s->isThreadFinished) {
															if (s->thread.joinable())
																s->thread.join();
															return true;
														}
														return false;
													}),
		this->_webSocks.end());
	for (auto &wsock : this->_webSocks)
		try {
			wsock->wsock.send(msg);
		} catch (...) {
			wsock->wsock.disconnect();
		}
}

void WebServer::onWebSocketConnect(const std::function<void(WebSocket &)> &fct) {
	this->_onConnect = fct;
}

void WebServer::onWebSocketMessage(const std::function<void(WebSocket &, const std::string &)> &fct) {
	this->_onMessage = fct;
}

void WebServer::onWebSocketError(const std::function<void(WebSocket &, const std::exception &)> &fct) {
	this->_onError = fct;
}
