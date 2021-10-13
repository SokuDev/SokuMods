//
// Created by Gegel85 on 31/10/2020
//

#include <SokuLib.hpp>
#include <shlwapi.h>
#include <list>
#include <map>
#include <process.h>
#include <thread>

#define FONT_HEIGHT 16
#define TEXTURE_SIZE 0x200

struct PingData {
	unsigned long long totalTime = 0;
	unsigned long long nbTime = 0;
	long long last = 0;
	unsigned long long oldTime = 0;
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

static void (*s_originalDrawGradiantBar)(float param1, float param2, float param3);
static SOCKET (__stdcall *realSocket)(int af, int type, int protocol);
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
static bool konniConnect = false;


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
	if (s == mySocket)
		mySocket = INVALID_SOCKET;
	realCloseSocket(s);
}

SOCKET __stdcall fakeSocket(int af, int type, int protocol)
{
	if (mySocket != INVALID_SOCKET)
		return mySocket;
	return realSocket(af, type, protocol);
}

int __stdcall fakeBind(SOCKET s, sockaddr *addr, int namelen)
{
	if (mySocket == INVALID_SOCKET)
		return realBind(s, addr, namelen);
	return 0;
}

void cancelHost(SokuLib::MenuConnect &menu)
{
	*retAddr = 0x446B1B - (int)(retAddr) - 4;
	menu.subchoice = 0;
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
	while (true) {
		SokuLib::Packet packet;

		addr.sin_family = AF_INET;
		addr.sin_port = htons(port);
		addr.sin_addr.S_un.S_addr = 0;
		puts("Recv");
		len = sizeof(addr);

		int size = SokuLib::DLL::ws2_32.recvfrom(mySocket, reinterpret_cast<char *>(&packet), sizeof(packet), 0, reinterpret_cast<sockaddr *>(&addr), &len);

		if (size == -1)
			return -1;
		printf("Received %i bytes: opcode %s (%i) from %s\n", size, SokuLib::PacketTypeToString(packet.type).c_str(), packet.type, inet_ntoa(addr.sin_addr));

		QueryPerformanceCounter(&counter);
		unsigned long long time = counter.QuadPart * 1000 / timer_frequency.QuadPart;

		auto &ping = data[addr.sin_addr.s_addr];

		if (packet.type == SokuLib::HELLO) {
			packet.type = SokuLib::OLLEH;
			if (
				addr.sin_addr.S_un.S_un_b.s_b1 == 51 &&
				addr.sin_addr.S_un.S_un_b.s_b2 == 15 &&
				addr.sin_addr.S_un.S_un_b.s_b3 == 50 &&
				addr.sin_addr.S_un.S_un_b.s_b4 == 91
			) {
				puts("Everyone say hi to Konni :wave:");
				konniConnect = false;
			}
			ping.oldTime = time;
			ping.nbTime = 0;
			puts("Reply OLLEH");
			SokuLib::DLL::ws2_32.sendto(mySocket, reinterpret_cast<char *>(&packet), sizeof(packet.type), 0, reinterpret_cast<sockaddr *>(&addr), sizeof(addr));
		} else if (packet.type == SokuLib::INIT_REQUEST) {
			ping.last = time - ping.oldTime;
			ping.oldTime = time;
			ping.totalTime += ping.last;
			ping.nbTime++;
			printf("%s (%s) is joining as ", inet_ntoa(addr.sin_addr), std::string{packet.initRequest.name, packet.initRequest.name + packet.initRequest.nameLength}.c_str());
			if (packet.initRequest.reqType == SokuLib::SPECTATE_REQU) {
				printf("spectator (last: %llims)\n", ping.last);
				if (
					addr.sin_addr.S_un.S_un_b.s_b1 == 51 &&
					addr.sin_addr.S_un.S_un_b.s_b2 == 15 &&
					addr.sin_addr.S_un.S_un_b.s_b3 == 50 &&
					addr.sin_addr.S_un.S_un_b.s_b4 == 91 &&
					!konniConnect
				) {
					konniConnect = true;
					puts("Waiting 2 seconds for Konni to be happy");
					std::this_thread::sleep_for(std::chrono::seconds(2));
				}
				packet.type = SokuLib::INIT_ERROR;
				packet.initError.reason = spec ? SokuLib::ERROR_GAME_STATE_INVALID : SokuLib::ERROR_SPECTATE_DISABLED;
				puts("Reply INIT_ERROR");
				SokuLib::DLL::ws2_32.sendto(mySocket, reinterpret_cast<char *>(&packet), sizeof(packet.initError), 0, reinterpret_cast<sockaddr *>(&addr), sizeof(addr));
			} else {
				printf("player (last: %llims)\n", ping.last);
				if (!someoneConnected) {
					DWORD old;

					SokuLib::playSEWaveBuffer(0x39);
					boxText.texture.createFromText(
						(std::string{packet.initRequest.name, packet.initRequest.name + packet.initRequest.nameLength} +
						" (" + std::to_string(ping.last) + "ms) joined.<br>Accept ?").c_str(),
						font,
						{TEXTURE_SIZE, FONT_HEIGHT * 2}
					);
					yesSelected = true;
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
				}
				someoneConnected = true;
				packet.type = SokuLib::OLLEH;
				puts("Reply OLLEH");
				SokuLib::DLL::ws2_32.sendto(mySocket, reinterpret_cast<char *>(&packet), sizeof(packet.type), 0, reinterpret_cast<sockaddr *>(&addr), sizeof(addr));
			}
		} else if (packet.type == SokuLib::QUIT) {
			packet.type = SokuLib::QUIT;
			puts("Reply QUIT");
			SokuLib::DLL::ws2_32.sendto(mySocket, reinterpret_cast<char *>(&packet), sizeof(packet.type), 0, reinterpret_cast<sockaddr *>(&addr), sizeof(addr));
		}
	}
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

	CloseHandle((HANDLE)threadPtr);
	SokuLib::DLL::ws2_32.WSAStartup(MAKEWORD(2, 2), &WSAData);
	if (hosting) {
		hosting = false;
		assert(mySocket != INVALID_SOCKET);
		closesocket(mySocket);
		mySocket = INVALID_SOCKET;
		return;
	}
	puts("Starting host");
	puts("Creating socket...");
	mySocket = realSocket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
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
	hostTrampoline->operator() < void(__thiscall *)(SokuLib::MenuConnect *) > (menu);
	puts("My job here is done");
}

void onUpdate()
{
	if (!loaded)
		loadAssets();
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
		if (SokuLib::inputMgrs.input.a == 1) {
			DWORD old;

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
	if (hosting) {
		if (SokuLib::inputMgrs.input.b == 1 || SokuLib::checkKeyOneshot(1, false, false, false)) {
			SokuLib::playSEWaveBuffer(0x29);
			return false;
		}
		if (SokuLib::inputMgrs.input.a == 1) {
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
	realSocket      = SokuLib::TamperDword(&SokuLib::DLL::ws2_32.socket,      &fakeSocket);
	realBind        = SokuLib::TamperDword(&SokuLib::DLL::ws2_32.bind,        &fakeBind);
	realCloseSocket = SokuLib::TamperDword(&SokuLib::DLL::ws2_32.closesocket, &fakeCloseSocket);
	connectOnRender = SokuLib::TamperDword(&SokuLib::VTable_ConnectMenu.onRender, ConnectMenu_OnRender);
	connectOnProcess= SokuLib::TamperDword(&SokuLib::VTable_ConnectMenu.onProcess, ConnectMenu_OnProcess);
	VirtualProtect((PVOID)RDATA_SECTION_OFFSET, RDATA_SECTION_SIZE, old, &old);

	VirtualProtect((PVOID)TEXT_SECTION_OFFSET, TEXT_SECTION_SIZE, PAGE_EXECUTE_WRITECOPY, &old);
	//ogOnRenderCall = SokuLib::TamperNearJmpOpr(0x40817d, onRender);
	s_originalDrawGradiantBar = reinterpret_cast<void (*)(float, float, float)>(SokuLib::TamperNearJmpOpr(0x445e6f, drawGradiantBar));
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

	puts("Initializing...");
	QueryPerformanceFrequency(&timer_frequency);
	GetModuleFileName(hMyModule, profilePath, 1024);
	PathRemoveFileSpec(profilePath);
	PathAppend(profilePath, "DiscordIntegration.ini");
	LoadSettings(profilePath);
	placeHooks();
	puts("Done...");
	return true;
}

extern "C" int APIENTRY DllMain(HMODULE hModule, DWORD fdwReason, LPVOID lpReserved)
{
	return TRUE;
}