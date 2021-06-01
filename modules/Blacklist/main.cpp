//
// Created by PinkySmile on 01/06/2021.
//

#include <SokuLib.hpp>
#include <Windows.h>
#include <Shlwapi.h>
#include <nlohmann/json.hpp>
#include <fstream>

#ifndef _DEBUG
#define puts(...)
#define printf(...)
#endif
#define checkKey(key) (GetKeyState(key) & 0x8000)

static int (SokuLib::Title::*og_titleOnProcess)();
static int (SokuLib::SelectClient::*og_selectCLOnProcess)();
static int (SokuLib::SelectServer::*og_selectSVOnProcess)();
static std::vector<unsigned> blacklistedIps;
static std::vector<std::string> blacklistedIpsStr;
static std::vector<std::string> blacklistedProfiles;
static char listsPath[1024 + MAX_PATH];
static std::string lastSeenProfile;
static in_addr lastSeenIP;
static unsigned char reloadKey;
static unsigned char blacklistIpKey;
static unsigned char blacklistProfileKey;
static unsigned char blacklistBothKey;
static int (__stdcall *og_recvfrom)(SOCKET s, char *buf, int len, int flags, sockaddr *from, int *fromlen);

int __stdcall my_recvfrom(SOCKET s, char *buf, int len, int flags, sockaddr *from, int *fromlen)
{
	while (true) {
		int bytes = og_recvfrom(s, buf, len, flags, from, fromlen);

		if (bytes <= 0)
			return bytes;

		auto &addr = reinterpret_cast<sockaddr_in *>(from)->sin_addr;
		auto *packet = reinterpret_cast<SokuLib::Packet *>(buf);

		if (lastSeenIP.s_addr != addr.s_addr && packet->type == SokuLib::HELLO) {
			if (std::find(blacklistedIps.begin(), blacklistedIps.end(), addr.s_addr) != blacklistedIps.end())
				// I don't think so
				continue;
			lastSeenIP = from ? reinterpret_cast<sockaddr_in *>(from)->sin_addr : in_addr{0};
		}

		if (packet->type == SokuLib::INIT_REQUEST) {
			if (packet->initRequest.reqType != SokuLib::PLAY_REQU)
				return bytes;

			std::string name{packet->initRequest.name, packet->initRequest.name + packet->initRequest.nameLength};

			if (std::find(blacklistedProfiles.begin(), blacklistedProfiles.end(), name) != blacklistedProfiles.end())
				continue;
			lastSeenProfile = name;
		}
		return bytes;
	}
}

void saveLists()
{
	std::ofstream stream{listsPath};
	
	if (stream.fail()) {
		MessageBox(nullptr, ("Cannot open " + std::string(listsPath) + " for writing.").c_str(), "Saving failed.", MB_ICONERROR);
		return;
	}

	stream << nlohmann::json{
		{"ips", blacklistedIpsStr},
		{"profiles", blacklistedProfiles},
	}.dump(4);
}

static void loadLists()
{
	std::ifstream stream{listsPath};

	if (stream.fail()) {
		saveLists();
		return;
	}

	nlohmann::json result;

	stream >> result;
	blacklistedIpsStr   = result["ips"].get<std::vector<std::string>>();
	blacklistedProfiles = result["profiles"].get<std::vector<std::string>>();
	blacklistedIps.clear();
	blacklistedIps.reserve(blacklistedIpsStr.size());
	for (auto &str : blacklistedIpsStr) {
		unsigned addr = inet_addr(str.c_str());

		if (addr == -1)
			continue;
		blacklistedIps.push_back(addr);
	}
}

static int processKeys(int og)
{
	if (GetForegroundWindow() != SokuLib::window)
		return og;

	if (checkKey(reloadKey)) {
		loadLists();
		MessageBox(nullptr, "Successfully reloaded blacklist from file.", "Reload complete !", MB_ICONINFORMATION);
		lastSeenIP = {0};
		lastSeenProfile.clear();
		return og;
	}

	if (checkKey(blacklistIpKey) && lastSeenIP.s_addr) {
		auto str = inet_ntoa(lastSeenIP);

		if (std::find(blacklistedIps.begin(), blacklistedIps.end(), lastSeenIP.s_addr) != blacklistedIps.end()) {
			MessageBox(nullptr, ("You already blacklisted " + std::string(str) + ".").c_str(), "Already blacklisted.", MB_ICONERROR);
			return og;
		}
		if (MessageBox(nullptr, ("Do you want to blacklist " + std::string(str) + " ?").c_str(), "Blacklisted ?", MB_ICONINFORMATION | MB_YESNO) == IDNO)
			return og;
		blacklistedIpsStr.emplace_back(str);
		blacklistedIps.push_back(lastSeenIP.s_addr);
		MessageBox(nullptr, ("Successfully blacklisted " + std::string(str) + ".").c_str(), "Blacklisted.", MB_ICONINFORMATION);
		saveLists();
		lastSeenIP = {0};
		lastSeenProfile.clear();
		return SokuLib::SCENE_TITLE;
	}
	if (checkKey(blacklistProfileKey) && !lastSeenProfile.empty()) {
		if (std::find(blacklistedProfiles.begin(), blacklistedProfiles.end(), lastSeenProfile) != blacklistedProfiles.end()) {
			MessageBox(nullptr, ("You already blacklisted profile name \"" + lastSeenProfile + "\".").c_str(), "Already blacklisted.", MB_ICONERROR);
			return og;
		}
		if (MessageBox(nullptr, ("Do you want to blacklist profile name \"" + lastSeenProfile + "\" ?").c_str(), "Blacklisted ?", MB_ICONINFORMATION | MB_YESNO) == IDNO)
			return og;
		blacklistedProfiles.push_back(lastSeenProfile);
		MessageBox(nullptr, ("Successfully blacklisted profile name \"" + lastSeenProfile + "\".").c_str(), "Blacklisted.", MB_ICONINFORMATION);
		saveLists();
		lastSeenIP = {0};
		lastSeenProfile.clear();
		return SokuLib::SCENE_TITLE;
	}
	if (checkKey(blacklistBothKey) && (lastSeenIP.s_addr || !lastSeenProfile.empty())) {
		auto str = inet_ntoa(lastSeenIP);

		if (std::find(blacklistedIps.begin(), blacklistedIps.end(), lastSeenIP.s_addr) != blacklistedIps.end() && std::find(blacklistedProfiles.begin(), blacklistedProfiles.end(), lastSeenProfile) != blacklistedProfiles.end()) {
			MessageBox(nullptr, ("You already blacklisted " + std::string(str) + " and profile name \"" + lastSeenProfile + "\".").c_str(), "Already blacklisted.", MB_ICONERROR);
			return og;
		}
		if (MessageBox(nullptr, ("Do you want to blacklist " + std::string(str) + " and profile name \"" + lastSeenProfile + "\" ?").c_str(), "Blacklisted ?", MB_ICONINFORMATION | MB_YESNO) == IDNO)
			return og;
		if (std::find(blacklistedIps.begin(), blacklistedIps.end(), lastSeenIP.s_addr) == blacklistedIps.end()) {
			blacklistedIpsStr.emplace_back(str);
			blacklistedIps.push_back(lastSeenIP.s_addr);
		}
		if (std::find(blacklistedProfiles.begin(), blacklistedProfiles.end(), lastSeenProfile) == blacklistedProfiles.end())
			blacklistedProfiles.push_back(lastSeenProfile);
		MessageBox(nullptr, ("Successfully blacklisted " + std::string(str) + " and profile name \"" + lastSeenProfile + "\".").c_str(), "Blacklisted.", MB_ICONINFORMATION);
		saveLists();
		lastSeenIP = {0};
		lastSeenProfile.clear();
		return SokuLib::SCENE_TITLE;
	}
	return og;
}

int __fastcall Title_OnProcess(SokuLib::Title *This)
{
	return processKeys((This->*og_titleOnProcess)());
}

int __fastcall SelectClient_OnProcess(SokuLib::SelectClient *This)
{
	return processKeys((This->*og_selectCLOnProcess)());
}

int __fastcall SelectServer_OnProcess(SokuLib::SelectServer *This)
{
	return processKeys((This->*og_selectSVOnProcess)());
}

// �ݒ胍�[�h
void LoadSettings(LPCSTR profilePath) {
#ifdef _DEBUG
	FILE *_;

	AllocConsole();
	freopen_s(&_, "CONOUT$", "w", stdout);
#endif
	reloadKey           = GetPrivateProfileInt("Keys", "Reload",               0x52, profilePath);
	blacklistIpKey      = GetPrivateProfileInt("Keys", "BlacklistIp",          0x54, profilePath);
	blacklistProfileKey = GetPrivateProfileInt("Keys", "BlacklistProfile",     0x59, profilePath);
	blacklistBothKey    = GetPrivateProfileInt("Keys", "BlacklistProfileAndIp",0x55, profilePath);
	loadLists();
}

extern "C" __declspec(dllexport) bool CheckVersion(const BYTE hash[16]) {
	return memcmp(hash, SokuLib::targetHash, sizeof(SokuLib::targetHash)) == 0;
}

extern "C" __declspec(dllexport) bool Initialize(HMODULE hMyModule, HMODULE hParentModule) {
	char profilePath[1024 + MAX_PATH];
	DWORD old;

	GetModuleFileName(hMyModule, profilePath, 1024);
	PathRemoveFileSpec(profilePath);
	strcpy(listsPath, profilePath);
	PathAppend(profilePath, "Blacklist.ini");
	PathAppend(listsPath, "Blacklist.json");
	LoadSettings(profilePath);

	::VirtualProtect((PVOID)RDATA_SECTION_OFFSET, RDATA_SECTION_SIZE, PAGE_EXECUTE_WRITECOPY, &old);
	og_recvfrom          = SokuLib::TamperDword(&SokuLib::DLL::ws2_32.recvfrom,          my_recvfrom);
	og_titleOnProcess    = SokuLib::TamperDword(&SokuLib::VTable_Title.onProcess,        Title_OnProcess);
	og_selectCLOnProcess = SokuLib::TamperDword(&SokuLib::VTable_SelectClient.onProcess, SelectClient_OnProcess);
	og_selectSVOnProcess = SokuLib::TamperDword(&SokuLib::VTable_SelectServer.onProcess, SelectServer_OnProcess);
	::VirtualProtect((PVOID)RDATA_SECTION_OFFSET, RDATA_SECTION_SIZE, old, &old);
	::FlushInstructionCache(GetCurrentProcess(), nullptr, 0);
	return true;
}

extern "C" int APIENTRY DllMain(HMODULE hModule, DWORD fdwReason, LPVOID lpReserved) {
	return TRUE;
}