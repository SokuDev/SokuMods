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
				(std::string(SokuLib::getNetObject().profile1name) + " wants to start a practice session.\n\nAccept?").c_str(),
				"Start practice?",
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
			printf("Received packet %s, %i|%f|%s|%s\n", packet->apmElemUpdated.name, packet->apmElemUpdated.property.selected, packet->apmElemUpdated.property.value, packet->apmElemUpdated.property.checked ? "true" : "false", packet->apmElemUpdated.property.text);
			setElem(packet->apmElemUpdated.name, packet->apmElemUpdated.property);
			return true;
		default:
			return false;
		}
	}

	void elemChangedCommon(const std::string &name, SokuLib::PacketApmElemUpdated &buffer)
	{
		if (name.size() >= 64) {
			MessageBoxA(SokuLib::window, ("Widget " + name + " has a too long name.").c_str(), "Too long name", MB_ICONERROR);
			exit(1);
		}
		buffer.type = SokuLib::APM_ELEM_UPDATED;
		strcpy(buffer.name, name.c_str());
		printf("Sending packet %s, %i|%f|%s|%s\n", name.c_str(), buffer.property.selected, buffer.property.value, buffer.property.checked ? "true" : "false", buffer.property.text);
		sendto(socket, reinterpret_cast<const char *>(&buffer), sizeof(buffer), 0, reinterpret_cast<const sockaddr *>(&peer), sizeof(peer));
	}

	void onElemChanged(const std::string &name, bool checked)
	{
		SokuLib::PacketApmElemUpdated packet;

		packet.property.checked = checked;
		elemChangedCommon(name, packet);
	}

	void onElemChanged(const std::string &name, float value)
	{
		SokuLib::PacketApmElemUpdated packet;

		packet.property.value = value;
		elemChangedCommon(name, packet);
	}

	void onElemChanged(const std::string &name, int selected)
	{
		SokuLib::PacketApmElemUpdated packet;

		packet.property.selected = selected;
		elemChangedCommon(name, packet);
	}

	void onElemChanged(const std::string &name, const std::string &text)
	{
		SokuLib::PacketApmElemUpdated packet;

		if (text.size() > 4) {
			MessageBoxA(SokuLib::window, "Text is too long to be sent over.", "Too long text", MB_ICONERROR);
			exit(1);
		}
		strncpy(packet.property.text, text.c_str(), 4);
		elemChangedCommon(name, packet);
	}
}