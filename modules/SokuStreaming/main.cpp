//
// Created by Gegel85 on 04/12/2020
//

#include <Windows.h>
#include "Exceptions.hpp"
#include "Network/Handlers.hpp"
#include "State.hpp"
#include "Utils/InputBox.hpp"
#include <Shlwapi.h>
#include <SokuLib.hpp>
#include <DeprecatedElements.hpp>
#include <algorithm>
#include <dinput.h>

int __fastcall CTitle_OnProcess(SokuLib::Title *This) {
	// super
	int ret = (This->*s_origCTitle_Process)();

	needReset = true;
	needRefresh = true;
	checkKeyInputs();
	return ret;
}

int __fastcall CBattleWatch_OnProcess(SokuLib::BattleWatch *This) {
	// super
	int ret = (This->*s_origCBattleWatch_Process)();

	updateCache(true);
	return ret;
}

int __fastcall CBattle_OnProcess(SokuLib::Battle *This) {
	// super
	int ret = (This->*s_origCBattle_Process)();

	if (SokuLib::mainMode == SokuLib::BATTLE_MODE_VSPLAYER)
		updateCache(false);
	return ret;
}

void loadCommon() {
	needRefresh = true;
	checkKeyInputs();
}

int __fastcall CLoading_OnProcess(SokuLib::Loading *This) {
	// super
	int ret = (This->*s_origCLoading_Process)();

	loadCommon();
	return ret;
}

int __fastcall CLoadingWatch_OnProcess(SokuLib::LoadingWatch *This) {
	// super
	int ret = (This->*s_origCLoadingWatch_Process)();

	loadCommon();
	return ret;
}

int __fastcall CBattleManager_KO(SokuLib::BattleManager *This) {
	// super
	int ret = (This->*s_origCBattleManager_KO)();

	onKO();
	return ret;
}

int __fastcall CBattleManager_Start(SokuLib::BattleManager *This) {
	// super
	int ret = (This->*s_origCBattleManager_Start)();

	onRoundStart();
	return ret;
}

int __fastcall CBattleManager_Render(SokuLib::BattleManager *This) {
	// super
	int ret = (This->*s_origCBattleManager_Render)();

	checkKeyInputs();
	return ret;
}

// �ݒ胍�[�h
void LoadSettings(LPCSTR profilePath, LPCSTR parentPath) {
	/*FILE *_;

	AllocConsole();
	freopen_s(&_, "CONOUT$", "w", stdout);*/
	port = GetPrivateProfileInt("Server", "Port", 80, profilePath);
	keys[KEY_DECREASE_L_SCORE] = GetPrivateProfileInt("Keys", "DecreaseLeftScore", '1', profilePath);
	keys[KEY_INCREASE_L_SCORE] = GetPrivateProfileInt("Keys", "IncreaseLeftScore", '2', profilePath);
	keys[KEY_CHANGE_L_NAME] = GetPrivateProfileInt("Keys", "ChangeLeftName", '3', profilePath);
	keys[KEY_RESET_SCORES] = GetPrivateProfileInt("Keys", "ResetScores", '5', profilePath);
	keys[KEY_RESET_STATE] = GetPrivateProfileInt("Keys", "ResetState", '6', profilePath);
	keys[KEY_CHANGE_ROUND] = GetPrivateProfileInt("Keys", "ChangeRound", '7', profilePath);
	keys[KEY_DECREASE_R_SCORE] = GetPrivateProfileInt("Keys", "DecreaseRightScore", '8', profilePath);
	keys[KEY_INCREASE_R_SCORE] = GetPrivateProfileInt("Keys", "IncreaseRightScore", '9', profilePath);
	keys[KEY_CHANGE_R_NAME] = GetPrivateProfileInt("Keys", "ChangeRightName", '0', profilePath);

	webServer = std::make_unique<WebServer>();
	webServer->addRoute("/", root);
	webServer->addRoute("/state", state);
	Socket::HttpResponse connect(const Socket::HttpRequest &requ);
	webServer->addRoute("/connect", connect);
	webServer->addStaticFolder("/static", std::string(parentPath) + "/static");
	webServer->start(port);
	webServer->onWebSocketConnect(onNewWebSocket);
}

void hookFunctions() {
	DWORD old;

	::VirtualProtect((PVOID)RDATA_SECTION_OFFSET, RDATA_SECTION_SIZE, PAGE_EXECUTE_WRITECOPY, &old);
	s_origCTitle_Process        = SokuLib::TamperDword(&SokuLib::VTable_Title.onProcess,          CTitle_OnProcess);
	s_origCBattleWatch_Process  = SokuLib::TamperDword(&SokuLib::VTable_BattleWatch.onProcess,    CBattleWatch_OnProcess);
	s_origCLoadingWatch_Process = SokuLib::TamperDword(&SokuLib::VTable_LoadingWatch.onProcess,   CLoadingWatch_OnProcess);
	s_origCBattle_Process       = SokuLib::TamperDword(&SokuLib::VTable_Battle.onProcess,    CBattle_OnProcess);
	s_origCLoading_Process      = SokuLib::TamperDword(&SokuLib::VTable_Loading.onProcess,        CLoading_OnProcess);
	s_origCBattleManager_Start  = SokuLib::TamperDword(&SokuLib::VTable_BattleManager.onSayStart, CBattleManager_Start);
	s_origCBattleManager_KO     = SokuLib::TamperDword(&SokuLib::VTable_BattleManager.onKO,       CBattleManager_KO);
	::VirtualProtect((PVOID)RDATA_SECTION_OFFSET, RDATA_SECTION_SIZE, old, &old);
	::FlushInstructionCache(GetCurrentProcess(), NULL, 0);
}

extern "C" __declspec(dllexport) bool CheckVersion(const BYTE hash[16]) {
	return true;
}

extern "C" __declspec(dllexport) bool Initialize(HMODULE hMyModule, HMODULE hParentModule) {
	try {
		char profilePath[1024 + MAX_PATH];
		char profileParent[1024 + MAX_PATH];

		GetModuleFileName(hMyModule, profilePath, 1024);
		PathRemoveFileSpec(profilePath);
		strcpy(profileParent, profilePath);
		PathAppend(profilePath, "SokuStreaming.ini");
		LoadSettings(profilePath, profileParent);

		hookFunctions();
	} catch (std::exception &e) {
		MessageBoxA(nullptr, e.what(), "Cannot init SokuStreaming", MB_OK | MB_ICONERROR);
		abort();
	} catch (...) {
		MessageBoxA(nullptr, "Wtf ?", "Huh... ok", MB_OK | MB_ICONERROR);
		abort();
	}
	return true;
}

extern "C" int APIENTRY DllMain(HMODULE hModule, DWORD fdwReason, LPVOID lpReserved) {
	if(fdwReason == DLL_PROCESS_DETACH)
		webServer.reset();
	return TRUE;
}