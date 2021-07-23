//
// Created by PinkySmile on 19/07/2021.
//

#include <fstream>
#include "Menu.hpp"
#include "Pack.hpp"

static unsigned currentPack = 0;
static int currentEntry = -1;
static bool loaded = false;
static SokuLib::DrawUtils::Sprite missingIcon;
static SokuLib::DrawUtils::Sprite packContainer;
static SokuLib::DrawUtils::Sprite previewContainer;

bool loadRequest;
SokuLib::SWRFont defaultFont10;
SokuLib::SWRFont defaultFont12;
SokuLib::SWRFont defaultFont16;
HMODULE myModule;
char profilePath[1024 + MAX_PATH];
char profileFolderPath[1024 + MAX_PATH];

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
	desc.weight = FW_BOLD;
	desc.italic = 0;
	desc.shadow = 2;
	desc.bufferSize = 1000000;
	desc.charSpaceX = 0;
	desc.charSpaceY = 0;
	desc.offsetX = 0;
	desc.offsetY = 0;
	desc.useOffset = 0;
	strcpy(desc.faceName, SokuLib::defaultFontName);
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

	*(char *)0x899D0C = value["stage"].get<char>();
	*(char *)0x899D0D = value["music"].get<char>();
	return true;
}

void prepareGameLoading(const std::string &path)
{
	SokuLib::setBattleMode(SokuLib::BATTLE_MODE_VSPLAYER, SokuLib::BATTLE_SUBMODE_PLAYING2);
	if (!prepareReplayBuffer(path))
		return;
	loadRequest = true;
}

void handlePlayerInputs(const SokuLib::KeyInput &input)
{
	if (input.a && currentEntry != -1) {
		puts("Start game !");
		SokuLib::playSEWaveBuffer(0x28);
		prepareGameLoading(loadedPacks[currentPack]->scenarios[currentEntry]->file);
		return;
	}
	if (input.verticalAxis == -1 || (input.verticalAxis <= -36 && input.verticalAxis % 6 == 0)) {
		SokuLib::playSEWaveBuffer(0x27);
		if (currentEntry == -1) {
			currentPack--;
			if (currentPack == -1)
				currentPack += loadedPacks.size();
			currentEntry += loadedPacks[currentPack]->scenarios.size();
		} else
			currentEntry--;
	} else if (input.verticalAxis == 1 || (input.verticalAxis >= 36 && input.verticalAxis % 6 == 0)) {
		SokuLib::playSEWaveBuffer(0x27);
		if (currentEntry == loadedPacks[currentPack]->scenarios.size() - 1) {
			currentPack++;
			if (currentPack == loadedPacks.size())
				currentPack = 0;
			currentEntry = -1;
		} else
			currentEntry++;
	}
}

int menuOnProcess(SokuLib::MenuResult *This)
{
	auto keys = reinterpret_cast<SokuLib::KeyManager *>(0x89A394);

	menuLoadAssets();
	if (keys->keymapManager->input.b) {
		SokuLib::playSEWaveBuffer(0x29);
		return 0;
	}
	handlePlayerInputs(reinterpret_cast<SokuLib::KeyManager *>(0x89A394)->keymapManager->input);
	return !loadRequest;
}

void renderOnePackBack(Pack &pack, SokuLib::Vector2<float> &pos, bool deployed)
{
	packContainer.setPosition({
		static_cast<int>(pos.x),
		static_cast<int>(pos.y)
	});
	packContainer.draw();
	pos.y += 35;
	if (deployed)
		pos.y += 15 * pack.scenarios.size();
	else
		pos.y += 5;
}

void renderOnePack(Pack &pack, SokuLib::Vector2<float> &pos, bool deployed)
{
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

	auto p = pos;
	auto &sprite = pack.error.texture.hasTexture() ? pack.error : pack.author;

	sprite.setPosition({
		static_cast<int>(pos.x + 75),
		static_cast<int>(pos.y + 17)
	});
	sprite.draw();

	if (currentEntry != -1) {
		p.x += 25;
		p.y += currentEntry * 15 + 33;
	}
	if (deployed)
		((void (*)(float, float, float))0x443a50)(p.x + 70, p.y + 1, 300);

	pack.name.setPosition({
		static_cast<int>(pos.x + 74),
		static_cast<int>(pos.y + 2)
	});
	pack.name.draw();
	pos.y += 35;

	if (!deployed) {
		pos.y += 5;
		return;
	}

	for (auto &scenario : pack.scenarios) {
		scenario->name.setPosition({
			static_cast<int>(pos.x + 100),
			static_cast<int>(pos.y)
		});
		scenario->name.draw();
		pos.y += 15;
	};
}

void menuOnRender(SokuLib::MenuResult *This)
{
	SokuLib::Vector2<float> pos{16, 116};

	if (!loaded)
		return;

	for (int i = 0; i < loadedPacks.size(); i++)
		renderOnePackBack(*loadedPacks[i], pos, i == currentPack);
	pos = {16, 116};
	for (int i = 0; i < loadedPacks.size(); i++)
		renderOnePack(*loadedPacks[i], pos, i == currentPack);

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