//
// Created by Gegel85 on 06/04/2019.
//

#ifndef DISC_ORD_WebSocket_HPP
#define DISC_ORD_WebSocket_HPP


#include <random>
#include "Socket.hpp"

class WebSocket : public Socket {
private:
	bool _masks = true;
	std::random_device _rand;

	void _establishHandshake(const std::string &host);
	void _pong(const std::string &validator);
	static std::string _solveHandshakeToken(const std::string &token);

public:
	static const char * const codesStrings[];

	WebSocket(const WebSocket &);
	explicit WebSocket(const Socket &);
	explicit WebSocket(const Socket &&);
	WebSocket() = default;
	~WebSocket() override = default;

	void needsMask(bool masks);

	void send(const std::string &value) override;
	void disconnect() override;
	void connect(const std::string &host, unsigned short portno) override;
	void sendHttpRequest(const HttpRequest &request);
	std::string getAnswer();
	std::string getRawAnswer();
	std::string strictRead(size_t i);
	static std::vector<unsigned char> hashString(const std::string &str);
	static HttpResponse solveHandshake(const HttpRequest &request);
	WebSocket &operator=(const WebSocket &);
};


#endif //DISC_ORD_WebSocket_HPP
