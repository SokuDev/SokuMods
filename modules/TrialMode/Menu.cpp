//
// Created by PinkySmile on 19/07/2021.
//

#define _USE_MATH_DEFINES
#include <fstream>
#include <dinput.h>
#include <process.h>
#include <thread>
#include "Menu.hpp"
#include "Pack.hpp"
#include "version.h"

#ifndef _DEBUG
#define puts(...)
#define printf(...)
#endif

#define FILTER_TEXT_SIZE 120

static int currentPack = -3;
static int currentEntry = -1;
static bool loaded = false;
static bool loadNextTrial = false;
static unsigned shownPack = 0;
static unsigned nameFilter = -1;
static unsigned modeFilter = -1;
static unsigned topicFilter = -1;
static unsigned nbPacks = 0;
static unsigned nbName = 0;
static unsigned nbMode = 0;
static unsigned nbTopic = 0;
static SokuLib::Vector2i versionStrSize;
static SokuLib::Vector2i nameFilterSize;
static SokuLib::Vector2i modeFilterSize;
static SokuLib::Vector2i topicFilterSize;
static SokuLib::DrawUtils::Sprite lock;
static SokuLib::DrawUtils::Sprite arrow;
static SokuLib::DrawUtils::Sprite title;
static SokuLib::DrawUtils::Sprite score;
static SokuLib::DrawUtils::Sprite frame;
static SokuLib::DrawUtils::Sprite arrowSprite;
static SokuLib::DrawUtils::Sprite missingIcon;
static SokuLib::DrawUtils::Sprite packContainer;
static SokuLib::DrawUtils::Sprite questionMarks;
static SokuLib::DrawUtils::Sprite nameFilterText;
static SokuLib::DrawUtils::Sprite modeFilterText;
static SokuLib::DrawUtils::Sprite topicFilterText;
static SokuLib::DrawUtils::Sprite previewContainer;
static SokuLib::DrawUtils::Sprite blackSilouettes;
static SokuLib::DrawUtils::Sprite lockedNoise;
static SokuLib::DrawUtils::Sprite lockedText;
static SokuLib::DrawUtils::Sprite lockedImg;
static SokuLib::DrawUtils::Sprite CRTBands;
static SokuLib::DrawUtils::Sprite loadingGear;
static SokuLib::DrawUtils::Sprite version;

static IDirect3DTexture9 **pphandle = nullptr;
static IDirect3DTexture9 **pphandle2 = nullptr;
static unsigned packStart = 0;
static unsigned entryStart = 0;
static unsigned band1Start = 0;
static unsigned band2Start = 0;

std::unique_ptr<Trial> loadedTrial;
bool loadRequest;
unsigned loading = false;
SokuLib::SWRFont defaultFont10;
SokuLib::SWRFont defaultFont12;
SokuLib::SWRFont defaultFont16;
HMODULE myModule;
bool editorMode = false;
char profilePath[1024 + MAX_PATH];
char profileFolderPath[1024 + MAX_PATH];
std::vector<SokuLib::KeyInput> _debug{
	{1, 0, 0, 0, 0, 0, 0, 0}, // Right
	{0, 0, 0, 0, 0, 0, 0, 0},
	{0, 0, 0, 0, 1, 0, 0, 0}, // C
	{0, 0, 0, 0, 0, 0, 0, 0},
	{-1, 0, 0, 0, 0, 0, 0, 0}, // Left
	{-1, 0, 0, 0, 0, 1, 0, 0}, // Left D (A on keyboard)
	{0, 0, 0, 0, 0, 1, 0, 0}, // D (A on keyboard)
	{0, 0, 0, 0, 0, 0, 0, 0},
	{0, 1, 0, 0, 0, 0, 0, 0}, // Down
	{0, 0, 0, 0, 0, 0, 0, 0},
	{0, -1, 0, 0, 0, 0, 0, 0}, // Up
	{0, 0, 0, 0, 0, 0, 0, 0},
	{0, 0, 0, 0, 0, 0, 1, 0}, // CH (S on keyboard)
	{0, 0, 0, 0, 0, 0, 0, 0},
	{0, 0, 0, 0, 0, 0, 0, 1}, // SC (D on keyboard)
};
std::vector<SokuLib::KeyInput> lastInputs{
	{0, 0, 0, 0, 0, 0, 0, 0},
	{0, 0, 0, 0, 0, 0, 0, 0},
	{0, 0, 0, 0, 0, 0, 0, 0},
	{0, 0, 0, 0, 0, 0, 0, 0},
	{0, 0, 0, 0, 0, 0, 0, 0},
	{0, 0, 0, 0, 0, 0, 0, 0},
	{0, 0, 0, 0, 0, 0, 0, 0},
	{0, 0, 0, 0, 0, 0, 0, 0},
	{0, 0, 0, 0, 0, 0, 0, 0},
	{0, 0, 0, 0, 0, 0, 0, 0},
	{0, 0, 0, 0, 0, 0, 0, 0},
	{0, 0, 0, 0, 0, 0, 0, 0},
	{0, 0, 0, 0, 0, 0, 0, 0},
	{0, 0, 0, 0, 0, 0, 0, 0},
	{0, 0, 0, 0, 0, 0, 0, 0},
	{0, 0, 0, 0, 0, 0, 0, 0},
};

void saveScores()
{
	std::ofstream stream{loadedPacks[currentPack]->scorePath, std::ofstream::binary};

	for (auto &scenario : loadedPacks[currentPack]->scenarios)
		stream.write(&scenario->score, 1);
}

bool checkField(const std::string &field, const nlohmann::json &value, bool (nlohmann::json::*fct)() const noexcept)
{
	if (!value.contains(field) || !(value[field].*fct)()) {
		MessageBox(
			SokuLib::window,
			("The field \"" + field + "\" is not valid but is mandatory.").c_str(),
			"Loading error",
			MB_ICONERROR
		);
		return false;
	}
	return true;
}

bool addCharacterToBuffer(const std::string &name, const nlohmann::json &chr, SokuLib::PlayerInfo &info, bool isRight)
{
	if (!checkField("character", chr, &nlohmann::json::is_string))
		return false;
	if (!checkField("palette", chr, &nlohmann::json::is_number))
		return false;
	if (!checkField("deck", chr, &nlohmann::json::is_array))
		return false;

	std::string str = chr["character"];
	auto it = std::find_if(
		validCharacters.begin(),
		validCharacters.end(),
		[str](const std::pair<unsigned, std::string> &s) {
			return s.second == str;
		}
	);
	auto itSWR = std::find_if(
		swrCharacters.begin(),
		swrCharacters.end(),
		[str](const std::pair<unsigned, std::string> &s) {
			return s.second == str;
		}
	);

	if (it == validCharacters.end() && (SokuLib::SWRUnlinked || itSWR == swrCharacters.end())) {
		MessageBox(
			SokuLib::window,
			("Error in field \"" + name + "\": " + str + " is not a valid character.").c_str(),
			"Loading error",
			MB_ICONERROR
		);
		return false;
	}

	std::vector<int> deck;

	try {
		std::map<unsigned short, int> cards;

		deck = chr["deck"].get<std::vector<int>>();
		if (deck.size() != 20 && !deck.empty())
			throw std::out_of_range("Deck must either be empty or have 20 cards");
		for (int card : deck) {
			cards[card]++;
			if (cards[card] > 4)
				throw std::out_of_range("More than 4 card " + std::to_string(card) + " are in this deck.");
		}
	} catch (std::exception &e) {
		MessageBox(
			SokuLib::window,
			("Error in field \"" + name + "\": Deck is not valid: " + std::string()).c_str(),
			"Loading error",
			MB_ICONERROR
		);
		return false;
	}

	info.character = static_cast<SokuLib::Character>(it == validCharacters.end() ? itSWR->first : it->first);
	info.palette = chr["palette"].get<int>();
	info.isRight = isRight;

	auto FUN_00434bf0 = reinterpret_cast<void (__thiscall *)(SokuLib::Profile *, char)>(0x434bf0);

	if (isRight) {
		*((SokuLib::KeyManager **)0x00898684) = reinterpret_cast<SokuLib::KeyManager *>(0x8986a8);
		FUN_00434bf0(&SokuLib::profile2, -1);
		info.keyManager = (SokuLib::KeyManager **)0x0089918c;
	} else {
		*((SokuLib::KeyManager **)0x00898680) = *(char *)0x898678 < '\0' ?
			reinterpret_cast<SokuLib::KeyManager *>(0x8986a8) :
			((SokuLib::KeyManager *(__thiscall *)(int, char))0x43e3b0)(0x899cec, *(char *)0x898678);
		FUN_00434bf0(&SokuLib::profile1, *(char *)0x898678);
		info.keyManager = (SokuLib::KeyManager **)0x008989A0;
	}

	info.effectiveDeck.clear();
	for (auto card : deck)
		info.effectiveDeck.push_back(card);
	return true;
}

bool prepareReplayBuffer(const std::string &path, const char *folder)
{
	std::ifstream stream{path};
	nlohmann::json value;

	if (stream.fail()) {
		MessageBox(
			SokuLib::window,
			("Cannot load file " + path + ": " + strerror(errno)).c_str(),
			"Trial loading error",
			MB_ICONERROR
		);
		return false;
	}

	try {
		stream >> value;
	} catch (std::exception &e) {
		MessageBox(
			SokuLib::window,
			("File " + path + " is not valid: " + e.what() + ".\n").c_str(),
			"Trial loading error",
			MB_ICONERROR
		);
		return false;
	}

	if (!checkField("player", value, &nlohmann::json::is_object))
		return false;
	if (!checkField("dummy", value, &nlohmann::json::is_object))
		return false;
	if (!checkField("stage", value, &nlohmann::json::is_number))
		return false;
	if (!value.contains("music") || (!value["music"].is_number() && !value["music"].is_string())) {
		MessageBox(
			SokuLib::window,
			"The field \"music\" is not valid but is mandatory.",
			"Loading error",
			MB_ICONERROR
		);
		return false;
	}
	if (!checkField("type", value, &nlohmann::json::is_string))
		return false;

	if (!addCharacterToBuffer("player", value["player"], SokuLib::leftPlayerInfo, false))
		return false;
	if (!addCharacterToBuffer("dummy", value["dummy"], SokuLib::rightPlayerInfo, true))
		return false;
	try {
		loadedTrial.reset(Trial::create(folder, SokuLib::leftPlayerInfo.character, value));
	} catch (std::exception &e) {
		MessageBox(
			SokuLib::window,
			("File " + path + " is not valid: " + e.what() + ".\n").c_str(),
			"Trial loading error",
			MB_ICONERROR
		);
		return false;
	}

	*(char *)0x899D0C = value["stage"].get<char>();
	if (value["music"].is_number())
		*(char *)0x899D0D = value["music"].get<char>();
	else {
		char nb = 6;
		std::string str = value["music"];

		if (str.size() == strlen("data/bgm/st00.ogg") && str.substr(0, 11) == "data/bgm/st" && str.substr(13) == ".ogg")
			try {
				nb = std::stoul(str.substr(11, 2));
			} catch (...) {}
		*(char *)0x899D0D = nb;
	}
	return true;
}

void prepareGameLoading(const char *folder, const std::string &path)
{
	SokuLib::setBattleMode(SokuLib::BATTLE_MODE_VSPLAYER, SokuLib::BATTLE_SUBMODE_PLAYING1);
	if (!prepareReplayBuffer(path, folder))
		return;
	loadRequest = true;
}

ResultMenu::ResultMenu(int score)
{
	loadedPacks[currentPack]->scenarios[currentEntry]->setScore(max(loadedPacks[currentPack]->scenarios[currentEntry]->score, score));
	saveScores();
	this->_selected += currentEntry == loadedPacks[currentPack]->scenarios.size() - 1;

	this->_resultTop.texture.loadFromGame("data/infoeffect/result/resultTitle.bmp");
	this->_resultTop.setPosition({128, 94});
	this->_resultTop.setSize(this->_resultTop.texture.getSize());
	this->_resultTop.rect.width = this->_resultTop.texture.getSize().x;
	this->_resultTop.rect.height = this->_resultTop.texture.getSize().y;

	this->_title.texture.loadFromGame("data/menu/result/result.bmp");
	this->_title.setSize(this->_title.texture.getSize());
	this->_title.rect.width = this->_title.texture.getSize().x;
	this->_title.rect.height = this->_title.texture.getSize().y;

	this->_score.texture.loadFromGame("data/infoeffect/result/rankFont.bmp");
	this->_score.setPosition({378, 164});
	this->_score.setSize({128, 128});
	this->_score.rect.left = score * this->_score.texture.getSize().x / 4;
	this->_score.rect.width = this->_score.texture.getSize().x / 4;
	this->_score.rect.height = this->_score.texture.getSize().y;

	for (int i = 0; i < Trial::menuActionText.size(); i++) {
		auto &sprite = this->_text[i];

		sprite.texture.createFromText(Trial::menuActionText[i].c_str(), defaultFont16, {230, 24});
		sprite.setPosition({128, 182 + i * 24});
		sprite.setSize(sprite.texture.getSize());
		sprite.rect.width = sprite.texture.getSize().x;
		sprite.rect.height = sprite.texture.getSize().y;
	}
	if (currentEntry == loadedPacks[currentPack]->scenarios.size() - 1)
		this->_text[0].tint = SokuLib::DrawUtils::DxSokuColor{0x40, 0x40, 0x40};
}

void ResultMenu::_()
{
	puts("_ !");
	*(int *)0x882a94 = 0x16;
}

int ResultMenu::onProcess()
{
	if (SokuLib::inputMgrs.input.b == 1 || SokuLib::checkKeyOneshot(DIK_ESCAPE, 0, 0, 0)) {
		SokuLib::playSEWaveBuffer(0x29);
		this->_selected = Trial::RETURN_TO_TITLE_SCREEN;
	}
	if (SokuLib::inputMgrs.input.a == 1) {
		SokuLib::playSEWaveBuffer(0x28);
		if (this->_selected == Trial::GO_TO_NEXT_TRIAL)
			loadNextTrial = true;
		loadedTrial->onMenuClosed(static_cast<Trial::MenuAction>(this->_selected));
		return false;
	}
	if (SokuLib::inputMgrs.input.verticalAxis == -1 || (SokuLib::inputMgrs.input.verticalAxis <= -36 && SokuLib::inputMgrs.input.verticalAxis % 6 == 0)) {
		SokuLib::playSEWaveBuffer(0x27);
		this->_selected--;
		if (this->_selected == Trial::GO_TO_NEXT_TRIAL)
			this->_selected -= currentEntry == loadedPacks[currentPack]->scenarios.size() - 1;
		this->_selected += Trial::NB_MENU_ACTION;
		this->_selected %= Trial::NB_MENU_ACTION;
	} else if (SokuLib::inputMgrs.input.verticalAxis == 1 || (SokuLib::inputMgrs.input.verticalAxis >= 36 && SokuLib::inputMgrs.input.verticalAxis % 6 == 0)) {
		SokuLib::playSEWaveBuffer(0x27);
		this->_selected++;
		if (this->_selected == Trial::NB_MENU_ACTION)
			this->_selected += currentEntry == loadedPacks[currentPack]->scenarios.size() - 1;
		this->_selected %= Trial::NB_MENU_ACTION;
	}
	return true;
}

int ResultMenu::onRender()
{
	this->_title.draw();
	this->_resultTop.draw();
	this->_score.draw();
	((void (*)(float, float, float))0x443a50)(128, 184 + this->_selected * 24, 300);
	for (auto &sprite : this->_text)
		sprite.draw();
	return 0;
}

void loadFont()
{
	SokuLib::FontDescription desc;

	desc.r1 = 255;
	desc.r2 = 255;
	desc.g1 = 255;
	desc.g2 = 255;
	desc.b1 = 255;
	desc.b2 = 255;
	desc.height = 10;
	desc.weight = FW_NORMAL;
	desc.italic = 0;
	desc.shadow = 1;
	desc.bufferSize = 1000000;
	desc.charSpaceX = 0;
	desc.charSpaceY = 0;
	desc.offsetX = 0;
	desc.offsetY = 0;
	desc.useOffset = 0;
	strcpy(desc.faceName, "MonoSpatialModSWR");
	desc.weight = FW_REGULAR;
	defaultFont10.create();
	defaultFont10.setIndirect(desc);

	desc.height = 12;
	defaultFont12.create();
	defaultFont12.setIndirect(desc);

	desc.height = 16;
	defaultFont16.create();
	defaultFont16.setIndirect(desc);
}

void checkScrollDown()
{
	if (currentPack < 0) {
		packStart = 0;
		entryStart = 0;
		return;
	}
	if (currentPack >= 0 && currentEntry == -1) {
		packStart = max(0, min(currentPack, 1.f * currentPack - static_cast<int>(264 - (currentPack == loadedPacks.size() - 1 ? 0 : 20) - 25 - 15.f * loadedPacks[currentPack]->scenarios.size()) / 35));
		entryStart = 0;
		return;
	}
	if (currentEntry - entryStart > 15)
		entryStart = currentEntry - 15;
}

void menuLoadAssets()
{
	if (loaded)
		return;
	loaded = true;
	puts("Loading assets");

	loadFont();

	previewContainer.texture.loadFromGame("data/menu/profile_list_seat.bmp");
	previewContainer.rect = {
		0, 0,
		static_cast<int>(previewContainer.texture.getSize().x),
		static_cast<int>(previewContainer.texture.getSize().y),
	};
	previewContainer.setPosition({310, 92});
	previewContainer.setSize({365, 345});

	packContainer.texture.loadFromResource(myModule, MAKEINTRESOURCE(4));
	packContainer.rect = {
		0, 0,
		static_cast<int>(packContainer.texture.getSize().x),
		static_cast<int>(packContainer.texture.getSize().y),
	};
	packContainer.setSize(packContainer.texture.getSize() - 1);

	missingIcon.texture.loadFromResource(myModule, MAKEINTRESOURCE(8));
	missingIcon.rect = {
		0, 0,
		static_cast<int>(missingIcon.texture.getSize().x),
		static_cast<int>(missingIcon.texture.getSize().y),
	};
	missingIcon.setSize({32, 32});
	missingIcon.fillColors[SokuLib::DrawUtils::GradiantRect::RECT_BOTTOM_LEFT_CORNER] = SokuLib::DrawUtils::DxSokuColor::White * 0.25;
	missingIcon.fillColors[SokuLib::DrawUtils::GradiantRect::RECT_BOTTOM_RIGHT_CORNER]= SokuLib::DrawUtils::DxSokuColor::White * 0.25;

	arrow.texture.loadFromGame("data/profile/deck2/000_Cursor.bmp");
	arrow.setSize(arrow.texture.getSize());
	arrow.rect.width = arrow.texture.getSize().x;
	arrow.rect.height = arrow.texture.getSize().y;

	score.texture.loadFromGame("data/infoeffect/result/rankFont.bmp");
	score.setSize({32, 32});
	score.tint = SokuLib::DrawUtils::DxSokuColor::White;
	score.rect.width = score.texture.getSize().x / 4;
	score.rect.height = score.texture.getSize().y;

	title.texture.loadFromResource(myModule, MAKEINTRESOURCE(24));
	title.setSize(title.texture.getSize());
	title.rect.width = title.texture.getSize().x;
	title.rect.height = title.texture.getSize().y;

	lock.texture.loadFromResource(myModule, MAKEINTRESOURCE(28));
	lock.setSize({32, 32});
	lock.rect.width = lock.texture.getSize().x;
	lock.rect.height = lock.texture.getSize().y;

	lockedImg.texture.loadFromResource(myModule, MAKEINTRESOURCE(32));
	lockedImg.setSize({200, 150});
	lockedImg.rect.width = lockedImg.texture.getSize().x;
	lockedImg.rect.height = lockedImg.texture.getSize().y;
	lockedImg.setPosition({398, 128});

	frame.texture.loadFromResource(myModule, MAKEINTRESOURCE(36));
	frame.setSize({212, 162});
	frame.rect.width = frame.texture.getSize().x;
	frame.rect.height = frame.texture.getSize().y;
	frame.setPosition({392, 122});

	blackSilouettes.texture.loadFromResource(myModule, MAKEINTRESOURCE(40));
	blackSilouettes.setSize({200, 150});
	blackSilouettes.rect.width = blackSilouettes.texture.getSize().x;
	blackSilouettes.rect.height = blackSilouettes.texture.getSize().y;
	blackSilouettes.setPosition({398, 128});

	lockedText.setSize({300, 150});
	lockedText.rect.width = 300;
	lockedText.rect.height = 150;
	lockedText.setPosition({356, 286});

	questionMarks.texture.createFromText("????????????????", defaultFont12, {0x100, 15});
	questionMarks.setSize(questionMarks.texture.getSize());
	questionMarks.rect.width = questionMarks.texture.getSize().x;
	questionMarks.rect.height = questionMarks.texture.getSize().y;
	questionMarks.tint = SokuLib::DrawUtils::DxSokuColor{0x80, 0x80, 0x80};

	arrowSprite.texture.loadFromGame("data/profile/deck2/sayuu.bmp");
	arrowSprite.setSize({
		arrowSprite.texture.getSize().x / 2 + 1,
		arrowSprite.texture.getSize().y + 1
	});
	arrowSprite.rect.width = arrowSprite.texture.getSize().x / 2;
	arrowSprite.rect.height = arrowSprite.texture.getSize().y;

	loadingGear.texture.loadFromGame("data/scene/logo/gear.bmp");
	loadingGear.setSize({
		loadingGear.texture.getSize().x,
		loadingGear.texture.getSize().y
	});
	loadingGear.rect.width = loadingGear.texture.getSize().x;
	loadingGear.rect.height = loadingGear.texture.getSize().y;

	version.texture.createFromText(
#ifdef _DEBUG
		"Version " VERSION_STR " DEBUG",
#else
		"Version " VERSION_STR,
#endif
		defaultFont10, {300, 20}, &versionStrSize
	);
	version.setPosition({
		639 - versionStrSize.x,
		479 - versionStrSize.y
	});
	version.setSize({
		static_cast<unsigned int>(versionStrSize.x),
		static_cast<unsigned int>(versionStrSize.y)
	});
	version.rect.width = versionStrSize.x;
	version.rect.height = versionStrSize.y;

	nameFilterText.texture.createFromText( "Any name",  defaultFont12, {300, 20}, &nameFilterSize);
	nameFilterText.setSize({
		nameFilterText.texture.getSize().x,
		nameFilterText.texture.getSize().y
	});
	nameFilterText.rect.width = nameFilterText.texture.getSize().x;
	nameFilterText.rect.height = nameFilterText.texture.getSize().y;

	modeFilterText.texture.createFromText( "Any mode",  defaultFont12, {300, 20}, &modeFilterSize);
	modeFilterText.setSize({
		modeFilterText.texture.getSize().x,
		modeFilterText.texture.getSize().y
	});
	modeFilterText.rect.width = modeFilterText.texture.getSize().x;
	modeFilterText.rect.height = modeFilterText.texture.getSize().y;

	topicFilterText.texture.createFromText("Any topic", defaultFont12, {300, 20}, &topicFilterSize);
	topicFilterText.setSize({
		topicFilterText.texture.getSize().x,
		topicFilterText.texture.getSize().y
	});
	topicFilterText.rect.width = topicFilterText.texture.getSize().x;
	topicFilterText.rect.height = topicFilterText.texture.getSize().y;

	int id;
	HRESULT ret;

	pphandle = SokuLib::textureMgr.allocate(&id);
	if (FAILED(ret = D3DXCreateTexture(SokuLib::pd3dDev, 200, 150, 1, 0, D3DFMT_A8R8G8B8, D3DPOOL_MANAGED, pphandle))) {
		pphandle = nullptr;
		SokuLib::textureMgr.deallocate(id);
		fprintf(stderr, "D3DXCreateTexture(SokuLib::pd3dDev, 200, 150, D3DX_DEFAULT, D3DUSAGE_RENDERTARGET, D3DFMT_A8B8G8R8, D3DPOOL_DEFAULT, %p) failed with code %li\n", pphandle, ret);
		goto failed;
	}
	lockedNoise.texture.setHandle(id, {200, 150});
	lockedNoise.setSize({200, 150});
	lockedNoise.setPosition({398, 128});
	lockedNoise.rect.width = 200;
	lockedNoise.rect.height = 150;

failed:
	pphandle2 = SokuLib::textureMgr.allocate(&id);
	if (FAILED(ret = D3DXCreateTexture(SokuLib::pd3dDev, 200, 150, 1, 0, D3DFMT_A8R8G8B8, D3DPOOL_MANAGED, pphandle2))) {
		pphandle = nullptr;
		SokuLib::textureMgr.deallocate(id);
		fprintf(stderr, "D3DXCreateTexture(SokuLib::pd3dDev, 200, 150, D3DX_DEFAULT, D3DUSAGE_RENDERTARGET, D3DFMT_A8B8G8R8, D3DPOOL_DEFAULT, %p) failed with code %li\n", pphandle2, ret);
		goto failed2;
	}
	CRTBands.texture.setHandle(id, {200, 150});
	CRTBands.setSize({200, 150});
	CRTBands.setPosition({398, 128});
	CRTBands.rect.width = 200;
	CRTBands.rect.height = 150;

failed2:
	loadPacks();

	if (
		nbPacks != loadedPacks.size() ||
		nbName != uniqueNames.size() ||
		nbMode != uniqueModes.size() ||
		nbTopic != uniqueCategories.size()
	) {
		currentPack = -3;
		currentEntry = -1;
		shownPack = 0;
		nameFilter = -1;
		modeFilter = -1;
		topicFilter = -1;
		packStart = 0;
		entryStart = 0;
		nbPacks = loadedPacks.size();
		nbName = uniqueNames.size();
		nbMode = uniqueModes.size();
		nbTopic = uniqueCategories.size();
	} else {
		nameFilterText.texture.createFromText( nameFilter == -1  ? "Any name" : uniqueNames[nameFilter].c_str(), defaultFont12, {300, 20}, &nameFilterSize);
		modeFilterText.texture.createFromText( modeFilter == -1  ? "Any mode" : uniqueModes[modeFilter].c_str(), defaultFont12, {300, 20}, &modeFilterSize);
		topicFilterText.texture.createFromText(topicFilter == -1 ? "Any topic" : uniqueCategories[topicFilter].c_str(), defaultFont12, {300, 20}, &topicFilterSize);
	}
}

#define NOISE_DELTA 50
#define RANDOM_VAL rand() % ((NOISE_DELTA) * 2 + 1)
#define RANDOM(r) ((r) + (RANDOM_VAL) - (NOISE_DELTA))

static void updateNoiseTexture(SokuLib::DrawUtils::DxSokuColor *array)
{
	unsigned char r = rand() % 206;

	for (int y = 0; y < 150; y++)
		for (int x = 0; x < 200; x++) {
			int g = RANDOM((int)r);

			if (g > 205)
				r = 205;
			else if (g < 0)
				r = 0;
			else
				r = g;
			array[x + y * 200] = SokuLib::DrawUtils::DxSokuColor{r, r, r};
		}
}

static void updateBandTexture(SokuLib::DrawUtils::DxSokuColor *array)
{
	static bool b = false;

	b = !b;
	for (int y = 0; y < 150; y++)
		for (int x = 0; x < 200; x++)
			array[x + y * 200] = SokuLib::DrawUtils::DxSokuColor{0xFF, 0xFF, 0xFF, 0x00};
	band1Start += 1 + b;
	band2Start += 1;
	if (band1Start > 220)
		band1Start = 0;
	if (band2Start > 255)
		band2Start = 0;

	for (int y = -10; y; y++) {
		if (band1Start < -y)
			continue;
		if (band1Start + y >= 150)
			break;
		for (int x = 0; x < 200; x++)
			array[x + (band1Start + y) * 200].a += 0x55;
	}
	for (int y = -20; y; y++) {
		if (band2Start < -y)
			continue;
		if (band2Start + y >= 150)
			break;
		for (int x = 0; x < 200; x++)
			array[x + (band2Start + y) * 200].a += 0x80;
	}
}

static void updateNoiseTexture()
{
	HRESULT ret;
	D3DLOCKED_RECT r;

	if (!pphandle)
		return;
	if (FAILED(ret = (*pphandle)->LockRect(0, &r, nullptr, 0))) {
		fprintf(stderr, "(*pphandle)->LockRect(0, &r, nullptr, D3DLOCK_DISCARD) failed with code %li\n", ret);
		return;
	}
	updateNoiseTexture(reinterpret_cast<SokuLib::DrawUtils::DxSokuColor *>(r.pBits));
	if (FAILED(ret = (*pphandle)->UnlockRect(0)))
		fprintf(stderr, "(*pphandle)->UnlockRect(0) failed with code %li\n", ret);
}

static void updateBandTexture()
{
	HRESULT ret;
	D3DLOCKED_RECT r;

	if (!pphandle2)
		return;
	if (FAILED(ret = (*pphandle2)->LockRect(0, &r, nullptr, 0))) {
		fprintf(stderr, "(*pphandle2)->LockRect(0, &r, nullptr, D3DLOCK_DISCARD) failed with code %li\n", ret);
		return;
	}
	updateBandTexture(reinterpret_cast<SokuLib::DrawUtils::DxSokuColor *>(r.pBits));
	if (FAILED(ret = (*pphandle2)->UnlockRect(0)))
		fprintf(stderr, "(*pphandle2)->UnlockRect(0) failed with code %li\n", ret);
}

void menuUnloadAssets()
{
	if (!loaded)
		return;
	loaded = false;
	puts("Unloading assets");

	defaultFont10.destruct();
	defaultFont12.destruct();
	defaultFont16.destruct();

	lock.texture.destroy();
	arrow.texture.destroy();
	title.texture.destroy();
	score.texture.destroy();
	arrowSprite.texture.destroy();
	missingIcon.texture.destroy();
	packContainer.texture.destroy();
	questionMarks.texture.destroy();
	nameFilterText.texture.destroy();
	modeFilterText.texture.destroy();
	topicFilterText.texture.destroy();
	previewContainer.texture.destroy();
	lockedNoise.texture.destroy();
	lockedText.texture.destroy();
	lockedImg.texture.destroy();
	frame.texture.destroy();
	CRTBands.texture.destroy();
	loadingGear.texture.destroy();
	blackSilouettes.texture.destroy();

	loadedPacks.clear();
	uniqueNames.clear();
	uniqueCategories.clear();
	packsByName.clear();
	packsByCategory.clear();

	editorMode = false;
}

static void displayFilters()
{
	arrowSprite.rect.left = 0;
	arrowSprite.setPosition({
		static_cast<int>(100 - FILTER_TEXT_SIZE / 2 - arrowSprite.getSize().x),
		70
	});
	arrowSprite.draw();
	arrowSprite.setPosition({
		static_cast<int>(320 - FILTER_TEXT_SIZE / 2 - arrowSprite.getSize().x),
		80
	});
	arrowSprite.draw();
	arrowSprite.setPosition({
		static_cast<int>(540 - FILTER_TEXT_SIZE / 2 - arrowSprite.getSize().x),
		90
	});
	arrowSprite.draw();

	arrowSprite.rect.left = arrowSprite.texture.getSize().x / 2;
	arrowSprite.setPosition({
		static_cast<int>(100 + FILTER_TEXT_SIZE / 2),
		70
	});
	arrowSprite.draw();
	arrowSprite.setPosition({
		static_cast<int>(320 + FILTER_TEXT_SIZE / 2),
		80
	});
	arrowSprite.draw();
	arrowSprite.setPosition({
		static_cast<int>(540 + FILTER_TEXT_SIZE / 2),
		90
	});
	arrowSprite.draw();

	if (currentPack == -1)
		((void (*)(float, float, float))0x443a50)(540 - FILTER_TEXT_SIZE / 2, 90 + arrowSprite.getSize().y / 2 - 7, FILTER_TEXT_SIZE + FILTER_TEXT_SIZE / 2);
	else if (currentPack == -2)
		((void (*)(float, float, float))0x443a50)(320 - FILTER_TEXT_SIZE / 2, 80 + arrowSprite.getSize().y / 2 - 7, FILTER_TEXT_SIZE + FILTER_TEXT_SIZE / 2);
	else if (currentPack == -3)
		((void (*)(float, float, float))0x443a50)(100 - FILTER_TEXT_SIZE / 2, 70 + arrowSprite.getSize().y / 2 - 7, FILTER_TEXT_SIZE + FILTER_TEXT_SIZE / 2);

	nameFilterText.setPosition({
		static_cast<int>(100 - nameFilterSize.x / 2),
		static_cast<int>(70 + arrowSprite.getSize().y / 2 - nameFilterSize.y / 2)
	});
	nameFilterText.draw();
	modeFilterText.setPosition({
		static_cast<int>(320 - modeFilterSize.x / 2),
		static_cast<int>(80 + arrowSprite.getSize().y / 2 - modeFilterSize.y / 2)
	});
	modeFilterText.draw();
	topicFilterText.setPosition({
		static_cast<int>(540 - topicFilterSize.x / 2),
		static_cast<int>(90 + arrowSprite.getSize().y / 2 - topicFilterSize.y / 2)
	});
	topicFilterText.draw();
}

static void switchEditorMode()
{
	for (auto &pack : loadedPacks) {
		if (!pack->error.texture.hasTexture()) {
			pack->name.fillColors[SokuLib::DrawUtils::GradiantRect::RECT_BOTTOM_LEFT_CORNER] = SokuLib::DrawUtils::DxSokuColor::Green * 0.25;
			pack->name.fillColors[SokuLib::DrawUtils::GradiantRect::RECT_BOTTOM_RIGHT_CORNER]= SokuLib::DrawUtils::DxSokuColor::Green * 0.25;
		}
	}
	SokuLib::playSEWaveBuffer(48);
}

void checkScrollUp()
{
	if (currentPack < 0) {
		entryStart = 0;
		return;
	}
	if (currentEntry == -1)
		return;
	printf("%i %i %i\n", currentEntry, currentPack, loadedPacks[currentPack]->scenarios.size());
	if (currentEntry == loadedPacks[currentPack]->scenarios.size() - 1) {
		packStart = max(0, min(currentPack, 1.f * currentPack - static_cast<int>(264 - (currentPack == loadedPacks.size() - 1 ? 0 : 20) - 35 - 15.f * loadedPacks[currentPack]->scenarios.size()) / 35));
		if (currentEntry > 15)
			entryStart = currentEntry - 15;
		return;
	}
	if (currentEntry < entryStart)
		entryStart = currentEntry;
}

inline bool isCompleted(int entry)
{
	if (entry < 0)
		return true;
	return loadedPacks[currentPack]->scenarios[entry]->score != -1;
}

inline bool isLocked(int entry)
{
	if (entry <= 0)
		return false;
	if (!loadedPacks[currentPack]->scenarios[entry]->canBeLocked)
		return false;
	return !isCompleted(entry - 1);
}

static void handleGoLeft()
{
	if (currentPack >= 0)
		return;
	SokuLib::playSEWaveBuffer(0x27);
	switch (currentPack) {
	case -3:
		nameFilter--;
		if (nameFilter == -2)
			nameFilter = uniqueNames.size() - 1;
		nameFilterText.texture.createFromText( nameFilter == -1  ? "Any name" : uniqueNames[nameFilter].c_str(),  defaultFont12, {300, 20}, &nameFilterSize);
		return;
	case -2:
		modeFilter--;
		if (modeFilter == -2)
			modeFilter = uniqueModes.size() - 1;
		modeFilterText.texture.createFromText( modeFilter == -1  ? "Any mode" : uniqueModes[modeFilter].c_str(),  defaultFont12, {300, 20}, &modeFilterSize);
		return;
	case -1:
		topicFilter--;
		if (topicFilter == -2)
			topicFilter = uniqueCategories.size() - 1;
		topicFilterText.texture.createFromText(topicFilter == -1 ? "Any topic" : uniqueCategories[topicFilter].c_str(), defaultFont12, {300, 20}, &topicFilterSize);
		return;
	default:
		return;
	}
}

static void handleGoRight()
{
	if (currentPack >= 0)
		return;
	SokuLib::playSEWaveBuffer(0x27);
	switch (currentPack) {
	case -3:
		nameFilter++;
		if (nameFilter == uniqueNames.size())
			nameFilter = -1;
		nameFilterText.texture.createFromText( nameFilter == -1  ? "Any name" : uniqueNames[nameFilter].c_str(),  defaultFont12, {300, 20}, &nameFilterSize);
		return;
	case -2:
		modeFilter++;
		if (modeFilter == uniqueModes.size())
			modeFilter = -1;
		modeFilterText.texture.createFromText( modeFilter == -1  ? "Any mode" : uniqueModes[modeFilter].c_str(),  defaultFont12, {300, 20}, &modeFilterSize);
		return;
	case -1:
		topicFilter++;
		if (topicFilter == uniqueCategories.size())
			topicFilter = -1;
		topicFilterText.texture.createFromText(topicFilter == -1 ? "Any topic" : uniqueCategories[topicFilter].c_str(), defaultFont12, {300, 20}, &topicFilterSize);
		return;
	default:
		return;
	}
}

static void handleGoUp()
{
	SokuLib::playSEWaveBuffer(0x27);
	if (currentEntry == -1) {
		do {
			currentEntry = -1;
			currentPack--;
			if (currentPack == -4)
				currentPack += loadedPacks.size() + 3;
			if (currentPack >= 0) {
				currentEntry += loadedPacks[currentPack]->scenarios.size();
				shownPack = currentPack;
			}
			if (currentPack < 0)
				break;

			auto &pack = loadedPacks[currentPack];

			if (nameFilter != -1 && pack->nameStr != uniqueNames[nameFilter])
				continue;
			if (modeFilter != -1 && std::find(pack->modes.begin(), pack->modes.end(), uniqueModes[modeFilter]) == pack->modes.end())
				continue;
			if (topicFilter != -1 && pack->category != uniqueCategories[topicFilter])
				continue;
			if (!loadedPacks[currentPack]->scenarios[currentEntry]->loading && loadedPacks[currentPack]->scenarios[currentEntry]->preview)
				loadedPacks[currentPack]->scenarios[currentEntry]->preview->reset();
			else
				loadedPacks[currentPack]->scenarios[currentEntry]->loadPreview();
			break;
		} while (true);
	} else {
		currentEntry--;
		if (currentEntry != -1) {
			if (!loadedPacks[currentPack]->scenarios[currentEntry]->loading && loadedPacks[currentPack]->scenarios[currentEntry]->preview)
				loadedPacks[currentPack]->scenarios[currentEntry]->preview->reset();
			else
				loadedPacks[currentPack]->scenarios[currentEntry]->loadPreview();
		}
	}
	checkScrollUp();
	printf("Pack: %i, Entry %i, Shown %i\n", currentPack, currentEntry, shownPack);
	if (currentEntry != -1 && isLocked(currentEntry)) {
		auto &other = loadedPacks[currentPack]->scenarios[currentEntry - 1];

		lockedText.texture.createFromText(("Unlocked by completing " + (isLocked(currentEntry - 1) && other->nameHiddenIfLocked ? std::string("????????????????") : other->nameStr)).c_str(), defaultFont12, {300, 150});
	}
}

static void handleGoDown()
{
	SokuLib::playSEWaveBuffer(0x27);
	if (currentPack < 0 || currentEntry == loadedPacks[currentPack]->scenarios.size() - 1) {
		do {
			currentPack++;
			if (currentPack == loadedPacks.size())
				currentPack = -3;
			shownPack = max(0, currentPack);
			currentEntry = -1;
			if (currentPack < 0)
				break;

			auto &pack = loadedPacks[currentPack];

			if (nameFilter != -1 && pack->nameStr != uniqueNames[nameFilter])
				continue;
			if (modeFilter != -1 && std::find(pack->modes.begin(), pack->modes.end(), uniqueModes[modeFilter]) == pack->modes.end())
				continue;
			if (topicFilter != -1 && pack->category != uniqueCategories[topicFilter])
				continue;
			break;
		} while (true);
	} else {
		currentEntry++;
		if (!loadedPacks[currentPack]->scenarios[currentEntry]->loading && loadedPacks[currentPack]->scenarios[currentEntry]->preview)
			loadedPacks[currentPack]->scenarios[currentEntry]->preview->reset();
		else
			loadedPacks[currentPack]->scenarios[currentEntry]->loadPreview();
	}
	checkScrollDown();
	printf("Pack: %i, Entry %i, Shown %i\n", currentPack, currentEntry, shownPack);
	if (currentEntry != -1 && isLocked(currentEntry)) {
		auto &other = loadedPacks[currentPack]->scenarios[currentEntry - 1];

		lockedText.texture.createFromText(("Unlocked by completing " + (isLocked(currentEntry - 1) && other->nameHiddenIfLocked ? std::string("????????????????") : other->nameStr)).c_str(), defaultFont12, {300, 150});
	}
}

void handlePlayerInputs(const SokuLib::KeyInput &input)
{
	if (editorMode)
		goto nothing;
	if (
		lastInputs.back().verticalAxis != ((input.verticalAxis > 0) - (input.verticalAxis < 0)) ||
		lastInputs.back().horizontalAxis != ((input.horizontalAxis > 0) - (input.horizontalAxis < 0)) ||
		!lastInputs.back().a != !input.a ||
		!lastInputs.back().b != !input.b ||
		!lastInputs.back().c != !input.c ||
		!lastInputs.back().d != !input.d ||
		!lastInputs.back().changeCard != !input.changeCard ||
		!lastInputs.back().spellcard != !input.spellcard
	) {
		lastInputs.erase(lastInputs.begin());
		lastInputs.push_back({
			(input.horizontalAxis > 0) - (input.horizontalAxis < 0),
			(input.verticalAxis > 0) - (input.verticalAxis < 0),
			!!input.a,
			!!input.b,
			!!input.c,
			!!input.d,
			!!input.changeCard,
			!!input.spellcard
		});
	}

	for (int i = 0; i < _debug.size(); i++)
		if (memcmp(&_debug[i], &lastInputs[i], sizeof(_debug[i])) != 0)
			goto nothing;
	editorMode = true;
	switchEditorMode();
nothing:
	if (input.a == 1 && currentEntry != -1 && SokuLib::newSceneId == SokuLib::SCENE_TITLE) {
		if (!isLocked(currentEntry)) {
			puts("Start game !");
			SokuLib::playSEWaveBuffer(0x28);
			prepareGameLoading(
				loadedPacks[currentPack]->scenarios[currentEntry]->folder.c_str(),
				loadedPacks[currentPack]->scenarios[currentEntry]->file
			);
			return;
		}
		SokuLib::playSEWaveBuffer(0x29);
	}
	if (input.verticalAxis == -1 || (input.verticalAxis <= -36 && input.verticalAxis % 6 == 0))
		handleGoUp();
	else if (input.verticalAxis == 1 || (input.verticalAxis >= 36 && input.verticalAxis % 6 == 0))
		handleGoDown();
	if (input.horizontalAxis == -1 || (input.horizontalAxis <= -36 && input.horizontalAxis % 6 == 0))
		handleGoLeft();
	else if (input.horizontalAxis == 1 || (input.horizontalAxis >= 36 && input.horizontalAxis % 6 == 0))
		handleGoRight();
}

int menuOnProcess(SokuLib::MenuResult *This)
{
	if (!loading && SokuLib::checkKeyOneshot(DIK_ESCAPE, 0, 0, 0)) {
		SokuLib::playSEWaveBuffer(0x29);
		return loading;
	}

	if (SokuLib::newSceneId != SokuLib::sceneId)
		return true;

	if (loadNextTrial) {
		loadNextTrial = false;
		++currentEntry;
		prepareGameLoading(
			loadedPacks[currentPack]->scenarios[currentEntry]->folder.c_str(),
			loadedPacks[currentPack]->scenarios[currentEntry]->file
		);
		if (loadRequest) {
			return true;
		}
	}
	menuLoadAssets();
	if (SokuLib::inputMgrs.input.b == 1) {
		puts("Quit");
		SokuLib::playSEWaveBuffer(0x29);
		return loading;
	}
	if (currentEntry >= 0) {
		if (!loadedPacks[currentPack]->scenarios[currentEntry]->loading && loadedPacks[currentPack]->scenarios[currentEntry]->preview)
			loadedPacks[shownPack]->scenarios[currentEntry]->preview->update();
		else {
			if (!loadedPacks[currentPack]->scenarios[currentEntry]->loading)
				loadedPacks[currentPack]->scenarios[currentEntry]->loadPreview();
			loadingGear.setRotation(loadingGear.getRotation() + 0.1);
		}
	}
	handlePlayerInputs(SokuLib::inputMgrs.input);
	SokuLib::currentScene->to<SokuLib::Title>().cursorPos = 8;
	SokuLib::currentScene->to<SokuLib::Title>().cursorPos2 = 8;
	updateNoiseTexture();
	updateBandTexture();
	return true;
}

void renderOnePackBack(Pack &pack, SokuLib::Vector2<float> &pos, bool deployed)
{
	if (nameFilter != -1 && pack.nameStr != uniqueNames[nameFilter])
		return;
	if (modeFilter != -1 && std::find(pack.modes.begin(), pack.modes.end(), uniqueModes[modeFilter]) == pack.modes.end())
		return;
	if (topicFilter != -1 && pack.category != uniqueCategories[topicFilter])
		return;

	if (pos.y >= 100) {
		packContainer.setPosition({
			static_cast<int>(pos.x),
			static_cast<int>(pos.y)
		});
		packContainer.draw();
	}
	pos.y += 35;
	if (deployed) {
		for (int i = entryStart; i < pack.scenarios.size(); i++) {
			pos.y += 15;
			if (pos.y > 379)
				break;
		}
	} else
		pos.y += 5;
}

void renderOnePack(Pack &pack, SokuLib::Vector2<float> &pos, bool deployed)
{
	int i;
	auto p = pos;
	auto &sprite = pack.error.texture.hasTexture() ? pack.error : pack.author;

	if (nameFilter != -1 && pack.nameStr != uniqueNames[nameFilter])
		return;
	if (modeFilter != -1 && std::find(pack.modes.begin(), pack.modes.end(), uniqueModes[modeFilter]) == pack.modes.end())
		return;
	if (topicFilter != -1 && pack.category != uniqueCategories[topicFilter])
		return;

	//100 <= y <= 406
	if (pos.y >= 100) {
		auto sumScore = 0;
		bool hasScore = false;

		for (auto &scenario : pack.scenarios) {
			hasScore |= scenario->score != -1;
			sumScore += scenario->score;
		}

		if (pack.icon) {
			pack.icon->sprite.setPosition(SokuLib::Vector2i{
				static_cast<int>(pos.x + 4),
				static_cast<int>(pos.y + 2)
			} + pack.icon->translate);
			pack.icon->sprite.draw();
		} else {
			missingIcon.setPosition({
				static_cast<int>(pos.x + 34),
				static_cast<int>(pos.y - 1)
			});
			missingIcon.draw();
		}

		sprite.setPosition({
			static_cast<int>(pos.x + 75),
			static_cast<int>(pos.y + 17)
		});
		sprite.draw();

		if (hasScore) {
			int val = sumScore / static_cast<int>(pack.scenarios.size());

			score.rect.left = max(0, val) * score.texture.getSize().x / 4;
			score.setPosition({
				static_cast<int>(pos.x + 296),
				static_cast<int>(pos.y - 8)
			});
			score.draw();
		}
	}

	if (currentEntry != -1) {
		p.x += 25;
		p.y += (currentEntry - entryStart) * 15 + 33;
	}
	if (deployed)
		((void (*)(float, float, float))0x443a50)(p.x + 70, p.y + 1, 300);

	if (pos.y >= 100 && pos.y <= 406) {
		pack.name.setPosition({
			static_cast<int>(pos.x + 74),
			static_cast<int>(pos.y + 2)
		});
		pack.name.draw();
	}
	pos.y += 35;

	if (!deployed) {
		pos.y += 5;
		return;
	}

	for (i = entryStart; i < pack.scenarios.size(); i++) {
		auto &scenario = pack.scenarios[i];

		if (pos.y >= 100) {
			if (isLocked(i)) {
				lock.setPosition({
					static_cast<int>(pos.x + 271),
					static_cast<int>(pos.y - 10)
				});
				lock.draw();
				if (scenario->nameHiddenIfLocked) {
					questionMarks.setPosition({
						static_cast<int>(pos.x + 100),
						static_cast<int>(pos.y)
					});
					for (int j = 0; j < 4; j++)
						questionMarks.fillColors[j] = scenario->name.fillColors[j];
					questionMarks.draw();
				} else {
					scenario->name.setPosition({
						static_cast<int>(pos.x + 100),
						static_cast<int>(pos.y)
					});
					scenario->name.tint = SokuLib::DrawUtils::DxSokuColor{0x80, 0x80, 0x80};
					scenario->name.draw();
				}
			} else {
				scenario->name.setPosition({
					static_cast<int>(pos.x + 100),
					static_cast<int>(pos.y)
				});
				scenario->name.tint = SokuLib::DrawUtils::DxSokuColor::White;
				scenario->name.draw();
				if (scenario->score != -1) {
					scenario->scoreSprite.setPosition({
						static_cast<int>(pos.x + 271),
						static_cast<int>(pos.y - 10)
					});
					scenario->scoreSprite.draw();
				}
			}
		}
		pos.y += 15;
		if (pos.y > 379)
			break;
	};
	if (entryStart) {
		arrow.setRotation(-M_PI_2 - 0.063);
		arrow.setMirroring(false, false);
		arrow.setPosition({72, 148});
		arrow.draw();
	}
	if (i < pack.scenarios.size() - 1) {
		arrow.setRotation(M_PI_2 + 0.062);
		arrow.setMirroring(false, false);
		arrow.setPosition({72, 372});
		arrow.draw();
	}
}

void menuOnRender(SokuLib::MenuResult *This)
{
	SokuLib::Vector2<float> pos = {16, 116};

	if (!loaded)
		return;

	displayFilters();
	title.draw();
	for (unsigned i = packStart; i < loadedPacks.size(); i++) {
		// 100 <= y <= 364
		renderOnePackBack(*loadedPacks[i], pos, i == currentPack);
		if (pos.y > 394)
			break;
	}
	pos = {16, 116};
	for (unsigned i = packStart; i < loadedPacks.size(); i++) {
		// 100 <= y <= 364
		renderOnePack(*loadedPacks[i], pos, i == currentPack);
		if (pos.y > 394)
			break;
	}

	previewContainer.draw();
	version.draw();
	if (loadedPacks.empty()) {
		return;
	}

	if (currentEntry < 0) {
		if (loadedPacks[shownPack]->preview.texture.hasTexture())
			loadedPacks[shownPack]->preview.draw();
		if (loadedPacks[shownPack]->description.texture.hasTexture())
			loadedPacks[shownPack]->description.draw();
		return;
	}
	if (!isLocked(currentEntry)) {
		if (!loadedPacks[shownPack]->scenarios[currentEntry]->loading) {
			if (loadedPacks[shownPack]->scenarios[currentEntry]->preview && loadedPacks[shownPack]->scenarios[currentEntry]->preview->isValid())
				loadedPacks[shownPack]->scenarios[currentEntry]->preview->render();
			else {
				lockedNoise.draw();
				blackSilouettes.draw();
			}
		} else {
			lockedNoise.draw();
			loadingGear.setRotation(-loadingGear.getRotation());
			loadingGear.setPosition({540, 243});
			loadingGear.draw();
			loadingGear.setRotation(-loadingGear.getRotation());
			loadingGear.setPosition({563, 225});
			loadingGear.draw();
		}
		if (loadedPacks[shownPack]->scenarios[currentEntry]->description.texture.hasTexture())
			loadedPacks[shownPack]->scenarios[currentEntry]->description.draw();
		CRTBands.draw();
	} else {
		lockedNoise.draw();
		CRTBands.draw();
		lockedText.draw();
		lockedImg.draw();
	}
	frame.draw();
}