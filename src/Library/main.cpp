//
// Created by Gegel85 on 31/10/2020
//

#include <ctime>
#include <string>
#include <array>
#include <discord.h>
#include <shlwapi.h>
#include <fstream>
// #define SWRS_USES_HASH
#include <swrs.h>
#include "offsetsAndAddresses.hpp"
#include "logger.hpp"

static time_t timestamp;
static discord::Core *core;
static bool enabled;
static unsigned long long clientId;

enum Character {
	CHARACTER_REIMU,
	CHARACTER_MARISA,
	CHARACTER_SAKUYA,
	CHARACTER_ALICE,
	CHARACTER_PATCHOULI,
	CHARACTER_YOUMU,
	CHARACTER_REMILIA,
	CHARACTER_YUYUKO,
	CHARACTER_YUKARI,
	CHARACTER_SUIKA,
	CHARACTER_REISEN,
	CHARACTER_AYA,
	CHARACTER_KOMACHI,
	CHARACTER_IKU,
	CHARACTER_TENSHI,
	CHARACTER_SANAE,
	CHARACTER_CIRNO,
	CHARACTER_MEILING,
	CHARACTER_UTSUHO,
	CHARACTER_SUWAKO,
};

std::vector<std::string> charactersName{
	"Reimu Hakurei",
	"Marisa Kirisame",
	"Sakuya Izayoi",
	"Alice Margatroid",
	"Patchouli Knowledge",
	"Youmu Konpaku",
	"Remilia Scarlet",
	"Yuyuko Saigyouji",
	"Yukari Yakumo",
	"Suika Ibuki",
	"Reisen Undongein Inaba",
	"Aya Shameimaru",
	"Komachi Onozuka",
	"Iku Nagae",
	"Tenshi Hinanawi",
	"Sanae Kochiya",
	"Cirno",
	"Hong Meiling",
	"Utsuho Reiuji",
	"Suwako Moriya",
};

std::vector<std::string> stagesName{
	"Hakurei Shrine",
	"Forest of Magic",
	"Creek of Genbu",
	"Youkai Mountain",
	"Mysterious Sea of Cloud",
	"Bhava-Agra",
	"Hakurei Shrine",
	"Kirisame Magic Shop",
	"Scarlet Devil Mansion Clock Tower",
	"Forest of Dolls",
	"Scarlet Devil Mansion Library",
	"Netherworld",
	"Scarlet Devil Mansion Foyer",
	"Hakugyokurou Snowy Garden",
	"Bamboo Forest of the Lost",
	"Shore of Misty Lake",
	"Moriya Shrine",
	"Mouth of Geyser",
	"Catwalk of Geyser",
	"Fusion Reactor Core",
};

std::vector<std::string> charactersImg{
	"reimu",
	"marisa",
	"sakuya",
	"alice",
	"patchouli",
	"youmu",
	"remilia",
	"yuyuko",
	"yukari",
	"suika",
	"reisen",
	"aya",
	"komachi",
	"iku",
	"tenshi",
	"sanae",
	"cirno",
	"meiling",
	"utsuho",
	"suwako",
};

// 0:1 = story, 1:0 = arcade, 2:1 = vscom, 3:1 = vsplayer, 5:1=Player 6:2=Watch  8:0 = practice, 0-3:2 = Replay
std::vector<std::array<std::string, 3>> modeNames{
	{"...",                      "Playing Story mode",           "..."},
	{"Playing in Arcade mode",   "...",                          "..."},
	{"...",                      "Playing against computer",     "..."},
	{"...",                      "Playing multiplayer (Local)",  "Watching a replay"},
	{"...",                      "Playing multiplayer (Online)", "Spectating game"},
	{"...",                      "...",                          "..."},
	{"...",                      "...",                          "Watching something"},
	{"...",                      "...",                          "..."},
	{"Playing in practice mode", "...",                          "..."}
};

int __fastcall CLogo_OnProcess(void *This)
{
	discord::Activity activity{};
	auto &assets = activity.GetAssets();

	timestamp = time(nullptr);
	activity.SetState("Title screen");
	assets.SetLargeImage("cover");
	core->ActivityManager().UpdateActivity(activity, [](discord::Result result) {
		auto code = static_cast<unsigned>(result);

		if (code)
			logMessagef("Error: %u\n");
	});
	core->RunCallbacks();
	return CLogo_Process(This);
}

int __fastcall CTitle_OnProcess(void *This)
{
	logMessage("CTitle...\n");
	timestamp = time(nullptr);
	core->RunCallbacks();
	return CTitle_Process(This);
}

int __fastcall CSelect_OnProcess(void *This)
{
	logMessage("CSelect...\n");
	core->RunCallbacks();
	return CSelect_Process(This);
}

int __fastcall CBattle_OnProcess(void *This)
{
	unsigned chr = g_leftCharID;
	unsigned stage = 13; //TODO: Add real stage
	discord::Activity activity{};
	auto &assets = activity.GetAssets();
	auto timeStamp = activity.GetTimestamps();
	const char *profile1 = *reinterpret_cast<VC9STRING *>(ADDR_PLAYER1_PROFILE_STR);
	const char *profile2 = *reinterpret_cast<VC9STRING *>(ADDR_PLAYER2_PROFILE_STR);

	timeStamp.SetStart(timestamp);
	assets.SetLargeImage(("stage_" + std::to_string(stage)).c_str());
	assets.SetLargeText(stagesName[stage - 1].c_str());
	assets.SetSmallImage(charactersImg[chr].c_str());
	assets.SetSmallText(charactersName[chr].c_str());

	activity.SetDetails((modeNames[g_mainMode][g_subMode] + " (" + profile1 + ")").c_str());
	if (g_mainMode != SWRSMODE_PRACTICE)
		activity.SetState((
			std::string("Against ") + profile2 + " as " + charactersName[g_rightCharID]
		).c_str());

	core->ActivityManager().UpdateActivity(activity, [](discord::Result result) {
		auto code = static_cast<unsigned>(result);

		if (code)
			logMessagef("Error: %u\n");
	});
	core->RunCallbacks();
	return CBattle_Process(This);
}

// �ݒ胍�[�h
void LoadSettings(LPCSTR profilePath)
{
	char buffer[64];

	logMessage("Loading settings...\n");
	// �����V���b�g�_�E��
	enabled = GetPrivateProfileInt("DiscordIntegration", "Enabled", 1, profilePath) != 0;
	GetPrivateProfileString("DiscordIntegration", "ClientID", ClientID, buffer, sizeof(buffer), profilePath);
	clientId = atoll(buffer);
	logMessagef("Enabled: %s, ClientID: %llu\n", enabled ? "true" : "false", clientId);
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
	FILE *_;

	initLogger();
	AllocConsole();
	freopen_s(&_, "CONOUT$", "w", stdout);
	logMessage("Initializing...\n");

	GetModuleFileName(hMyModule, profilePath, 1024);
	PathRemoveFileSpec(profilePath);
	PathAppend(profilePath, "DiscordIntegration.ini");
	LoadSettings(profilePath);

	DWORD old;
	::VirtualProtect((PVOID)rdata_Offset, rdata_Size, PAGE_EXECUTE_WRITECOPY, &old);
	s_origCLogo_OnProcess   = TamperDword(vtbl_CLogo   + 4, (DWORD)CLogo_OnProcess);
	s_origCBattle_OnProcess = TamperDword(vtbl_CBattle + 4, (DWORD)CBattle_OnProcess);
	//s_origCTitle_OnProcess  = TamperDword(vtbl_CTitle  + 4, (DWORD)CTitle_OnProcess);
	//s_origCSelect_OnProcess = TamperDword(vtbl_CSelect + 4, (DWORD)CSelect_OnProcess);
	::VirtualProtect((PVOID)rdata_Offset, rdata_Size, old, &old);

	::FlushInstructionCache(GetCurrentProcess(), nullptr, 0);

	discord::Core::Create(clientId, DiscordCreateFlags_Default, &core);
	logMessage("Done...\n");
	return true;
}

extern "C"
int APIENTRY DllMain(HMODULE hModule, DWORD fdwReason, LPVOID lpReserved)
{
	return TRUE;
}