//
// Created by Gegel85 on 06/04/2019.
//

#include <windows.h>
#include <Wincrypt.h>
#include <sstream>
#include <iostream>
#include "../Exceptions.hpp"
#include "WebSocket.hpp"
#include "base64.hpp"

#define WEBSOCKET_CODE(code) ((code - 1000 < 0 || code - 1000 > 15) ? ("???") : (codesStrings[code - 1000]))
#define SHA1LEN 20

const char * const WebSocket::codesStrings[] = {
	"Normal Closure",
	"Going Away",
	"Protocol error",
	"Unsupported Data",
	"???",
	"No Status Rcvd",
	"Abnormal Closure",
	"Invalid frame payload data",
	"Policy Violation",
	"Message Too Big",
	"Mandatory Ext",
	"Internal Server Error",
	"???",
	"???",
	"???",
	"TLS handshake",
};

void WebSocket::_establishHandshake(const std::string &host)
{
	Socket::HttpRequest	request;
	Socket::HttpResponse	response;

	request.host = host;
	request.path = "/chat";
	request.method = "GET";
	request.header = {
		{"Sec-WebSocket-Key",      "x3JJHMbDL1EzLkh9GBhXDw=="},
		{"Sec-WebSocket-Version",  "13"},
		{"Sec-WebSocket-Protocol", "chat, superchat"},
		{"Upgrade",                "websocket"},
		{"Connection",             "Upgrade"},
	};
	this->sendHttpRequest(request);
	response = this->parseHttpResponse(this->getRawAnswer());
	if (response.returnCode != 101) {
		this->disconnect();
		throw InvalidHandshakeException("WebSocket Handshake failed: Server answered with code " + std::to_string(response.returnCode) + " but 101 was expected");
	}
}

void WebSocket::send(const std::string &value)
{
	std::stringstream stream;
	std::string	result = value;
	unsigned	random_value = this->_rand();
	std::string	key = std::string("") +
		static_cast<char>((random_value >> 24U) & 0xFFU) +
		static_cast<char>((random_value >> 16U) & 0xFFU) +
		static_cast<char>((random_value >> 8U) & 0xFFU) +
		static_cast<char>(random_value & 0xFFU);

	if (this->_masks)
		for (unsigned i = 0; i < result.size(); i++)
			result[i] = result[i] ^ key[i % 4];
	stream << static_cast<char>(0x81);
	stream << static_cast<char>(0x80 * this->_masks + (value.size() <= 125 ? value.size() : (126 + (value.size() > 65535))));
	if (value.size() > 65535) {
		stream << static_cast<char>(value.size() >> 24U);
		stream << static_cast<char>(value.size() >> 16U);
		stream << static_cast<char>(value.size() >> 8U);
		stream << static_cast<char>(value.size());
	} else if (value.size() > 125) {
		stream << static_cast<char>(value.size() >> 8U);
		stream << static_cast<char>(value.size());
	}
	if (this->_masks)
		stream << key;
	stream << result;
	Socket::send(stream.str());
}

void WebSocket::_pong(const std::string &validator)
{
	std::stringstream stream;
	unsigned char byte = 0x8A;

	if (validator.size() > 125)
		throw InvalidPongException("Pong validator cannot be longer than 125B");
	stream << byte;
	byte = 0x80 + validator.size();
	stream << byte;
	Socket::send(stream.str());
}

std::string WebSocket::strictRead(size_t i)
{
	std::string result = this->read(i);

	if (i != result.size())
		throw EOFException("EOFException");
	return result;
}

std::string WebSocket::getAnswer()
{
	std::string	result;
	std::string	key;
	unsigned long	length;
	unsigned char	opcode;
	bool	isMasked;
	bool	isEnd;

	if (!this->isOpen())
		throw NotConnectedException("This socket is not connected to a server");

	opcode = this->strictRead(1)[0];
	isEnd = (opcode >> 7U);
	opcode &= 0xFU;

	length = this->strictRead(1)[0];
	isMasked = (length >> 7U);
	length &= 0x7FU;

	if (length == 126)
		length = (static_cast<unsigned char>(this->strictRead(1)[0]) << 8U) +
			static_cast<unsigned char>(this->strictRead(1)[0]);
	else if (length == 127)
		length = (static_cast<unsigned char>(this->strictRead(1)[0]) << 24U) +
			(static_cast<unsigned char>(this->strictRead(1)[0]) << 16U) +
			(static_cast<unsigned char>(this->strictRead(1)[0]) << 8U) +
			static_cast<unsigned char>(this->strictRead(1)[0]);

	if (isMasked)
		key = this->strictRead(4);

	if (opcode == 0x9) {
		this->_pong(result);
		return this->getAnswer();
	}

	result = this->strictRead(length);
	if (isMasked) {
		for (unsigned i = 0; i < result.size(); i++)
			result.at(i) ^= key.at(i % 4);
	}

	if (opcode == 0x8) {
		this->disconnect();
		int code = (static_cast<unsigned char>(result[0]) << 8U) + static_cast<unsigned char>(result[1]);
		throw ConnectionTerminatedException("Server closed connection with code " + std::to_string(code) + " (" + WEBSOCKET_CODE(code) + ")", code);
	}

	if (!isEnd)
		return result + this->getAnswer();
	return result;
}

void WebSocket::disconnect()
{
	try {
		std::stringstream stream;
		std::string result = "\x03\xe8";
		unsigned random_value = this->_rand();
		std::string key = std::string() +
				  static_cast<char>((random_value >> 24U) & 0xFFU) +
				  static_cast<char>((random_value >> 16U) & 0xFFU) +
				  static_cast<char>((random_value >> 8U) & 0xFFU) +
				  static_cast<char>(random_value & 0xFFU);

		for (unsigned i = 0; i < result.size(); i++)
			result[i] = result[i] ^ key[i % 4];
		stream << "\x88\x82" << key << result;
		Socket::send(stream.str());
		Socket::disconnect();
	} catch (...) {
		Socket::disconnect();
	}
}

std::string WebSocket::getRawAnswer()
{
	return Socket::readUntilEOF();
}

void WebSocket::sendHttpRequest(const Socket::HttpRequest &request)
{
	std::string requestString = generateHttpRequest(request);

	Socket::send(requestString);
}

void WebSocket::connect(const std::string &host, unsigned short portno)
{
	Socket::connect(host, portno);
	this->_establishHandshake(host);
}

WebSocket::WebSocket(const Socket &sock) :
	Socket(sock)
{
}

WebSocket::WebSocket(const Socket &&sock) :
	Socket(sock)
{
}

Socket::HttpResponse WebSocket::solveHandshake(const Socket::HttpRequest &request)
{
	Socket::HttpRequest requ = request;
	HttpResponse response;

	if (request.method != "GET")
		throw AbortConnectionException(405);
	if (requ.header["Upgrade"] != "websocket")
		throw AbortConnectionException(400);
	if (requ.header["Connection"].find("Upgrade") == std::string::npos)
		throw AbortConnectionException(400);
	response.returnCode = 101;
	response.header["Upgrade"] = "websocket";
	response.header["Connection"] = "Upgrade";
	response.header["Sec-WebSocket-Accept"] = _solveHandshakeToken(requ.header["Sec-WebSocket-Key"]);
	return response;
}

std::string WebSocket::_solveHandshakeToken(const std::string &token)
{
	if (token.empty())
		throw AbortConnectionException(400);
	return base64::encode(hashString(token + "258EAFA5-E914-47DA-95CA-C5AB0DC85B11"));
}

std::vector<unsigned char> WebSocket::hashString(const std::string &str)
{
	HCRYPTPROV hProv = 0;
	HCRYPTHASH hHash = 0;
	DWORD cbRead = 0;
	BYTE rgbHash[SHA1LEN];
	DWORD cbHash = 0;
	// Logic to check usage goes here.

	// Get handle to the crypto provider
	if (!CryptAcquireContext(&hProv, nullptr, nullptr, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT))
		throw CryptFailedException("CryptAcquireContext failed");

	if (!CryptCreateHash(hProv, CALG_SHA1, 0, 0, &hHash)) {
		CryptReleaseContext(hProv, 0);
		throw CryptFailedException("CryptCreateHash failed");
	}

	if (!CryptHashData(hHash, reinterpret_cast<const BYTE *>(str.c_str()), str.size(), 0)) {
		CryptReleaseContext(hProv, 0);
		CryptDestroyHash(hHash);
		throw CryptFailedException("CryptHashData failed");
	}

	cbHash = SHA1LEN;
	if (!CryptGetHashParam(hHash, HP_HASHVAL, rgbHash, &cbHash, 0)) {
		CryptDestroyHash(hHash);
		CryptReleaseContext(hProv, 0);
		throw CryptFailedException("CryptGetHashParam failed");
	}

	CryptDestroyHash(hHash);
	CryptReleaseContext(hProv, 0);

	return {rgbHash, rgbHash + 20};
}

WebSocket &WebSocket::operator=(const WebSocket &s)
{
	Socket::operator=(s);
	return *this;
}

WebSocket::WebSocket(const WebSocket &s) :
	WebSocket(static_cast<Socket>(s))
{
	this->_masks = s._masks;
}

void WebSocket::needsMask(bool masks)
{
	this->_masks = masks;
}
