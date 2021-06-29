#include <Windows.h>
#include <SokuLib.hpp>
#include <map>
#include <iostream>
#include <fstream>
#include <nlohmann/json.hpp>
#include <dinput.h>
#include <Shlwapi.h>
#include <sstream>
#include "DrawUtils.hpp"

#define FONT_HEIGHT 16
#define TEXTURE_SIZE 0x200

#ifdef _DEBUG
#define _DEBUG_BUILD
#endif

//#define _DEBUG

#ifndef _DEBUG
#define puts(x)
#define printf(...)
#endif

#define SHADOW_OFFSET 4
#define SEND_FREQUENCY 32
#define BOTTOM_POS {294, 430}

static int (SokuLib::BattleClient::*s_originalBattleCLOnProcess)();
static int (SokuLib::BattleServer::*s_originalBattleSVOnProcess)();
static int (SokuLib::BattleClient::*s_originalBattleCLOnRender)();
static int (SokuLib::BattleServer::*s_originalBattleSVOnRender)();
static int (SokuLib::SelectClient::*s_originalSelectCLOnRender)();
static int (SokuLib::SelectServer::*s_originalSelectSVOnRender)();
static int (SokuLib::Battle::*s_originalBattleOnRender)();
static int (SokuLib::Select::*s_originalSelectOnRender)();
static int (__stdcall *realSendTo)(SOCKET s, char *buf, int len, int flags, sockaddr *to, int tolen);
static int (__stdcall *realRecvFrom)(SOCKET s, char *buf, int len, int flags, sockaddr *to, int *tolen);

static HMODULE myModule;
static int renderCounter = 0;
static int loaded = false;
static int hasMod = false;
static int desyncType = 0;
static unsigned desyncTimer = 0;
static float angle = 0;
static DrawUtils::Sprite bg;
static DrawUtils::Sprite desyncBg;
static DrawUtils::Sprite desyncBgRed;
static DrawUtils::Sprite desyncBgBlue;
static DrawUtils::Sprite gear;
static DrawUtils::Sprite gearShadow;
static DrawUtils::Sprite exclamation;
static DrawUtils::Sprite exclamationShadow;
static char latestBuffer = 0;
static char opBuffer = 0;
static SOCKET sock;
static std::vector<unsigned char> toD;
static unsigned lastHadMod = 0;
static SokuLib::PacketDesDetState myBuffers[4];
static SokuLib::PacketDesDetState opBuffers[4];

int *CTextureManager_LoadTextureFromResource(int *ret, HMODULE hSrcModule, LPCTSTR pSrcResource) {
	int id = 0;
	long int result;
	D3DXIMAGE_INFO info;

	printf("Loading resource %p from module %p\n", pSrcResource, hSrcModule);
	if (FAILED(result = D3DXGetImageInfoFromResource(hSrcModule, pSrcResource, &info))) {
		fprintf(stderr, "D3DXGetImageInfoFromResource(%p, %p, %p) failed with code %li.\n", hSrcModule, pSrcResource, &info, result);
		*ret = 0;
		return ret;
	}

	LPDIRECT3DTEXTURE9 *pphandle = SokuLib::textureMgr.allocate(&id);

	*pphandle = nullptr;
	if (SUCCEEDED(D3DXCreateTextureFromResourceEx(
		SokuLib::pd3dDev,
		hSrcModule,
		pSrcResource,
		info.Width,
		info.Height,
		info.MipLevels,
		D3DUSAGE_RENDERTARGET,
		info.Format,
		D3DPOOL_DEFAULT,
		D3DX_DEFAULT,
		D3DX_DEFAULT,
		0,
		&info,
		nullptr,
		pphandle
	))) {
		*ret = id;
	} else {
		SokuLib::textureMgr.deallocate(id);
		*ret = 0;
	}
	return ret;
}

void fillPacket(SokuLib::PacketDesDetState &packet)
{
	auto &mgr = SokuLib::getBattleMgr();

	packet.type = SokuLib::DESDET_STATE;
	packet.lX = mgr.leftCharacterManager.objectBase.position.x;
	packet.lY = mgr.leftCharacterManager.objectBase.position.y;
	packet.lHP = mgr.leftCharacterManager.objectBase.hp / 10;
	packet.rX = mgr.rightCharacterManager.objectBase.position.x;
	packet.rY = mgr.rightCharacterManager.objectBase.position.y;
	packet.rHP = mgr.rightCharacterManager.objectBase.hp / 10;
	packet.displayedWeather = SokuLib::displayedWeather;
	packet.weatherCounter = SokuLib::weatherCounter;
	packet._ = false;
}

int getDesyncType(const SokuLib::PacketDesDetState &packet1, const SokuLib::PacketDesDetState &packet2)
{
	if (packet1.lHP != packet2.lHP) {
		printf("lHP %u != %u\n", packet1.lHP, packet2.lHP);
		return 2;
	}
	if (packet1.rHP != packet2.rHP) {
		printf("rHP %u != %u\n", packet1.rHP, packet2.rHP);
		return 2;
	}
	if (packet1.displayedWeather != packet2.displayedWeather) {
		printf("DW %u != %u\n", packet1.displayedWeather, packet2.displayedWeather);
		return 2;
	}
	if (packet1.weatherCounter != packet2.weatherCounter) {
		printf("WC %u != %u\n", packet1.weatherCounter, packet2.weatherCounter);
		return 2;
	}
	return 1;
}

void compareState()
{
	if (latestBuffer != opBuffer)
		return;
	if (memcmp(&myBuffers[latestBuffer], &opBuffers[opBuffer], sizeof(*opBuffers)) != 0)
		desyncType = getDesyncType(myBuffers[latestBuffer], opBuffers[opBuffer]);
	else {
		desyncType = 0;
		renderCounter = 0;
		desyncTimer = 0;
	}
	printf("We are %s desynced\n", desyncType == 0 ? "not" : (desyncType == 1 ? "soft" : "hard"));
}

int __stdcall myRecvFrom(SOCKET s, char *buf, int len, int flags, sockaddr *to, int *tolen)
{
	auto pack = reinterpret_cast<SokuLib::Packet *>(buf);

	while (true) {
		int bytes = realRecvFrom(s, buf, len, flags, to, tolen);

		if (pack->type == SokuLib::DESDET_MOD_ENABLE_REQUEST) {
			puts("Opponent has mod !");
			hasMod = true;
		} else if (pack->type == SokuLib::DESDET_STATE) {
			if (pack->desDetState.weatherCounter != opBuffers[opBuffer].weatherCounter)
				opBuffer = (opBuffer + 1) % 4;
			memcpy(&opBuffers[opBuffer], &pack->desDetState, sizeof(*opBuffers));
			compareState();
		} else if (pack->type == SokuLib::INIT_REQUEST) {
			if (lastHadMod == reinterpret_cast<sockaddr_in *>(to)->sin_addr.s_addr) {
				desyncType = 0;
				desyncTimer = 0;
				hasMod = false;
				lastHadMod = reinterpret_cast<sockaddr_in *>(to)->sin_addr.s_addr;
			}

			SokuLib::PacketType op = SokuLib::DESDET_MOD_ENABLE_REQUEST;

			realSendTo(s, reinterpret_cast<char *>(&op), sizeof(op), flags, to, *tolen);
			return bytes;
		} else if (pack->type == SokuLib::INIT_SUCCESS) {
			SokuLib::PacketType op = SokuLib::DESDET_MOD_ENABLE_REQUEST;

			realSendTo(s, reinterpret_cast<char *>(&op), sizeof(op), flags, to, *tolen);
			return bytes;
		} else
			return bytes;
	}
}

int __stdcall mySendTo(SOCKET s, char *buf, int len, int flags, sockaddr *to, int tolen)
{
	auto pack = reinterpret_cast<SokuLib::Packet *>(buf);

	sock = s;
	toD.resize(tolen);
	memcpy(toD.data(), to, tolen);
	if (pack->type == SokuLib::INIT_REQUEST) {
		if (lastHadMod == reinterpret_cast<sockaddr_in *>(to)->sin_addr.s_addr) {
			desyncType = 0;
			desyncTimer = 0;
			hasMod = false;
			lastHadMod = reinterpret_cast<sockaddr_in *>(to)->sin_addr.s_addr;
		}
	} else if (pack->type == SokuLib::HOST_GAME || pack->type == SokuLib::CLIENT_GAME && hasMod) {
		if (pack->game.event.type == SokuLib::GAME_MATCH_REQUEST) {
			SokuLib::activeWeather = SokuLib::WEATHER_CLEAR;
			latestBuffer = 0;
			opBuffer = 0;
			desyncType = 0;
			desyncTimer = 0;
			memset(myBuffers, 0, sizeof(*myBuffers));
			memset(opBuffers, 0, sizeof(*opBuffers));
		}
	}
	return realSendTo(s, buf, len, flags, to, tolen);
}

static void setupSprite(int text, DrawUtils::Sprite &sprite, DrawUtils::Vector2<unsigned> size, DrawUtils::Vector2<int> pos)
{
	sprite.texture.setHandle(text, size);
	sprite.setPosition(pos);
	sprite.setSize(size);
	sprite.rect.top = sprite.rect.width = 0;
	sprite.rect.width = size.x;
	sprite.rect.height = size.y;
	sprite.tint = DrawUtils::DxSokuColor::White;
}

static void loadResources()
{
	int text;

	if (loaded)
		return;
	loaded = true;
	setupSprite(*CTextureManager_LoadTextureFromResource(&text, myModule, MAKEINTRESOURCE(4)), bg, {48, 48}, {584, 8});
	setupSprite(*CTextureManager_LoadTextureFromResource(&text, myModule, MAKEINTRESOURCE(8)), gear, {48, 48}, BOTTOM_POS);
	setupSprite(*CTextureManager_LoadTextureFromResource(&text, myModule, MAKEINTRESOURCE(12)), gearShadow, {48, 48}, BOTTOM_POS);
	setupSprite(*CTextureManager_LoadTextureFromResource(&text, myModule, MAKEINTRESOURCE(16)), exclamation, {48, 48}, BOTTOM_POS);
	setupSprite(*CTextureManager_LoadTextureFromResource(&text, myModule, MAKEINTRESOURCE(20)), exclamationShadow, {48, 48}, BOTTOM_POS);
	setupSprite(*CTextureManager_LoadTextureFromResource(&text, myModule, MAKEINTRESOURCE(24)), desyncBg, {48, 48}, BOTTOM_POS);
	setupSprite(*CTextureManager_LoadTextureFromResource(&text, myModule, MAKEINTRESOURCE(28)), desyncBgRed, {48, 48}, BOTTOM_POS);
	setupSprite(*CTextureManager_LoadTextureFromResource(&text, myModule, MAKEINTRESOURCE(32)), desyncBgBlue, {48, 48}, BOTTOM_POS);
}

static void showDesyncIcon()
{
	auto &color = desyncType == 1 ? desyncBgBlue : desyncBgRed;

	renderCounter++;
	angle += 0.025;
	gearShadow.setPosition(DrawUtils::Vector2<int> BOTTOM_POS + DrawUtils::Vector2<int>{SHADOW_OFFSET, SHADOW_OFFSET});
	gearShadow.setRotation(angle);
	gearShadow.draw();

	gearShadow.setPosition(BOTTOM_POS);
	gearShadow.setRotation(angle);
	gearShadow.draw();

	if (renderCounter & 0x20) {
		color.setPosition(BOTTOM_POS);
		color.setRotation(0);
		color.draw();
	} else {
		desyncBg.setPosition(BOTTOM_POS);
		desyncBg.setRotation(0);
		desyncBg.draw();
	}

	gear.setPosition(BOTTOM_POS);
	gear.setRotation(angle);
	gear.draw();
}

int battleRenderingCommon(int ret)
{
	if (desyncType && ++desyncTimer > 16)
		showDesyncIcon();
	//if ((renderCounter & 0x20) && desyncType)
	//	(desyncType == 1 ? softDesync : hardDesync).draw();
	return ret;
}

static void showNoMod()
{
	angle += 0.025;
	gearShadow.setPosition({584 + SHADOW_OFFSET, 8 + SHADOW_OFFSET});
	gearShadow.setRotation(angle);
	gearShadow.draw();

	bg.setPosition({584, 8});
	bg.setRotation(0);
	bg.draw();

	exclamationShadow.setPosition({584 + SHADOW_OFFSET, 8 + SHADOW_OFFSET});
	exclamationShadow.setRotation(0);
	exclamationShadow.tint = DrawUtils::DxSokuColor::White * 0.5;
	exclamationShadow.draw();

	exclamation.setPosition({584, 8});
	exclamation.setRotation(0);
	exclamation.draw();

	gear.setPosition({584, 8});
	gear.setRotation(angle);
	gear.draw();
}

int chrSelRenderingCommon(int ret)
{
	loadResources();
	if (!hasMod)
		showNoMod();
	return ret;
}

static int battleProcessCommon(int ret)
{
	if (SokuLib::displayedWeather == SokuLib::WEATHER_CLEAR)
		return ret;
	if (hasMod && SokuLib::weatherCounter % SEND_FREQUENCY == 0 || SokuLib::weatherCounter == 999) {
		auto buff = myBuffers[latestBuffer];

		if (SokuLib::weatherCounter != myBuffers[latestBuffer].weatherCounter)
			latestBuffer = (latestBuffer + 1) % 4;
		fillPacket(myBuffers[latestBuffer]);
		if (memcmp(&buff, &myBuffers[latestBuffer], sizeof(buff)) != 0)
			realSendTo(
				sock,
				reinterpret_cast<char *>(&myBuffers[latestBuffer]),
				sizeof(myBuffers[latestBuffer]),
				0,
				reinterpret_cast<sockaddr *>(toD.data()),
				toD.size()
			);
		compareState();
	}
	return ret;
}

int __fastcall CBattleCL_OnProcess(SokuLib::BattleClient *This) {
	return battleProcessCommon((This->*s_originalBattleCLOnProcess)());
}

int __fastcall CBattleSV_OnProcess(SokuLib::BattleServer *This) {
	return battleProcessCommon((This->*s_originalBattleSVOnProcess)());
}

int __fastcall CBattleCL_OnRender(SokuLib::BattleClient *This) {
	return battleRenderingCommon((This->*s_originalBattleCLOnRender)());
}

int __fastcall CBattleSV_OnRender(SokuLib::BattleServer *This) {
	return battleRenderingCommon((This->*s_originalBattleSVOnRender)());
}

int __fastcall CSelectCL_OnRender(SokuLib::SelectClient *This) {
	return chrSelRenderingCommon((This->*s_originalSelectCLOnRender)());
}

int __fastcall CSelectSV_OnRender(SokuLib::SelectServer *This) {
	return chrSelRenderingCommon((This->*s_originalSelectSVOnRender)());
}

int __fastcall CSelect_OnRender(SokuLib::Select *This)
{
	int ret = (This->*s_originalSelectOnRender)();

	loadResources();
	showNoMod();
	return ret;
}

int __fastcall CBattle_OnRender(SokuLib::Battle *This)
{
	int ret = (This->*s_originalBattleOnRender)();

	desyncType = (renderCounter & 0x80) == 0;
	showDesyncIcon();
	return ret;
}

extern "C" __declspec(dllexport) bool CheckVersion(const BYTE hash[16]) {
	return memcmp(hash, SokuLib::targetHash, 16) == 0;
}

extern "C" __declspec(dllexport) bool Initialize(HMODULE hMyModule, HMODULE hParentModule) {
	DWORD old;
	FILE *_;

#ifdef _DEBUG
	AllocConsole();
	freopen_s(&_, "CONOUT$", "w", stdout);
	freopen_s(&_, "CONOUT$", "w", stderr);
#endif
	puts(":D");
	::VirtualProtect((PVOID)RDATA_SECTION_OFFSET, RDATA_SECTION_SIZE, PAGE_EXECUTE_WRITECOPY, &old);
	s_originalBattleCLOnProcess = SokuLib::TamperDword(&SokuLib::VTable_BattleClient.onProcess, CBattleCL_OnProcess);
	s_originalBattleSVOnProcess = SokuLib::TamperDword(&SokuLib::VTable_BattleServer.onProcess, CBattleSV_OnProcess);
	s_originalSelectCLOnRender  = SokuLib::TamperDword(&SokuLib::VTable_SelectClient.onRender, CSelectCL_OnRender);
	s_originalSelectSVOnRender  = SokuLib::TamperDword(&SokuLib::VTable_SelectServer.onRender, CSelectSV_OnRender);
	s_originalBattleCLOnRender  = SokuLib::TamperDword(&SokuLib::VTable_BattleClient.onRender, CBattleCL_OnRender);
	s_originalBattleSVOnRender  = SokuLib::TamperDword(&SokuLib::VTable_BattleServer.onRender, CBattleSV_OnRender);

#ifdef _DEBUG
	s_originalSelectOnRender  = SokuLib::TamperDword(&SokuLib::VTable_Select.onRender, CSelect_OnRender);
	s_originalBattleOnRender  = SokuLib::TamperDword(&SokuLib::VTable_Battle.onRender, CBattle_OnRender);
#endif
	realSendTo = SokuLib::TamperDword(&SokuLib::DLL::ws2_32.sendto, &mySendTo);
	realRecvFrom = SokuLib::TamperDword(&SokuLib::DLL::ws2_32.recvfrom, &myRecvFrom);
	::VirtualProtect((PVOID)RDATA_SECTION_OFFSET, RDATA_SECTION_SIZE, old, &old);
	::FlushInstructionCache(GetCurrentProcess(), nullptr, 0);
	return true;
}

extern "C" int APIENTRY DllMain(HMODULE hModule, DWORD fdwReason, LPVOID lpReserved) {
	myModule = hModule;
	return TRUE;
}