//
// Created by Gegel85 on 31/10/2020
//

#include "CompiledString/CompiledStringFactory.hpp"
#include "CompiledString/Vars/vars.hpp"
#include "Exceptions.hpp"
#include "Network/getPublicIp.hpp"
#include "logger.hpp"
#include "nlohmann/json.hpp"
#include <SokuLib.hpp>
#include <array>
#include <ctime>
#include <discord.h>
#include <fstream>
#include <shlwapi.h>
#include <string>
#include <thread>

enum StringIndex {
	STRING_INDEX_LOGO,
	STRING_INDEX_OPENING,
	STRING_INDEX_TITLE,
	STRING_INDEX_SELECT_VSCOMPUTER,
	STRING_INDEX_BATTLE_VSCOMPUTER,
	STRING_INDEX_LOADING_VSCOMPUTER,
	STRING_INDEX_SELECT_STORY,
	STRING_INDEX_BATTLE_STORY,
	STRING_INDEX_LOADING_STORY,
	STRING_INDEX_SELECT_ARCADE,
	STRING_INDEX_BATTLE_ARCADE,
	STRING_INDEX_LOADING_ARCADE,
	STRING_INDEX_BATTLE_TIMETRIAL,
	STRING_INDEX_LOADING_TIMETRIAL,
	STRING_INDEX_SELECT_VSPLAYER,
	STRING_INDEX_BATTLE_VSPLAYER,
	STRING_INDEX_LOADING_VSPLAYER,
	STRING_INDEX_SELECT_PRACTICE,
	STRING_INDEX_BATTLE_PRACTICE,
	STRING_INDEX_LOADING_PRACTICE,
	STRING_INDEX_SELECT_VSNETWORK,
	STRING_INDEX_BATTLE_VSNETWORK,
	STRING_INDEX_LOADING_VSNETWORK,
	STRING_INDEX_BATTLE_SPECTATOR,
	STRING_INDEX_LOADING_SPECTATOR,
	STRING_INDEX_BATTLE_REPLAY,
	STRING_INDEX_LOADING_REPLAY,
	STRING_INDEX_BATTLE_STORY_REPLAY,
	STRING_INDEX_LOADING_STORY_REPLAY,
	STRING_INDEX_ENDING,
	STRING_INDEX_HOSTING,
	STRING_INDEX_CONNECTING,
	STRING_INDEX_MAX
};

static const char *const allElems[]{
	"logo",
	"opening",
	"title",
	"select_vscomputer",
	"battle_vscomputer",
	"loading_vscomputer",
	"select_story",
	"battle_story",
	"loading_story",
	"select_arcade",
	"battle_arcade",
	"loading_arcade",
	"battle_timetrial",
	"loading_timetrial",
	"select_vsplayer",
	"battle_vsplayer",
	"loading_vsplayer",
	"select_practice",
	"battle_practice",
	"loading_practice",
	"select_vsnetwork",
	"battle_vsnetwork",
	"loading_vsnetwork",
	"battle_spectator",
	"loading_spectator",
	"battle_replay",
	"loading_replay",
	"battle_story_replay",
	"loading_story_replay",
	"ending",
	"hosting",
	"connecting",
};

struct StringConfig {
	bool timestamp;
	bool set_timestamp;
	std::shared_ptr<CompiledString> title;
	std::shared_ptr<CompiledString> description;
	std::shared_ptr<CompiledString> large_image;
	std::shared_ptr<CompiledString> large_text;
	std::shared_ptr<CompiledString> small_image;
	std::shared_ptr<CompiledString> small_text;
};
struct Config {
	std::vector<StringConfig> strings;
	time_t refreshRate = 0;
};
struct State {
	unsigned host = 0;
	time_t gameTimestamp = 0;
	time_t totalTimestamp = 0;
	discord::Core *core = nullptr;
	std::string roomIp;
	std::pair<unsigned, unsigned> won = {0, 0};
};

static State state;
static Config config;

static const std::vector<const char *> discordResultToString{
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

void updateActivity(StringIndex index, unsigned party) {
	if (index >= config.strings.size())
		return;

	discord::Activity activity{};
	auto &assets = activity.GetAssets();
	auto &timeStamp = activity.GetTimestamps();
	auto &partyObj = activity.GetParty();
	auto &secrets = activity.GetSecrets();
	auto &elem = config.strings[index];

	if (party) {
		if (!state.roomIp.empty()) {
			if (party == 1)
				secrets.SetJoin(("join" + state.roomIp).c_str());
			secrets.SetSpectate(("spec" + state.roomIp).c_str());
		}

		partyObj.SetId(state.roomIp.c_str());
		partyObj.GetSize().SetCurrentSize(party);
		partyObj.GetSize().SetMaxSize(2);
	}

	if (elem.timestamp)
		timeStamp.SetStart(state.totalTimestamp);
	if (elem.set_timestamp)
		timeStamp.SetStart(state.gameTimestamp);

	if (elem.title)
		activity.SetState(elem.title->getString().c_str());
	if (elem.description)
		activity.SetDetails(elem.description->getString().c_str());

	if (elem.large_image)
		assets.SetLargeImage(elem.large_image->getString().c_str());
	if (elem.large_text)
		assets.SetLargeText(elem.large_text->getString().c_str());

	if (elem.small_image)
		assets.SetSmallImage(elem.small_image->getString().c_str());
	if (elem.small_text)
		assets.SetSmallText(elem.small_text->getString().c_str());

	state.core->ActivityManager().UpdateActivity(activity, [](discord::Result result) {
		auto code = static_cast<unsigned>(result);

		if (code)
			logMessagef("Error updating presence: %s\n", discordResultToString[code]);
	});
}

void getActivityParams(StringIndex &index, unsigned &party) {
	party = 0;

	switch (SokuLib::sceneId) {
	case SokuLib::SCENE_SELECT:
		state.host = 0;
		switch (SokuLib::mainMode) {
		case SokuLib::BATTLE_MODE_PRACTICE:
			index = STRING_INDEX_SELECT_PRACTICE;
			return;
		case SokuLib::BATTLE_MODE_VSPLAYER:
			index = STRING_INDEX_SELECT_VSPLAYER;
			return;
		default:
			index = STRING_INDEX_SELECT_VSCOMPUTER;
			return;
		}
	case SokuLib::SCENE_SELECTSV:
	case SokuLib::SCENE_SELECTCL:
		state.host = 0;
		index = STRING_INDEX_SELECT_VSNETWORK;
		party = 2;
		return;
	case SokuLib::SCENE_LOADING:
		state.host = 0;
		switch (SokuLib::mainMode) {
		case SokuLib::BATTLE_MODE_PRACTICE:
			index = STRING_INDEX_LOADING_PRACTICE;
			return;
		case SokuLib::BATTLE_MODE_VSPLAYER:
			if (SokuLib::subMode == SokuLib::BATTLE_SUBMODE_REPLAY)
				index = STRING_INDEX_LOADING_REPLAY;
			else
				index = STRING_INDEX_LOADING_VSPLAYER;
			return;
		case SokuLib::BATTLE_MODE_STORY:
			if (SokuLib::subMode == SokuLib::BATTLE_SUBMODE_REPLAY)
				index = STRING_INDEX_LOADING_STORY_REPLAY;
			else
				index = STRING_INDEX_LOADING_STORY;
			return;
		case SokuLib::BATTLE_MODE_ARCADE:
			index = STRING_INDEX_LOADING_ARCADE;
			return;
		case SokuLib::BATTLE_MODE_TIME_TRIAL:
			index = STRING_INDEX_LOADING_TIMETRIAL;
			return;
		default:
			index = STRING_INDEX_LOADING_VSCOMPUTER;
			return;
		}
	case SokuLib::SCENE_LOADINGSV:
	case SokuLib::SCENE_LOADINGCL:
		state.host = 0;
		index = STRING_INDEX_LOADING_VSNETWORK;
		party = 2;
		return;
	case SokuLib::SCENE_LOADINGWATCH:
		state.host = 0;
		index = STRING_INDEX_LOADING_SPECTATOR;
		party = 2;
		return;
	case SokuLib::SCENE_BATTLE:
		state.host = 0;
		switch (SokuLib::mainMode) {
		case SokuLib::BATTLE_MODE_PRACTICE:
			index = STRING_INDEX_BATTLE_PRACTICE;
			return;
		case SokuLib::BATTLE_MODE_VSPLAYER:
			if (SokuLib::subMode == SokuLib::BATTLE_SUBMODE_REPLAY)
				index = STRING_INDEX_BATTLE_REPLAY;
			else
				index = STRING_INDEX_BATTLE_VSPLAYER;
			return;
		case SokuLib::BATTLE_MODE_STORY:
			if (SokuLib::subMode == SokuLib::BATTLE_SUBMODE_REPLAY)
				index = STRING_INDEX_BATTLE_STORY_REPLAY;
			else
				index = STRING_INDEX_BATTLE_STORY;
			return;
		case SokuLib::BATTLE_MODE_ARCADE:
			index = STRING_INDEX_BATTLE_ARCADE;
			return;
		case SokuLib::BATTLE_MODE_TIME_TRIAL:
			index = STRING_INDEX_BATTLE_TIMETRIAL;
			return;
		default:
			index = STRING_INDEX_BATTLE_VSCOMPUTER;
			return;
		}
	case SokuLib::SCENE_BATTLEWATCH:
		state.host = 0;
		index = STRING_INDEX_BATTLE_SPECTATOR;
		party = 2;
		return;
	case SokuLib::SCENE_BATTLESV:
	case SokuLib::SCENE_BATTLECL:
		state.host = 0;
		index = STRING_INDEX_BATTLE_VSNETWORK;
		party = 2;
		return;
	case SokuLib::SCENE_SELECTSENARIO:
		state.host = 0;
		// STRING_INDEX_SELECT_ARCADE;
		index = STRING_INDEX_SELECT_STORY;
		return;
	case SokuLib::SCENE_ENDING:
		state.host = 0;
		index = STRING_INDEX_ENDING;
		return;
	case SokuLib::SCENE_LOGO:
		state.host = 0;
		index = STRING_INDEX_LOGO;
		return;
	case SokuLib::SCENE_OPENING:
		state.host = 0;
		index = STRING_INDEX_OPENING;
		return;
	default:
		state.host = 0;
		return;
	case SokuLib::SCENE_TITLE:
		auto *menuObj = SokuLib::getMenuObj<SokuLib::MenuConnect>();

		index = STRING_INDEX_TITLE;
		if (!SokuLib::isInNetworkMenu()) {
			logMessage("We are not in a proper submenu\n");
			return;
		}

		if ((menuObj->choice >= SokuLib::MenuConnect::CHOICE_ASSIGN_IP_CONNECT && menuObj->choice < SokuLib::MenuConnect::CHOICE_SELECT_PROFILE
					&& menuObj->subchoice == 3)
			|| (menuObj->choice >= SokuLib::MenuConnect::CHOICE_HOST && menuObj->choice < SokuLib::MenuConnect::CHOICE_SELECT_PROFILE && menuObj->subchoice == 255)) {
			if (state.host != 2)
				state.totalTimestamp = time(nullptr);
			state.host = 2;
			index = STRING_INDEX_CONNECTING;
			party = 2;
		} else if (menuObj->choice == SokuLib::MenuConnect::CHOICE_HOST && menuObj->subchoice == 2) {
			if (state.host != 1)
				state.totalTimestamp = time(nullptr);
			state.host = 1;
			index = STRING_INDEX_HOSTING;
			party = 1;
		} else
			state.host = 0;
	}
}

void titleScreenStateUpdate() {
	auto *menuObj = SokuLib::getMenuObj<SokuLib::MenuConnect>();

	if (!SokuLib::isInNetworkMenu()) {
		logMessage("We are not in a proper submenu\n");
		return;
	}

	if ((menuObj->choice >= SokuLib::MenuConnect::CHOICE_ASSIGN_IP_CONNECT && menuObj->choice < SokuLib::MenuConnect::CHOICE_SELECT_PROFILE
				&& menuObj->subchoice == 3)
		|| (menuObj->choice >= SokuLib::MenuConnect::CHOICE_HOST && menuObj->choice < SokuLib::MenuConnect::CHOICE_SELECT_PROFILE && menuObj->subchoice == 255)) {
		if (state.roomIp.empty())
			state.roomIp = menuObj->IPString + (":" + std::to_string(menuObj->port));
	} else if (menuObj->choice == SokuLib::MenuConnect::CHOICE_HOST && menuObj->subchoice == 2) {
		if (state.roomIp.empty())
			try {
				state.roomIp = getMyIp() + std::string(":") + std::to_string(menuObj->port);
			} catch (...) {
			}
		logMessagef("Hosting. Room ip is %s. Spectator are %sallowed\n", state.roomIp.c_str(), menuObj->spectate ? "" : "not ");
	} else if (!state.roomIp.empty())
		state.roomIp.clear();
}

void updateState() {
	switch (SokuLib::sceneId) {
	case SokuLib::SCENE_SELECT:
	case SokuLib::SCENE_SELECTSV:
	case SokuLib::SCENE_SELECTCL:
	case SokuLib::SCENE_LOADING:
	case SokuLib::SCENE_LOADINGSV:
	case SokuLib::SCENE_LOADINGCL:
	case SokuLib::SCENE_LOADINGWATCH:
		wins.first += state.won.first == 2;
		wins.second += state.won.second == 2;
		state.won = {0, 0};
		break;

	case SokuLib::SCENE_TITLE:
		titleScreenStateUpdate();
	case SokuLib::SCENE_SELECTSENARIO:
	case SokuLib::SCENE_ENDING:
	case SokuLib::SCENE_LOGO:
	case SokuLib::SCENE_OPENING:
		state.gameTimestamp = time(nullptr);
		break;

	default:
		break;

	case SokuLib::SCENE_BATTLE:
	case SokuLib::SCENE_BATTLEWATCH:
	case SokuLib::SCENE_BATTLESV:
	case SokuLib::SCENE_BATTLECL:
		auto &battle = SokuLib::getBattleMgr();

		if (SokuLib::mainMode == SokuLib::BATTLE_MODE_VSSERVER) {
			state.won.first = battle.rightCharacterManager.score;
			state.won.second = battle.leftCharacterManager.score;
		} else {
			state.won.first = battle.leftCharacterManager.score;
			state.won.second = battle.rightCharacterManager.score;
		}
		break;
	}
}

void tick() {
	StringIndex index;
	unsigned party = 0;

	updateState();
	getActivityParams(index, party);
	logMessagef("Calling callback %u\n", index);
	updateActivity(index, party);
}

class MyThread: public std::thread {
private:
	bool _done;
	int _connectTimeout = 1;

public:
	bool isDone() const {
		return this->_done;
	}
	template<typename... Args> MyThread(): std::thread(){};
	~MyThread() {
		this->_done = true;
		if (this->joinable())
			this->join();
	}

	static void onActivityJoin(const char *sec) {
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

		if (menuObj->choice >= SokuLib::MenuConnect::CHOICE_ASSIGN_IP_CONNECT && menuObj->choice < SokuLib::MenuConnect::CHOICE_SELECT_PROFILE
			&& menuObj->subchoice == 3)
			return;
		if (menuObj->choice >= SokuLib::MenuConnect::CHOICE_HOST && menuObj->choice < SokuLib::MenuConnect::CHOICE_SELECT_PROFILE && menuObj->subchoice == 255)
			return;
		if (menuObj->choice == SokuLib::MenuConnect::CHOICE_HOST && menuObj->subchoice == 2)
			return;

		logMessagef("Connecting to %s:%u as %s\n", ip.c_str(), port, isSpec ? "spectator" : "player");
		SokuLib::joinHost(ip.c_str(), port, isSpec);
		state.roomIp = ip + ":" + std::to_string(port);
	}

	void init() {
		logMessage("Connecting to discord client...\n");
		discord::Result result;

		do {
			result = discord::Core::Create(atoll(ClientID), DiscordCreateFlags_NoRequireDiscord, &state.core);

			if (result != discord::Result::Ok) {
				logMessagef("Error connecting to discord: %s\n", discordResultToString[static_cast<unsigned>(result)]);
				logMessagef("Retrying in %i seconds\n", this->_connectTimeout);
				std::this_thread::sleep_for(std::chrono::seconds(this->_connectTimeout));
				if (this->_connectTimeout < 64)
					this->_connectTimeout *= 2;
			}
		} while (result != discord::Result::Ok);
		logMessage("Connected !\n");
		state.core->ActivityManager().OnActivityJoin.Connect(MyThread::onActivityJoin);
	}

	void run() const {
		logMessage("Entering loop\n");
		while (!this->isDone()) {
			auto currentScene = SokuLib::sceneId;
			auto newScene = SokuLib::newSceneId;

			logMessagef("Current scene is %i vs new scene %i\n", currentScene, newScene);
			if (currentScene != newScene)
				state.totalTimestamp = time(nullptr);

			if (currentScene >= 0 && currentScene == newScene) {
				tick();
				logMessage("Callback returned\n");
			} else if (currentScene == SokuLib::SCENE_TITLE && (newScene == SokuLib::SCENE_SELECTSV || newScene == SokuLib::SCENE_SELECTCL))
				updateActivity(STRING_INDEX_CONNECTING, 2);
			else
				logMessage("No callback call\n");
			logMessage("Running discord callbacks\n");
			state.core->RunCallbacks();
			logMessagef("Waiting for next cycle (%llu ms)\n", config.refreshRate);
			std::this_thread::sleep_for(std::chrono::milliseconds(config.refreshRate));
		}
		logMessage("Exit game\n");
	}

	void start() {
		std::thread::operator=(std::thread([this] {
			this->init();
			this->run();
		}));
	}
};
static MyThread updateThread;

void loadJsonStrings(const std::string &path) {
	std::ifstream stream{path};
	nlohmann::json value;

	logMessagef("Loading config file %s\n", path.c_str());
	if (stream.fail())
		throw std::invalid_argument(strerror(errno));
	stream >> value;
	for (int i = 0; i < 21; i++) {
		logMessagef("Loading character %i %s\n", i, value["characters"][i].dump().c_str());
		charactersNames[i].first = value["characters"][i]["short_name"];
		charactersNames[i].second = value["characters"][i]["full_name"];
	}
	logMessagef("Loading stages\n");
	stagesNames = value["stages"].get<std::vector<std::string>>();
	if (stagesNames.size() < 20)
		throw std::invalid_argument("Stage list is too small");
	logMessagef("Loading submenus\n");
	submenusNames = value["submenus"].get<std::vector<std::string>>();
	if (submenusNames.size() < 7)
		throw std::invalid_argument("Submenus list is too small");
	config.strings.clear();
	config.strings.reserve(sizeof(allElems) / sizeof(*allElems));
	for (auto &elem : allElems) {
		auto &val = value[elem];
		StringConfig cfg;

		logMessagef("Loading elem %s %s\n", elem, val.dump().c_str());
		cfg.timestamp = val.contains("has_timer") && val["has_timer"].get<bool>();
		cfg.set_timestamp = val.contains("has_set_length_timer") && val["has_set_length_timer"].get<bool>();
		cfg.title = val.contains("title") ? CompiledStringFactory::compileString(val["title"]) : nullptr;
		cfg.description = val.contains("description") ? CompiledStringFactory::compileString(val["description"]) : nullptr;
		cfg.large_image = val.contains("image") ? CompiledStringFactory::compileString(val["image"]) : nullptr;
		cfg.small_image = val.contains("small_image") ? CompiledStringFactory::compileString(val["small_image"]) : nullptr;
		cfg.large_text = val.contains("image_text") ? CompiledStringFactory::compileString(val["image_text"]) : nullptr;
		cfg.small_text = val.contains("small_image_text") ? CompiledStringFactory::compileString(val["small_image_text"]) : nullptr;
		config.strings.push_back(cfg);
	}
}

// �ݒ胍�[�h
void LoadSettings(LPCSTR profilePath) {
	int i;
	char buffer[64];
	char file[MAX_PATH];
	char path[MAX_PATH];
	char local[LOCALE_NAME_MAX_LENGTH];
	wchar_t localw[LOCALE_NAME_MAX_LENGTH];

	logMessage("Loading settings...\n");
	config.refreshRate = GetPrivateProfileInt("DiscordIntegration", "RefreshTime", 1000, profilePath);

	auto ret = GetPrivateProfileString("DiscordIntegration", "InviteIp", "", buffer, sizeof(buffer), profilePath);

	if (inet_addr(buffer) != -1 && ret)
		myIp = strdup(buffer);

	ret = GetPrivateProfileString("DiscordIntegration", "StringFile", "", file, sizeof(file), profilePath);
	strcpy(path, profilePath);
	PathRemoveFileSpec(path);
	PathAppend(path, file);

	logMessagef("ClientID: %llu\nInviteIp: %s\nFile: %s", ClientID, myIp, path);

	try {
		if (ret) {
			loadJsonStrings(path);
			return;
		}
	} catch (std::exception &e) {
		MessageBoxA(
			nullptr, ("Cannot load file " + std::string(path) + ": " + e.what() + "\nFalling back to default files...").c_str(), "Loading error", MB_ICONWARNING);
	}

	strcpy(path, profilePath);
	PathRemoveFileSpec(path);
	strcat(path, "\\langs\\");
	GetUserDefaultLocaleName(localw, sizeof(localw));
	for (i = 0; localw[i]; i++)
		local[i] = localw[i];
	local[i] = 0;
	if (strchr(local, '-') != nullptr)
		*strchr(local, '-') = 0;
	strcat(path, local);
	strcat(path, ".json");
	try {
		loadJsonStrings(path);
		return;
	} catch (std::exception &e) {
		logMessagef("%s: %s\n", path, e.what());
	}

	strcpy(path, profilePath);
	PathRemoveFileSpec(path);
	PathAppend(path, "langs/en.json");
	try {
		loadJsonStrings(path);
	} catch (std::exception &e) {
		MessageBoxA(nullptr, ("Cannot load file " + std::string(path) + ": " + e.what()).c_str(), "Loading error", MB_ICONERROR);
		abort();
	}
}

extern "C" __declspec(dllexport) bool CheckVersion(const BYTE hash[16]) {
	return true;
}

extern "C" __declspec(dllexport) bool Initialize(HMODULE hMyModule, HMODULE hParentModule) {
	char profilePath[1024 + MAX_PATH];

	initLogger();
	logMessage("Initializing...\n");

	GetModuleFileName(hMyModule, profilePath, 1024);
	PathRemoveFileSpec(profilePath);
	PathAppend(profilePath, "DiscordIntegration.ini");
	LoadSettings(profilePath);

	// DWORD old;
	//::VirtualProtect((PVOID)rdata_Offset, rdata_Size, PAGE_EXECUTE_WRITECOPY, &old);
	// s_origCLogo_OnProcess   = TamperDword(vtbl_CLogo   + 4, (DWORD)CLogo_OnProcess);
	// s_origCBattle_OnProcess = TamperDword(vtbl_CBattle + 4, (DWORD)CBattle_OnProcess);
	// s_origCBattleSV_OnProcess = TamperDword(vtbl_CBattleSV + 4, (DWORD)CBattleSV_OnProcess);
	// s_origCBattleCL_OnProcess = TamperDword(vtbl_CBattleCL + 4, (DWORD)CBattleCL_OnProcess);
	// s_origCTitle_OnProcess  = TamperDword(vtbl_CTitle  + 4, (DWORD)CTitle_OnProcess);
	// s_origCSelect_OnProcess = TamperDword(vtbl_CSelect + 4, (DWORD)CSelect_OnProcess);
	//::VirtualProtect((PVOID)rdata_Offset, rdata_Size, old, &old);

	//::FlushInstructionCache(GetCurrentProcess(), nullptr, 0);

	updateThread.start();
	logMessage("Done...\n");
	return true;
}

extern "C" int APIENTRY DllMain(HMODULE hModule, DWORD fdwReason, LPVOID lpReserved) {
	return TRUE;
}