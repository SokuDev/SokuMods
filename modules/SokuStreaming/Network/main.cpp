#include <iostream>
#include <fstream>
#include "WebServer.hpp"
#include "../Exceptions.hpp"

Socket::HttpResponse fct(const Socket::HttpRequest &requ)
{
	throw NotImplementedException();
}

Socket::HttpResponse favicon(const Socket::HttpRequest &requ)
{
	Socket::HttpResponse response;
	std::ifstream stream{"favicon.ico", std::ifstream::binary};

	response.returnCode = 200;
	response.header["content-type"] = "image/x-icon";
	response.body = {std::istreambuf_iterator<char>(stream), std::istreambuf_iterator<char>()};
	return response;
}

int main()
{
	WebServer server;

	try {
		server.addStaticFolder("/static", "CMakeFiles");
		server.addRoute("/", fct);
		server.addRoute("/favicon.ico", favicon);
		server.start(80);
		while (true);
		return 0;
	} catch (std::exception &e) {
		std::cerr << e.what() <<std::endl;
		return 1;
	}
}