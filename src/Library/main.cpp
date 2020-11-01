//
// Created by Gegel85 on 31/10/2020
//

#include <thread>
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

bool enabled;
static time_t gameTimestamp;
static time_t totalTimestamp;
static discord::Core *core;
static unsigned long long clientId;

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
	"Random Select",
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
	"okuu",
	"suwako",
	"random_select"
};

// 0:1 = story, 1:0 = arcade, 2:1 = vscom, 3:1 = vsplayer, 5:1=Player 6:2=Watch  8:0 = practice, 0-3:2 = Replay
std::vector<std::array<std::string, 3>> modeNames{
	{"0.0",                      "Playing story mode",           "Watching a story mode replay"},
	{"Playing in Arcade mode",   "1.1",                          "1.2"},
	{"2.0",                      "Playing against computer",     "2.2"},
	{"3.0",                      "Playing multiplayer (Offline)","Watching a replay"},
	{"4.0",                      "Playing multiplayer (Online)", "4.2"},
	{"5.0",                      "Playing multiplayer (Online)", "Spectating game"},
	{"6.0",                      "6.1",                          "6.2"},
	{"7.0",                      "Playing time trial",           "Watching a time trial replay"},
	{"Playing in practice mode", "8.1",                          "8.2"}
};

std::vector<std::string> sceneNames{
	"Watching opening scene",//SWRSSCENE_LOGO         = 0,
	"Watching opening scene",//SWRSSCENE_OPENING      = 1,
	"Title screen",          //SWRSSCENE_TITLE        = 2,
	"SELECT",                //SWRSSCENE_SELECT       = 3,
	"Scene 4",               //???                    = 4,
	"BATTLE",                //SWRSSCENE_BATTLE       = 5,
	"Loading...",            //SWRSSCENE_LOADING      = 6,
	"Scene 7",               //???                    = 7,
	"SELECTSV",              //SWRSSCENE_SELECTSV     = 8,
	"SELECTCL",              //SWRSSCENE_SELECTCL     = 9,
	"Loading...",            //SWRSSCENE_LOADINGSV    = 10,
	"Loading...",            //SWRSSCENE_LOADINGCL    = 11,
	"Loading...",            //SWRSSCENE_LOADINGWATCH = 12,
	"BATTLESV",              //SWRSSCENE_BATTLESV     = 13,
	"BATTLECL",              //SWRSSCENE_BATTLECL     = 14,
	"Watching battle",       //SWRSSCENE_BATTLEWATCH  = 15,
	"Selecting scenario",    //SWRSSCENE_SELECTSENARIO= 16,
	"Scene 17",              //???                    = 17,
	"Scene 18",              //???                    = 18,
	"Scene 19",              //???                    = 19,
	"Watching credits",      //SWRSSCENE_ENDING       = 20,
};

unsigned char getStageId()
{
	unsigned char stage = *reinterpret_cast<unsigned char *>(ADDR_LOADED_STAGE_ID);

	if (stage >= 30)
		stage -= 11;
	if (stage >= 10)
		stage -= 4;
	return stage;
}

void genericScreen()
{
	discord::Activity activity{};
	auto &assets = activity.GetAssets();

	totalTimestamp = time(nullptr);
	activity.SetState(sceneNames[g_sceneId].c_str());
	assets.SetLargeImage("cover");
	core->ActivityManager().UpdateActivity(activity, [](discord::Result result) {
		auto code = static_cast<unsigned>(result);

		if (code)
			logMessagef("Error: %u\n");
	});
}

void localBattle()
{
	unsigned stage = getStageId();
	discord::Activity activity{};
	auto &assets = activity.GetAssets();
	auto &timeStamp = activity.GetTimestamps();
	const char *profile1 = *reinterpret_cast<VC9STRING *>(ADDR_PLAYER1_PROFILE_STR);
	const char *profile2 = *reinterpret_cast<VC9STRING *>(ADDR_PLAYER2_PROFILE_STR);

	timeStamp.SetStart(gameTimestamp);

	if (g_subMode == SWRSSUBMODE_REPLAY) {
		assets.SetLargeImage(("stage_" + std::to_string(stage + 1)).c_str());
		assets.SetLargeText(stagesName[stage].c_str());
		activity.SetDetails(modeNames[g_mainMode][g_subMode].c_str());
		activity.SetState(std::string(charactersName[g_leftCharID] + " vs " + charactersName[g_rightCharID]).c_str());
	} else {
		assets.SetLargeImage(charactersImg[g_leftCharID].c_str());
		assets.SetLargeText(charactersName[g_leftCharID].c_str());
		assets.SetSmallImage(("stage_" + std::to_string(stage + 1)).c_str());
		assets.SetSmallText(stagesName[stage].c_str());
		activity.SetDetails((modeNames[g_mainMode][g_subMode] + " (" + profile1 + ")").c_str());
		if (g_mainMode == SWRSMODE_VSPLAYER)
			activity.SetState((std::string("Against ") + profile2 + " as " + charactersName[g_rightCharID]).c_str());
		else
			activity.SetState(("Against " + charactersName[g_rightCharID]).c_str());
	}

	core->ActivityManager().UpdateActivity(activity, [](discord::Result result) {
		auto code = static_cast<unsigned>(result);

		if (code)
			logMessagef("Error: %u\n");
	});
}

void loadMatch()
{
	unsigned stage = getStageId();
	discord::Activity activity{};
	auto &assets = activity.GetAssets();
	auto &timeStamp = activity.GetTimestamps();
	const char *profile1 = *reinterpret_cast<VC9STRING *>(ADDR_PLAYER1_PROFILE_STR);

	gameTimestamp = time(nullptr);
	timeStamp.SetStart(totalTimestamp);
	assets.SetLargeImage(charactersImg[g_leftCharID].c_str());
	assets.SetLargeText(charactersName[g_leftCharID].c_str());

	if (g_subMode == SWRSSUBMODE_REPLAY) {
		assets.SetSmallImage(charactersImg[g_rightCharID].c_str());
		assets.SetSmallText(charactersName[g_rightCharID].c_str());
		activity.SetDetails(modeNames[g_mainMode][g_subMode].c_str());
	} else {
		assets.SetSmallImage(("stage_" + std::to_string(stage + 1)).c_str());
		assets.SetSmallText(stagesName[stage].c_str());
		activity.SetDetails((modeNames[g_mainMode][g_subMode] + " (" + profile1 + ")").c_str());
	}
	activity.SetState("Loading...");

	core->ActivityManager().UpdateActivity(activity, [](discord::Result result) {
		auto code = static_cast<unsigned>(result);

		if (code)
			logMessagef("Error: %u\n");
	});
}

void charSelect()
{
	discord::Activity activity{};
	auto &assets = activity.GetAssets();
	auto &timeStamp = activity.GetTimestamps();
	const char *profile1 = *reinterpret_cast<VC9STRING *>(ADDR_PLAYER1_PROFILE_STR);

	timeStamp.SetStart(totalTimestamp);
	assets.SetSmallImage(charactersImg[g_rightCharID].c_str());
	assets.SetSmallText(charactersName[g_rightCharID].c_str());
	assets.SetLargeImage(charactersImg[g_leftCharID].c_str());
	assets.SetLargeText(charactersName[g_leftCharID].c_str());

	activity.SetDetails((modeNames[g_mainMode][g_subMode] + " (" + profile1 + ")").c_str());
	activity.SetState("Character select...");
	core->ActivityManager().UpdateActivity(activity, [](discord::Result result) {
		auto code = static_cast<unsigned>(result);

		if (code)
			logMessagef("Error: %u\n");
	});
}

void onlineBattle()
{
	unsigned stage = getStageId();
	discord::Activity activity{};
	auto &assets = activity.GetAssets();
	auto &timeStamp = activity.GetTimestamps();
	const char *profile1 = *reinterpret_cast<VC9STRING *>(ADDR_PLAYER1_PROFILE_STR);
	char myChar;
	char opChar;
	char *opName;
	OnlineInfo *infos = *reinterpret_cast<OnlineInfo **>(ADDR_ONLINE_INFOS_PTR);

	if (g_mainMode == SWRSMODE_VSCLIENT) {
		opName = infos->profile2name;
		myChar = g_leftCharID;
		opChar = g_rightCharID;
	} else {
		opName = infos->profile1name;
		myChar = g_rightCharID;
		opChar = g_leftCharID;
	}

	timeStamp.SetStart(gameTimestamp);
	assets.SetSmallImage(("stage_" + std::to_string(stage + 1)).c_str());
	assets.SetSmallText(stagesName[stage].c_str());
	assets.SetLargeImage(charactersImg[myChar].c_str());
	assets.SetLargeText(charactersName[myChar].c_str());

	activity.SetDetails((modeNames[g_mainMode][g_subMode] + " (" + profile1 + ")").c_str());
	activity.SetState((std::string("Against ") + opName + " as " + charactersName[opChar]).c_str());

	core->ActivityManager().UpdateActivity(activity, [](discord::Result result) {
		auto code = static_cast<unsigned>(result);

		if (code)
			logMessagef("Error: %u\n");
	});
}

void loadOnlineMatch()
{
	unsigned stage = getStageId();
	discord::Activity activity{};
	auto &assets = activity.GetAssets();
	auto &timeStamp = activity.GetTimestamps();
	const char *profile1 = *reinterpret_cast<VC9STRING *>(ADDR_PLAYER1_PROFILE_STR);
	char myChar;

	if (g_mainMode == SWRSMODE_VSCLIENT)
		myChar = g_leftCharID;
	else
		myChar = g_rightCharID;

	gameTimestamp = time(nullptr);
	timeStamp.SetStart(totalTimestamp);
	assets.SetSmallImage(("stage_" + std::to_string(stage + 1)).c_str());
	assets.SetSmallText(stagesName[stage].c_str());
	assets.SetLargeImage(charactersImg[myChar].c_str());
	assets.SetLargeText(charactersName[myChar].c_str());

	activity.SetDetails((modeNames[g_mainMode][g_subMode] + " (" + profile1 + ")").c_str());
	activity.SetState("Loading...");

	core->ActivityManager().UpdateActivity(activity, [](discord::Result result) {
		auto code = static_cast<unsigned>(result);

		if (code)
			logMessagef("Error: %u\n");
	});
}

void onlineCharSelect()
{
	discord::Activity activity{};
	auto &assets = activity.GetAssets();
	auto &timeStamp = activity.GetTimestamps();
	const char *profile1 = *reinterpret_cast<VC9STRING *>(ADDR_PLAYER1_PROFILE_STR);
	char myChar;
	char opChar;
	char *opName;
	OnlineInfo *infos = *reinterpret_cast<OnlineInfo **>(ADDR_ONLINE_INFOS_PTR);

	if (g_mainMode == SWRSMODE_VSCLIENT) {
		opName = infos->profile2name;
		myChar = g_leftCharID;
		opChar = g_rightCharID;
	} else {
		opName = infos->profile1name;
		myChar = g_rightCharID;
		opChar = g_leftCharID;
	}

	timeStamp.SetStart(totalTimestamp);
	assets.SetSmallImage(charactersImg[opChar].c_str());
	assets.SetSmallText(charactersName[opChar].c_str());
	assets.SetLargeImage(charactersImg[myChar].c_str());
	assets.SetLargeText(charactersName[myChar].c_str());

	activity.SetDetails((modeNames[g_mainMode][g_subMode] + " (" + profile1 + ")").c_str());
	activity.SetState(("Character select... (vs " + std::string(opName) + ")").c_str());

	core->ActivityManager().UpdateActivity(activity, [](discord::Result result) {
		auto code = static_cast<unsigned>(result);

		if (code)
			logMessagef("Error: %u\n");
	});
}

std::vector<std::function<void()>> sceneCallbacks{
	genericScreen,    //SWRSSCENE_LOGO         = 0,
	genericScreen,    //SWRSSCENE_OPENING      = 1,
	genericScreen,    //SWRSSCENE_TITLE        = 2,
	charSelect,       //SWRSSCENE_SELECT       = 3,
	genericScreen,    //???                    = 4,
	localBattle,      //SWRSSCENE_BATTLE       = 5,
	loadMatch,        //SWRSSCENE_LOADING      = 6,
	genericScreen,    //???                    = 7,
	onlineCharSelect, //SWRSSCENE_SELECTSV     = 8,
	onlineCharSelect, //SWRSSCENE_SELECTCL     = 9,
	loadOnlineMatch,  //SWRSSCENE_LOADINGSV    = 10,
	loadOnlineMatch,  //SWRSSCENE_LOADINGCL    = 11,
	genericScreen,    //SWRSSCENE_LOADINGWATCH = 12,
	onlineBattle,     //SWRSSCENE_BATTLESV     = 13,
	onlineBattle,     //SWRSSCENE_BATTLECL     = 14,
	genericScreen,    //SWRSSCENE_BATTLEWATCH  = 15,
	genericScreen,    //SWRSSCENE_SELECTSENARIO= 16,
	genericScreen,    //???                    = 17,
	genericScreen,    //???                    = 18,
	genericScreen,    //???                    = 19,
	genericScreen,    //SWRSSCENE_ENDING       = 20,
};

class MyThread : public std::thread {
private:
	bool _done;

public:
	bool isDone() const { return this->_done; }
	template<typename ...Args>
	MyThread() : std::thread() {};
	~MyThread() {
		this->_done = true;
		if (this->joinable())
			this->join();
	}

	void start() {
		std::thread::operator=(std::thread([this] {
			logMessage("Connecting to discord client...\n");
			discord::Core::Create(clientId, DiscordCreateFlags_Default, &core);
			logMessage("Connected !\n");
			while (!this->isDone()) {
				if (g_sceneId >= 0 && g_sceneId < sceneCallbacks.size())
					sceneCallbacks[g_sceneId]();
				core->RunCallbacks();
				std::this_thread::sleep_for(std::chrono::milliseconds(100));
			}
		}));
	}
};
static MyThread updateThread;

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
	bool &_en = enabled;
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

	//DWORD old;
	//::VirtualProtect((PVOID)rdata_Offset, rdata_Size, PAGE_EXECUTE_WRITECOPY, &old);
	//s_origCLogo_OnProcess   = TamperDword(vtbl_CLogo   + 4, (DWORD)CLogo_OnProcess);
	//s_origCBattle_OnProcess = TamperDword(vtbl_CBattle + 4, (DWORD)CBattle_OnProcess);
	//s_origCTitle_OnProcess  = TamperDword(vtbl_CTitle  + 4, (DWORD)CTitle_OnProcess);
	//s_origCSelect_OnProcess = TamperDword(vtbl_CSelect + 4, (DWORD)CSelect_OnProcess);
	//::VirtualProtect((PVOID)rdata_Offset, rdata_Size, old, &old);

	//::FlushInstructionCache(GetCurrentProcess(), nullptr, 0);

	if (enabled)
		updateThread.start();
	else
		logMessage("Disabled ;(\n");
	logMessage("Done...\n");
	return true;
}

extern "C"
int APIENTRY DllMain(HMODULE hModule, DWORD fdwReason, LPVOID lpReserved)
{
	return TRUE;
}