//
// Created by Gegel85 on 31/10/2020
//

#include <SokuLib.hpp>
#include <shlwapi.h>
#include <list>
#include <map>
#include <process.h>
#include <iostream>
#include <thread>
#include <fstream>

#ifndef _DEBUG
#define puts(...)
#define printf(...)
#endif

#define FONT_HEIGHT 16
#define TEXTURE_SIZE 0x200

struct PingData {
	unsigned long long totalTime = 0;
	unsigned long long nbTime = 0;
	long long last = 0;
	long long peak = 0;
	unsigned long long oldTime = 0;
	unsigned long long lastUpdate = 0;
	unsigned ignoreTime = 0;
};

struct CDesignSprite {
	void *vftable; // =008576ac
	float UNKNOWN_1[2];
	float x;
	float y;
	byte active;
	byte UNKNOWN_2[3];
	int UNKNOWN_3;
};

static int (__stdcall *realRecvFrom)(SOCKET s, char *buf, int len, int flags, sockaddr *from, int * fromlen);
static int (__stdcall *realSendTo)(SOCKET s, char *buf, int len, int flags, sockaddr *to, int tolen);
static void (*s_originalDrawGradiantBar)(float param1, float param2, float param3);
//static SOCKET (__stdcall *realSocket)(int af, int type, int protocol);
static int (__stdcall *realCloseSocket)(SOCKET s);
static int (__stdcall *realBind)(SOCKET s, sockaddr * addr, int namelen);

static int (SokuLib::MenuConnect::*connectOnRender)();
static int (SokuLib::MenuConnect::*connectOnProcess)();

static SokuLib::SWRFont font;
static SokuLib::DrawUtils::Sprite stop;
static SokuLib::DrawUtils::Sprite box;
static SokuLib::DrawUtils::Sprite text;
static SokuLib::DrawUtils::Sprite boxText;
static SokuLib::DrawUtils::Sprite gear1;
static SokuLib::DrawUtils::Sprite gear2;
static SokuLib::DrawUtils::Sprite confirmBox;
static SokuLib::DrawUtils::Sprite yesH;
static SokuLib::DrawUtils::Sprite yes;
static SokuLib::DrawUtils::Sprite noH;
static SokuLib::DrawUtils::Sprite no;

static bool soku2Found = false;
static unsigned char soku2Major = 0;
static unsigned char soku2Minor = 0;
static char soku2Letter = 0;
static unsigned deadTime = 0;
static unsigned short port;
static bool spec;
static unsigned timer = 10000;
static bool someoneConnected = false;
static bool yesSelected = true;
static bool error = false;
static bool loaded = false;
static bool hosting = false;
static LARGE_INTEGER timer_frequency;
static uintptr_t threadPtr = 0;
static SokuLib::Trampoline *hostTrampoline;
static SOCKET mySocket = INVALID_SOCKET;
static int *retAddr = nullptr;
static PingData connectedPing;
static std::map<unsigned, PingData> data;
static std::map<unsigned, std::list<std::vector<unsigned char>>> queues;
static bool pingRequested = false;
static bool gotPingAnswer = false;
static bool sentPing = false;
static std::string name;
static sockaddr_in currentAddress;
static bool inQueue = false;
static bool needHook = false;

void getSoku2Version(wchar_t *path)
{
	std::ifstream stream{path};
	std::string line;

	printf("Loading file %S\n", path);
	soku2Found = true;
	while (std::getline(stream, line)) {
		auto pos = line.find("set_version(");

		if (pos == std::string::npos)
			continue;
		printf("Line is %s\n", line.c_str());
		line = line.substr(pos + strlen("set_version(") + 1);
		pos = line.find('"');
		if (pos == std::string::npos) {
			MessageBox(nullptr, "Cannot parse Soku2 version: Missing closing \" in set_version line.", "Init error", MB_ICONERROR);
			abort();
		}
		line = line.substr(0, pos);
		printf("Version string is %s\n", line.c_str());
		pos = line.find('.');
		if (pos == std::string::npos) {
			MessageBox(nullptr, "Cannot parse Soku2 version: Cannot find . chaarcter in version string.", "Init error", MB_ICONERROR);
			abort();
		}
		try {
			soku2Major = std::stoul(line.substr(0, pos));
			line = line.substr(pos + 1);
			soku2Minor = std::stoul(line, &pos);
			if (pos != line.size() - 1) {
				MessageBox(nullptr, "Cannot parse Soku2 version: Trailing letters found.", "Init error", MB_ICONERROR);
				abort();
			}
			soku2Letter = line.back();
		} catch (std::exception &e) {
			MessageBox(nullptr, ("Cannot parse Soku2 version: " + std::string(e.what()) + ".").c_str(), "Init error", MB_ICONERROR);
			abort();
		}
		printf("Soku2 version is %i.%i%c\n", soku2Major, soku2Minor, soku2Letter);
		return;
	}
	MessageBox(nullptr, "Cannot parse Soku2 version: The set_version line was not found.", "Init error", MB_ICONERROR);
	abort();
}

void loadSoku2Config()
{
	puts("Looking for Soku2 config...");

	int argc;
	wchar_t app_path[MAX_PATH];
	wchar_t setting_path[MAX_PATH];
	wchar_t **arg_list = CommandLineToArgvW(GetCommandLineW(), &argc);

	wcsncpy(app_path, arg_list[0], MAX_PATH);
	PathRemoveFileSpecW(app_path);
	if (GetEnvironmentVariableW(L"SWRSTOYS", setting_path, sizeof(setting_path)) <= 0) {
		if (arg_list && argc > 1 && StrStrIW(arg_list[1], L"ini")) {
			wcscpy(setting_path, arg_list[1]);
			LocalFree(arg_list);
		} else {
			wcscpy(setting_path, app_path);
			PathAppendW(setting_path, L"\\SWRSToys.ini");
		}
		if (arg_list) {
			LocalFree(arg_list);
		}
	}
	printf("Config file is %S\n", setting_path);

	wchar_t moduleKeys[1024];
	wchar_t moduleValue[MAX_PATH];
	GetPrivateProfileStringW(L"Module", nullptr, nullptr, moduleKeys, sizeof(moduleKeys), setting_path);
	for (wchar_t *key = moduleKeys; *key; key += wcslen(key) + 1) {
		wchar_t module_path[MAX_PATH];

		GetPrivateProfileStringW(L"Module", key, nullptr, moduleValue, sizeof(moduleValue), setting_path);

		wchar_t *filename = wcsrchr(moduleValue, '/');

		printf("Check %S\n", moduleValue);
		if (!filename)
			filename = app_path;
		else
			filename++;
		for (int i = 0; filename[i]; i++)
			filename[i] = tolower(filename[i]);
		if (wcscmp(filename, L"soku2.dll") != 0)
			continue;

		wcscpy(module_path, app_path);
		PathAppendW(module_path, moduleValue);
		while (auto result = wcschr(module_path, '/'))
			*result = '\\';
		PathRemoveFileSpecW(module_path);
		printf("Found Soku2 module folder at %S\n", module_path);
		PathAppendW(module_path, L"\\config\\SOKU2.lua");
		getSoku2Version(module_path);
		return;
	}
}

int __stdcall fakeRecvFrom(SOCKET s, char *buf, int len, int flags, sockaddr *from, int *fromlen)
{
	auto it = queues.find(currentAddress.sin_addr.s_addr);

	if (s != mySocket)
		return realRecvFrom(s, buf, len, flags, from, fromlen);
	if (it == queues.end())
		return realRecvFrom(s, buf, len, flags, from, fromlen);
	if (it->second.empty()) {
		puts("List is empty!");
		queues.clear();
		return realRecvFrom(s, buf, len, flags, from, fromlen);
	}
	inQueue = true;

	auto &entry = it->second.front();

	printf("Send fake packet ");
	SokuLib::displayPacketContent(std::cout, *reinterpret_cast<SokuLib::Packet *>(buf));
	printf("\n");
	len = min(len, entry.size());
	memcpy(buf, entry.data(), len);
	it->second.pop_front();
	if (from) {
		*reinterpret_cast<sockaddr_in *>(from) = currentAddress;
		if (fromlen)
			*fromlen = sizeof(currentAddress);
	}
	return len;
}

int __stdcall fakeSendTo(SOCKET s, char *buf, int len, int flags, sockaddr *to, int tolen)
{
	printf("Fake send %i: ", len);
	if (!inQueue) {
		printf("real ");
		SokuLib::displayPacketContent(std::cout, *reinterpret_cast<SokuLib::Packet *>(buf));
		printf("\n");
		return realSendTo(s, buf, len, flags, to, tolen);
	}
	printf("fake ");
	SokuLib::displayPacketContent(std::cout, *reinterpret_cast<SokuLib::Packet *>(buf));
	printf("\n");
	inQueue = false;
	return 0;
}

std::string getLastSocketError(int err = WSAGetLastError()) {
	char *s = nullptr;

	FormatMessageA(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		nullptr,
		err,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPSTR)&s,
		0,
		nullptr
	);

	auto result = "WSAGetLastError " + std::to_string(err) + ": " + s;

	LocalFree(s);
	return result;
}

void __stdcall fakeCloseSocket(SOCKET s)
{
	puts("Destroy socket");
	if (s == mySocket)
		mySocket = INVALID_SOCKET;
	realCloseSocket(s);
}

SOCKET __stdcall fakeSocket(int af, int type, int protocol)
{
	puts("Call to socket");
	if (mySocket != INVALID_SOCKET) {
		puts("Creating fake socket");
		return mySocket;
	}
	puts("Real socket");
	return SokuLib::DLL::ws2_32.socket(af, type, protocol);
}

int __stdcall fakeBind(SOCKET s, sockaddr *addr, int namelen)
{
	puts("Fake bind");
	if (s != mySocket)
		return realBind(s, addr, namelen);
	puts("No bind");
	return 0;
}

void cancelHost(SokuLib::MenuConnect &menu)
{
	*retAddr = 0x446B1B - (int)(retAddr) - 4;
	menu.subchoice = 0;
	puts("Cancel host sock");
	realCloseSocket(mySocket);
	mySocket = INVALID_SOCKET;
}

unsigned __stdcall hostLoop(void *)
{
	int len;
	struct sockaddr_in addr;
	auto menu = SokuLib::getMenuObj<SokuLib::MenuConnect>();
	LARGE_INTEGER counter;

	port = menu->port;
	spec = menu->spectate;
	puts("Thread start");
	while (hosting) {
		SokuLib::Packet packet;

		addr.sin_family = AF_INET;
		addr.sin_port = htons(port);
		addr.sin_addr.S_un.S_addr = 0;
		printf("Recv %p\n", realRecvFrom);
		len = sizeof(addr);

		int size = realRecvFrom(mySocket, reinterpret_cast<char *>(&packet), sizeof(packet), 0, reinterpret_cast<sockaddr *>(&addr), &len);

		if (size == -1) {
			puts("Error!");
			return -1;
		}
		printf("Received %i bytes: opcode %s (%i) from %s\n", size, SokuLib::PacketTypeToString(packet.type).c_str(), packet.type, inet_ntoa(addr.sin_addr));

		QueryPerformanceCounter(&counter);
		unsigned long long time = counter.QuadPart * 1000 / timer_frequency.QuadPart;

		auto &ping = data[addr.sin_addr.s_addr];
		auto &queue = queues[addr.sin_addr.s_addr];

		if (packet.type != SokuLib::SOKUROLL_TIME_ACK) {
			queue.emplace_back();
			queue.back().resize(size);
			memcpy(queue.back().data(), &packet, size);
		}

		if (packet.type == SokuLib::HELLO) {
			//ping.nbTime = 0;
			packet.type = SokuLib::OLLEH;
			printf("Reply OLLEH %lli\n", ping.last);
			realSendTo(mySocket, reinterpret_cast<char *>(&packet), sizeof(packet.type), 0, reinterpret_cast<sockaddr *>(&addr), sizeof(addr));
		} else if (packet.type == SokuLib::INIT_REQUEST) {
			printf("Game ID: (");
			for (unsigned char c : packet.initRequest.gameId)
				printf("0x%02X ", c);
			printf(
				"\"%s\") %s (%s) is joining as ",
				std::string(packet.initRequest.gameId, packet.initRequest.gameId + sizeof(packet.initRequest.gameId)).c_str(),
				inet_ntoa(addr.sin_addr),
				std::string{packet.initRequest.name, packet.initRequest.name + packet.initRequest.nameLength}.c_str()
			);
			//TODO: Replace with the gameid the game is currently using
			if (memcmp(
				packet.initRequest.gameId,
				SokuLib::SWRUnlinked ? SokuLib::Soku110acNoSWRAllChars : SokuLib::Soku110acRollSWRAllChars,
				sizeof(packet.initRequest.gameId)
			) != 0) {
				printf("Invalid game version!\n");
				continue;
			}
			if (packet.initRequest.reqType == SokuLib::SPECTATE_REQU) {
				printf("spectator (last: %llims)\n", ping.last);
				packet.type = SokuLib::INIT_ERROR;
				packet.initError.reason = spec ? SokuLib::ERROR_GAME_STATE_INVALID : SokuLib::ERROR_SPECTATE_DISABLED;
				puts("Reply INIT_ERROR");
				realSendTo(mySocket, reinterpret_cast<char *>(&packet), sizeof(packet.initError), 0, reinterpret_cast<sockaddr *>(&addr), sizeof(addr));
			} else {
				printf("player (last: %llims)\n", ping.last);
				if (someoneConnected)
					continue;
				DWORD old;

				SokuLib::playSEWaveBuffer(0x39);
				name = std::string{packet.initRequest.name, packet.initRequest.name + packet.initRequest.nameLength};
				boxText.texture.createFromText(
					(name + " joined. Accept?<br>Calulating ping...").c_str(),
					font,
					{TEXTURE_SIZE, FONT_HEIGHT * 2}
				);
				deadTime = 90;
				needHook = true;
				yesSelected = true;
				currentAddress = addr;
				someoneConnected = true;
				pingRequested = false;
				gotPingAnswer = true;
				sentPing = false;
				memset(&ping, 0, sizeof(ping));
				memset(&packet.initSuccess, 0, sizeof(packet.initSuccess));
				packet.type = SokuLib::INIT_SUCCESS;
				printf("strncpy(%p, %p, %i)\n", packet.initSuccess.clientProfileName, packet.initRequest.name, min(name.size(), sizeof(packet.initSuccess.clientProfileName)));
				strncpy(packet.initSuccess.clientProfileName, name.c_str(), sizeof(packet.initSuccess.clientProfileName));
				strcpy(packet.initSuccess.hostProfileName, SokuLib::profile1.name);
				packet.initSuccess.swrDisabled = SokuLib::SWRUnlinked;
				packet.initSuccess.dataSize = 68;
				packet.initSuccess.unknown1[4] = 0x10;
				printf("Reply ");
				SokuLib::displayPacketContent(std::cout, packet);
				printf("\n");
				realSendTo(mySocket, reinterpret_cast<char *>(&packet), sizeof(packet.initSuccess), 0, reinterpret_cast<sockaddr *>(&addr), sizeof(addr));
			}
		} else if (packet.type == SokuLib::QUIT) {
			packet.type = SokuLib::QUIT;
			puts("Reply QUIT");
			realSendTo(mySocket, reinterpret_cast<char *>(&packet), sizeof(packet.type), 0, reinterpret_cast<sockaddr *>(&addr), sizeof(addr));
		} else if (packet.type == SokuLib::CLIENT_GAME) {
			if (!someoneConnected)
				continue;
			packet.type = SokuLib::HOST_GAME;
			puts("Reply HOST_GAME");
			realSendTo(mySocket, reinterpret_cast<char *>(&packet), sizeof(packet.game), 0, reinterpret_cast<sockaddr *>(&addr), sizeof(addr));
			pingRequested &= time - ping.lastUpdate <= 1000;
			if (!pingRequested) {
				sentPing = true;
				pingRequested = true;
				packet.type = SokuLib::SOKUROLL_TIME;
				packet.rollTime.timeStamp = 0;
				packet.rollTime.sequenceId = 0;
				if (gotPingAnswer)
					ping.oldTime = time;
				ping.lastUpdate = time;
				puts("Reply ROLLTIME");
				gotPingAnswer = false;
				realSendTo(mySocket, reinterpret_cast<char *>(&packet), sizeof(packet.rollTime), 0, reinterpret_cast<sockaddr *>(&addr), sizeof(addr));
			}
		} else if (packet.type == SokuLib::SOKUROLL_TIME_ACK) {
			if (!sentPing)
				continue;
			gotPingAnswer = true;
			sentPing = false;
			ping.last = time - ping.oldTime;
			ping.peak = max(ping.peak, ping.last);
			ping.oldTime = time;
			ping.totalTime += ping.last;
			ping.nbTime++;
			boxText.texture.createFromText(
				(name + " joined. Accept?<br>Ping: Last:" + std::to_string(ping.last) + "ms, Aver:" + std::to_string(ping.totalTime / ping.nbTime) + "ms, Peak:" + std::to_string(ping.peak) + "ms").c_str(),
				font,
				{TEXTURE_SIZE, FONT_HEIGHT * 2}
			);
		} else if (packet.type == SokuLib::SOKU2_PLAY_REQU && soku2Found) {
			puts("Reply SOKU2_PLAY_REQU");
			packet.soku2PlayRequ.major = soku2Major;
			packet.soku2PlayRequ.minor = soku2Minor;
			packet.soku2PlayRequ.letter = soku2Letter;
			realSendTo(mySocket, reinterpret_cast<char *>(&packet), sizeof(packet.soku2PlayRequ), 0, reinterpret_cast<sockaddr *>(&addr), sizeof(addr));
		}
	}
	return 0;
}

void onRender()
{
	//rect.setFillColor(SokuLib::DrawUtils::DxSokuColor::White);
	//rect.setPosition({20, 20});
	//rect.setSize({20, 20});
	//rect.draw();
	if (hosting || error) {
		box.draw();
		text.draw();
		if (!error) {
			gear1.draw();
			gear2.draw();
		}
	}
	if (someoneConnected) {
		confirmBox.draw();
		boxText.draw();
		(yesSelected ? yesH : yes).draw();
		(yesSelected ? no   : noH).draw();
	}
}

void loadAssets()
{
	SokuLib::FontDescription desc;

	desc.r1 = 255;
	desc.r2 = 255;
	desc.g1 = 255;
	desc.g2 = 255;
	desc.b1 = 255;
	desc.b2 = 255;
	desc.height = 12;
	desc.weight = FW_NORMAL;
	desc.italic = 0;
	desc.shadow = 1;
	desc.bufferSize = 1000000;
	desc.charSpaceX = 0;
	desc.charSpaceY = 0;
	desc.offsetX = 0;
	desc.offsetY = 0;
	desc.useOffset = 0;
	strcpy(desc.faceName, "MonoSpatialModSWR");
	desc.weight = FW_REGULAR;
	font.create();
	font.setIndirect(desc);

	stop.texture.createFromText("Stop hosting", font, {200, 20});
	stop.setPosition({66, 100});
	stop.setSize(stop.texture.getSize());
	stop.rect = {
		0, 0,
		static_cast<int>(stop.texture.getSize().x),
		static_cast<int>(stop.texture.getSize().y)
	};
	stop.tint = SokuLib::DrawUtils::DxSokuColor::White;

	confirmBox.texture.loadFromGame("data/menu/21_Base.bmp");
	confirmBox.setPosition({160, 192});
	confirmBox.setSize(confirmBox.texture.getSize());
	confirmBox.rect = {
		0, 0,
		static_cast<int>(confirmBox.texture.getSize().x),
		static_cast<int>(confirmBox.texture.getSize().y)
	};
	confirmBox.tint = SokuLib::DrawUtils::DxSokuColor::White;

	box.texture.loadFromGame("data/menu/21_Base.bmp");
	box.setMirroring(true, false);
	box.setSize(confirmBox.texture.getSize());
	box.rect = {
		0, 0,
		static_cast<int>(box.texture.getSize().x),
		static_cast<int>(box.texture.getSize().y)
	};
	box.tint = SokuLib::DrawUtils::DxSokuColor::White;
	box.draw();

	yesH.texture.loadFromGame("data/menu/22b_Yes.bmp");
	yesH.setPosition({242, 228});
	yesH.setSize(yesH.texture.getSize());
	yesH.rect = {
		0, 0,
		static_cast<int>(yesH.texture.getSize().x),
		static_cast<int>(yesH.texture.getSize().y)
	};
	yesH.tint = SokuLib::DrawUtils::DxSokuColor::White;
	yesH.draw();

	yes.texture.loadFromGame("data/menu/22a_Yes.bmp");
	yes.setPosition({242, 228});
	yes.setSize(yes.texture.getSize());
	yes.rect = {
		0, 0,
		static_cast<int>(yes.texture.getSize().x),
		static_cast<int>(yes.texture.getSize().y)
	};
	yes.tint = SokuLib::DrawUtils::DxSokuColor::White;
	yes.draw();

	noH.texture.loadFromGame("data/menu/23b_No.bmp");
	noH.setPosition({338, 228});
	noH.setSize(noH.texture.getSize());
	noH.rect = {
		0, 0,
		static_cast<int>(noH.texture.getSize().x),
		static_cast<int>(noH.texture.getSize().y)
	};
	noH.tint = SokuLib::DrawUtils::DxSokuColor::White;
	noH.draw();

	no.texture.loadFromGame("data/menu/23a_No.bmp");
	no.setPosition({338, 228});
	no.setSize(no.texture.getSize());
	no.rect = {
		0, 0,
		static_cast<int>(no.texture.getSize().x),
		static_cast<int>(no.texture.getSize().y)
	};
	no.tint = SokuLib::DrawUtils::DxSokuColor::White;
	no.draw();

	gear1.texture.loadFromGame("data/menu/Cursor3.bmp");
	gear1.setSize({
		gear1.texture.getSize().x,
		gear1.texture.getSize().y
	});
	gear1.rect.width = gear1.texture.getSize().x;
	gear1.rect.height = gear1.texture.getSize().y;

	gear2.texture.loadFromGame("data/menu/Cursor4.bmp");
	gear2.setSize({
		gear2.texture.getSize().x,
		gear2.texture.getSize().y
	});
	gear2.rect.width = gear2.texture.getSize().x;
	gear2.rect.height = gear2.texture.getSize().y;

	boxText.setPosition({164, 202});
	boxText.setSize({TEXTURE_SIZE, FONT_HEIGHT * 2});
	boxText.rect = {
		0, 0, TEXTURE_SIZE, FONT_HEIGHT * 2
	};
	boxText.tint = SokuLib::DrawUtils::DxSokuColor::White;
	boxText.fillColors[2] = boxText.fillColors[3] = SokuLib::DrawUtils::DxSokuColor{0xA0, 0xA0, 0xFF};
	boxText.draw();
	loaded = true;
}

void setMessageText(std::string msg)
{
	SokuLib::Vector2i realSize;

	text.texture.createFromText(msg.c_str(), font, {TEXTURE_SIZE, FONT_HEIGHT + 18}, &realSize);
	text.setSize({static_cast<unsigned int>(realSize.x), static_cast<unsigned int>(realSize.y)});
	text.rect = {
		0, 0, realSize.x, realSize.y
	};
	text.tint = SokuLib::DrawUtils::DxSokuColor::White;
	text.fillColors[2] = text.fillColors[3] = SokuLib::DrawUtils::DxSokuColor{
		static_cast<unsigned char>(error ? 0xFF : 0xA0),
		0xA0,
		static_cast<unsigned char>(error ? 0xA0 : 0xFF)
	};
	box.setSize({
		static_cast<unsigned int>(realSize.x) + 80,
		static_cast<unsigned int>(realSize.y) + 40
	});
	timer = 0;
}

void fakeHost()
{
	struct sockaddr_in addr;
	auto menu = SokuLib::getMenuObj<SokuLib::MenuConnect>();
	WSADATA WSAData;

	for (auto &[ip, packets] : queues)
		if (ip != currentAddress.sin_addr.s_addr)
			packets.clear();
	CloseHandle((HANDLE)threadPtr);
	threadPtr = 0;
	SokuLib::DLL::ws2_32.WSAStartup(MAKEWORD(2, 2), &WSAData);
	if (hosting) {
		puts("Stopping host");
		hosting = false;
		return;
	}
	if (mySocket != INVALID_SOCKET) {
		closesocket(mySocket);
		mySocket = INVALID_SOCKET;
	}
	queues.clear();
	puts("Starting host");
	puts("Creating socket...");
	mySocket = SokuLib::DLL::ws2_32.socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (mySocket == INVALID_SOCKET) {
		puts(getLastSocketError().c_str());
		error = true;
		setMessageText(getLastSocketError());
		SokuLib::DLL::ws2_32.WSACleanup();
		cancelHost(*menu);
		return;
	}

	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(menu->port);
	addr.sin_addr.S_un.S_addr = 0;
	puts("Binding...");
	if (realBind(mySocket, reinterpret_cast<sockaddr *>(&addr), sizeof(addr))) {
		puts(getLastSocketError().c_str());
		error = true;
		setMessageText(getLastSocketError());
		SokuLib::DLL::ws2_32.WSACleanup();
		cancelHost(*menu);
		return;
	}

	error = false;
	setMessageText("Hosting on port " + std::to_string(menu->port) + "...");
	hosting = true;

	threadPtr = _beginthreadex(nullptr, 0, hostLoop, nullptr, 0, nullptr);
	*retAddr = 0x446B1B - (int)(retAddr) - 4;
	menu->choice = 0;
	menu->subchoice = 0;
	(*(CDesignSprite **)0x089a390)->active = false;
}

void acceptHost(void *)
{
	puts("Moving to connect menu");
	SokuLib::MenuConnect::moveToConnectMenu();
	puts("Starting game host");
	auto menu = SokuLib::getMenuObj<SokuLib::MenuConnect>();
	menu->port = port;
	menu->spectate = spec;
	menu->choice = 1;
	menu->subchoice = 2;
	hostTrampoline->operator()<void(__thiscall *)(SokuLib::MenuConnect *)>(menu);
	puts("My job here is done");
}

void onUpdate()
{
	DWORD old;

	if (!loaded)
		loadAssets();
	if (needHook) {
		VirtualProtect((PVOID)TEXT_SECTION_OFFSET, TEXT_SECTION_SIZE, PAGE_EXECUTE_WRITECOPY, &old);
		*(char *)0x407f43 = 0x90;
		*(char *)0x407f44 = 0x90;
		*(char *)0x407f45 = 0x90;
		*(char *)0x407f46 = 0x90;
		*(char *)0x407f47 = 0x90;
		*(char *)0x407f48 = 0x90;
		*(char *)0x407f49 = 0x90;
		*(char *)0x407f4A = 0x90;
		*(char *)0x407f4B = 0x90;
		*(char *)0x407f4C = 0x90;
		*(char *)0x407f4D = 0x90;
		VirtualProtect((PVOID)TEXT_SECTION_OFFSET, TEXT_SECTION_SIZE, old, &old);
		needHook = false;
	}
	if (timer <= 60) {
		box.setPosition({
			static_cast<int>(640 - box.getSize().x * timer / 60),
			0
		});
		text.setPosition({
			static_cast<int>(640 - box.getSize().x * timer / 60) + 20,
			15
		});
		if (!error) {
			gear1.setPosition({
				static_cast<int>(640 - box.getSize().x * timer / 60 + box.getSize().x - 50),
				10
			});
			gear2.setPosition({
				static_cast<int>(640 - box.getSize().x * timer / 60 + box.getSize().x - 50 + 15),
				20
			});
		}
	} else if (timer > 180 && timer <= 240 && error) {
		box.setPosition({
			static_cast<int>(640 - box.getSize().x * (60 - (timer - 180)) / 60),
			0
				});
		text.setPosition({
			static_cast<int>(640 - box.getSize().x * (60 - (timer - 180)) / 60) + 20,
			15
		});
		if (!error) {
			gear1.setPosition({
				static_cast<int>(640 - box.getSize().x * (60 - (timer - 180)) / 60 + box.getSize().x - 50),
				10
			});
			gear2.setPosition({
				static_cast<int>(640 - box.getSize().x * (60 - (timer - 180)) / 60 + box.getSize().x - 50 + 15),
				20
			});
		}
	}
	gear1.setRotation(gear1.getRotation() + 0.075);
	gear2.setRotation(gear2.getRotation() - 0.075);
	if (timer < 240)
		timer++;
	if (someoneConnected) {
		if (deadTime)
			deadTime--;
		else if (SokuLib::inputMgrs.input.a == 1) {
			SokuLib::inputMgrs.input.a = 2;
			VirtualProtect((PVOID)TEXT_SECTION_OFFSET, TEXT_SECTION_SIZE, PAGE_EXECUTE_WRITECOPY, &old);
			*(char *)0x407f43 = 0xFF;
			*(char *)0x407f44 = 0xD0;
			*(char *)0x407f45 = 0x83;
			*(char *)0x407f46 = 0xF8;
			*(char *)0x407f47 = 0xFF;
			*(char *)0x407f48 = 0x89;
			*(char *)0x407f49 = 0x86;
			*(char *)0x407f4A = 0xB0;
			*(char *)0x407f4B = 0x00;
			*(char *)0x407f4C = 0x00;
			*(char *)0x407f4D = 0x00;
			VirtualProtect((PVOID)TEXT_SECTION_OFFSET, TEXT_SECTION_SIZE, old, &old);
			if (yesSelected) {
				SokuLib::playSEWaveBuffer(40);
				fakeHost();
				someoneConnected = false;
				*retAddr = 0x446A46 - (int)(retAddr) - 4;
				_beginthread(acceptHost, 0, nullptr);
			} else {
				SokuLib::playSEWaveBuffer(41);
				someoneConnected = false;
				queues.clear();
			}
		}
		if (std::abs(SokuLib::inputMgrs.input.horizontalAxis) == 1) {
			SokuLib::playSEWaveBuffer(39);
			yesSelected = !yesSelected;
		}
	}
}

void drawGradiantBar(float x, float y, float maxY)
{
	if (hosting)
		y = 100;
	s_originalDrawGradiantBar(x, y, maxY);
}

int __fastcall ConnectMenu_OnRender(SokuLib::MenuConnect *This)
{
	int ret = (This->*connectOnRender)();

	if (hosting)
		stop.draw();
	return ret;
}

int __fastcall ConnectMenu_OnProcess(SokuLib::MenuConnect *This)
{
	if (hosting && !someoneConnected) {
		if (SokuLib::inputMgrs.input.b == 1 || SokuLib::checkKeyOneshot(1, false, false, false)) {
			SokuLib::playSEWaveBuffer(0x29);
			return false;
		}
		if (SokuLib::inputMgrs.input.a == 1) {
			puts("Cancel host");
			SokuLib::playSEWaveBuffer(0x29);
			fakeHost();
		}
		return true;
	}
	return (This->*connectOnProcess)();
}

void placeHooks()
{
	DWORD old;

	//Setup hooks
	VirtualProtect((PVOID)RDATA_SECTION_OFFSET, RDATA_SECTION_SIZE, PAGE_EXECUTE_WRITECOPY, &old);
	realBind        = SokuLib::TamperDword(&SokuLib::DLL::ws2_32.bind,            &fakeBind);
	realCloseSocket = SokuLib::TamperDword(&SokuLib::DLL::ws2_32.closesocket,     &fakeCloseSocket);
	realRecvFrom    = SokuLib::TamperDword(&SokuLib::DLL::ws2_32.recvfrom,        &fakeRecvFrom);
	realSendTo      = SokuLib::TamperDword(&SokuLib::DLL::ws2_32.sendto,          &fakeSendTo);
	connectOnRender = SokuLib::TamperDword(&SokuLib::VTable_ConnectMenu.onRender, ConnectMenu_OnRender);
	connectOnProcess= SokuLib::TamperDword(&SokuLib::VTable_ConnectMenu.onProcess,ConnectMenu_OnProcess);
	VirtualProtect((PVOID)RDATA_SECTION_OFFSET, RDATA_SECTION_SIZE, old, &old);

	VirtualProtect((PVOID)TEXT_SECTION_OFFSET, TEXT_SECTION_SIZE, PAGE_EXECUTE_WRITECOPY, &old);
	//ogOnRenderCall = SokuLib::TamperNearJmpOpr(0x40817d, onRender);
	s_originalDrawGradiantBar = reinterpret_cast<void (*)(float, float, float)>(SokuLib::TamperNearJmpOpr(0x445e6f, drawGradiantBar));
	SokuLib::TamperNearCall(0x41311d, &fakeSocket);
	*(char *)(0x41311d + 5) = 0x90;
	VirtualProtect((PVOID)TEXT_SECTION_OFFSET, TEXT_SECTION_SIZE, old, &old);

	hostTrampoline = new SokuLib::Trampoline(0x446a40, fakeHost, 6);
	retAddr = reinterpret_cast<int *>(&hostTrampoline->getTrampoline()[6 + 18]);
	new SokuLib::Trampoline(0x41e390, onRender, 7);
	new SokuLib::Trampoline(0x407e3b, onUpdate, 5);
}

// �ݒ胍�[�h
void LoadSettings(LPCSTR profilePath)
{
	puts("Loading settings...");
	//config.refreshRate = GetPrivateProfileInt("DiscordIntegration", "RefreshTime", 1000, profilePath);
}

extern "C" __declspec(dllexport) bool CheckVersion(const BYTE hash[16])
{
	return memcmp(hash, SokuLib::targetHash, sizeof(SokuLib::targetHash)) == 0;
}

extern "C" __declspec(dllexport) bool Initialize(HMODULE hMyModule, HMODULE hParentModule)
{
	char profilePath[1024 + MAX_PATH];

#ifdef _DEBUG
	AllocConsole();
	freopen("CONOUT$", "w", stdout);
	freopen("CONOUT$", "w", stderr);
#endif

	puts("BGHost v1 Initializing...");
	loadSoku2Config();
	QueryPerformanceFrequency(&timer_frequency);
	GetModuleFileName(hMyModule, profilePath, 1024);
	PathRemoveFileSpec(profilePath);
	PathAppend(profilePath, "HostInBackground.ini");
	LoadSettings(profilePath);
	placeHooks();
	puts("Done...");
	return true;
}

extern "C" int APIENTRY DllMain(HMODULE hModule, DWORD fdwReason, LPVOID lpReserved)
{
	return TRUE;
}