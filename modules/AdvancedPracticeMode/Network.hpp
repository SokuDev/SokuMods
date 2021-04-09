//
// Created by PinkySmile on 09/04/2021.
//

#ifndef SWRSTOYS_NETWORK_HPP
#define SWRSTOYS_NETWORK_HPP


#include <string>
#include "Gui.hpp"

namespace Practice
{
	extern sockaddr_in peer;
	extern SOCKET socket;

	bool processPacket(char *buf, int len);
	void onElemChanged(const std::string &name, bool checked);
	void onElemChanged(const std::string &name, float value);
	void onElemChanged(const std::string &name, int selected);
	void onElemChanged(const std::string &name, const std::string &text);
}


#endif //SWRSTOYS_NETWORK_HPP
