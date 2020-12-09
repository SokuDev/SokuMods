//
// Created by Gegel85 on 05/04/2019.
//

#include <cstring>
#include <sstream>
#include "Socket.hpp"
#include "../Exceptions.hpp"

#ifndef _WIN32
#include <unistd.h>
#else
#define close(fd) closesocket(fd)
#endif


#ifndef _WIN32
#	include <netdb.h>
#	include <arpa/inet.h>
#	include <sys/select.h>
	typedef fd_set FD_SET;
#endif


std::string getLastSocketError()
{
#ifdef _WIN32
	wchar_t *s = nullptr;

	FormatMessageW(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		nullptr,
		WSAGetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPWSTR)&s,
		0,
		NULL
	);

	std::stringstream stream;

	stream << "WSAGetLastError " << WSAGetLastError() << ": ";
	for (int i = 0; s[i]; i++)
		stream << static_cast<char>(s[i]);
	LocalFree(s);
	return stream.str();
#else
	return strerror(errno);
#endif
}

Socket::Socket()
{
#ifdef _WIN32
	WSADATA WSAData;
	if (WSAStartup(MAKEWORD(2,2), &WSAData))
		throw WSAStartupFailedException(getLastSocketError());
#endif
}

Socket::~Socket()
{
	Socket::disconnect();
}

void Socket::connect(const std::string &host, unsigned short portno)
{
	struct hostent	*server;

	if (this->isOpen())
		throw AlreadyOpenedException("This socket is already opened");

	/* create the socket */
	this->_sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (this->_sockfd == INVALID_SOCKET)
		throw SocketCreationErrorException(getLastSocketError());

	/* lookup the ip address */
	server = gethostbyname(host.c_str());
	if (server == nullptr)
		throw HostNotFoundException("Cannot find host '" + host + "'");
	this->connect(*reinterpret_cast<unsigned *>(server->h_addr), portno);
}

void Socket::connect(unsigned int ip, unsigned short portno)
{
	struct sockaddr_in	serv_addr = {};

	if (this->isOpen())
		throw AlreadyOpenedException("This socket is already opened");

	/* fill in the structure */
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(portno);
	serv_addr.sin_addr.s_addr = ip;

	/* connect the socket */
	if (::connect(this->_sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
		throw ConnectException(std::string("Cannot connect to ") + inet_ntoa(serv_addr.sin_addr));
	this->_opened = true;
}

Socket::HttpResponse Socket::makeHttpRequest(const Socket::HttpRequest &request)
{
	if (this->isOpen())
		throw AlreadyOpenedException("This socket is already opened");

	std::string requestString = this->generateHttpRequest(request);
	std::string response = this->makeRawRequest(request.host, request.portno, requestString);
	auto answer = this->parseHttpResponse(response);

	answer.request = request;

	if (answer.returnCode >= 400)
		throw HTTPErrorException(answer);
	return answer;
}

void Socket::disconnect()
{
	if (!this->isOpen())
		return;
	if (!this->_noDestroy)
		close(this->_sockfd);
	this->_opened = false;
}

std::string Socket::makeRawRequest(const std::string &host, unsigned short portno, const std::string &content)
{
	unsigned	ip;
	std::string	message;

	ip = inet_addr(host.c_str());
	if (ip != INADDR_NONE)
		this->connect(ip, portno);
	else
		this->connect(host, portno);

	this->send(content);
	message = this->readUntilEOF();
	this->disconnect();

	return message;
}

std::string Socket::read(int size)
{
	size_t totalSize = 0;
	char *result = nullptr;
	char  buffer[1024];

	while (size != 0) {
		int bytes = recv(this->_sockfd, buffer, (static_cast<unsigned>(size) >= sizeof(buffer)) ? sizeof(buffer) : size, 0);

		if (bytes <= 0) {
			if (size < 0)
				break;
			throw EOFException(getLastSocketError());
		}
		result = reinterpret_cast<char *>(std::realloc(result, totalSize + bytes));
		std::memcpy(&result[totalSize], buffer, bytes);
		totalSize += bytes;
		size -= bytes;
	}
	return {result, totalSize};
}

void Socket::send(const std::string &msg)
{
	unsigned	pos = 0;

	while (pos < msg.length()) {
		int bytes = ::send(this->_sockfd, &msg.c_str()[pos], msg.length() - pos, 0);

		if (bytes <= 0)
			throw EOFException(getLastSocketError());
		pos += bytes;
	}
}

std::string Socket::readUntilEOF()
{
	size_t totalSize = 0;
	char *result = nullptr;
	char  buffer[1024];

	while (true) {
		int bytes = recv(this->_sockfd, buffer, sizeof(buffer), 0);

		if (bytes < 0)
			throw EOFException(getLastSocketError());
		result = reinterpret_cast<char *>(std::realloc(result, totalSize + bytes));
		std::memcpy(&result[totalSize], buffer, bytes);
		totalSize += bytes;
		if (bytes < sizeof(buffer))
			return {result, totalSize};
	}
}

bool	Socket::isOpen() const
{
	FD_SET	set;
	timeval time = {0, 0};

	FD_ZERO(&set);
	FD_SET(this->_sockfd, &set);
	if (this->_opened && select(0, &set, nullptr, nullptr, &time) == -1)
		this->_opened = false;
	return (this->_opened);
}

Socket::Socket(SOCKET sockfd, struct sockaddr_in addr) :
	Socket()
{
	this->_sockfd = sockfd;
	this->_opened = true;
	this->_remote = addr;
}

void Socket::bind(unsigned short port)
{
	struct sockaddr_in	serv_addr = {};

	if (this->isOpen())
		throw AlreadyOpenedException("This socket is already opened");

	this->_sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (this->_sockfd == INVALID_SOCKET)
		throw SocketCreationErrorException(getLastSocketError());
	this->_opened = true;
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(port);
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	if (::bind(this->_sockfd, reinterpret_cast<const sockaddr *>(&serv_addr), sizeof(serv_addr)) < 0)
		throw BindFailedException(getLastSocketError());
	if (listen(this->_sockfd, 16) < 0)
		throw ListenFailedException(getLastSocketError());
}

const sockaddr_in &Socket::getRemote() const
{
	return this->_remote;
}

Socket Socket::accept()
{
	struct sockaddr_in serv_addr = {};
	int size = sizeof(serv_addr);
	SOCKET fd = ::accept(this->_sockfd, reinterpret_cast<sockaddr *>(&serv_addr), &size);

	if (fd == INVALID_SOCKET)
		throw AcceptFailedException(getLastSocketError());
	return {fd, serv_addr};
}

std::string Socket::generateHttpResponse(const Socket::HttpResponse &response)
{
	std::stringstream	msg;

	/* fill in the parameters */
	msg << response.httpVer << " " << response.returnCode << " " << response.codeName << "\r\n";
	for (auto &entry : response.header)
		msg << entry.first << ": " << entry.second << "\r\n";
	msg << "\r\n" << response.body;
	return msg.str();
}

std::string Socket::generateHttpRequest(const Socket::HttpRequest &request)
{
	std::stringstream	msg;

	/* fill in the parameters */
	msg << request.method << " " << request.path << " " << request.httpVer << "\r\n";
	msg << "Host: " << request.host << "\r\n";
	for (auto &entry : request.header)
		msg << entry.first << ": " << entry.second << "\r\n";
	msg << "\r\n" << request.body;
	return msg.str();
}

Socket::HttpRequest Socket::parseHttpRequest(const std::string &requ)
{
	std::stringstream response(requ);
	std::string str;
	HttpRequest request;

	response >> request.method;
	response >> request.path;
	response >> request.httpVer;

	std::getline(response, str);
	while (std::getline(response, str) && str.length() > 2) {
		std::size_t	pos = str.find(':');

		if (pos == std::string::npos)
			throw InvalidHTTPAnswerException("Invalid HTTP request");
		request.header[str.substr(0, pos)] = str.substr(pos + 2, str.size() - pos - 3);
	}

	try {
		request.host = request.header.at("Host");
	} catch (std::out_of_range &) {
		throw InvalidHTTPAnswerException("Invalid HTTP request");
	}
	request.body = requ.substr(response.tellg());
	return request;
}

void Socket::setNoDestroy(bool noDestroy) const
{
	this->_noDestroy = noDestroy;
}

bool Socket::isDisconnected() const
{
	return !this->isOpen();
}

Socket::Socket(const Socket &socket) :
	_opened(socket.isOpen()),
	_sockfd(socket.getSockFd()),
	_remote(socket.getRemote())
{
	socket.setNoDestroy(true);
}

Socket &Socket::operator=(const Socket &socket)
{
	if (this->isOpen() && !this->_noDestroy)
		this->disconnect();
	this->_opened = socket.isOpen();
	this->_sockfd = socket.getSockFd();
	this->_remote = socket.getRemote();
	socket.setNoDestroy(true);
	return *this;
}

Socket::HttpResponse Socket::parseHttpResponse(const std::string &respon)
{
	std::stringstream	response(respon);
	HttpResponse		request;
	std::string		str;

	response >> request.httpVer;
	response >> request.returnCode;

	if (response.fail())
		throw InvalidHTTPAnswerException("Invalid HTTP response");

	std::getline(response, request.codeName);
	request.codeName = request.codeName.substr(1, request.codeName.length() - 2);
	while (std::getline(response, str) && str.length() > 2) {
		std::size_t	pos = str.find(':');

		if (pos == std::string::npos)
			throw InvalidHTTPAnswerException("Invalid HTTP response");
		request.header[str.substr(0, pos)] = str.substr(pos + 2, str.size() - pos - 3);
	}

	request.body = respon.substr(response.tellg());
	return request;
}