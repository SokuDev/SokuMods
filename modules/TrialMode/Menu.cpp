//
// Created by PinkySmile on 19/07/2021.
//

#define _USE_MATH_DEFINES
#include <fstream>
#include <dinput.h>
#include "Menu.hpp"
#include "Pack.hpp"

static unsigned currentPack = 0;
static int currentEntry = -1;
static bool loaded = false;
static bool loadNextTrial = false;
static SokuLib::DrawUtils::Sprite arrow;
static SokuLib::DrawUtils::Sprite title;
static SokuLib::DrawUtils::Sprite score;
static SokuLib::DrawUtils::Sprite missingIcon;
static SokuLib::DrawUtils::Sprite packContainer;
static SokuLib::DrawUtils::Sprite previewContainer;
static unsigned packStart = 0;
static unsigned entryStart = 0;

std::unique_ptr<Trial> loadedTrial;
bool loadRequest;
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

bool prepareReplayBuffer(const std::string &path)
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
	if (!checkField("music", value, &nlohmann::json::is_number))
		return false;
	if (!checkField("type", value, &nlohmann::json::is_string))
		return false;

	if (!addCharacterToBuffer("player", value["player"], SokuLib::leftPlayerInfo, false))
		return false;
	if (!addCharacterToBuffer("dummy", value["dummy"], SokuLib::rightPlayerInfo, true))
		return false;
	try {
		loadedTrial.reset(Trial::create(SokuLib::leftPlayerInfo.character, value));
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
	*(char *)0x899D0D = value["music"].get<char>();
	return true;
}

void prepareGameLoading(const std::string &path)
{
	SokuLib::setBattleMode(SokuLib::BATTLE_MODE_VSPLAYER, SokuLib::BATTLE_SUBMODE_PLAYING1);
	if (!prepareReplayBuffer(path))
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
	auto keys = reinterpret_cast<SokuLib::KeyManager *>(0x89A394);

	if (keys->keymapManager->input.b == 1 || SokuLib::checkKeyOneshot(DIK_ESCAPE, 0, 0, 0)) {
		SokuLib::playSEWaveBuffer(0x29);
		this->_selected = Trial::RETURN_TO_TITLE_SCREEN;
	}
	if (keys->keymapManager->input.a == 1) {
		SokuLib::playSEWaveBuffer(0x28);
		if (this->_selected == Trial::GO_TO_NEXT_TRIAL)
			loadNextTrial = true;
		loadedTrial->onMenuClosed(static_cast<Trial::MenuAction>(this->_selected));
		return false;
	}
	if (keys->keymapManager->input.verticalAxis == -1 || (keys->keymapManager->input.verticalAxis <= -36 && keys->keymapManager->input.verticalAxis % 6 == 0)) {
		SokuLib::playSEWaveBuffer(0x27);
		this->_selected--;
		if (this->_selected == Trial::GO_TO_NEXT_TRIAL)
			this->_selected -= currentEntry == loadedPacks[currentPack]->scenarios.size() - 1;
		this->_selected += Trial::NB_MENU_ACTION;
		this->_selected %= Trial::NB_MENU_ACTION;
	} else if (keys->keymapManager->input.verticalAxis == 1 || (keys->keymapManager->input.verticalAxis >= 36 && keys->keymapManager->input.verticalAxis % 6 == 0)) {
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

void menuLoadAssets()
{
	if (loaded)
		return;
	loaded = true;
	puts("Loading assets");

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

	loadFont();
	loadPacks();
	std::sort(loadedPacks.begin(), loadedPacks.end(), [](std::shared_ptr<Pack> pack1, std::shared_ptr<Pack> pack2){
		if (pack1->error.texture.hasTexture() != pack2->error.texture.hasTexture())
			return pack2->error.texture.hasTexture();
		return pack1->category < pack2->category;
	});
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
	previewContainer.texture.destroy();
	packContainer.texture.destroy();
	loadedPacks.clear();
	currentPack = 0;
	currentEntry = -1;
	editorMode = false;
}

bool wasPressed = false;

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
	if (currentEntry == -1)
		return;
	if (currentEntry == loadedPacks[currentPack]->scenarios.size() - 1) {
		packStart = max(0, min(currentPack, 1.f * currentPack - static_cast<int>(264 - (currentPack == loadedPacks.size() - 1 ? 0 : 20) - 35 - 15.f * loadedPacks[currentPack]->scenarios.size()) / 35));
		if (currentEntry > 15)
			entryStart = currentEntry - 15;
		return;
	}
	if (currentEntry < entryStart)
		entryStart = currentEntry;
}

void checkScrollDown()
{
	if (currentEntry == -1) {
		packStart = max(0, min(currentPack, 1.f * currentPack - static_cast<int>(264 - (currentPack == loadedPacks.size() - 1 ? 0 : 20) - 25 - 15.f * loadedPacks[currentPack]->scenarios.size()) / 35));
		entryStart = 0;
		return;
	}
	if (currentEntry - entryStart > 15)
		entryStart = currentEntry - 15;
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
	if (!wasPressed && input.a && currentEntry != -1 && SokuLib::newSceneId == SokuLib::SCENE_TITLE) {
		puts("Start game !");
		SokuLib::playSEWaveBuffer(0x28);
		prepareGameLoading(loadedPacks[currentPack]->scenarios[currentEntry]->file);
		return;
	}
	wasPressed = input.a || SokuLib::newSceneId != SokuLib::SCENE_TITLE;
	if (input.verticalAxis == -1 || (input.verticalAxis <= -36 && input.verticalAxis % 6 == 0)) {
		SokuLib::playSEWaveBuffer(0x27);
		if (currentEntry == -1) {
			currentPack--;
			if (currentPack == -1)
				currentPack += loadedPacks.size();
			currentEntry += loadedPacks[currentPack]->scenarios.size();
		} else
			currentEntry--;
		checkScrollUp();
	} else if (input.verticalAxis == 1 || (input.verticalAxis >= 36 && input.verticalAxis % 6 == 0)) {
		SokuLib::playSEWaveBuffer(0x27);
		if (currentEntry == loadedPacks[currentPack]->scenarios.size() - 1) {
			currentPack++;
			if (currentPack == loadedPacks.size())
				currentPack = 0;
			currentEntry = -1;
		} else
			currentEntry++;
		checkScrollDown();
	}
}

int menuOnProcess(SokuLib::MenuResult *This)
{
	auto keys = reinterpret_cast<SokuLib::KeyManager *>(0x89A394);

	if (loadNextTrial) {
		loadNextTrial = false;
		prepareGameLoading(loadedPacks[currentPack]->scenarios[++currentEntry]->file);
		if (loadRequest)
			return true;
	}
	menuLoadAssets();
	if (keys->keymapManager->input.b) {
		SokuLib::playSEWaveBuffer(0x29);
		return false;
	}
	handlePlayerInputs(keys->keymapManager->input);
	SokuLib::currentScene->to<SokuLib::Title>().cursorPos = 8;
	SokuLib::currentScene->to<SokuLib::Title>().cursorPos2 = 8;
	return true;
}

void renderOnePackBack(Pack &pack, SokuLib::Vector2<float> &pos, bool deployed)
{
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
			scenario->name.setPosition({
				static_cast<int>(pos.x + 100),
				static_cast<int>(pos.y)
			});
			scenario->name.draw();
			scenario->scoreSprite.setPosition({
				static_cast<int>(pos.x + 271),
				static_cast<int>(pos.y - 10)
			});
			scenario->scoreSprite.draw();
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
	if (loadedPacks.empty())
		return;

	auto &preview     = currentEntry == -1 ? loadedPacks[currentPack]->preview     : loadedPacks[currentPack]->scenarios[currentEntry]->preview;
	auto &description = currentEntry == -1 ? loadedPacks[currentPack]->description : loadedPacks[currentPack]->scenarios[currentEntry]->description;

	if (preview.texture.hasTexture())
		preview.draw();
	if (description.texture.hasTexture())
		description.draw();
}