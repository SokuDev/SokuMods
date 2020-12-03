//
// Created by Gegel85 on 31/10/2020
//

#include <thread>
#include <ctime>
#include <string>
#include <array>
#include <discord.h>
#include <shlwapi.h>
#include <SokuLib.hpp>
#include "logger.hpp"
#include "Exceptions.hpp"
#include "Network/getPublicIp.hpp"
#include "ShiftJISDecoder.hpp"

static bool enabled;
static char smallImg[32];
static bool showWR = false;
static bool experimentalSpec;
static std::pair<unsigned, unsigned> won;
static std::pair<unsigned, unsigned> score;
static time_t gameTimestamp;
static time_t hostTimestamp;
static time_t totalTimestamp;
static time_t refreshRate;
static discord::Core *core;
static unsigned long long clientId;
static int currentScene;
static std::string roomIp = "";

const std::vector<const char *> discordResultToString{
	"Ok",
	"ServiceUnavailable",
	"InvalidVersion",
	"LockFailed",
	"InternalError",
	"InvalidPayload",
	"InvalidCommand",
	"InvalidPermissions",
	"NotFetched",
	"NotFound",
	"Conflict",
	"InvalidSecret",
	"InvalidJoinSecret",
	"NoEligibleActivity",
	"InvalidInvite",
	"NotAuthenticated",
	"InvalidAccessToken",
	"ApplicationMismatch",
	"InvalidDataUrl",
	"InvalidBase64",
	"NotFiltered",
	"LobbyFull",
	"InvalidLobbySecret",
	"InvalidFilename",
	"InvalidFileSize",
	"InvalidEntitlement",
	"NotInstalled",
	"NotRunning",
	"InsufficientBuffer",
	"PurchaseCanceled",
	"InvalidGuild",
	"InvalidEvent",
	"InvalidChannel",
	"InvalidOrigin",
	"RateLimited",
	"OAuth2Error",
	"SelectChannelTimeout",
	"GetGuildTimeout",
	"SelectVoiceForceRequired",
	"CaptureShortcutAlreadyListening",
	"UnauthorizedForAchievement",
	"InvalidGiftCode",
	"PurchaseError",
	"TransactionAborted",
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

void genericScreen()
{
	logMessagef("Generic menu on scene %i\n", currentScene);
	discord::Activity activity{};
	auto &assets = activity.GetAssets();

	if (!roomIp.empty())
		logMessage("No longer hosting/connecting.\n");
	roomIp = "";
	score = {0, 0};
	totalTimestamp = time(nullptr);
	hostTimestamp = time(nullptr);
	logMessage("Get scene name\n");
	activity.SetState(SokuLib::sceneNames[currentScene].c_str());
	logMessage("Done\n");
	assets.SetLargeImage("cover");
	core->ActivityManager().UpdateActivity(activity, [](discord::Result result) {
		auto code = static_cast<unsigned>(result);

		if (code)
			logMessagef("Error updating presence: %s\n", discordResultToString[code]);
	});
	logMessage("Callback end\n");
}

void showHost()
{
	logMessagef("Showing host... Internal ip is %s\n", roomIp.c_str());
	discord::Activity activity{};
	auto &assets = activity.GetAssets();
	auto *menuObj = SokuLib::getMenuObj<SokuLib::MenuConnect>();
	auto &timeStamp = activity.GetTimestamps();
	auto &party = activity.GetParty();
	auto &secrets = activity.GetSecrets();

	if (roomIp.empty()) {
		try {
			roomIp = getMyIp() + std::string(":") + std::to_string(menuObj->port);
			logMessagef("Hosting. Room ip is %s. Spectator are %sallowed\n", roomIp.c_str(), menuObj->spectate ? "" : "not ");
			party.SetId(roomIp.c_str());
			secrets.SetJoin(("join" + roomIp).c_str());
			if (menuObj->spectate)
				secrets.SetSpectate(("spec" + roomIp).c_str());
		} catch (...) {}
	} else {
		party.SetId(roomIp.c_str());
		secrets.SetJoin(("join" + roomIp).c_str());
		if (menuObj->spectate)
			secrets.SetSpectate(("spec" + roomIp).c_str());
	}
	party.GetSize().SetCurrentSize(1);
	party.GetSize().SetMaxSize(2);
	timeStamp.SetStart(hostTimestamp);
	activity.SetDetails(SokuLib::sceneNames[currentScene].c_str());
	activity.SetState("Hosting...");
	assets.SetLargeImage("cover");
	assets.SetSmallImage(smallImg);
	core->ActivityManager().UpdateActivity(activity, [](discord::Result result) {
		auto code = static_cast<unsigned>(result);

		if (code)
			logMessagef("Error updating presence: %s\n", discordResultToString[code]);
	});
	logMessage("Callback end\n");
}

void connectingToRemote()
{
	logMessage("Connecting to remote\n");
	auto *menuObj = SokuLib::getMenuObj<SokuLib::MenuConnect>();
	logMessagef("Menu object is at %#X\n", menuObj);
	discord::Activity activity{};
	auto &assets = activity.GetAssets();
	auto &party = activity.GetParty();
	auto &secrets = activity.GetSecrets();

	if (roomIp.empty())
		roomIp = menuObj->IPString + (":" + std::to_string(menuObj->port));
	logMessagef("The new room ip is %s\n", roomIp.c_str());
	totalTimestamp = time(nullptr);

	assets.SetLargeImage("cover");

	activity.SetDetails("Joining room...");
	activity.SetState("Playing multiplayer (Online)");
	party.SetId(roomIp.c_str());
	party.GetSize().SetCurrentSize(2);
	party.GetSize().SetMaxSize(2);
	secrets.SetJoin(("join" + roomIp).c_str());
	secrets.SetSpectate(("spec" + roomIp).c_str());
	core->ActivityManager().UpdateActivity(activity, [](discord::Result result) {
		auto code = static_cast<unsigned>(result);

		if (code)
			logMessagef("Error updating presence: %s\n", discordResultToString[code]);
	});
	logMessage("Callback end\n");
}

void connectedToRemoteLoadingCharSelect()
{
	logMessagef("Connected and waiting to load. Internal ip is %s\n", roomIp.c_str());
	discord::Activity activity{};
	auto &assets = activity.GetAssets();
	auto &party = activity.GetParty();
	auto &secrets = activity.GetSecrets();

	logMessagef("Room ip is %s\n", roomIp.c_str());
	totalTimestamp = time(nullptr);

	assets.SetLargeImage("cover");

	activity.SetState("Loading character select...");
	activity.SetDetails("Playing multiplayer (Online)");
	party.SetId(roomIp.c_str());
	party.GetSize().SetCurrentSize(2);
	party.GetSize().SetMaxSize(2);
	secrets.SetJoin(("join" + roomIp).c_str());
	secrets.SetSpectate(("spec" + roomIp).c_str());
	core->ActivityManager().UpdateActivity(activity, [](discord::Result result) {
		auto code = static_cast<unsigned>(result);

		if (code)
			logMessagef("Error updating presence: %s\n", discordResultToString[code]);
	});
	logMessage("Callback end\n");
}

#define UNKNOWN_GLOBAL 0x0089a888

#define PART1 (*(void****)UNKNOWN_GLOBAL)
#define PART2 (*(PART1+1))
#define PART3 (*(PART2+2))
#define CMENU_OBJ ( \
	*(                 \
		*( \
			( \
				*(void****)UNKNOWN_GLOBAL \
			)+1\
		)+2               \
	)                   \
)
#define IN_MENU *(char*)(0x089a888 + 4)

void titleScreen()
{
	logMessage("On title screen\n");
	auto *menuObj = SokuLib::getMenuObj<SokuLib::MenuConnect>();

	if (!SokuLib::isInNetworkMenu()) {
		logMessage("We are not in a proper submenu, falling back to generic screen\n");
		return genericScreen();
	}

	if (
		menuObj->choice >= SokuLib::MenuConnect::CHOICE_ASSIGN_IP_CONNECT &&
		menuObj->choice < SokuLib::MenuConnect::CHOICE_SELECT_PROFILE &&
		menuObj->subchoice == 3
	)
		connectingToRemote();
	else if (
		menuObj->choice >= SokuLib::MenuConnect::CHOICE_HOST &&
		menuObj->choice < SokuLib::MenuConnect::CHOICE_SELECT_PROFILE &&
		menuObj->subchoice == 255
	)
		connectedToRemoteLoadingCharSelect();
	else if (
		menuObj->choice == SokuLib::MenuConnect::CHOICE_HOST &&
		menuObj->subchoice == 2
	)
		showHost();
	else
		genericScreen();
	logMessage("Title screen callback end\n");
}

void localBattle()
{
	logMessage("Playing a local game\n");
	unsigned stage = SokuLib::flattenStageId(SokuLib::stageId);
	logMessagef("We are on stage %u\n", stage);
	discord::Activity activity{};
	auto &assets = activity.GetAssets();
	auto &timeStamp = activity.GetTimestamps();
	std::string profile1 = convertShiftJisToUTF8(SokuLib::player1Profile);
	std::string profile2 = convertShiftJisToUTF8(SokuLib::player2Profile);

	logMessagef("The 2 profiles are %s %s\n", profile1.c_str(), profile2.c_str());
	timeStamp.SetStart(gameTimestamp);

	if (SokuLib::subMode == SokuLib::BATTLE_SUBMODE_REPLAY) {
		logMessage("This is a replay\n");
		assets.SetLargeImage(("stage_" + std::to_string(stage + 1)).c_str());
		assets.SetLargeText(SokuLib::stagesName[stage].c_str());
		activity.SetDetails(SokuLib::modeNames[SokuLib::mainMode][SokuLib::subMode].c_str());
		activity.SetState(std::string(SokuLib::charactersName[SokuLib::leftChar] + " vs " + SokuLib::charactersName[SokuLib::rightChar]).c_str());
	} else {
		logMessage("This is not a replay\n");
		logMessagef("Stage: %i, Main: %i, Sub: %i, Left: %i, Right: %i\n", stage, SokuLib::mainMode, SokuLib::subMode, SokuLib::leftChar, SokuLib::rightChar);
		assets.SetLargeImage(charactersImg[SokuLib::leftChar].c_str());
		logMessage("Left\n");
		assets.SetLargeText(SokuLib::charactersName[SokuLib::leftChar].c_str());
		logMessage("Left\n");
		assets.SetSmallImage(("stage_" + std::to_string(stage + 1)).c_str());
		assets.SetSmallText(SokuLib::stagesName[stage].c_str());
		logMessage("Stage\n");
		activity.SetDetails((SokuLib::modeNames[SokuLib::mainMode][SokuLib::subMode] + " (" + profile1 + ")").c_str());
		logMessage("Modes\n");
		if (SokuLib::mainMode == SokuLib::BATTLE_MODE_VSPLAYER)
			activity.SetState((std::string("Against ") + profile2 + " as " + SokuLib::charactersName[SokuLib::rightChar]).c_str());
		else
			activity.SetState(("Against " + SokuLib::charactersName[SokuLib::rightChar]).c_str());
		logMessage("Right\n");
	}

	core->ActivityManager().UpdateActivity(activity, [](discord::Result result) {
		auto code = static_cast<unsigned>(result);

		if (code)
			logMessagef("Error updating presence: %s\n", discordResultToString[code]);
	});
	logMessage("Callback end\n");
}

void loadMatch()
{
	logMessage("Loading local match\n");
	unsigned stage = SokuLib::flattenStageId(SokuLib::stageId);
	discord::Activity activity{};
	auto &assets = activity.GetAssets();
	auto &timeStamp = activity.GetTimestamps();
	std::string profile1 = convertShiftJisToUTF8(SokuLib::player1Profile);

	logMessagef("profile is %s\n", profile1.c_str());
	gameTimestamp = time(nullptr);
	timeStamp.SetStart(totalTimestamp);
	assets.SetLargeImage(charactersImg[SokuLib::leftChar].c_str());
	assets.SetLargeText(SokuLib::charactersName[SokuLib::leftChar].c_str());

	if (SokuLib::subMode == SokuLib::BATTLE_SUBMODE_REPLAY) {
		assets.SetSmallImage(charactersImg[SokuLib::rightChar].c_str());
		assets.SetSmallText(SokuLib::charactersName[SokuLib::rightChar].c_str());
		activity.SetDetails(SokuLib::modeNames[SokuLib::mainMode][SokuLib::subMode].c_str());
	} else {
		assets.SetSmallImage(("stage_" + std::to_string(stage + 1)).c_str());
		assets.SetSmallText(SokuLib::stagesName[stage].c_str());
		activity.SetDetails((SokuLib::modeNames[SokuLib::mainMode][SokuLib::subMode] + " (" + profile1 + ")").c_str());
	}
	activity.SetState("Loading...");

	core->ActivityManager().UpdateActivity(activity, [](discord::Result result) {
		auto code = static_cast<unsigned>(result);

		if (code)
			logMessagef("Error updating presence: %s\n", discordResultToString[code]);
	});
	logMessage("Callback end\n");
}

void charSelect()
{
	logMessage("Choosing character\n");
	discord::Activity activity{};
	auto &assets = activity.GetAssets();
	auto &timeStamp = activity.GetTimestamps();
	std::string profile1 = convertShiftJisToUTF8(SokuLib::player1Profile);

	logMessagef("Profile name is %s\n", profile1.c_str());
	timeStamp.SetStart(totalTimestamp);
	assets.SetSmallImage(charactersImg[SokuLib::rightChar].c_str());
	assets.SetSmallText(SokuLib::charactersName[SokuLib::rightChar].c_str());
	assets.SetLargeImage(charactersImg[SokuLib::leftChar].c_str());
	assets.SetLargeText(SokuLib::charactersName[SokuLib::leftChar].c_str());

	activity.SetDetails((SokuLib::modeNames[SokuLib::mainMode][SokuLib::subMode] + " (" + profile1 + ")").c_str());
	activity.SetState("Character select...");
	core->ActivityManager().UpdateActivity(activity, [](discord::Result result) {
		auto code = static_cast<unsigned>(result);

		if (code)
			logMessagef("Error updating presence: %s\n", discordResultToString[code]);
	});
	logMessage("Callback end\n");
}

void onlineBattle()
{
	logMessagef("In online battle. Internal ip is %s\n", roomIp.c_str());
	unsigned stage = SokuLib::flattenStageId(SokuLib::stageId);
	logMessagef("We are on stage %u\n", stage);
	discord::Activity activity{};
	auto &assets = activity.GetAssets();
	auto &timeStamp = activity.GetTimestamps();
	std::string profile1 = convertShiftJisToUTF8(SokuLib::player1Profile);
	char myChar;
	char opChar;
	std::string opName;
	SokuLib::NetObject &infos = SokuLib::getNetObject();
	auto &battle_manager = SokuLib::getBattleMgr();

	auto &party = activity.GetParty();
	auto &secrets = activity.GetSecrets();

	if (SokuLib::mainMode == SokuLib::BATTLE_MODE_VSCLIENT) {
		opName = convertShiftJisToUTF8(infos.profile2name);
		myChar = SokuLib::leftChar;
		opChar = SokuLib::rightChar;
		won.first = battle_manager.leftCharacterManager->score;
		won.second = battle_manager.rightCharacterManager->score;
	} else {
		opName = convertShiftJisToUTF8(infos.profile1name);
		myChar = SokuLib::rightChar;
		opChar = SokuLib::leftChar;
		won.first = battle_manager.rightCharacterManager->score;
		won.second = battle_manager.leftCharacterManager->score;
	}
	logMessagef("Opponent name is %s\n", opName.c_str());
	logMessagef("My character is %u\n", myChar);
	logMessagef("Opponent character is %u\n", opChar);

	party.SetId(roomIp.c_str());
	secrets.SetSpectate(("spec" + roomIp).c_str());
	party.GetSize().SetCurrentSize(2);
	party.GetSize().SetMaxSize(2);
	timeStamp.SetStart(gameTimestamp);
	assets.SetSmallImage(("stage_" + std::to_string(stage + 1)).c_str());
	assets.SetSmallText(SokuLib::stagesName[stage].c_str());
	assets.SetLargeImage(charactersImg[myChar].c_str());
	assets.SetLargeText(SokuLib::charactersName[myChar].c_str());

	activity.SetDetails((SokuLib::modeNames[SokuLib::mainMode][SokuLib::subMode] + " (" + profile1 + ")").c_str());
	if (showWR)
		activity.SetState((
			std::string("Against ") + opName + " as " + SokuLib::charactersName[opChar] +
			" (" + std::to_string(won.first) + " - " + std::to_string(won.second) + ")"
		).c_str());
	else
		activity.SetState((std::string("Against ") + opName + " as " + SokuLib::charactersName[opChar]).c_str());

	core->ActivityManager().UpdateActivity(activity, [](discord::Result result) {
		auto code = static_cast<unsigned>(result);

		if (code)
			logMessagef("Error updating presence: %s\n", discordResultToString[code]);
	});
	logMessage("Callback end\n");
}

void onlineBattleSpec()
{
	logMessagef("Watching online game. Internal ip is %s\n", roomIp.c_str());
	unsigned stage = SokuLib::flattenStageId(SokuLib::stageId);
	logMessagef("We are on stage %u\n", stage);
	discord::Activity activity{};
	auto &assets = activity.GetAssets();
	auto &timeStamp = activity.GetTimestamps();
	auto &party = activity.GetParty();
	auto &secrets = activity.GetSecrets();
	SokuLib::NetObject &infos = SokuLib::getNetObject();
	auto &battle_manager = SokuLib::getBattleMgr();

	timeStamp.SetStart(gameTimestamp);

	won.first = battle_manager.leftCharacterManager->score;
	won.second = battle_manager.rightCharacterManager->score;
	party.SetId(roomIp.c_str());
	secrets.SetSpectate(("spec" + roomIp).c_str());
	party.GetSize().SetCurrentSize(2);
	party.GetSize().SetMaxSize(2);
	assets.SetLargeImage(("stage_" + std::to_string(stage + 1)).c_str());
	assets.SetLargeText(SokuLib::stagesName[stage].c_str());
	activity.SetDetails((
		SokuLib::modeNames[SokuLib::mainMode][SokuLib::subMode] + " (" +
		convertShiftJisToUTF8(infos.profile1name) + " vs " +
		convertShiftJisToUTF8(infos.profile2name) + ")"
	).c_str());
	activity.SetState((
		SokuLib::charactersName[SokuLib::leftChar] + " vs " +
		SokuLib::charactersName[SokuLib::rightChar] +
		" (" + std::to_string(won.first) + " - " + std::to_string(won.second) + ")"
	).c_str());

	core->ActivityManager().UpdateActivity(activity, [](discord::Result result) {
		auto code = static_cast<unsigned>(result);

		if (code)
			logMessagef("Error updating presence: %s\n", discordResultToString[code]);
	});
	logMessage("Callback end\n");
}

void loadOnlineMatch()
{
	logMessagef("Loading online match. Internal ip is %s\n", roomIp.c_str());
	unsigned stage = SokuLib::flattenStageId(SokuLib::stageId);
	discord::Activity activity{};
	auto &assets = activity.GetAssets();
	auto &timeStamp = activity.GetTimestamps();
	std::string profile1 = convertShiftJisToUTF8(SokuLib::player1Profile);
	char myChar;

	logMessagef("profile is %s\n", profile1.c_str());
	auto &party = activity.GetParty();
	auto &secrets = activity.GetSecrets();

	if (SokuLib::mainMode == SokuLib::BATTLE_MODE_VSCLIENT)
		myChar = SokuLib::leftChar;
	else
		myChar = SokuLib::rightChar;
	logMessagef("My character is %u\n", myChar);

	party.SetId(roomIp.c_str());
	secrets.SetSpectate(("spec" + roomIp).c_str());
	party.GetSize().SetCurrentSize(2);
	party.GetSize().SetMaxSize(2);
	gameTimestamp = time(nullptr);
	timeStamp.SetStart(totalTimestamp);
	assets.SetSmallImage(("stage_" + std::to_string(stage + 1)).c_str());
	assets.SetSmallText(SokuLib::stagesName[stage].c_str());
	assets.SetLargeImage(charactersImg[myChar].c_str());
	assets.SetLargeText(SokuLib::charactersName[myChar].c_str());

	activity.SetDetails((SokuLib::modeNames[SokuLib::mainMode][SokuLib::subMode] + " (" + profile1 + ")").c_str());
	activity.SetState("Loading...");

	core->ActivityManager().UpdateActivity(activity, [](discord::Result result) {
		auto code = static_cast<unsigned>(result);

		if (code)
			logMessagef("Error updating presence: %s\n", discordResultToString[code]);
	});
	logMessage("Callback end\n");
}

void loadOnlineMatchSpec()
{
	if (!experimentalSpec)
		return genericScreen();

	logMessagef("Loading online match as spectator. Internal ip is %s\n", roomIp.c_str());
	unsigned stage = SokuLib::flattenStageId(SokuLib::stageId);
	discord::Activity activity{};
	auto &assets = activity.GetAssets();
	auto &timeStamp = activity.GetTimestamps();
	SokuLib::NetObject &infos = SokuLib::getNetObject();
	auto &party = activity.GetParty();
	auto &secrets = activity.GetSecrets();

	party.SetId(roomIp.c_str());
	secrets.SetSpectate(("spec" + roomIp).c_str());
	party.GetSize().SetCurrentSize(2);
	party.GetSize().SetMaxSize(2);
	gameTimestamp = time(nullptr);
	timeStamp.SetStart(totalTimestamp);
	assets.SetLargeImage(charactersImg[SokuLib::leftChar].c_str());
	assets.SetLargeText(SokuLib::charactersName[SokuLib::leftChar].c_str());

	assets.SetSmallImage(charactersImg[SokuLib::rightChar].c_str());
	assets.SetSmallText(SokuLib::charactersName[SokuLib::rightChar].c_str());
	activity.SetDetails((
		SokuLib::modeNames[SokuLib::mainMode][SokuLib::subMode] + " (" +
		convertShiftJisToUTF8(infos.profile1name) + " vs " +
		convertShiftJisToUTF8(infos.profile2name) + ")"
	).c_str());
	activity.SetState("Loading...");

	core->ActivityManager().UpdateActivity(activity, [](discord::Result result) {
		auto code = static_cast<unsigned>(result);

		if (code)
			logMessagef("Error updating presence: %s\n", discordResultToString[code]);
	});
	logMessage("Callback end\n");
}

void onlineCharSelect()
{
	logMessagef("Online character select. Internal ip is %s\n", roomIp.c_str());
	SokuLib::NetObject &infos = SokuLib::getNetObject();
	discord::Activity activity{};
	auto &assets = activity.GetAssets();
	auto &timeStamp = activity.GetTimestamps();
	std::string profile1 = convertShiftJisToUTF8(SokuLib::player1Profile);
	char myChar;
	char opChar;
	std::string opName;
	auto &party = activity.GetParty();
	auto &secrets = activity.GetSecrets();

	logMessagef("profile is %s\n", profile1.c_str());
	if (SokuLib::mainMode == SokuLib::BATTLE_MODE_VSCLIENT) {
		opName = convertShiftJisToUTF8(infos.profile2name);
		myChar = SokuLib::leftChar;
		opChar = SokuLib::rightChar;
	} else {
		opName = convertShiftJisToUTF8(infos.profile1name);
		myChar = SokuLib::rightChar;
		opChar = SokuLib::leftChar;
	}
	logMessagef("Opponent name is %s\n", opName.c_str());
	logMessagef("My character is %u\n", myChar);
	logMessagef("Opponent character is %u\n", opChar);

	party.SetId(roomIp.c_str());
	secrets.SetSpectate(("spec" + roomIp).c_str());
	party.GetSize().SetCurrentSize(2);
	party.GetSize().SetMaxSize(2);
	score.first += won.first > won.second && won.first;
	score.second += won.second > won.first && won.second;
	logMessagef("Win %i:%i/Score %i:%i\n", won.first, won.second, score.first, score.second);
	won = {0, 0};
	timeStamp.SetStart(totalTimestamp);
	assets.SetSmallImage(charactersImg[opChar].c_str());
	assets.SetSmallText(SokuLib::charactersName[opChar].c_str());
	assets.SetLargeImage(charactersImg[myChar].c_str());
	assets.SetLargeText(SokuLib::charactersName[myChar].c_str());

	activity.SetDetails((SokuLib::modeNames[SokuLib::mainMode][SokuLib::subMode] + " (" + profile1 + ")").c_str());
	if (showWR)
		activity.SetState((
			"Character select... (vs " + std::string(opName) + " " +
			std::to_string(score.first) + "w " +
			std::to_string(score.second) + "l " +
			(score.first + score.second ? std::to_string(score.first * 100 / (score.first + score.second)) : "N/A") +
			"% wr)"
		).c_str());
	else
		activity.SetState(("Character select... (vs " + std::string(opName) + ")").c_str());

	core->ActivityManager().UpdateActivity(activity, [](discord::Result result) {
		auto code = static_cast<unsigned>(result);

		if (code)
			logMessagef("Error updating presence: %s\n", discordResultToString[code]);
	});
}

std::vector<std::function<void()>> sceneCallbacks{
	genericScreen,       //SWRSSCENE_LOGO         = 0,
	genericScreen,       //SWRSSCENE_OPENING      = 1,
	titleScreen,         //SWRSSCENE_TITLE        = 2,
	charSelect,          //SWRSSCENE_SELECT       = 3,
	genericScreen,       //???                    = 4,
	localBattle,         //SWRSSCENE_BATTLE       = 5,
	loadMatch,           //SWRSSCENE_LOADING      = 6,
	genericScreen,       //???                    = 7,
	onlineCharSelect,    //SWRSSCENE_SELECTSV     = 8,
	onlineCharSelect,    //SWRSSCENE_SELECTCL     = 9,
	loadOnlineMatch,     //SWRSSCENE_LOADINGSV    = 10,
	loadOnlineMatch,     //SWRSSCENE_LOADINGCL    = 11,
	loadOnlineMatchSpec, //SWRSSCENE_LOADINGWATCH = 12,
	onlineBattle,        //SWRSSCENE_BATTLESV     = 13,
	onlineBattle,        //SWRSSCENE_BATTLECL     = 14,
	onlineBattleSpec,    //SWRSSCENE_BATTLEWATCH  = 15,
	genericScreen,       //SWRSSCENE_SELECTSENARIO= 16,
	genericScreen,       //???                    = 17,
	genericScreen,       //???                    = 18,
	genericScreen,       //???                    = 19,
	genericScreen,       //SWRSSCENE_ENDING       = 20,
};

enum MenuEnum {
	MENU_NONE,
	MENU_CONNECT,
	MENU_REPLAY,
	MENU_MUSICROOM,
	MENU_RESULT,
	MENU_PROFILE,
	MENU_CONFIG,
	MENU_COUNT
};

void onActivityJoin(const char *sec)
{
	logMessagef("Got activity join with payload %s\n", sec);

	auto menuObj = SokuLib::getMenuObj<SokuLib::MenuConnect>();
	std::string secret = sec;
	auto ip = secret.substr(4, secret.find_last_of(':') - 4);
	unsigned short port = std::stol(secret.substr(secret.find_last_of(':') + 1));
	bool isSpec = secret.substr(0, 4) == "spec";

	if (!SokuLib::isInNetworkMenu()) {
		logMessage("Warping to connect screen.\n");
		SokuLib::moveToConnectMenu();
		menuObj = SokuLib::getMenuObj<SokuLib::MenuConnect>();
		logMessage("Done.\n");
	} else
		logMessage("Already in connect screen\n");

	if (
		menuObj->choice >= SokuLib::MenuConnect::CHOICE_ASSIGN_IP_CONNECT &&
		menuObj->choice < SokuLib::MenuConnect::CHOICE_SELECT_PROFILE &&
		menuObj->subchoice == 3
	)
		return;
	if (
		menuObj->choice >= SokuLib::MenuConnect::CHOICE_HOST &&
		menuObj->choice < SokuLib::MenuConnect::CHOICE_SELECT_PROFILE &&
		menuObj->subchoice == 255
	)
		return;
	if (
		menuObj->choice == SokuLib::MenuConnect::CHOICE_HOST &&
		menuObj->subchoice == 2
	)
		return;

	logMessagef("Connecting to %s:%u as %s\n", ip.c_str(), port, isSpec ? "spectator" : "player");
	SokuLib::joinHost(
		ip.c_str(),
		port,
		isSpec
	);
	roomIp = ip + ":" + std::to_string(port);
}

class MyThread : public std::thread {
private:
	bool _done;
	int _connectTimeout = 1;

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
			discord::Result result;

			do {
				result = discord::Core::Create(clientId, DiscordCreateFlags_NoRequireDiscord, &core);

				if (result != discord::Result::Ok) {
					logMessagef("Error connecting to discord: %s\n", discordResultToString[static_cast<unsigned>(result)]);
					logMessagef("Retrying in %i seconds\n", this->_connectTimeout);
					std::this_thread::sleep_for(std::chrono::seconds(this->_connectTimeout));
					if (this->_connectTimeout < 64)
						this->_connectTimeout *= 2;
				}
			} while (result != discord::Result::Ok);
			logMessage("Connected !\n");
			core->ActivityManager().OnActivityJoin.Connect(onActivityJoin);
			logMessage("Entering loop\n");
			while (!this->isDone()) {
				currentScene = SokuLib::sceneId;

				auto newScene = SokuLib::newSceneId;

				logMessagef("Current scene is %i vs new scene %i\n", currentScene, newScene);
				if (currentScene >= 0 && currentScene < sceneCallbacks.size() && currentScene == newScene) {
					logMessagef("Calling callback %u\n", currentScene);
					sceneCallbacks[currentScene]();
					logMessage("Callback returned\n");
				} else if (currentScene == SokuLib::SCENE_TITLE && (newScene == SokuLib::SCENE_SELECTSV || newScene == SokuLib::SCENE_SELECTCL))
					connectedToRemoteLoadingCharSelect();
				else
					logMessage("No callback call\n");
				logMessage("Running discord callbacks\n");
				core->RunCallbacks();
				logMessagef("Waiting for next cycle (%llu ms)\n", refreshRate);
				std::this_thread::sleep_for(std::chrono::milliseconds(refreshRate));
			}
			logMessage("Exit game\n");
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
	showWR = GetPrivateProfileInt("DiscordIntegration", "ShowWR", 0, profilePath) != 0;
	experimentalSpec = GetPrivateProfileInt("DiscordIntegration", "ExperimentalSpectator", 0, profilePath) != 0;
	refreshRate = GetPrivateProfileInt("DiscordIntegration", "RefreshTime", 1000, profilePath);
	GetPrivateProfileString("DiscordIntegration", "HostImg", "", smallImg, sizeof(smallImg), profilePath);
	GetPrivateProfileString("DiscordIntegration", "ClientID", ClientID, buffer, sizeof(buffer), profilePath);
	clientId = atoll(buffer);
	GetPrivateProfileString("DiscordIntegration", "InviteIp", "", buffer, sizeof(buffer), profilePath);
	if (inet_addr(buffer) != -1)
		myIp = strdup(buffer);
	logMessagef("Enabled: %s\nClientID: %llu\nShowWR: %s\nHostImg: %s\nInviteIp: %s\n", enabled ? "true" : "false", clientId, showWR ? "true" : "false", smallImg, myIp);
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

	initLogger();
	logMessage("Initializing...\n");

	GetModuleFileName(hMyModule, profilePath, 1024);
	PathRemoveFileSpec(profilePath);
	PathAppend(profilePath, "DiscordIntegration.ini");
	LoadSettings(profilePath);

	//DWORD old;
	//::VirtualProtect((PVOID)rdata_Offset, rdata_Size, PAGE_EXECUTE_WRITECOPY, &old);
	//s_origCLogo_OnProcess   = TamperDword(vtbl_CLogo   + 4, (DWORD)CLogo_OnProcess);
	//s_origCBattle_OnProcess = TamperDword(vtbl_CBattle + 4, (DWORD)CBattle_OnProcess);
	//s_origCBattleSV_OnProcess = TamperDword(vtbl_CBattleSV + 4, (DWORD)CBattleSV_OnProcess);
	//s_origCBattleCL_OnProcess = TamperDword(vtbl_CBattleCL + 4, (DWORD)CBattleCL_OnProcess);
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