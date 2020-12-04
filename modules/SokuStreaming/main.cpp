//
// Created by Gegel85 on 04/12/2020
//

#include <SokuLib.hpp>
#include <Windows.h>
#include <Shlwapi.h>
#include "Network/WebServer.hpp"

static bool enabled;
static unsigned short port;
static WebServer webServer;
struct CachedMatchData {
	SokuLib::Character left;
	SokuLib::Character right;
	std::vector<>
	std::string leftName;
	std::string rightName;
	unsigned int leftScore;
	unsigned int rightScore;
} cache;
struct SelectSV{};
struct SelectCL{};
static __thiscall int (SelectSV::*s_origCSelectSV_Render)();
static __thiscall int (SelectCL::*s_origCSelectCL_Render)();

Socket::HttpResponse root(const Socket::HttpRequest &requ)
{
	Socket::HttpResponse response;

	response.header["Location"] = "http://" + requ.host + "/static/html/overlay.html";
	response.returnCode = 301;
	return response;
}

void updateCache(int ret)
{
	auto battleMgr = SokuLib::getBattleMgr();

	if (ret != SokuLib::SCENE_BATTLECL && ret != SokuLib::SCENE_BATTLESV) {
		cache.leftScore += battleMgr.leftCharacterManager->score == 2;
		cache.rightScore += battleMgr.rightCharacterManager->score == 2;
	}
}

int __fastcall CSelectSV_OnRender(SelectSV *This) {
	// super
	int ret = (This->*s_origCSelectSV_Render)();

	updateCache(ret);
	return ret;
}

int __fastcall CSelectCL_OnRender(SelectCL *This) {
	// super
	int ret = (This->*s_origCSelectCL_Render)();

	updateCache(ret);
	return ret;
}

// �ݒ胍�[�h
void LoadSettings(LPCSTR profilePath, LPCSTR parentPath)
{
	// �����V���b�g�_�E��
	enabled = GetPrivateProfileInt("SokuStreaming", "Enabled", 1, profilePath) != 0;
	port = GetPrivateProfileInt("Server", "Enabled", 80, profilePath);
	webServer.addRoute("/", root);
	webServer.addStaticFolder("/static", std::string(parentPath) + "/static");
	webServer.start(port);
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
	s_origCSelectSV_Render = SokuLib::union_cast<int (SelectSV::*)()>(SokuLib::TamperDword(SokuLib::vtbl_CSelectSV + 0x08, reinterpret_cast<DWORD>(CSelectSV_OnRender)));
	s_origCSelectCL_Render = SokuLib::union_cast<int (SelectCL::*)()>(SokuLib::TamperDword(SokuLib::vtbl_CSelectCL + 0x08, reinterpret_cast<DWORD>(CSelectCL_OnRender)));
	::VirtualProtect((PVOID)RDATA_SECTION_OFFSET, RDATA_SECTION_SIZE, old, &old);

	::FlushInstructionCache(GetCurrentProcess(), NULL, 0);

	return true;
}

extern "C"
int APIENTRY DllMain(HMODULE hModule, DWORD fdwReason, LPVOID lpReserved)
{
	return TRUE;
}