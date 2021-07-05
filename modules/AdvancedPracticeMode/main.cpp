//
// Created by Gegel85 on 04/12/2020
//

#include <Windows.h>
#include <Shlwapi.h>
#include <SokuLib.hpp>
#include <algorithm>
#include <dinput.h>
#include "Logic.hpp"
#include "State.hpp"
#include "Gui.hpp"
#include "Network.hpp"

static int (__stdcall *original_recvfrom)(SOCKET s, char *buf, int len, int flags, sockaddr *from, int *fromlen);
static int (__thiscall SokuLib::BattleManager::*s_origCBattleManager_Process)();
static void (__thiscall SokuLib::BattleManager::*s_origCBattleManager_Render)();
static int (__thiscall SokuLib::Loading::*s_origCLoading_Process)();
static int (__thiscall SokuLib::SelectServer::*s_origCSelectSV_Process)();
static int (__thiscall SokuLib::SelectClient::*s_origCSelectCL_Process)();
static int (__thiscall SokuLib::Select::*s_origCSelect_Process)();
static int (__thiscall SokuLib::Title::*s_origCTitle_Process)();
float frameCounter = 0;


//sendto:   0x00857290
#define RECVFROM_JUMP_ADDR 0x00857280

int __stdcall myRecvfrom(SOCKET s, char *buf, int len, int flags, sockaddr *from, int *fromlen)
{
	int bytes;

	do {
		bytes = original_recvfrom(s, buf, len, flags, from, fromlen);
		if (from)
			memcpy(&Practice::peer, from, sizeof(Practice::peer));
		Practice::socket = s;
	} while (Practice::processPacket(buf, bytes));
	return bytes;
}

void networkSelectCommon()
{
	if (SokuLib::mainMode == SokuLib::BATTLE_MODE_VSCLIENT && SokuLib::checkKeyOneshot(DIK_F12, 0, 0, 0)) {
		auto res = MessageBoxA(SokuLib::window, (std::string("Do you want to start a training session against ") + SokuLib::getNetObject().profile2name + " ?").c_str(), "Start practice ?", MB_ICONQUESTION | MB_YESNO);

		if (res == IDYES) {
			SokuLib::PacketType packet = SokuLib::APM_START_SESSION_REQUEST;

			sendto(Practice::socket, reinterpret_cast<const char *>(&packet), 2, 0, reinterpret_cast<sockaddr *>(&Practice::peer), sizeof(Practice::peer));
		}
	}
}

int __fastcall CTitle_OnProcess(SokuLib::Title *This)
{
	Practice::deactivate();

	Practice::settings.nonSaved.enabled = false;
	// super
	return (This->*s_origCTitle_Process)();
}

int __fastcall CSelect_OnProcess(SokuLib::Select *This)
{
	Practice::deactivate();

	// super
	return (This->*s_origCSelect_Process)();
}

int __fastcall CSelectCL_OnProcess(SokuLib::SelectClient *This)
{
	Practice::deactivate();
#ifdef APMNET
	networkSelectCommon();
#endif

	// super
	return (This->*s_origCSelectCL_Process)();
}

int __fastcall CSelectSV_OnProcess(SokuLib::SelectServer *This)
{
	Practice::deactivate();
#ifdef APMNET
	networkSelectCommon();
#endif

	// super
	return (This->*s_origCSelectSV_Process)();
}

int __fastcall CLoading_OnProcess(SokuLib::Loading *This)
{
	Practice::deactivate();

	// super
	return (This->*s_origCLoading_Process)();
}

void __fastcall CBattleManager_Render(SokuLib::BattleManager *This)
{
	// super
	(This->*s_origCBattleManager_Render)();

	if (Practice::sfmlWindow)
		Practice::render();
}

int __fastcall CBattleManager_OnProcess(SokuLib::BattleManager *This)
{
	int result = 0;

	if (Practice::sfmlWindow);
	else if (SokuLib::mainMode == SokuLib::BATTLE_MODE_PRACTICE || SokuLib::subMode == SokuLib::BATTLE_SUBMODE_REPLAY || Practice::settings.nonSaved.enabled)
		Practice::activate();
	else
		return (This->*s_origCBattleManager_Process)();

	sf::Event event;

	while (Practice::sfmlWindow->pollEvent(event))
		Practice::gui.handleEvent(event);
	frameCounter += Practice::settings.requestedFrameRate / 60.f;
	while (frameCounter >= 1) {
		// super
		if (!Practice::settings.nonSaved.recordingMacro || GetForegroundWindow() == SokuLib::window)
			result = (This->*s_origCBattleManager_Process)();

		if (result > 0 && result < 4) {
			printf("CBattleManager_Process returned %i\n", result);
			frameCounter = 0;
			break;
		}

		Practice::update();
		frameCounter--;
	}
	return result;
}



// �ݒ胍�[�h
void LoadSettings(LPCSTR profilePath, LPCSTR parentPath)
{
	FILE *_;

	AllocConsole();
	freopen_s(&_, "CONOUT$", "w", stdout);
	freopen_s(&_, "CONOUT$", "w", stderr);
	puts("Hello !");
	//port = GetPrivateProfileInt("Server", "Port", 80, profilePath);
}

void hookFunctions()
{
	DWORD old;

	//Setup hooks
	VirtualProtect((PVOID)RDATA_SECTION_OFFSET, RDATA_SECTION_SIZE, PAGE_EXECUTE_WRITECOPY, &old);
	s_origCTitle_Process = SokuLib::TamperDword(&SokuLib::VTable_Title.onProcess, CTitle_OnProcess);
	s_origCSelect_Process = SokuLib::TamperDword(&SokuLib::VTable_Select.onProcess, CSelect_OnProcess);
	s_origCSelectSV_Process = SokuLib::TamperDword(&SokuLib::VTable_SelectServer.onProcess, CSelectSV_OnProcess);
	s_origCSelectCL_Process = SokuLib::TamperDword(&SokuLib::VTable_SelectClient.onProcess, CSelectCL_OnProcess);
	s_origCLoading_Process = SokuLib::TamperDword(&SokuLib::VTable_Loading.onProcess, CLoading_OnProcess);
	s_origCBattleManager_Process = SokuLib::TamperDword(&SokuLib::VTable_BattleManager.onProcess, CBattleManager_OnProcess);
	s_origCBattleManager_Render = SokuLib::TamperDword(&SokuLib::VTable_BattleManager.onRender, CBattleManager_Render);
	VirtualProtect((PVOID)RDATA_SECTION_OFFSET, RDATA_SECTION_SIZE, old, &old);

#ifdef APMNET
	VirtualProtect((PVOID)RECVFROM_JUMP_ADDR, 6, PAGE_EXECUTE_WRITECOPY, &old);
	original_recvfrom = reinterpret_cast<int (__stdcall *)(SOCKET, char *, int, int, sockaddr *, int *)>(SokuLib::TamperDword(RECVFROM_JUMP_ADDR, reinterpret_cast<DWORD>(myRecvfrom)));
	VirtualProtect((PVOID)RECVFROM_JUMP_ADDR, 6, old, &old);
#endif

	VirtualProtect((PVOID)TEXT_SECTION_OFFSET, TEXT_SECTION_SIZE, PAGE_EXECUTE_WRITECOPY, &old);
	int newOffset = reinterpret_cast<int>(Practice::loadDeckData) - PAYLOAD_NEXT_INSTR_DECK_INFOS;
	Practice::s_origLoadDeckData = SokuLib::union_cast<void (__stdcall *)(char *, void *, SokuLib::DeckInfo &, int, SokuLib::Dequeue<short> &)>(*(int *)PAYLOAD_ADDRESS_DECK_INFOS + PAYLOAD_NEXT_INSTR_DECK_INFOS);
	*(int *)PAYLOAD_ADDRESS_DECK_INFOS = newOffset;

	//We allow cards to have 0 as cost
	//This replaces
	// if (cost < 1)
	//    cost = 1;
	//by
	// if (cost < 1)
	//    cost = 0;
	*(char *)0x00469D1E = 0;
	VirtualProtect((PVOID)TEXT_SECTION_OFFSET, TEXT_SECTION_SIZE, old, &old);


	::FlushInstructionCache(GetCurrentProcess(), nullptr, 0);
}

extern "C" __declspec(dllexport) bool CheckVersion(const BYTE hash[16])
{
	return true;
}

extern "C" __declspec(dllexport) bool Initialize(HMODULE hMyModule, HMODULE hParentModule)
{
	GetModuleFileName(hMyModule, Practice::profilePath, 1024);
	PathRemoveFileSpec(Practice::profilePath);
	strcpy(Practice::profileParent, Practice::profilePath);
	PathAppend(Practice::profilePath, "AdvancedPracticeMode.ini");
	LoadSettings(Practice::profilePath, Practice::profileParent);
	hookFunctions();
	return true;
}

extern "C" int APIENTRY DllMain(HMODULE hModule, DWORD fdwReason, LPVOID lpReserved)
{
	if (fdwReason == DLL_PROCESS_DETACH && Practice::settings.nonSaved.activated)
		Practice::deactivate();
	return TRUE;
}