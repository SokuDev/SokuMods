//
// Created by PinkySmile on 09/04/2021.
//

#include <SokuLib.hpp>
#include "Network.hpp"
#include "State.hpp"

namespace Practice
{
	sockaddr_in peer;
	SOCKET socket;

	bool processPacket(char *buf, int len)
	{
		int resp;
		auto packet = reinterpret_cast<SokuLib::Packet *>(buf);

		if (len < 1)
			return false;

		switch (packet->type) {
		case SokuLib::APM_START_SESSION_REQUEST:
			resp = MessageBoxA(
				SokuLib::window,
				(std::string(SokuLib::getNetObject().profile1name) + " wants to start a practice session.\n\nAccept ?").c_str(),
				"Start practice ?",
				MB_YESNO | MB_ICONQUESTION
			);
			if (resp == IDYES)
				settings.nonSaved.enabled = true;
			packet->type = SokuLib::APM_START_SESSION_RESPONSE;
			packet->apmResponse.accepted = (resp == IDYES);
			sendto(socket, reinterpret_cast<const char *>(packet), 2, 0, reinterpret_cast<sockaddr *>(&peer), sizeof(peer));
			return true;
		case SokuLib::APM_START_SESSION_RESPONSE:
			if (SokuLib::sceneId != SokuLib::SCENE_SELECTCL && SokuLib::sceneId != SokuLib::SCENE_SELECTSV)
				return true;

			MessageBoxA(
				SokuLib::window,
				(
					std::string(SokuLib::getNetObject().profile2name) +
					(packet->apmResponse.accepted ? " accepted" : " refused") +
					" the request to start practice"
				).c_str(),
				"Opponent response",
				packet->apmResponse.accepted ? MB_ICONINFORMATION : MB_ICONERROR
			);

			settings.nonSaved.enabled = packet->apmResponse.accepted;
			return true;
		case SokuLib::APM_ELEM_UPDATED:
			return true;
		default:
			return false;
		}
	}

	void onElemChanged(const std::string &name, bool checked)
	{

	}

	void onElemChanged(const std::string &name, float value)
	{

	}

	void onElemChanged(const std::string &name, int selected)
	{

	}

	void onElemChanged(const std::string &name, const std::string &text)
	{

	}

}