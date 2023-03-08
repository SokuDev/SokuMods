#include <Windows.h>
#include <map>
#include <vector>
#include <cassert>
#include <iostream>
#include <fstream>
#include <nlohmann/json.hpp>
#include <dinput.h>
#include <Shlwapi.h>
#include <sstream>
#include <SokuLib.hpp>
#include <random>

template<typename T, typename T2>
struct MapNode {
	MapNode<T, T2> *prev;
	MapNode<T, T2> *next;
	MapNode<T, T2> *head;
	std::pair<T, T2> data;
};

#define FONT_HEIGHT 16
#define TEXTURE_SIZE 0x200

#ifdef _DEBUG
#define _DEBUG_BUILD
#endif

//#define _DEBUG

#ifndef _DEBUG
#define puts(x)
#define printf(...)
#endif

static bool displayCards = true;
static int (SokuLib::Title::*s_originalTitleOnProcess)();
static int (SokuLib::Select::*s_originalSelectOnProcess)();
static int (SokuLib::SelectClient::*s_originalSelectCLOnProcess)();
static int (SokuLib::SelectServer::*s_originalSelectSVOnProcess)();
static int (SokuLib::Select::*s_originalSelectOnRender)();
static int (SokuLib::SelectClient::*s_originalSelectCLOnRender)();
static int (SokuLib::SelectServer::*s_originalSelectSVOnRender)();
static int (SokuLib::ObjectSelect::*s_originalInputMgrGet)();
static int (SokuLib::ProfileDeckEdit::*s_originalCProfileDeckEdit_OnProcess)();
static int (SokuLib::ProfileDeckEdit::*s_originalCProfileDeckEdit_OnRender)();
static void (*s_originalDrawGradiantBar)(float param1, float param2, float param3);
static int (__stdcall *realSendTo)(SOCKET s, char *buf, int len, int flags, sockaddr *to, int tolen);
static BOOL (__stdcall *realMoveFileA)(LPCSTR lpExistingFileName, LPCSTR lpNewFileName);
static SokuLib::Select *(SokuLib::Select::*og_CSelect_Init_0041e55f)();
static SokuLib::Select *(SokuLib::Select::*og_CSelect_Init_0041e263)();
static SokuLib::Select *(SokuLib::Select::*og_CSelect_Init_0041e2c3)();
static SokuLib::SelectServer *(SokuLib::SelectServer::*og_CSelectSV_Init)();
static SokuLib::SelectClient *(SokuLib::SelectClient::*og_CSelectCL_Init)();
static SokuLib::ProfileDeckEdit *(SokuLib::ProfileDeckEdit::*s_originalCProfileDeckEdit_Destructor)(unsigned char param);
static SokuLib::ProfileDeckEdit *(SokuLib::ProfileDeckEdit::*og_CProfileDeckEdit_Init)(int param_2, int param_3, SokuLib::Sprite *param_4);
static std::random_device random;

std::map<unsigned char, std::map<unsigned short, SokuLib::DrawUtils::Sprite>> cardsTextures;
std::map<unsigned, std::vector<unsigned short>> characterSpellCards;
std::map<unsigned, std::array<unsigned short, 20>> defaultDecks;
std::map<unsigned, std::string> names{
	{ SokuLib::CHARACTER_REIMU, "reimu" },
	{ SokuLib::CHARACTER_MARISA, "marisa" },
	{ SokuLib::CHARACTER_SAKUYA, "sakuya" },
	{ SokuLib::CHARACTER_ALICE, "alice" },
	{ SokuLib::CHARACTER_PATCHOULI, "patchouli" },
	{ SokuLib::CHARACTER_YOUMU, "youmu" },
	{ SokuLib::CHARACTER_REMILIA, "remilia" },
	{ SokuLib::CHARACTER_YUYUKO, "yuyuko" },
	{ SokuLib::CHARACTER_YUKARI, "yukari" },
	{ SokuLib::CHARACTER_SUIKA, "suika" },
	{ SokuLib::CHARACTER_REISEN, "udonge" },
	{ SokuLib::CHARACTER_AYA, "aya" },
	{ SokuLib::CHARACTER_KOMACHI, "komachi" },
	{ SokuLib::CHARACTER_IKU, "iku" },
	{ SokuLib::CHARACTER_TENSHI, "tenshi" },
	{ SokuLib::CHARACTER_SANAE, "sanae" },
	{ SokuLib::CHARACTER_CIRNO, "chirno" },
	{ SokuLib::CHARACTER_MEILING, "meirin" },
	{ SokuLib::CHARACTER_UTSUHO, "utsuho" },
	{ SokuLib::CHARACTER_SUWAKO, "suwako" },
};

struct Deck {
	std::string name;
	std::array<unsigned short, 20> cards;
};

struct Guide {
	bool active = false;
	SokuLib::DrawUtils::Sprite sprite;
	unsigned char alpha = 0;
};

static HMODULE myModule;
static char profilePath[1024 + MAX_PATH];
static char editingBoxObject[0x164];
static bool hasSoku2 = false;
static bool escPressed = false;
static bool copyBoxDisplayed = false;
static bool renameBoxDisplayed = false;
static bool deleteBoxDisplayed = false;
static bool generated = false;
static bool saveError = false;
static bool side = false;
static bool fontLoaded = false;
static bool assetsLoaded = false;
static bool profileSelectReady = false;
static char nameBuffer[64];
static unsigned char copyBoxSelectedItem = 0;
static unsigned char deleteBoxSelectedItem = 0;
static unsigned char errorCounter = 0;
static unsigned upSelectedDeck = 0;
static unsigned downSelectedDeck = 0;
static unsigned editSelectedDeck = 0;
static unsigned editSelectedProfile = 0;
static std::string lastLoadedProfile;
static std::string leftLoadedProfile;
static std::string rightLoadedProfile;
static SokuLib::SWRFont font;
static SokuLib::SWRFont defaultFont;
static std::map<unsigned char, unsigned> nbSkills{
	{ SokuLib::CHARACTER_REIMU, 12 },
	{ SokuLib::CHARACTER_MARISA, 12 },
	{ SokuLib::CHARACTER_SAKUYA, 12 },
	{ SokuLib::CHARACTER_ALICE, 12 },
	{ SokuLib::CHARACTER_PATCHOULI, 15 },
	{ SokuLib::CHARACTER_YOUMU, 12 },
	{ SokuLib::CHARACTER_REMILIA, 12 },
	{ SokuLib::CHARACTER_YUYUKO, 12 },
	{ SokuLib::CHARACTER_YUKARI, 12 },
	{ SokuLib::CHARACTER_SUIKA, 12 },
	{ SokuLib::CHARACTER_REISEN, 12 },
	{ SokuLib::CHARACTER_AYA, 12 },
	{ SokuLib::CHARACTER_KOMACHI, 12 },
	{ SokuLib::CHARACTER_IKU, 12 },
	{ SokuLib::CHARACTER_TENSHI, 12 },
	{ SokuLib::CHARACTER_SANAE, 12 },
	{ SokuLib::CHARACTER_CIRNO, 12 },
	{ SokuLib::CHARACTER_MEILING, 12 },
	{ SokuLib::CHARACTER_UTSUHO, 12 },
	{ SokuLib::CHARACTER_SUWAKO, 12 }
};
static std::array<std::map<unsigned char, std::vector<Deck>>, 3> loadedDecks;
static std::vector<Deck> editedDecks;
static SokuLib::Character lastLeft;
static SokuLib::Character lastRight;
static SokuLib::DrawUtils::Sprite arrowSprite;
static SokuLib::DrawUtils::Sprite baseSprite;
static SokuLib::DrawUtils::Sprite nameSprite;
static SokuLib::DrawUtils::Sprite noSprite;
static SokuLib::DrawUtils::Sprite noSelectedSprite;
static SokuLib::DrawUtils::Sprite yesSprite;
static SokuLib::DrawUtils::Sprite yesSelectedSprite;
static Guide createDeckGuide;
static Guide selectDeckGuide;
static Guide editBoxGuide;
static std::unique_ptr<std::array<unsigned short, 20>> fakeLeftDeck;
static std::unique_ptr<std::array<unsigned short, 20>> fakeRightDeck;

int *CTextureManager_LoadTextureFromResource(int *ret, HMODULE hSrcModule, LPCTSTR pSrcResource) {
	int id = 0;
	long int result;
	D3DXIMAGE_INFO info;

	printf("Loading resource %p from module %p\n", pSrcResource, hSrcModule);
	if (FAILED(result = D3DXGetImageInfoFromResource(hSrcModule, pSrcResource, &info))) {
		fprintf(stderr, "D3DXGetImageInfoFromResource(%p, %p, %p) failed with code %li.\n", hSrcModule, pSrcResource, &info, result);
		*ret = 0;
		return ret;
	}

	LPDIRECT3DTEXTURE9 *pphandle = SokuLib::textureMgr.allocate(&id);

	*pphandle = nullptr;
	if (SUCCEEDED(D3DXCreateTextureFromResourceEx(
		SokuLib::pd3dDev,
		hSrcModule,
		pSrcResource,
		info.Width,
		info.Height,
		info.MipLevels,
		D3DUSAGE_RENDERTARGET,
		info.Format,
		D3DPOOL_DEFAULT,
		D3DX_DEFAULT,
		D3DX_DEFAULT,
		0,
		&info,
		nullptr,
		pphandle
	))) {
		*ret = id;
	} else {
		SokuLib::textureMgr.deallocate(id);
		*ret = 0;
	}
	return ret;
}

unsigned short getRandomCard(const std::vector<unsigned short> &list, const std::map<unsigned short, unsigned char> &other)
{
	unsigned short card;

	try {
		do
			card = list[random() % list.size()];
		while (other.at(card) >= 4);
	} catch (std::out_of_range &) {}
	return card;
}

void generateFakeDeck(SokuLib::Character chr, SokuLib::Character lastChr, const std::array<unsigned short, 20> *base, std::unique_ptr<std::array<unsigned short, 20>> &buffer)
{
	if (!base) {
		buffer.reset();
		return;
	}

	unsigned last = 100 + nbSkills[chr];
	std::map<unsigned short, unsigned char> used;
	unsigned char c = 0;
	int index = 0;

	buffer = std::make_unique<std::array<unsigned short, 20>>();
	for (int i = 0; i < 20; i++)
		if ((*base)[i] == 21)
			c++;
		else {
			(*buffer)[index] = (*base)[i];
			if (used.find((*buffer)[index]) == used.end())
				used[(*buffer)[index]] = 1;
			else
				used[(*buffer)[index]]++;
			index++;
		}
	if (!c)
		return;

	std::vector<unsigned short> cards;

	for (int i = 0; i < 21; i++)
		cards.push_back(i);
	for (int i = 100; i < last; i++)
		cards.push_back(i);
	for (auto &card : characterSpellCards[chr])
		cards.push_back(card);

	while (c--) {
		(*buffer)[19 - c] = getRandomCard(cards, used);
		if (used.find((*buffer)[19 - c]) == used.end())
			used[(*buffer)[19 - c]] = 1;
		else
			used[(*buffer)[19 - c]]++;
	}
}

void generateFakeDeck(SokuLib::Character chr, SokuLib::Character lastChr, unsigned id, const std::vector<Deck> &bases, std::unique_ptr<std::array<unsigned short, 20>> &buffer)
{
	std::array<unsigned short, 20> randomDeck{21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21};

	if (lastChr == SokuLib::CHARACTER_RANDOM) {
		if (bases.empty())
			return generateFakeDeck(chr, chr, &defaultDecks[chr], buffer);
		return generateFakeDeck(chr, lastChr, &bases[random() % bases.size()].cards, buffer);
	}
	if (id == bases.size())
		return generateFakeDeck(chr, lastChr, &defaultDecks[chr], buffer);
	if (id == bases.size() + 1)
		return generateFakeDeck(chr, lastChr, nullptr, buffer);
	if (id == bases.size() + 2)
		return generateFakeDeck(chr, lastChr, &randomDeck, buffer);
	if (id == bases.size() + 3)
		return generateFakeDeck(chr, lastChr, &bases[random() % bases.size()].cards, buffer);
	return generateFakeDeck(chr, lastChr, &bases[id].cards, buffer);
}

void loadSoku2CSV(LPWSTR path)
{
	std::ifstream stream{path};
	std::string line;

	printf("Loading character CSV from %S\n", path);
	if (stream.fail()) {
		printf("%S: %s\n", path, strerror(errno));
		return;
	}
	while (std::getline(stream, line)) {
		std::stringstream str{line};
		unsigned id;
		std::string idStr;
		std::string codeName;
		std::string shortName;
		std::string fullName;
		std::string skills;

		std::getline(str, idStr, ';');
		std::getline(str, codeName, ';');
		std::getline(str, shortName, ';');
		std::getline(str, fullName, ';');
		std::getline(str, skills, '\n');
		if (str.fail()) {
			printf("Skipping line %s: Stream failed\n", line.c_str());
			continue;
		}
		try {
			id = std::stoi(idStr);
		} catch (...){
			printf("Skipping line %s: Invalid id\n", line.c_str());
			continue;
		}
		names[id] = codeName;
		nbSkills[id] = (std::count(skills.begin(), skills.end(), ',') + 1 - skills.empty()) * 3;
		printf("%s has %i skills\n", codeName.c_str(), nbSkills[id]);
	}
}

void loadSoku2Config()
{
	puts("Looking for Soku2 config...");

	int argc;
	wchar_t app_path[MAX_PATH];
	wchar_t setting_path[MAX_PATH];
	wchar_t **arg_list = CommandLineToArgvW(GetCommandLineW(), &argc);

	wcsncpy(app_path, arg_list[0], MAX_PATH);
	PathRemoveFileSpecW(app_path);
	if (GetEnvironmentVariableW(L"SWRSTOYS", setting_path, sizeof(setting_path)) <= 0) {
		if (arg_list && argc > 1 && StrStrIW(arg_list[1], L"ini")) {
			wcscpy(setting_path, arg_list[1]);
			LocalFree(arg_list);
		} else {
			wcscpy(setting_path, app_path);
			PathAppendW(setting_path, L"\\SWRSToys.ini");
		}
		if (arg_list) {
			LocalFree(arg_list);
		}
	}
	printf("Config file is %S\n", setting_path);

	wchar_t moduleKeys[1024];
	wchar_t moduleValue[MAX_PATH];
	GetPrivateProfileStringW(L"Module", nullptr, nullptr, moduleKeys, sizeof(moduleKeys), setting_path);
	for (wchar_t *key = moduleKeys; *key; key += wcslen(key) + 1) {
		wchar_t module_path[MAX_PATH];

		GetPrivateProfileStringW(L"Module", key, nullptr, moduleValue, sizeof(moduleValue), setting_path);

		wchar_t *filename = wcsrchr(moduleValue, '/');

		printf("Check %S\n", moduleValue);
		if (!filename)
			filename = app_path;
		else
			filename++;
		for (int i = 0; filename[i]; i++)
			filename[i] = tolower(filename[i]);
		if (wcscmp(filename, L"soku2.dll") != 0)
			continue;

		hasSoku2 = true;
		wcscpy(module_path, app_path);
		PathAppendW(module_path, moduleValue);
		while (auto result = wcschr(module_path, '/'))
			*result = '\\';
		printf("Soku2 dll is at %S\n", module_path);
		PathRemoveFileSpecW(module_path);
		printf("Found Soku2 module folder at %S\n", module_path);
		PathAppendW(module_path, L"\\config\\info\\characters.csv");
		loadSoku2CSV(module_path);
		return;
	}
}

void generateFakeDecks()
{
	if (generated)
		return;
	generated = true;
	if (SokuLib::mainMode != SokuLib::BATTLE_MODE_VSSERVER)
		generateFakeDeck(SokuLib::leftChar,  lastLeft,  upSelectedDeck,  loadedDecks[0][SokuLib::leftChar], fakeLeftDeck);
	if (SokuLib::mainMode != SokuLib::BATTLE_MODE_VSCLIENT)
		generateFakeDeck(SokuLib::rightChar, lastRight, downSelectedDeck, loadedDecks[SokuLib::mainMode != SokuLib::BATTLE_MODE_VSSERVER][SokuLib::rightChar], fakeRightDeck);
}

void fillSokuDeck(SokuLib::Dequeue<unsigned short> &sokuDeck, const std::array<unsigned short, 20> &deck)
{
	if (!sokuDeck.data) {
		sokuDeck.data = SokuLib::New<unsigned short *>(8);
		memset(sokuDeck.data, 0, 8 * sizeof(*sokuDeck.data));
		sokuDeck.chunkSize = 8;
		*sokuDeck.data = SokuLib::New<unsigned short>(8);
		sokuDeck.size = 8;
	}
	while (sokuDeck.size < 20) {
		sokuDeck.data[(sokuDeck.size - 1) / 8 + 1] = SokuLib::New<unsigned short>(8);
		sokuDeck.size += 8;
	}
	assert(sokuDeck.size >= 16);
	for (int i = 0; i < 20; i++)
		sokuDeck[i] = deck[i];
	sokuDeck.size = 20;
}

void fillSokuDeck(SokuLib::Dequeue<unsigned short> &sokuDeck, const std::unique_ptr<std::array<unsigned short, 20>> &deck)
{
	if (deck)
		return fillSokuDeck(sokuDeck, *deck);
	sokuDeck.size = 0;
}

static bool saveProfile(const std::string &path, const std::map<unsigned char, std::vector<Deck>> &profile)
{
	nlohmann::json result;

	printf("Saving to %s\n", path.c_str());
	for (auto &elem : profile) {
		if (elem.second.empty())
			continue;

		auto &array = result[std::to_string(elem.first)];

		array = nlohmann::json::array();
		for (auto &deck : elem.second) {
			array.push_back({
				{"name", deck.name},
				{"cards", std::vector<unsigned short>{
					deck.cards.begin(),
					deck.cards.end()
				}}
			});
		}
	}
	if (std::ifstream(path + ".bck").fail())
		rename(path.c_str(), (path + ".bck").c_str());

	auto resultStr = result.dump(4);
	std::ofstream stream{path};

	if (stream.fail()) {
		MessageBoxA(SokuLib::window, ("Cannot open \"" + lastLoadedProfile + "\". Please make sure you have proper permissions and enough space on disk.").c_str(), "Saving error", MB_ICONERROR);
		return false;
	}
	stream << resultStr;
	if (stream.fail()) {
		stream.close();
		MessageBoxA(SokuLib::window, ("Cannot write to \"" + lastLoadedProfile + "\". Please make sure you have proper enough space on disk.").c_str(), "Saving error", MB_ICONERROR);
		return false;
	}
	stream.close();
	unlink((path + ".bck").c_str());
	return true;
}

int __stdcall mySendTo(SOCKET s, char *buf, int len, int flags, sockaddr *to, int tolen)
{
	auto packet = reinterpret_cast<SokuLib::Packet *>(buf);
	static bool a = false;

	//if (SokuLib::sceneId != SokuLib::SCENE_SELECTCL && SokuLib::sceneId != SokuLib::SCENE_SELECTSV && SokuLib::sceneId != SokuLib::SCENE_SELECT)
	//	return realSendTo(s, buf, len, flags, to, tolen);
	if (packet->type == SokuLib::CLIENT_GAME || packet->type == SokuLib::HOST_GAME) {
		bool needDelete = false;

		if (packet->game.event.type == SokuLib::GAME_MATCH) {
			generateFakeDecks();

			auto &fake = (SokuLib::mainMode == SokuLib::BATTLE_MODE_VSCLIENT ? fakeLeftDeck : fakeRightDeck);
			auto &replace = (SokuLib::mainMode == SokuLib::BATTLE_MODE_VSCLIENT ? packet->game.event.match.host : packet->game.event.match.client());

			if (fake)
				memcpy(replace.cards, fake->data(), fake->size() * sizeof(*fake->data()));
			else //We just send an invalid deck over if we want no decks
				memset(replace.cards, 0, 40);
		}
		if (!packet->game.event.input.inputs[0].charSelect.Z)
			a = false;
		if (packet->game.event.type == SokuLib::GAME_INPUT && packet->game.event.input.sceneId == SokuLib::SCENEID_CHARACTER_SELECT) {
			auto &scene = SokuLib::currentScene->to<SokuLib::Select>();

			if (scene.leftSelectionStage  != 1 && SokuLib::mainMode == SokuLib::BATTLE_MODE_VSCLIENT) {
				a = true;
				return realSendTo(s, buf, len, flags, to, tolen);
			}
			if (scene.rightSelectionStage != 1 && SokuLib::mainMode == SokuLib::BATTLE_MODE_VSSERVER) {
				a = true;
				return realSendTo(s, buf, len, flags, to, tolen);
			}

			char *buffer = new char[len];

			memcpy(buffer, buf, len);
			packet = reinterpret_cast<SokuLib::Packet *>(buffer);
			packet->game.event.input.inputs[0].charSelect.A |= packet->game.event.input.inputs[0].charSelect.Z && !a;
			packet->game.event.input.inputs[0].charSelect.Z = false;
			packet->game.event.input.inputs[0].charSelect.left = false;
			packet->game.event.input.inputs[0].charSelect.right = false;

			int bytes = realSendTo(s, buffer, len, flags, to, tolen);

			delete[] buffer;
			return bytes;
		}
	}
	return realSendTo(s, buf, len, flags, to, tolen);
}

static void sanitizeDeck(SokuLib::Character chr, Deck &deck)
{
	unsigned last = 100 + nbSkills[chr];
	std::map<unsigned short, unsigned char> used;
	std::vector<unsigned short> cards;
	auto name = names.find(chr);

	if (name == names.end())
		return;
	for (int i = 0; i <= 21; i++)
		cards.push_back(i);
	for (int i = 100; i < last; i++)
		cards.push_back(i);
	for (auto &card : characterSpellCards[chr])
		cards.push_back(card);
	for (int i = 0; i < 20; i++) {
		auto &card = deck.cards[i];

		if (card == 21)
			continue;
		if (std::find(cards.begin(), cards.end(), card) == cards.end() || used[card] >= 4) {
			card = 21;
			continue;
		}
		used[card]++;
	}
}

static bool loadOldProfileFile(nlohmann::json &json, std::map<unsigned char, std::vector<Deck>> &map, int index)
{
	if (json.size() != 20)
		throw std::invalid_argument("Not 20 characters");
	for (auto &arr : json) {
		for (auto &elem : arr) {
			elem.contains("name") && (elem["name"].get<std::string>(), true);
			if (!elem.contains("cards") || elem["cards"].get<std::vector<unsigned short>>().size() != 20)
				throw std::invalid_argument(elem.dump());
		}
	}
	for (auto &elem : map)
		elem.second.clear();
	for (int i = 0; i < 20; i++) {
		auto &array = json[i];

		for (int j = 0; j < array.size(); j++) {
			auto &elem = array[j];
			Deck deck;

			if (!elem.contains("name"))
				deck.name = "Deck #" + std::to_string(j + 1);
			else
				deck.name = elem["name"];
			memcpy(deck.cards.data(), elem["cards"].get<std::vector<unsigned short>>().data(), sizeof(*deck.cards.data()) * deck.cards.size());
			sanitizeDeck(static_cast<SokuLib::Character>(i), deck);
			std::sort(deck.cards.begin(), deck.cards.end());
			map[i].push_back(deck);
		}
		if (index == 2)
			map[i].push_back({"Create new deck", {21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21}});
	}
	return true;
}

bool allDecksDefault(unsigned short (*decks)[4][20], unsigned i)
{
	for (int j = 0; j < 4; j++) {
		std::sort(decks[i][j], decks[i][j] + 20);
		if (i >= (hasSoku2 ? defaultDecks.size() + 2 : 20) || memcmp(defaultDecks[i].data(), decks[i][j], sizeof(defaultDecks[i])) != 0)
			return false;
	}
	return true;
}

static void convertProfile(const char *jsonPath)
{
	char path[MAX_PATH];
	unsigned char length;
	unsigned short cards[255];
	FILE *json;
	FILE *profile;
	unsigned short (*decks)[4][20] = nullptr;
	int size;

	strcpy(path, jsonPath);
	*strrchr(path, '.') = 0;
	strcat(path, ".pf");
	printf("Loading decks from profile file %s to %s.\n", path, jsonPath);

	profile = fopen(path, "r");
	if (!profile) {
		printf("Can't open %s for reading %s\n", path, strerror(errno));
		return;
	}

	json = fopen(jsonPath, "w");
	if (!json) {
		fclose(profile);
		printf("Can't open %s for writing %s\n", jsonPath, strerror(errno));
		return;
	}

	fseek(profile, 106, SEEK_SET);
	for (size = 1; !feof(profile); size++) {
		decks = static_cast<unsigned short (*)[4][20]>(realloc(decks, sizeof(*decks) * size));
		for (int k = 0; k < 4; k++) {
			fread(&length, sizeof(length), 1, profile);
			fread(cards, sizeof(*cards), length, profile);
			for (int j = length; j < 20; j++)
				cards[j] = 21;
			memcpy(decks[size - 1][k], cards, 40);
		}
	}
	fclose(profile);

	fwrite("{", 1, 1, json);

	const char *deckNames[4] = {
		"yorokobi",
		"ikari",
		"ai",
		"tanoshii"
	};
	bool first = true;
	bool first2 = true;
	unsigned i = 0;

	size -= 2;
	if (size > 20)
		size -= 2;
	printf("There are %i characters...\n", size);
	while (size--) {
		if (allDecksDefault(decks, i)) {
			printf("Character %i has all default decks\n", i);
			i++;
			if (i == 20)
				i += 2;
			continue;
		}
		fprintf(json, "%s\n\t\"%i\": [", first2 ? "" : ",", i);
		first2 = false;
		first = true;
		for (int j = 0; j < 4; j++) {
			std::sort(decks[i][j], decks[i][j] + 20);
			if (i < (hasSoku2 ? defaultDecks.size() + 2 : 20) && memcmp(defaultDecks[i].data(), decks[i][j], sizeof(defaultDecks[i])) == 0)
				continue;
			fprintf(json, "%s\n\t\t{\n\t\t\t\"name\": \"%s\",\n\t\t\t\"cards\": [", first ? "" : ",", deckNames[j]);
			first = false;
			for (int k = 0; k < 20; k++)
				fprintf(json, "%s%i", k == 0 ? "" : ", ", decks[i][j][k]);
			fwrite("]\n\t\t}", 1, 5, json);
		}
		fwrite("\n\t]", 1, 3, json);
		i++;
		if (i == 20)
			i += 2;
	}
	fwrite("\n}", 1, 2, json);
	fclose(json);
	free(decks);
	decks = nullptr;
}

static bool loadProfileFile(const std::string &path, std::ifstream &stream, std::map<unsigned char, std::vector<Deck>> &map, int index, bool hasBackup = false)
{
	if (stream.fail()) {
		printf("Failed to open file %s: %s\n", path.c_str(), strerror(errno));
		if (hasBackup)
			throw std::exception();
		if (errno == ENOENT) {
			puts("Let's fix that");
			convertProfile(path.c_str());
			stream.open(path);
			return loadProfileFile(path, stream, map, index);
		}
		for (auto &elem : names)
			map[elem.first].clear();
		if (index == 2)
			for (auto &elem : map)
				elem.second.push_back({"Create new deck", {21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21}});
		return false;
	}

	nlohmann::json json;

	stream >> json;
	if (json.is_array()) {
		printf("%s is in the old format. Converting...\n", path.c_str());
		loadOldProfileFile(json, map, index);
		stream.close();
		saveProfile(path, map);
		return true;
	}
	if (json.is_null()) {
		for (auto &elem : names)
			map[elem.first].clear();
		if (index == 2)
			for (auto &elem : map)
				elem.second.push_back({"Create new deck", {21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21}});
		return true;
	}
	if (!json.is_object())
		throw std::invalid_argument("JSON is neither an array nor an object");
	for (auto &arr : json.items()) {
		std::stoi(arr.key());
		for (auto &elem : arr.value()) {
			elem.contains("name") && (elem["name"].get<std::string>(), true);
			if (!elem.contains("cards") || elem["cards"].get<std::vector<unsigned short>>().size() != 20)
				throw std::invalid_argument(elem.dump());
		}
	}
	for (auto &elem : names)
		map[elem.first].clear();
	for (auto &arr : json.items()) {
		auto &array = arr.value();
		auto index = std::stoi(arr.key());

		for (int j = 0; j < array.size(); j++) {
			auto &elem = array[j];
			Deck deck;

			if (!elem.contains("name"))
				deck.name = "Deck #" + std::to_string(j + 1);
			else
				deck.name = elem["name"];
			memcpy(deck.cards.data(), elem["cards"].get<std::vector<unsigned short>>().data(), sizeof(*deck.cards.data()) * deck.cards.size());
			sanitizeDeck(static_cast<SokuLib::Character>(index), deck);
			std::sort(deck.cards.begin(), deck.cards.end());
			map[index].push_back(deck);
		}
	}
	if (index == 2)
		for (auto &elem : map)
			elem.second.push_back({"Create new deck", {21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21}});
	return true;
}

static int weirdRand(int key, int delay)
{
	static std::map<int, std::pair<int, int>> elems;
	auto it = elems.find(key);

	if (it == elems.end() || it->second.first == 0) {
		int v = random();

		elems[key] = {delay, v};
		return v;
	}
	it->second.first--;
	return it->second.second;
}

static void initFont()
{
	SokuLib::FontDescription desc;

	desc.r1 = 255;
	desc.r2 = 255;
	desc.g1 = 255;
	desc.g2 = 255;
	desc.b1 = 255;
	desc.b2 = 255;
	desc.height = FONT_HEIGHT;
	desc.weight = FW_BOLD;
	desc.italic = 0;
	desc.shadow = 2;
	desc.bufferSize = 1000000;
	desc.charSpaceX = 0;
	desc.charSpaceY = 0;
	desc.offsetX = 0;
	desc.offsetY = 0;
	desc.useOffset = 0;
	strcpy(desc.faceName, "Tahoma");
	font.create();
	font.setIndirect(desc);
	strcpy(desc.faceName, SokuLib::defaultFontName);
	desc.weight = FW_REGULAR;
	defaultFont.create();
	defaultFont.setIndirect(desc);
}

bool saveDeckFromGame(SokuLib::ProfileDeckEdit *This, std::array<unsigned short, 20> &deck)
{
	unsigned index = 0;

	for (auto &pair : *This->editedDeck) {
		auto i = pair.second;

		while (i) {
			if (index == 20)
				return false;
			deck[index] = pair.first;
			i--;
			index++;
		}
	}
	while (index < 20) {
		deck[index] = 21;
		index++;
	}
	std::sort(deck.begin(), deck.end());
	return true;
}

static void onDeckSaved()
{
	auto menu = SokuLib::getMenuObj<SokuLib::ProfileDeckEdit>();
	std::string path;

	if (!saveDeckFromGame(menu, editedDecks[editSelectedDeck].cards))
		return;

	loadedDecks[2][menu->editedCharacter] = editedDecks;

	auto toSave = loadedDecks[2];

	for (auto &elem : toSave)
		elem.second.pop_back();
	if (editSelectedProfile != 2) {
		loadedDecks[editSelectedProfile] = toSave;
		path = editSelectedProfile == 0 ? leftLoadedProfile : rightLoadedProfile;
	} else
		path = lastLoadedProfile;

	if (!saveProfile(path, toSave)) {
		if (menu->displayedNumberOfCards == 20) {
			menu->editedDeck->begin()->second++;
			saveError = true;
		}
		return;
	}

	for (auto &card : *menu->editedDeck)
		card.second = 0;
	for (int i = 0; i < 5; i++)
		menu->editedDeck->operator[](i) = 4;
}

void renderDeck(SokuLib::Character chr, unsigned select, const std::vector<Deck> &decks, SokuLib::Vector2i pos, const char *overridingName = nullptr)
{
	std::vector<unsigned short> deck;
	std::string name;
	SokuLib::Vector2i base = pos;

	if (select == decks.size()) {
		name = "Default deck";
		deck.resize(20, 21);
		memcpy(deck.data(), defaultDecks[chr].data(), 40);
	} else if (select == decks.size() + 1) {
		name = "No deck";
		deck.resize(0);
	} else if (select == decks.size() + 2) {
		name = "Randomized deck";
		deck.resize(20, 21);
	} else if (select == decks.size() + 3 && !decks.empty())
		return renderDeck(chr, weirdRand((int)&decks, 3) % decks.size(), decks, pos, "Any deck");
	else if (select <= decks.size()) {
		name = decks[select].name;
		deck = {decks[select].cards.begin(), decks[select].cards.end()};
	}

	if (overridingName)
		name = overridingName;

	if (!deck.empty() && displayCards) {
		for (int i = 0; i < 10; i++) {
			SokuLib::DrawUtils::Sprite &sprite = (deck[i] < 100 ? cardsTextures[SokuLib::CHARACTER_RANDOM][deck[i]] : cardsTextures[chr][deck[i]]);

			sprite.setPosition(pos);
			sprite.setSize({10, 16});
			sprite.rect.top = sprite.rect.width = 0;
			sprite.rect.width = sprite.texture.getSize().x;
			sprite.rect.height = sprite.texture.getSize().y;
			sprite.tint = SokuLib::Color::White;
			sprite.draw();
			pos.x += 10;
		}
		pos.x = base.x;
		pos.y += 16;
		for (int i = 0; i < 10; i++) {
			SokuLib::DrawUtils::Sprite &sprite = (deck[i + 10] < 100 ? cardsTextures[SokuLib::CHARACTER_RANDOM][deck[i + 10]] : cardsTextures[chr][deck[i + 10]]);

			sprite.setPosition(pos);
			sprite.setSize({10, 16});
			sprite.rect.top = sprite.rect.width = 0;
			sprite.rect.width = sprite.texture.getSize().x;
			sprite.rect.height = sprite.texture.getSize().y;
			sprite.tint = SokuLib::Color::White;
			sprite.draw();
			pos.x += 10;
		}
	}
	pos.y = base.y + 32;

	SokuLib::DrawUtils::Sprite sprite;
	int text;
	int width = 0;

	if (!SokuLib::textureMgr.createTextTexture(&text, name.c_str(), font, TEXTURE_SIZE, FONT_HEIGHT + 18, &width, nullptr)) {
		puts("C'est vraiment pas de chance");
		return;
	}

	pos.x = base.x + 50 - width / 2;
	sprite.setPosition(pos);
	sprite.texture.setHandle(text, {TEXTURE_SIZE, FONT_HEIGHT + 18});
	sprite.setSize({TEXTURE_SIZE, FONT_HEIGHT + 18});
	sprite.rect = {
		0, 0, TEXTURE_SIZE, FONT_HEIGHT + 18
	};
	sprite.tint = SokuLib::Color::White;
	sprite.draw();
}

static int counter = 0;

int renderingCommon(int ret)
{
	std::string name;
	auto &scene = SokuLib::currentScene->to<SokuLib::Select>();

	if (
		scene.leftSelectionStage == 1 &&
		SokuLib::leftChar != SokuLib::CHARACTER_RANDOM &&
		SokuLib::mainMode != SokuLib::BATTLE_MODE_VSSERVER
	)
		renderDeck(SokuLib::leftChar,  upSelectedDeck,   loadedDecks[0][SokuLib::leftChar],  {28, 98});
	if (
		scene.rightSelectionStage == 1 &&
		SokuLib::rightChar != SokuLib::CHARACTER_RANDOM &&
		SokuLib::mainMode != SokuLib::BATTLE_MODE_VSCLIENT
	)
		renderDeck(SokuLib::rightChar, downSelectedDeck, loadedDecks[SokuLib::mainMode != SokuLib::BATTLE_MODE_VSSERVER][SokuLib::rightChar], {28, 384});
	return ret;
}

int __fastcall CSelectCL_OnRender(SokuLib::SelectClient *This) {
	return renderingCommon((This->*s_originalSelectCLOnRender)());
}

int __fastcall CSelectSV_OnRender(SokuLib::SelectServer *This) {
	return renderingCommon((This->*s_originalSelectSVOnRender)());
}

int __fastcall CSelect_OnRender(SokuLib::Select *This) {
	return renderingCommon((This->*s_originalSelectOnRender)());
}

static void loadTexture(SokuLib::DrawUtils::Texture &container, const char *path, bool shouldExist = true)
{
	int text = 0;
	SokuLib::Vector2u size;
	int *ret = SokuLib::textureMgr.loadTexture(&text, path, &size.x, &size.y);

	printf("Loading texture %s\n", path);
	if (!ret || !text) {
		puts("Couldn't load texture...");
		if (shouldExist)
			MessageBoxA(SokuLib::window, ("Cannot load game asset " + std::string(path)).c_str(), "Game texture loading failed", MB_ICONWARNING);
	}
	container.setHandle(text, size);
}

static inline void loadTexture(SokuLib::DrawUtils::Sprite &container, const char *path, bool shouldExist = true)
{
	loadTexture(container.texture, path, shouldExist);
}

static void initGuide(Guide &guide)
{
	guide.sprite.setPosition({0, 464});
	guide.sprite.setSize({640, 16});
	guide.sprite.rect.width = guide.sprite.getSize().x;
	guide.sprite.rect.height = guide.sprite.getSize().y;
	guide.sprite.rect.top = 0;
	guide.sprite.rect.left = 0;
}

static void loadCardAssets()
{
	int text;
	char buffer[128];
	SokuLib::DrawUtils::Texture tmp;

	puts("Loading card assets");
	for (int i = 0; i <= 20; i++) {
		sprintf(buffer, "data/card/common/card%03i.bmp", i);
		loadTexture(cardsTextures[SokuLib::CHARACTER_RANDOM][i], buffer);
	}
	loadTexture(cardsTextures[SokuLib::CHARACTER_RANDOM][21], "data/battle/cardFaceDown.bmp");
	for (auto &elem : names) {
		auto j = elem.first;

		for (int i = nbSkills[j]; i; i--) {
			sprintf(buffer, "data/card/%s/card%03i.bmp", names[j].c_str(), 99 + i);
			loadTexture(cardsTextures[j][99 + i], buffer);
		}
		for (auto &card : characterSpellCards.at(static_cast<SokuLib::Character>(j))) {
			sprintf(buffer, "data/card/%s/card%03i.bmp", names[j].c_str(), card);
			loadTexture(cardsTextures[j][card], buffer);
		}
	}
	loadTexture(baseSprite,         "data/menu/21_Base.bmp");
	loadTexture(nameSprite,         "data/profile/20_Name.bmp");
	loadTexture(arrowSprite,        "data/profile/deck2/sayuu.bmp");
	loadTexture(noSprite,           "data/menu/23a_No.bmp");
	loadTexture(noSelectedSprite,   "data/menu/23b_No.bmp");
	loadTexture(yesSprite,          "data/menu/22a_Yes.bmp");
	loadTexture(yesSelectedSprite,  "data/menu/22b_Yes.bmp");
	loadTexture(editBoxGuide.sprite,"data/guide/09.bmp");
	CTextureManager_LoadTextureFromResource(&text, myModule, MAKEINTRESOURCE(4));
	createDeckGuide.sprite.texture.setHandle(text, {640, 16});
	CTextureManager_LoadTextureFromResource(&text, myModule, MAKEINTRESOURCE(8));
	selectDeckGuide.sprite.texture.setHandle(text, {640, 16});
	initGuide(createDeckGuide);
	initGuide(selectDeckGuide);
	initGuide(editBoxGuide);
}

void saveDeckToProfile(SokuLib::Dequeue<unsigned short> &array, const std::unique_ptr<std::array<unsigned short, 20>> &deck)
{
	generateFakeDecks();
	printf("Saving decks to profile (%p | %p (%p))\n", &array, &deck, deck.get());
}

static int selectProcessCommon(int v)
{
	static int left = 0;
	static int right = 0;
	auto &scene = SokuLib::currentScene->to<SokuLib::Select>();

	if ((scene.leftSelect.keys && scene.leftSelect.keys->spellcard == 1) || (scene.rightSelect.keys && scene.rightSelect.keys->spellcard == 1)) {
		displayCards = !displayCards;
		SokuLib::playSEWaveBuffer(0x27);
	}
	if (scene.leftSelectionStage == 3 && scene.rightSelectionStage == 3) {
		if (counter < 60)
			counter++;
	} else {
		counter = 0;
		generated = false;
	}

	if (SokuLib::mainMode == SokuLib::BATTLE_MODE_VSSERVER && scene.rightSelectionStage == 1) {
		auto &decks = loadedDecks[0][SokuLib::rightChar];
		auto keys = reinterpret_cast<SokuLib::KeyManager *>(0x89A394);

		if (keys->keymapManager->input.horizontalAxis < 0)
			left++;
		else
			left = 0;

		if (keys->keymapManager->input.horizontalAxis > 0)
			right++;
		else
			right = 0;

		if (left == 1 || (left >= 36 && left % 6 == 0)) {
			SokuLib::playSEWaveBuffer(0x27);
			if (downSelectedDeck == 0)
				downSelectedDeck = decks.size() + 3 - decks.empty();
			else
				downSelectedDeck--;
		} else if (right == 1 || (right >= 36 && right % 6 == 0)) {
			SokuLib::playSEWaveBuffer(0x27);
			if (downSelectedDeck == decks.size() + 3 - decks.empty())
				downSelectedDeck = 0;
			else
				downSelectedDeck++;
		}
	}

	if (scene.leftRandomDeck && SokuLib::mainMode != SokuLib::BATTLE_MODE_VSSERVER) {
		if (SokuLib::mainMode != SokuLib::BATTLE_MODE_VSCLIENT)
			scene.leftRandomDeck = false;
		if (SokuLib::leftChar != SokuLib::CHARACTER_RANDOM)
			upSelectedDeck = loadedDecks[0][SokuLib::leftChar].size() + 3;
	}
	if (scene.rightRandomDeck && SokuLib::mainMode != SokuLib::BATTLE_MODE_VSCLIENT) {
		if (SokuLib::mainMode != SokuLib::BATTLE_MODE_VSSERVER)
			scene.rightRandomDeck = false;
		if (SokuLib::rightChar != SokuLib::CHARACTER_RANDOM)
			downSelectedDeck = loadedDecks[SokuLib::mainMode != SokuLib::BATTLE_MODE_VSSERVER][SokuLib::rightChar].size() + 3;
	}
	if (scene.leftSelectionStage != 3 || scene.rightSelectionStage != 3 || counter < 30) {
		if (lastLeft != SokuLib::leftChar)
			upSelectedDeck = 0;
		lastLeft = SokuLib::leftChar;
		if (lastRight != SokuLib::rightChar)
			downSelectedDeck = 0;
		lastRight = SokuLib::rightChar;
	}
	if (v == SokuLib::SCENE_LOADING || v == SokuLib::SCENE_LOADINGSV || v == SokuLib::SCENE_LOADINGCL) {
		bool pickedRandom = false;

		if (SokuLib::mainMode == SokuLib::BATTLE_MODE_VSSERVER) {
			pickedRandom = scene.rightRandomDeck || lastRight == SokuLib::CHARACTER_RANDOM;
			scene.rightRandomDeck = false;
		} else if (SokuLib::mainMode == SokuLib::BATTLE_MODE_VSCLIENT) {
			pickedRandom = scene.leftRandomDeck  || lastLeft  == SokuLib::CHARACTER_RANDOM;
			scene.leftRandomDeck = false;
		}
		printf("Picked %srandom deck\n", pickedRandom ? "" : "not ");
		generateFakeDecks();
		if (SokuLib::mainMode != SokuLib::BATTLE_MODE_VSSERVER)
			fillSokuDeck(SokuLib::leftPlayerInfo.effectiveDeck, fakeLeftDeck);
		if (SokuLib::mainMode != SokuLib::BATTLE_MODE_VSCLIENT)
			fillSokuDeck(SokuLib::rightPlayerInfo.effectiveDeck, fakeRightDeck);
		return v;
	}
	return v;
}

int __fastcall CSelectCL_OnProcess(SokuLib::SelectClient *This) {
	return selectProcessCommon((This->*s_originalSelectCLOnProcess)());
}

int __fastcall CSelectSV_OnProcess(SokuLib::SelectServer *This) {
	return selectProcessCommon((This->*s_originalSelectSVOnProcess)());
}

int __fastcall CSelect_OnProcess(SokuLib::Select *This) {
	if (!SokuLib::menuManager.empty() && *SokuLib::getMenuObj<int>() == 0x859820) {
		auto obj = SokuLib::getMenuObj<int>();
		auto selected = obj[0x1A];

		if (selected >= 2 && selected <= 3)
			editSelectedProfile = selected - 2;
		else
			editSelectedProfile = 2;
	}
	return selectProcessCommon((This->*s_originalSelectOnProcess)());
}

static void handleInput(const SokuLib::KeyInput &inputs, int index)
{
	bool isRight = index == 1;
	auto &selectedDeck = (isRight ? downSelectedDeck : upSelectedDeck);
	auto &decks = loadedDecks[index][isRight ? SokuLib::rightChar : SokuLib::leftChar];

	if (inputs.horizontalAxis < 0) {
		if (selectedDeck == 0)
			selectedDeck = decks.size() + 3 - decks.empty();
		else
			selectedDeck--;
	} else {
		if (selectedDeck == decks.size() + 3 - decks.empty())
			selectedDeck = 0;
		else
			selectedDeck++;
	}
}

int __fastcall myGetInput(SokuLib::ObjectSelect *This)
{
	int ret = (This->*s_originalInputMgrGet)();
	auto &scene = SokuLib::currentScene->to<SokuLib::Select>();
	auto &battle = SokuLib::getBattleMgr();

	if (
		(SokuLib::mainMode != SokuLib::BATTLE_MODE_VSSERVER && This == &scene.leftSelect) ||
		(SokuLib::mainMode != SokuLib::BATTLE_MODE_VSCLIENT && This == &scene.rightSelect)
	)
		This->deck = 0;
	if (SokuLib::mainMode == SokuLib::BATTLE_MODE_VSSERVER)
		return ret;
	if (ret) {
		int a = abs(This->keys->horizontalAxis);

		if (a != 1 && (a < 36 || a % 6 != 0))
			return ret;
		SokuLib::playSEWaveBuffer(0x27);
		handleInput(*This->keys, This != &scene.leftSelect);
		return 0;
	}
	return ret;
}

void drawGradiantBar(float x, float y, float maxY)
{
	if (y == 114)
		y = 90;
	s_originalDrawGradiantBar(x, y, maxY);
}

void loadDeckToGame(SokuLib::ProfileDeckEdit *This, const std::array<unsigned short, 20> &deck)
{
	int count = 0;

	This->editedDeck->clear();
	for (int i = 0; i < 20; i++)
		if (deck[i] != 21) {
			auto iter = This->editedDeck->find(deck[i]);
			if (iter == This->editedDeck->end())
				(*This->editedDeck)[deck[i]] = 1;
			else
				iter->second++;
			count++;
		}
	This->displayedNumberOfCards = count;
}

void __fastcall CProfileDeckEdit_SwitchCurrentDeck(SokuLib::ProfileDeckEdit *This, int, int DeckID)
{
	auto FUN_0044f930 = SokuLib::union_cast<void (SokuLib::ProfileDeckEdit::*)(char param_1)>(0x44F930);

	This->selectedDeck = 0;
	if (!saveDeckFromGame(This, editedDecks[editSelectedDeck].cards)) {
		errorCounter = 120;
		return;
	}
	if (DeckID == 1) {
		if (editSelectedDeck == editedDecks.size() - 1)
			editSelectedDeck = 0;
		else
			editSelectedDeck++;
	} else {
		if (editSelectedDeck == 0)
			editSelectedDeck = editedDecks.size() - 1;
		else
			editSelectedDeck--;
	}
	loadDeckToGame(This, editedDecks[editSelectedDeck].cards);
	(This->*FUN_0044f930)('\0');
}

void renameBoxRender()
{
	int text;
	int width;
	SokuLib::DrawUtils::Sprite textSprite;
	SokuLib::DrawUtils::RectangleShape rect;

	nameSprite.setPosition({160, 192});
	nameSprite.setSize(nameSprite.texture.getSize());
	nameSprite.rect = {
		0, 0,
		static_cast<int>(nameSprite.texture.getSize().x),
		static_cast<int>(nameSprite.texture.getSize().y)
	};
	nameSprite.tint = SokuLib::Color::White;
	nameSprite.draw();

	if (!SokuLib::textureMgr.createTextTexture(&text, nameBuffer, font, TEXTURE_SIZE, 18, &width, nullptr)) {
		puts("C'est vraiment pas de chance");
		return;
	}
	auto render = (int(__thiscall*) (void*, float, float))0x42a050;

	render(editingBoxObject, 276, 217);
}

void openRenameBox(SokuLib::ProfileDeckEdit *This)
{
	auto setup_global = (int(__thiscall*) (void*, bool))0x40ea10;
	auto init_fun = (void(__thiscall*) (void*, int, int))0x429e70;

	SokuLib::playSEWaveBuffer(0x28);
	if (editSelectedDeck == editedDecks.size() - 1) {
		editedDecks.back().name = "Deck #" + std::to_string(editedDecks.size());
		editedDecks.push_back({"Create new deck", {21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21}});
	}

	//:magic_wand:
	init_fun(editingBoxObject, 0x89A4F8, 24);
	setup_global((void*)0x8A02F0, true);
	strncpy((char *)0x8A02F8, editedDecks[editSelectedDeck].name.c_str(), 24);
	renameBoxDisplayed = true;
}

void renameBoxUpdate(SokuLib::KeyManager *keys)
{
	auto update = (char (__thiscall*) (void*))0x429F00;

	renameBoxDisplayed = *(bool *)0x8A0CFF;
	if (!renameBoxDisplayed)
		SokuLib::playSEWaveBuffer(0x29);
	else if (update(editingBoxObject) == 1) {
		SokuLib::playSEWaveBuffer(0x28);
		editedDecks[editSelectedDeck].name = (char *)0x8A02F8;
		renameBoxDisplayed = false;
	}
}

void deleteBoxRender()
{
	int text;
	SokuLib::DrawUtils::Sprite textSprite;
	SokuLib::DrawUtils::Sprite &yes = deleteBoxSelectedItem ? yesSelectedSprite : yesSprite;
	SokuLib::DrawUtils::Sprite &no  = deleteBoxSelectedItem ? noSprite : noSelectedSprite;

	if (deleteBoxSelectedItem == 2)
		return;

	baseSprite.setPosition({160, 192});
	baseSprite.setSize(baseSprite.texture.getSize());
	baseSprite.rect = {
		0, 0,
		static_cast<int>(baseSprite.texture.getSize().x),
		static_cast<int>(baseSprite.texture.getSize().y)
	};
	baseSprite.tint = SokuLib::Color::White;
	baseSprite.draw();

	yes.setPosition({242, 228});
	yes.setSize(yes.texture.getSize());
	yes.rect = {
		0, 0,
		static_cast<int>(yes.texture.getSize().x),
		static_cast<int>(yes.texture.getSize().y)
	};
	yes.tint = SokuLib::Color::White;
	yes.draw();

	no.setPosition({338, 228});
	no.setSize(no.texture.getSize());
	no.rect = {
		0, 0,
		static_cast<int>(no.texture.getSize().x),
		static_cast<int>(no.texture.getSize().y)
	};
	no.tint = SokuLib::Color::White;
	no.draw();

	if (!SokuLib::textureMgr.createTextTexture(&text, ("Delete deck " + editedDecks[editSelectedDeck].name + " ?").c_str(), defaultFont, TEXTURE_SIZE, FONT_HEIGHT + 18, nullptr, nullptr)) {
		puts("C'est vraiment pas de chance");
		return;
	}
	textSprite.setPosition({164, 202});
	textSprite.texture.setHandle(text, {TEXTURE_SIZE, FONT_HEIGHT + 18});
	textSprite.setSize({TEXTURE_SIZE, FONT_HEIGHT + 18});
	textSprite.rect = {
		0, 0, TEXTURE_SIZE, FONT_HEIGHT + 18
	};
	textSprite.tint = SokuLib::Color::White;
	textSprite.fillColors[2] = textSprite.fillColors[3] = SokuLib::Color::Blue;
	textSprite.draw();
}

void openDeleteBox()
{
	SokuLib::playSEWaveBuffer(0x28);
	deleteBoxDisplayed = true;
	deleteBoxSelectedItem = 0;
}

void deleteBoxUpdate(SokuLib::KeyManager *keys)
{
	auto horizontal = abs(keys->keymapManager->input.horizontalAxis);

	SokuLib::getMenuObj<SokuLib::ProfileDeckEdit>()->guideVector[2].active = true;
	if (deleteBoxSelectedItem == 2) {
		if (!keys->keymapManager->input.a)
			deleteBoxDisplayed = false;
		return;
	}
	if (keys->keymapManager->input.b || SokuLib::checkKeyOneshot(1, false, false, false)) {
		SokuLib::playSEWaveBuffer(0x29);
		deleteBoxDisplayed = false;
	}
	if (horizontal == 1 || (horizontal >= 36 && horizontal % 6 == 0)) {
		deleteBoxSelectedItem = !deleteBoxSelectedItem;
		SokuLib::playSEWaveBuffer(0x27);
	}
	if (keys->keymapManager->input.a == 1) {
		if (deleteBoxSelectedItem) {
			editedDecks.erase(editedDecks.begin() + editSelectedDeck);
			loadDeckToGame(SokuLib::getMenuObj<SokuLib::ProfileDeckEdit>(), editedDecks[editSelectedDeck].cards);
			SokuLib::playSEWaveBuffer(0x28);
		} else
			SokuLib::playSEWaveBuffer(0x29);
		deleteBoxSelectedItem = 2;
	}
}

void copyBoxRender()
{
	int text;
	int width;
	SokuLib::DrawUtils::Sprite textSprite;

	baseSprite.setPosition({160, 192});
	baseSprite.setSize(baseSprite.texture.getSize());
	baseSprite.rect = {
		0, 0,
		static_cast<int>(baseSprite.texture.getSize().x),
		static_cast<int>(baseSprite.texture.getSize().y)
	};
	baseSprite.tint = SokuLib::Color::White;
	baseSprite.draw();

	const std::string &name = copyBoxSelectedItem == editedDecks.size() - 1 ? "Default deck" : editedDecks[copyBoxSelectedItem].name;

	if (!SokuLib::textureMgr.createTextTexture(&text, name.c_str(), font, TEXTURE_SIZE, FONT_HEIGHT + 18, &width, nullptr)) {
		puts("C'est vraiment pas de chance");
		return;
	}

	constexpr float increase = 1;
	SokuLib::Vector2i pos{static_cast<int>(321 - (width / 2) * increase), 230};

	textSprite.setPosition(pos);
	textSprite.texture.setHandle(text, {TEXTURE_SIZE, FONT_HEIGHT + 18});
	textSprite.setSize({static_cast<unsigned>(TEXTURE_SIZE * increase), static_cast<unsigned>((FONT_HEIGHT + 18) * increase)});
	textSprite.rect = {
		0, 0, TEXTURE_SIZE, FONT_HEIGHT + 18
	};
	textSprite.tint = SokuLib::Color::White;
	textSprite.draw();

	pos.x -= 32 * increase;
	pos.y -= 6 * increase;
	arrowSprite.rect = {0, 0, 32, 32};
	arrowSprite.setPosition(pos);
	arrowSprite.setSize({static_cast<unsigned>(32 * increase + 1), static_cast<unsigned>(32 * increase + 1)});
	arrowSprite.tint = SokuLib::Color::White;
	arrowSprite.draw();

	pos.x += 32 * increase + width * increase;
	arrowSprite.rect.left = 32;
	arrowSprite.setPosition(pos);
	arrowSprite.draw();

	if (!SokuLib::textureMgr.createTextTexture(&text, "Choose a deck to copy", defaultFont, TEXTURE_SIZE, FONT_HEIGHT + 18, &width, nullptr)) {
		puts("C'est vraiment pas de chance");
		return;
	}
	textSprite.setPosition({166, 200});
	textSprite.texture.setHandle(text, {TEXTURE_SIZE, FONT_HEIGHT + 18});
	textSprite.fillColors[2] = textSprite.fillColors[3] = SokuLib::Color::Blue;
	textSprite.draw();
}

void openCopyBox(SokuLib::ProfileDeckEdit *This)
{
	SokuLib::playSEWaveBuffer(0x28);
	copyBoxDisplayed = true;
	copyBoxSelectedItem = 0;
}

void copyBoxUpdate(SokuLib::KeyManager *keys)
{
	if (keys->keymapManager->input.b || SokuLib::checkKeyOneshot(1, false, false, false)) {
		SokuLib::playSEWaveBuffer(0x29);
		copyBoxDisplayed = false;
	}

	auto horizontal = abs(keys->keymapManager->input.horizontalAxis);

	SokuLib::getMenuObj<SokuLib::ProfileDeckEdit>()->guideVector[2].active = true;
	if (horizontal == 1 || (horizontal >= 36 && horizontal % 6 == 0)) {
		if (keys->keymapManager->input.horizontalAxis < 0) {
			if (copyBoxSelectedItem == 0)
				copyBoxSelectedItem = editedDecks.size() - 1;
			else
				copyBoxSelectedItem--;
		} else {
			if (copyBoxSelectedItem == editedDecks.size() - 1)
				copyBoxSelectedItem = 0;
			else
				copyBoxSelectedItem++;
		}
		SokuLib::playSEWaveBuffer(0x27);
	}
	if (keys->keymapManager->input.a == 1) {
		auto menu = SokuLib::getMenuObj<SokuLib::ProfileDeckEdit>();

		editedDecks.back().name = "Deck #" + std::to_string(editedDecks.size());
		loadDeckToGame(SokuLib::getMenuObj<SokuLib::ProfileDeckEdit>(), copyBoxSelectedItem == editedDecks.size() - 1 ? defaultDecks[menu->editedCharacter] : editedDecks[copyBoxSelectedItem].cards);
		editedDecks.push_back({"Create new deck", {21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21}});
		copyBoxDisplayed = false;
		openRenameBox(menu);
	}
}

static void loadDefaultDecks()
{
	char buffer[] = "data/csv/000000000000/deck.csv";

	for (auto [id, name] : names) {
		sprintf(buffer, "data/csv/%s/deck.csv", name.c_str());
		printf("Loading default deck %s\n", buffer);

		SokuLib::CSVParser parser{buffer};
		std::array<unsigned short, 20> deck;
		auto &cards = characterSpellCards[id];

		for (int i = 0; i < 20; i++) {
			auto str = parser.getNextCell();

			try {
				auto card = std::stoul(str);

				if (card > 20 && std::find(cards.begin(), cards.end(), card) == cards.end())
					MessageBoxA(
						SokuLib::window,
						("Warning: Default deck for " + name + " contains invalid card " + str).c_str(),
						"Default deck invalid",
						MB_ICONWARNING
					);
				deck[i] = card;
			} catch (std::exception &e) {
				MessageBoxA(
					SokuLib::window,
					(
						"Fatal error: Cannot load default deck for " + name + ":\n" +
						"In file " + buffer + ": Cannot parse cell #" + std::to_string(i + 1) +
						" \"" + str + "\": " + e.what()
					).c_str(),
					"Loading default deck failed",
					MB_ICONERROR
				);
				abort();
			}
		}
		std::sort(deck.begin(), deck.end());
		defaultDecks[id] = deck;
	}
}

static void loadAllExistingCards()
{
	char buffer[] = "data/csv/000000000000/spellcard.csv";

	for (auto [id, name] : names) {
		sprintf(buffer, "data/csv/%s/spellcard.csv", name.c_str());
		printf("Loading cards from %s\n", buffer);

		SokuLib::CSVParser parser{buffer};
		std::vector<unsigned short> valid;
		int i = 0;

		do {
			auto str = parser.getNextCell();

			i++;
			try {
				valid.push_back(std::stoul(str));
			} catch (std::exception &e) {
				MessageBoxA(
					SokuLib::window,
					(
						"Fatal error: Cannot load cards list for " + name + ":\n" +
						"In file " + buffer + ": Cannot parse cell #1 at line #" + std::to_string(i) +
						" \"" + str + "\": " + e.what()
					).c_str(),
					"Loading default deck failed",
					MB_ICONERROR
				);
				abort();
			}
		} while (parser.goToNextLine());
		characterSpellCards[id] = valid;
	}
}

static void initAssets()
{
	if (assetsLoaded)
		return;
	assetsLoaded = true;
	loadAllExistingCards();
	loadDefaultDecks();
	loadCardAssets();
	initFont();
}

static void handleProfileChange(SokuLib::Profile *This, SokuLib::String *str)
{
	initAssets();

	char *arg = *str;
	std::string profileName{arg, strstr(arg, ".pf")};
	std::string profile = "profile/" + profileName + ".json";
	int index = 2;
	bool hasBackup;

	if (This == &SokuLib::profile1) {
		//P1
		index = 0;
		leftLoadedProfile = profile;
	} else if (This == &SokuLib::profile2) {
		//P2
		index = 1;
		rightLoadedProfile = profile;
	} //Else is deck construct
	printf("Loading %s in buffer %i\n", profile.c_str(), index);

	bool result = false;
	auto &arr = loadedDecks[index];
	std::ifstream stream{profile + ".bck"};

	hasBackup = !stream.fail();
	stream.close();
	if (hasBackup)
		printf("%s has backup data !\n", profile.c_str());

	stream.open(profile, std::ifstream::in);
	try {
		result = loadProfileFile(profile, stream, arr, index, hasBackup);
	} catch (std::exception &e) {
		auto answer = IDNO;

		if (!hasBackup)
			MessageBoxA(SokuLib::window, ("Cannot load file " + profile + ": " + e.what()).c_str(), "Fatal error", MB_ICONERROR);
		else
			answer = MessageBoxA(SokuLib::window, ("Cannot load file " + profile + ": " + e.what() + "\n\nDo you want to load backup file ?").c_str(), "Loading error", MB_ICONERROR | MB_YESNO);
		if (answer != IDYES) {
			try {
				result = loadProfileFile(profile, stream, arr, index);
			} catch (std::exception &e) {
				MessageBoxA(SokuLib::window, ("Cannot load file " + profile + ": " + e.what()).c_str(), "Fatal error", MB_ICONERROR);
				throw;
			}
		}
	}
	stream.close();

	if (!result && hasBackup) {
		try {
			stream.open(profile + ".bck", std::ifstream::in);
			printf("Loading %s\n", (profile + ".bck").c_str());
			result = loadProfileFile(profile + ".bck", stream, arr, index);
			stream.close();
		} catch (std::exception &e) {
			MessageBoxA(SokuLib::window, ("Cannot load file " + profile + ".bck: " + e.what()).c_str(), "Fatal error", MB_ICONERROR);
			throw;
		}
		unlink(profile.c_str());
		rename((profile + ".bck").c_str(), profile.c_str());
		lastLoadedProfile = profile;
		return;
	}

	lastLoadedProfile = profile;
	if (hasBackup)
		unlink((profile + ".bck").c_str());
}

static void onProfileChanged()
{
	SokuLib::Profile *This;
	char *arg;

	__asm mov This, esi;
	__asm mov arg, esp;

#ifdef _DEBUG_BUILD
	arg = *(char **)(arg + 0x28);
#else
	arg = *(char **)(arg + 0x24);
#endif
	arg += 0x1C;
	handleProfileChange(This, reinterpret_cast<SokuLib::String *>(arg));
}

SokuLib::ProfileDeckEdit *__fastcall CProfileDeckEdit_Init(SokuLib::ProfileDeckEdit *This, int, int param_2, int param_3, SokuLib::Sprite *param_4)
{
	auto ret = (This->*og_CProfileDeckEdit_Init)(param_2, param_3, param_4);

	if (profileSelectReady)
		return ret;
	profileSelectReady = true;
	errorCounter = 0;
	editSelectedDeck = 0;
	if (editSelectedProfile != 2) {
		loadedDecks[2] = loadedDecks[editSelectedProfile];
		for (auto &val : loadedDecks[2])
			val.second.push_back({"Create new deck", {21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21}});
	} else if (loadedDecks[2][This->editedCharacter].empty()) {
		loadedDecks[2] = loadedDecks[0];
		for (auto &val : loadedDecks[2])
			val.second.push_back({"Create new deck", {21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21}});
	}
	editedDecks = loadedDecks[2][This->editedCharacter];
	loadDeckToGame(This, editedDecks[editSelectedDeck].cards);
	deleteBoxDisplayed = false;
	renameBoxDisplayed = false;
	copyBoxDisplayed = false;
	return ret;
}

void updateGuide(Guide &guide)
{
	if (guide.active && guide.alpha != 255)
		guide.alpha += 15;
	else if (!guide.active && guide.alpha)
		guide.alpha -= 15;
}

void renderGuide(Guide &guide)
{
	guide.sprite.tint.a = guide.alpha;
	guide.sprite.draw();
}

int __fastcall CProfileDeckEdit_OnProcess(SokuLib::ProfileDeckEdit *This)
{
	auto keys = reinterpret_cast<SokuLib::KeyManager *>(0x89A394);
	bool ogDialogsActive = This->guideVector[3].active || This->guideVector[2].active;

	profileSelectReady = false;
	selectDeckGuide.active = This->cursorOnDeckChangeBox && editSelectedDeck != editedDecks.size() - 1 && !ogDialogsActive;
	createDeckGuide.active = This->cursorOnDeckChangeBox && editSelectedDeck == editedDecks.size() - 1 && !ogDialogsActive;
	This->guideVector[4].active = false;
	if (renameBoxDisplayed && ogDialogsActive) {
		renameBoxDisplayed = false;
		((int(__thiscall*) (void*, bool))0x40ea10)((void*)0x8A02F0, true);
	}
	deleteBoxDisplayed &= !ogDialogsActive;
	copyBoxDisplayed   &= !ogDialogsActive;

	bool renameBox = renameBoxDisplayed;
	bool deleteBox = deleteBoxDisplayed;
	bool copyBox   = copyBoxDisplayed;

	editBoxGuide.active = renameBoxDisplayed;
	if (ogDialogsActive);
	else if (renameBox)
		renameBoxUpdate(keys);
	else if (deleteBox)
		deleteBoxUpdate(keys);
	else if (copyBox)
		copyBoxUpdate(keys);
	else if (keys->keymapManager->input.a == 1 && This->cursorOnDeckChangeBox) {
		if (editSelectedDeck == editedDecks.size() - 1)
			openCopyBox(This);
		else
			openRenameBox(This);
	} else if (keys->keymapManager->input.c && This->cursorOnDeckChangeBox && editSelectedDeck != editedDecks.size() - 1)
		openDeleteBox();
	else if (keys->keymapManager->input.c && This->cursorOnDeckChangeBox) {
		editedDecks.back().name = "Deck #" + std::to_string(editedDecks.size());
		editedDecks.push_back({"Create new deck", {21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21}});
		openRenameBox(This);
	}
	if (renameBox || deleteBox || copyBox) {
		selectDeckGuide.active = false;
		createDeckGuide.active = false;
		This->guideVector[0].active = false;
		escPressed = ((int *)0x8998D8)[1];
		((int *)0x8998D8)[1] = 0;
		((int *)0x8998D8)[DIK_F1] = 0;
		memset(&keys->keymapManager->input, 0, sizeof(keys->keymapManager->input));
	} else if (escPressed) {
		escPressed = ((int *)0x8998D8)[1];
		((int *)0x8998D8)[1] = 0;
	} else if (editSelectedDeck == editedDecks.size() - 1 && This->displayedNumberOfCards != 0) {
		SokuLib::playSEWaveBuffer(0x28);
		editedDecks.back().name = "Deck #" + std::to_string(editedDecks.size());
		editedDecks.push_back({"Create new deck", {21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21}});
	}
	// This hides the deck select arrow
	((bool ***)This)[0x10][0x2][20] = false;
	updateGuide(selectDeckGuide);
	updateGuide(createDeckGuide);
	updateGuide(editBoxGuide);
	return (This->*s_originalCProfileDeckEdit_OnProcess)();
}

int __fastcall CProfileDeckEdit_OnRender(SokuLib::ProfileDeckEdit *This)
{
	int ret = (This->*s_originalCProfileDeckEdit_OnRender)();

	SokuLib::DrawUtils::Sprite &sprite = cardsTextures[SokuLib::CHARACTER_RANDOM][21];
	SokuLib::DrawUtils::Sprite textSprite;
	SokuLib::Vector2i pos{38, 88};
	int text;
	int width = 0;

	if (saveError) {
		saveError = false;
		This->editedDeck->begin()->second--;
	}

	sprite.rect.top = sprite.rect.width = 0;
	sprite.rect.width = sprite.texture.getSize().x;
	sprite.rect.height = sprite.texture.getSize().y;
	sprite.tint = SokuLib::Color::White;
	sprite.setSize({20, 32});
	for (int i = 20; i > This->displayedNumberOfCards; i--) {
		sprite.setPosition({304 + 24 * ((i - 1) % 10), 260 + 38 * ((i - 1) / 10)});
		sprite.draw();
	}

	if (errorCounter) {
		float alpha = min(1.f, errorCounter / 30.f);

		errorCounter--;
		if (!SokuLib::textureMgr.createTextTexture(&text, "Please keep the number of cards in the deck at or below 20", font, TEXTURE_SIZE, FONT_HEIGHT + 18, &width, nullptr)) {
			puts("C'est vraiment pas de chance");
			return ret;
		}

		auto realX = 53;

		if (errorCounter >= 105) {
			realX += random() % 31 - 15;
			if (errorCounter >= 115)
				alpha = 1 - (errorCounter - 115.f) / 5;
		}

		textSprite.setPosition({realX, pos.y - 20});
		textSprite.texture.setHandle(text, {TEXTURE_SIZE, FONT_HEIGHT + 18});
		textSprite.setSize({TEXTURE_SIZE, FONT_HEIGHT + 18});
		textSprite.rect = {
			0, 0, TEXTURE_SIZE, FONT_HEIGHT + 18
		};
		textSprite.tint = SokuLib::Color::Red * alpha;
		textSprite.draw();
	}

	if (!SokuLib::textureMgr.createTextTexture(&text, editedDecks[editSelectedDeck].name.c_str(), font, TEXTURE_SIZE, FONT_HEIGHT + 18, &width, nullptr)) {
		puts("C'est vraiment pas de chance");
		return ret;
	}
	pos.x = 153 - width / 2;
	textSprite.setPosition(pos);
	textSprite.texture.setHandle(text, {TEXTURE_SIZE, FONT_HEIGHT + 18});
	textSprite.setSize({TEXTURE_SIZE, FONT_HEIGHT + 18});
	textSprite.rect = {
		0, 0, TEXTURE_SIZE, FONT_HEIGHT + 18
	};
	textSprite.tint = SokuLib::Color::White;
	textSprite.draw();

	pos.x -= 32;
	pos.y -= 6;
	arrowSprite.rect = {0, 0, 32, 32};
	arrowSprite.setPosition(pos);
	arrowSprite.setSize({33, 33});
	arrowSprite.tint = SokuLib::Color::White;
	arrowSprite.draw();

	pos.x += 32 + width;
	arrowSprite.rect.left = 32;
	arrowSprite.setPosition(pos);
	arrowSprite.draw();

	renderGuide(selectDeckGuide);
	renderGuide(createDeckGuide);
	renderGuide(editBoxGuide);
	if (renameBoxDisplayed)
		renameBoxRender();
	else if (deleteBoxDisplayed)
		deleteBoxRender();
	else if (copyBoxDisplayed)
		copyBoxRender();
	return ret;
}

int __fastcall CTitle_OnProcess(SokuLib::Title *This)
{
	if (SokuLib::menuManager.empty() || *SokuLib::getMenuObj<int>() != SokuLib::ADDR_VTBL_DECK_CONSTRUCTION_CHR_SELECT_MENU)
		editSelectedProfile = 2;
	return (This->*s_originalTitleOnProcess)();
}

SokuLib::ProfileDeckEdit *__fastcall CProfileDeckEdit_Destructor(SokuLib::ProfileDeckEdit *This, int, unsigned char param)
{
	auto setup_global = (int(__thiscall*) (void*, bool))0x40ea10;

	setup_global((void*)0x8A02F0, false);
	return (This->*s_originalCProfileDeckEdit_Destructor)(param);
}

BOOL __stdcall myMoveFileA(LPCSTR lpExistingFileName, LPCSTR lpNewFileName)
{
	auto len = strlen(lpExistingFileName);

	printf("%s -> %s\n", lpExistingFileName, lpNewFileName);
	if (
		strncmp(lpExistingFileName, "profile/", strlen("profile/")) == 0 &&
		len >= 3 && strcmp(&lpExistingFileName[len - strlen(".pf")], ".pf") == 0
	) {
		auto jsonExistingFileName = new char[strlen(lpExistingFileName) + 4];
		auto jsonNewFileName = new char[strlen(lpNewFileName) + 4];

		strcpy(jsonExistingFileName, lpExistingFileName);
		strcpy(jsonNewFileName, lpNewFileName);
		strcpy(&jsonExistingFileName[len - strlen(".pf")], ".json");
		strcpy(&jsonNewFileName[strlen(lpNewFileName) - strlen(".pf")], ".json");
		printf("We need to do stuff %s -> %s\n", jsonExistingFileName, jsonNewFileName);
		if (!realMoveFileA(jsonExistingFileName, jsonNewFileName) && GetLastError() != ERROR_FILE_NOT_FOUND)
			return FALSE;
		if (!realMoveFileA(lpExistingFileName, lpNewFileName)) {
			realMoveFileA(jsonNewFileName, jsonExistingFileName);
			return FALSE;
		}
		return TRUE;
	}
	return realMoveFileA(lpExistingFileName, lpNewFileName);
}

extern "C" __declspec(dllexport) bool CheckVersion(const BYTE hash[16]) {
	return memcmp(hash, SokuLib::targetHash, 16) == 0;
}

extern "C" __declspec(dllexport) bool Initialize(HMODULE hMyModule, HMODULE hParentModule) {
	DWORD old;
	FILE *_;

	GetModuleFileName(hMyModule, profilePath, 1024);
	while (*profilePath && profilePath[strlen(profilePath) - 1] == '\\')
		profilePath[strlen(profilePath) - 1] = 0;

	auto result = strrchr(profilePath, '\\');

	if (result)
		*result = 0;
#ifdef _DEBUG
	AllocConsole();
	freopen_s(&_, "CONOUT$", "w", stdout);
	freopen_s(&_, "CONOUT$", "w", stderr);
#endif
	puts("Hey !");
	loadSoku2Config();
	::VirtualProtect((PVOID)RDATA_SECTION_OFFSET, RDATA_SECTION_SIZE, PAGE_EXECUTE_WRITECOPY, &old);
	s_originalSelectCLOnProcess          = SokuLib::TamperDword(&SokuLib::VTable_SelectClient.onProcess, CSelectCL_OnProcess);
	s_originalSelectSVOnProcess          = SokuLib::TamperDword(&SokuLib::VTable_SelectServer.onProcess, CSelectSV_OnProcess);
	s_originalSelectOnProcess            = SokuLib::TamperDword(&SokuLib::VTable_Select.onProcess, CSelect_OnProcess);
	s_originalSelectCLOnRender           = SokuLib::TamperDword(&SokuLib::VTable_SelectClient.onRender, CSelectCL_OnRender);
	s_originalSelectSVOnRender           = SokuLib::TamperDword(&SokuLib::VTable_SelectServer.onRender, CSelectSV_OnRender);
	s_originalSelectOnRender             = SokuLib::TamperDword(&SokuLib::VTable_Select.onRender, CSelect_OnRender);
	s_originalTitleOnProcess             = SokuLib::TamperDword(&SokuLib::VTable_Title.onRender, CTitle_OnProcess);
	s_originalCProfileDeckEdit_OnProcess = SokuLib::TamperDword(&SokuLib::VTable_ProfileDeckEdit.onProcess, CProfileDeckEdit_OnProcess);
	s_originalCProfileDeckEdit_OnRender  = SokuLib::TamperDword(&SokuLib::VTable_ProfileDeckEdit.onRender, CProfileDeckEdit_OnRender);
	s_originalCProfileDeckEdit_Destructor= SokuLib::TamperDword(&SokuLib::VTable_ProfileDeckEdit.onDestruct, CProfileDeckEdit_Destructor);
	realSendTo = SokuLib::TamperDword(&SokuLib::DLL::ws2_32.sendto, &mySendTo);
	realMoveFileA = SokuLib::TamperDword(&SokuLib::DLL::kernel32.MoveFileA, &myMoveFileA);
	::VirtualProtect((PVOID)RDATA_SECTION_OFFSET, RDATA_SECTION_SIZE, old, &old);

	::VirtualProtect((PVOID)TEXT_SECTION_OFFSET, TEXT_SECTION_SIZE, PAGE_EXECUTE_WRITECOPY, &old);
	//Force deck icon to be hidden in character select
	*(unsigned char *)0x4210e2 = 0xEB;
	//Force deck icon to be hidden in deck construction
	memset((void *)0x0044E4ED, 0x90, 35);
	SokuLib::TamperNearJmpOpr(0x450230, CProfileDeckEdit_SwitchCurrentDeck);
	s_originalDrawGradiantBar = reinterpret_cast<void (*)(float, float, float)>(SokuLib::TamperNearJmpOpr(0x44E4C8, drawGradiantBar));
	s_originalInputMgrGet = SokuLib::union_cast<int (SokuLib::ObjectSelect::*)()>(SokuLib::TamperNearJmpOpr(0x4206B3, myGetInput));
	og_CProfileDeckEdit_Init = SokuLib::union_cast<SokuLib::ProfileDeckEdit *(SokuLib::ProfileDeckEdit::*)(int, int, SokuLib::Sprite *)>(
		SokuLib::TamperNearJmpOpr(0x0044d529, CProfileDeckEdit_Init)
	);
	::VirtualProtect((PVOID)TEXT_SECTION_OFFSET, TEXT_SECTION_SIZE, old, &old);

	new SokuLib::Trampoline(0x435377, onProfileChanged, 7);
	new SokuLib::Trampoline(0x450121, onDeckSaved, 6);

	::FlushInstructionCache(GetCurrentProcess(), nullptr, 0);
	return true;
}

extern "C" int APIENTRY DllMain(HMODULE hModule, DWORD fdwReason, LPVOID lpReserved) {
	myModule = hModule;
	return TRUE;
}