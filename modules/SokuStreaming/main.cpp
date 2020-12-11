//
// Created by Gegel85 on 04/12/2020
//

#include <algorithm>
#include <SokuLib.hpp>
#include <Windows.h>
#include <Shlwapi.h>
#include <dinput.h>
#include "Utils/InputBox.hpp"
#include "Exceptions.hpp"
#include "Network/Handlers.hpp"
#include "State.hpp"

int __fastcall CTitle_OnProcess(Title *This) {
	// super
	int ret = (This->*s_origCTitle_Process)();

	needReset = true;
	needRefresh = true;
	return ret;
}

int __fastcall CBattleWatch_OnProcess(BattleWatch *This) {
	// super
	int ret = (This->*s_origCBattleWatch_Process)();

	updateCache(true);
	return ret;
}

int __fastcall CBattle_OnProcess(Battle *This) {
	// super
	int ret = (This->*s_origCBattle_Process)();

	if (SokuLib::mainMode == SokuLib::BATTLE_MODE_VSPLAYER)
		updateCache(false);
	return ret;
}

void loadCommon()
{
	needRefresh = true;
}

int __fastcall CLoading_OnProcess(Loading *This) {
	// super
	int ret = (This->*s_origCLoading_Process)();

	loadCommon();
	return ret;
}

int __fastcall CLoadingWatch_OnProcess(LoadingWatch *This) {
	// super
	int ret = (This->*s_origCLoadingWatch_Process)();

	loadCommon();
	return ret;
}

// �ݒ胍�[�h
void LoadSettings(LPCSTR profilePath, LPCSTR parentPath)
{
	// �����V���b�g�_�E��
	enabled = GetPrivateProfileInt("SokuStreaming", "Enabled", 1, profilePath) != 0;
	if (!enabled)
		return;

	port = GetPrivateProfileInt("Server", "Port", 80, profilePath);
	keys[KEY_DECREASE_L_SCORE] = GetPrivateProfileInt("Keys", "DecreaseLeftScore",  DIK_1, profilePath);
	keys[KEY_DECREASE_R_SCORE] = GetPrivateProfileInt("Keys", "DecreaseRightScore", DIK_2, profilePath);
	keys[KEY_INCREASE_L_SCORE] = GetPrivateProfileInt("Keys", "IncreaseLeftScore",  DIK_3, profilePath);
	keys[KEY_INCREASE_R_SCORE] = GetPrivateProfileInt("Keys", "IncreaseRightScore", DIK_4, profilePath);
	keys[KEY_CHANGE_L_NAME]    = GetPrivateProfileInt("Keys", "ChangeLeftName",     DIK_5, profilePath);
	keys[KEY_CHANGE_R_NAME]    = GetPrivateProfileInt("Keys", "ChangeRightName",    DIK_6, profilePath);

	webServer = std::make_unique<WebServer>();
	webServer->addRoute("/", root);
	webServer->addRoute("/state", state);
	webServer->addStaticFolder("/static", std::string(parentPath) + "/static");
	webServer->start(port);
	webServer->onWebSocketConnect(onNewWebSocket);
}

void hookFunctions()
{
	DWORD old;

	::VirtualProtect((PVOID)RDATA_SECTION_OFFSET, RDATA_SECTION_SIZE, PAGE_EXECUTE_WRITECOPY, &old);
	s_origCTitle_Process = SokuLib::union_cast<int (Title::*)()>(
		SokuLib::TamperDword(
			SokuLib::vtbl_CTitle + SokuLib::OFFSET_ON_PROCESS,
			reinterpret_cast<DWORD>(CTitle_OnProcess)
		)
	);
	s_origCBattleWatch_Process = SokuLib::union_cast<int (BattleWatch::*)()>(
		SokuLib::TamperDword(
			SokuLib::vtbl_CBattleWatch + SokuLib::OFFSET_ON_PROCESS,
			reinterpret_cast<DWORD>(CBattleWatch_OnProcess)
		)
	);
	s_origCLoadingWatch_Process = SokuLib::union_cast<int (LoadingWatch::*)()>(
		SokuLib::TamperDword(
			SokuLib::vtbl_CLoadingWatch + SokuLib::OFFSET_ON_PROCESS,
			reinterpret_cast<DWORD>(CLoadingWatch_OnProcess)
		)
	);
	s_origCBattle_Process = SokuLib::union_cast<int (Battle::*)()>(
		SokuLib::TamperDword(
			SokuLib::vtbl_CBattle + SokuLib::OFFSET_ON_PROCESS,
			reinterpret_cast<DWORD>(CBattle_OnProcess)
		)
	);
	s_origCLoading_Process = SokuLib::union_cast<int (Loading::*)()>(
		SokuLib::TamperDword(
			SokuLib::vtbl_CLoading + SokuLib::OFFSET_ON_PROCESS,
			reinterpret_cast<DWORD>(CLoading_OnProcess)
		)
	);
	::VirtualProtect((PVOID)RDATA_SECTION_OFFSET, RDATA_SECTION_SIZE, old, &old);
	::FlushInstructionCache(GetCurrentProcess(), NULL, 0);
}

extern "C"
__declspec(dllexport) bool CheckVersion(const BYTE hash[16])
{
	return true;
}

extern "C"
__declspec(dllexport) bool Initialize(HMODULE hMyModule, HMODULE hParentModule)
{
	try {
		char profilePath[1024 + MAX_PATH];
		char profileParent[1024 + MAX_PATH];

		GetModuleFileName(hMyModule, profilePath, 1024);
		PathRemoveFileSpec(profilePath);
		strcpy(profileParent, profilePath);
		PathAppend(profilePath, "SokuStreaming.ini");
		LoadSettings(profilePath, profileParent);

		if (!enabled)
			return true;
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

extern "C"
int APIENTRY DllMain(HMODULE hModule, DWORD fdwReason, LPVOID lpReserved)
{
	return TRUE;
}