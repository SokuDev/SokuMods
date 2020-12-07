//
// Created by Gegel85 on 04/12/2020
//

#include <algorithm>
#include <SokuLib.hpp>
#include <Windows.h>
#include <Shlwapi.h>
#include "Exceptions.hpp"
#include "Network/Handlers.hpp."
#include "State.hpp"

int __fastcall CTitle_OnRender(Title *This) {
	// super
	int ret = (This->*s_origCTitle_Render)();

	needReset = true;
	return ret;
}

int __fastcall CBattleWatch_OnRender(BattleWatch *This) {
	// super
	int ret = (This->*s_origCBattleWatch_Render)();

	updateCache();
	return ret;
}

// �ݒ胍�[�h
void LoadSettings(LPCSTR profilePath, LPCSTR parentPath)
{
	// �����V���b�g�_�E��
	enabled = GetPrivateProfileInt("SokuStreaming", "Enabled", 1, profilePath) != 0;
	port = GetPrivateProfileInt("Server", "Enabled", 80, profilePath);
	webServer.addRoute("/", root);
	webServer.addRoute("/state", state);
	webServer.addStaticFolder("/static", std::string(parentPath) + "/static");
	webServer.start(port);
	webServer.onWebSocketConnect(onNewWebSocket);
}

extern "C"
__declspec(dllexport) bool CheckVersion(const BYTE hash[16])
{
	return true;
}

extern "C"
__declspec(dllexport) bool Initialize(HMODULE hMyModule, HMODULE hParentModule)
{
	char profilePath[1024 + MAX_PATH];
	char profileParent[1024 + MAX_PATH];

	GetModuleFileName(hMyModule, profilePath, 1024);
	PathRemoveFileSpec(profilePath);
	strcpy(profileParent, profilePath);
	PathAppend(profilePath, "SokuStreaming.ini");
	LoadSettings(profilePath, profileParent);
	DWORD old;
	::VirtualProtect((PVOID)RDATA_SECTION_OFFSET, RDATA_SECTION_SIZE, PAGE_EXECUTE_WRITECOPY, &old);
	s_origCTitle_Render = SokuLib::union_cast<int (Title::*)()>(SokuLib::TamperDword(SokuLib::vtbl_CTitle + 8, reinterpret_cast<DWORD>(CTitle_OnRender)));
	s_origCBattleWatch_Render = SokuLib::union_cast<int (BattleWatch::*)()>(SokuLib::TamperDword(SokuLib::vtbl_CBattleWatch + 0x08, reinterpret_cast<DWORD>(CBattleWatch_OnRender)));
	::VirtualProtect((PVOID)RDATA_SECTION_OFFSET, RDATA_SECTION_SIZE, old, &old);
	::FlushInstructionCache(GetCurrentProcess(), NULL, 0);

	return true;
}

extern "C"
int APIENTRY DllMain(HMODULE hModule, DWORD fdwReason, LPVOID lpReserved)
{
	return TRUE;
}