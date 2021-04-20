#include <Windows.h>
#include <SokuLib.hpp>
#include <map>
#include <iostream>
#include <fstream>
#include <nlohmann/json.hpp>
#include "DrawUtils.hpp"

#define FONT_HEIGHT 16
#define TEXTURE_SIZE 0x100

struct Select {};

static bool firstLoad = true;
static bool loaded = false;
static int (Select::*s_originalSelectOnProcess)();
static int (Select::*s_originalSelectCLOnProcess)();
static int (Select::*s_originalSelectSVOnProcess)();
static int (Select::*s_originalSelectOnRender)();
static int (Select::*s_originalSelectCLOnRender)();
static int (Select::*s_originalSelectSVOnRender)();
static int (Select::*s_originalInputMgrGet)();
void (__stdcall *s_origLoadDeckData)(char *, void *, SokuLib::deckInfo &, int, SokuLib::mVC9Dequeue<short> &);

std::array<std::map<unsigned short, DrawUtils::Sprite>, SokuLib::CHARACTER_RANDOM + 1> cardsTextures;
std::map<SokuLib::Character, std::vector<unsigned short>> characterSpellCards{
	{SokuLib::CHARACTER_ALICE, {200, 201, 202, 203, 204, 205, 206, 207, 208, 209, 210, 211}},
	{SokuLib::CHARACTER_AYA, {200, 201, 202, 203, 205, 206, 207, 208, 211, 212}},
	{SokuLib::CHARACTER_CIRNO, {200, 201, 202, 203, 204, 205, 206, 207, 208, 210, 213}},
	{SokuLib::CHARACTER_IKU, {200, 201, 202, 203, 206, 207, 208, 209, 210, 211}},
	{SokuLib::CHARACTER_KOMACHI, {200, 201, 202, 203, 204, 205, 206, 207, 211}},
	{SokuLib::CHARACTER_MARISA, {200, 202, 203, 204, 205, 206, 207, 208, 209, 211, 212, 214, 215, 219}},
	{SokuLib::CHARACTER_MEILING, {200, 201, 202, 203, 204, 205, 206, 207, 208, 209, 211}},
	{SokuLib::CHARACTER_PATCHOULI, {200, 201, 202, 203, 204, 205, 206, 207, 210, 211, 212, 213}},
	{SokuLib::CHARACTER_REIMU, {200, 201, 204, 206, 207, 208, 209, 210, 214, 219}},
	{SokuLib::CHARACTER_REMILIA, {200, 201, 202, 203, 204, 205, 206, 207, 208, 209}},
	{SokuLib::CHARACTER_SAKUYA, {200, 201, 202, 203, 204, 205, 206, 207, 208, 209, 210, 211, 212}},
	{SokuLib::CHARACTER_SANAE, {200, 201, 202, 203, 204, 205, 206, 207, 210}},
	{SokuLib::CHARACTER_SUIKA, {200, 201, 202, 203, 204, 205, 206, 207, 208, 212}},
	{SokuLib::CHARACTER_SUWAKO, {200, 201, 202, 203, 204, 205, 206, 207, 208, 209, 212}},
	{SokuLib::CHARACTER_TENSHI, {200, 201, 202, 203, 204, 205, 206, 207, 208, 209}},
	{SokuLib::CHARACTER_REISEN, {200, 201, 202, 203, 204, 205, 206, 207, 208, 209, 210, 211}},
	{SokuLib::CHARACTER_UTSUHO, {200, 201, 203, 204, 205, 206, 207, 208, 209, 210, 211, 212, 213, 214}},
	{SokuLib::CHARACTER_YOUMU, {200, 201, 202, 203, 204, 205, 206, 207, 208, 212}},
	{SokuLib::CHARACTER_YUKARI, {200, 201, 202, 203, 204, 205, 206, 207, 208, 215}},
	{SokuLib::CHARACTER_YUYUKO, {200, 201, 202, 203, 204, 205, 206, 207, 208, 209, 219}}
};
std::array<std::array<unsigned short, 20>, 20> defaultDecks = {
	std::array<unsigned short, 20>{1, 1, 1, 100, 100, 101, 101, 102, 102, 103, 103, 200, 200, 200, 200, 201, 201, 208, 208, 208},
	std::array<unsigned short, 20>{ 0, 1, 1, 1, 1, 100, 100, 101, 101, 102, 103, 200, 200, 200, 202, 202, 208, 208, 208, 208 },
	std::array<unsigned short, 20>{ 0, 1, 1, 100, 100, 101, 101, 102, 102, 103, 103, 200, 200, 200, 200, 201, 201, 201, 203, 203 },
	std::array<unsigned short, 20>{ 0, 1, 1, 100, 100, 101, 101, 102, 102, 103, 103, 200, 200, 200, 200, 202, 202, 202, 203, 203 },
	std::array<unsigned short, 20>{ 100, 100, 101, 101, 102, 102, 103, 103, 104, 104, 200, 200, 200, 200, 201, 201, 201, 202, 202, 202 },
	std::array<unsigned short, 20>{ 1, 1, 1, 100, 100, 101, 101, 102, 102, 103, 103, 200, 200, 200, 200, 201, 201, 201, 203, 203 },
	std::array<unsigned short, 20>{ 1, 1, 1, 100, 100, 101, 101, 102, 102, 103, 103, 200, 200, 200, 200, 201, 201, 201, 203, 203 },
	std::array<unsigned short, 20>{ 0, 0, 0, 1, 1, 1, 1, 100, 101, 102, 103, 200, 200, 200, 201, 201, 201, 202, 202, 202 },
	std::array<unsigned short, 20>{ 1, 1, 1, 100, 100, 101, 101, 102, 102, 103, 103, 200, 200, 200, 200, 201, 201, 201, 203, 203 },
	std::array<unsigned short, 20>{ 0, 0, 1, 1, 100, 100, 101, 101, 102, 102, 103, 200, 200, 200, 201, 201, 201, 202, 202, 202 },
	std::array<unsigned short, 20>{ 0, 0, 0, 1, 1, 1, 1, 100, 101, 102, 103, 200, 200, 200, 202, 202, 202, 206, 206, 206 },
	std::array<unsigned short, 20>{ 0, 0, 1, 1, 1, 1, 100, 100, 101, 102, 103, 200, 200, 200, 201, 201, 201, 205, 205, 205 },
	std::array<unsigned short, 20>{ 1, 1, 1, 100, 100, 101, 101, 102, 102, 103, 103, 200, 200, 200, 202, 202, 202, 205, 205, 205 },
	std::array<unsigned short, 20>{ 0, 0, 0, 1, 1, 1, 1, 100, 101, 102, 103, 200, 200, 200, 201, 201, 201, 202, 202, 202 },
	std::array<unsigned short, 20>{ 0, 0, 0, 100, 100, 101, 101, 102, 102, 103, 103, 200, 200, 200, 201, 201, 201, 202, 202, 202 },
	std::array<unsigned short, 20>{ 100, 100, 101, 101, 102, 102, 103, 103, 200, 200, 200, 200, 201, 201, 201, 201, 203, 203, 203, 203 },
	std::array<unsigned short, 20>{ 100, 100, 101, 101, 102, 102, 103, 103, 200, 200, 200, 200, 201, 201, 201, 201, 202, 202, 202, 202 },
	std::array<unsigned short, 20>{ 100, 100, 101, 101, 102, 102, 103, 103, 200, 200, 200, 200, 202, 202, 202, 202, 204, 204, 204, 204 },
	std::array<unsigned short, 20>{ 100, 100, 101, 101, 102, 102, 103, 103, 200, 200, 200, 200, 203, 203, 203, 203, 213, 213, 213, 213 },
	std::array<unsigned short, 20>{ 100, 100, 101, 101, 102, 102, 103, 103, 200, 200, 200, 200, 201, 201, 201, 201, 204, 204, 204, 204 }
};
std::array<const char *, SokuLib::CHARACTER_RANDOM> names{
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
	"udonge",
	"aya",
	"komachi",
	"iku",
	"tenshi",
	"sanae",
	"chirno",
	"meirin",
	"utsuho",
	"suwako",
};

struct Deck {
	std::string name;
	std::array<unsigned short, 20> cards;
};

static bool created = false;
static SokuLib::SWRFont font;
static std::array<std::array<std::vector<Deck>, 20>, 3> loadedDecks;
static unsigned upSelectedDeck = 0;
static unsigned downSelectedDeck = 0;
static SokuLib::Trampoline *profileTrampoline;
static SokuLib::Character lastLeft;
static SokuLib::Character lastRight;
static bool generated = false;
static std::array<unsigned short, 20> fakeLeftDeck;
static std::array<unsigned short, 20> fakeRightDeck;
static bool side = false;

#define SENDTO_JUMP_ADDR 0x00857290

unsigned short getRandomCard(const std::vector<unsigned short> &list, const std::map<unsigned short, unsigned char> &other)
{
	unsigned short card;

	try {
		do
			card = list[rand() % list.size()];
		while (other.at(card) >= 4);
	} catch (std::out_of_range &) {}
	return card;
}

void generateFakeDeck(SokuLib::Character chr, SokuLib::Character lastChr, const std::array<unsigned short, 20> *base, std::array<unsigned short, 20> &buffer)
{
	unsigned last = 100 + 3 * (4 + (chr == SokuLib::CHARACTER_PATCHOULI));
	std::map<unsigned short, unsigned char> used;
	std::vector<unsigned short> cards;
	unsigned char c = 0;

	for (int i = 0; i < 21; i++)
		cards.push_back(i);
	for (int i = 100; i < last; i++)
		cards.push_back(i);
	for (auto &card : characterSpellCards[chr])
		cards.push_back(card);

	if (base) {
		int index = 0;

		for (int i = 0; i < 20; i++)
			if ((*base)[i] == 21)
				c++;
			else {
				buffer[index] = (*base)[i];
				if (used.find(buffer[index]) == used.end())
					used[buffer[index]] = 1;
				else
					used[buffer[index]]++;
				index++;
			}
		return;
	} else
		c = 20;

	while (c--) {
		buffer[19 - c] = getRandomCard(cards, used);
		if (used.find(buffer[19 - c]) == used.end())
			used[buffer[19 - c]] = 1;
		else
			used[buffer[19 - c]]++;
	}
}

void generateFakeDeck(SokuLib::Character chr, SokuLib::Character lastChr, unsigned id, const std::vector<Deck> &bases, std::array<unsigned short, 20> &buffer)
{
	if (lastChr == SokuLib::CHARACTER_RANDOM)
		return generateFakeDeck(chr, lastChr, nullptr, buffer);
//#error kkk
	if (id == bases.size() + 1) {
		if (bases.empty())
			return generateFakeDeck(chr, lastChr, nullptr, buffer);
		return generateFakeDeck(chr, lastChr, rand() % bases.size(), bases, buffer);
	} else if (id == bases.size())
		return generateFakeDeck(chr, lastChr, nullptr, buffer);
	else
		return generateFakeDeck(chr, lastChr, &bases[id].cards, buffer);
}

void generateFakeDecks()
{
	if (generated)
		return;
	generated = true;
	generateFakeDeck(SokuLib::leftChar, lastLeft, upSelectedDeck, loadedDecks[0][SokuLib::leftChar], fakeLeftDeck);
	generateFakeDeck(SokuLib::rightChar, lastRight, downSelectedDeck, loadedDecks[0][SokuLib::rightChar], fakeRightDeck);
}

void __stdcall loadDeckData(char *charName, void *csvFile, SokuLib::deckInfo &deck, int param4, SokuLib::mVC9Dequeue<short> &newDeck)
{
	generateFakeDecks();
	side = !side;
	if (side && SokuLib::mainMode != SokuLib::BATTLE_MODE_VSSERVER) //Left
		for (int i = 0; i < 20; i++)
			newDeck[i] = fakeLeftDeck[i];
	else if (!side && SokuLib::mainMode != SokuLib::BATTLE_MODE_VSCLIENT) //Right
		for (int i = 0; i < 20; i++)
			newDeck[i] = fakeRightDeck[i];
	s_origLoadDeckData(charName, csvFile, deck, param4, newDeck);
}

int __stdcall mySendTo(SOCKET s, char *buf, int len, int flags, sockaddr *to, int tolen)
{
	static int left = 0;
	static int right = 0;
	auto packet = reinterpret_cast<SokuLib::Packet *>(buf);

	if (SokuLib::sceneId != SokuLib::SCENE_SELECTCL && SokuLib::sceneId != SokuLib::SCENE_SELECTSV)
		return sendto(s, buf, len, flags, to, tolen);
	if (packet->type == SokuLib::CLIENT_GAME || packet->type == SokuLib::HOST_GAME) {
		if (packet->game.event.type == SokuLib::GAME_MATCH) {
			generateFakeDecks();

			auto &fake = (SokuLib::mainMode == SokuLib::BATTLE_MODE_VSCLIENT ? fakeLeftDeck : fakeRightDeck);
			auto &replace = (SokuLib::mainMode == SokuLib::BATTLE_MODE_VSCLIENT ? packet->game.event.match.host : packet->game.event.match.client());

			memcpy(replace.cards, fake.data(), fake.size() * sizeof(*fake.data()));
		}
		if (packet->game.event.type == SokuLib::GAME_INPUT && packet->game.event.input.sceneId == SokuLib::SCENEID_CHARACTER_SELECT) {
			if (*reinterpret_cast<unsigned char *>(*(int *)0x8a000c + 0x22C0) != 1 && SokuLib::mainMode == SokuLib::BATTLE_MODE_VSCLIENT)
				return sendto(s, buf, len, flags, to, tolen);
			if (*reinterpret_cast<unsigned char *>(*(int *)0x8a000c + 0x22C1) != 1 && SokuLib::mainMode == SokuLib::BATTLE_MODE_VSSERVER)
				return sendto(s, buf, len, flags, to, tolen);

			if (SokuLib::mainMode == SokuLib::BATTLE_MODE_VSSERVER && SokuLib::rightChar != SokuLib::CHARACTER_RANDOM) {
				auto &decks = loadedDecks[1][SokuLib::rightChar];

				if (packet->game.event.input.inputs[0].charSelect.left)
					left++;
				else
					left = 0;

				if (packet->game.event.input.inputs[0].charSelect.right)
					right++;
				else
					right = 0;

				if (left == 1 || (left >= 36 && left % 6 == 0)) {
					SokuLib::playSEWaveBuffer(0x27);
					if (downSelectedDeck == 0) {
						downSelectedDeck = decks.size() + 1;
					} else {
						downSelectedDeck--;
					}
				} else if (right == 1 || (right >= 36 && right % 6 == 0)) {
					SokuLib::playSEWaveBuffer(0x27);
					if (downSelectedDeck == decks.size() + 1) {
						downSelectedDeck = 0;
					} else {
						downSelectedDeck++;
					}
				}
			} else {
				left = 0;
				right = 0;
			}

			char *buffer = new char[len];

			memcpy(buffer, buf, len);
			packet = reinterpret_cast<SokuLib::Packet *>(buffer);
			//Me changing deck ? I would never do that !
			for (int i = 0; i < packet->game.event.input.inputCount; i++) {
				packet->game.event.input.inputs[i].charSelect.left = packet->game.event.input.inputs[i].charSelect.right = false;
			}

			int bytes = sendto(s, buffer, len, flags, to, tolen);

			delete[] buffer;
			return bytes;
		}
	}
	return sendto(s, buf, len, flags, to, tolen);
}

static void sanitizeDeck(SokuLib::Character chr, Deck &deck)
{
	unsigned last = 100 + 3 * (4 + (chr == SokuLib::CHARACTER_PATCHOULI));
	std::map<unsigned short, unsigned char> used;
	std::vector<unsigned short> cards;

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

static void handleProfileChange(int This, SokuLib::String *str)
{
	char *arg = *str;
	std::string profile{arg, strstr(arg, ".pf")};
	int index = 2;

	profile = "profile/" + profile + ".json";
	std::cout << "Loading " << profile << std::endl;

	std::ifstream stream{profile};

	if (!stream) {
		std::cout << "Failed to open file: " << strerror(errno) << std::endl;
		return;
	}

	nlohmann::json json;

	try {
		stream >> json;
		if (json.size() != 20)
			throw std::invalid_argument("Not 20 characters");
		for (auto &arr : json) {
			for (auto &elem : arr) {
				elem.contains("name") && (elem["name"].get<std::string>(), true);
				if (!elem.contains("cards") || elem["cards"].get<std::vector<unsigned short>>().size() != 20)
					throw std::invalid_argument(elem.dump());
			}
		}
	} catch (std::exception &e) {
		MessageBoxA(SokuLib::window, e.what(), "Fatal error", MB_ICONERROR);
		throw;
	}

	if (This == 0x898868) {
		//P1
		index = 0;
	} else if (This == 0x899054) {
		//P2
		index = 1;
	} //Else is deck construct

	auto &arr = loadedDecks[index];

	for (int i = 0; i < 20; i++) {
		auto &array = json[i];

		arr[i].clear();
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
			arr[i].push_back(deck);
		}
	}
}

static int weirdRand(int key, int delay)
{
	static std::map<int, std::pair<int, int>> elems;
	auto it = elems.find(key);

	if (it == elems.end() || it->second.first == 0) {
		int v = rand();

		elems[key] = {delay, v};
		return v;
	}
	it->second.first--;
	return it->second.second;
}

static void initFont() {
	SokuLib::FontDescription desc;

	if (created)
		return;
	created = true;
	desc.r1 = 255;
	desc.r2 = 255;
	desc.g1 = 255;
	desc.g2 = 255;
	desc.b1 = 255;
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
}

static void onProfileChanged()
{
	int This;
	char *arg;

	__asm mov This, esi;
	__asm mov arg, esp;

	arg = *(char **)(arg + 0x28);
	arg += 0x1C;
	handleProfileChange(This, reinterpret_cast<SokuLib::String *>(arg));
}

void renderDeck(SokuLib::Character chr, unsigned select, const std::vector<Deck> &decks, DrawUtils::Vector2<int> pos, const char *overridingName = nullptr)
{
	std::vector<unsigned short> deck;
	std::string name;
	DrawUtils::Vector2<int> base = pos;

//#error lll
	if (select == decks.size()) {
		name = "Randomize deck";
		deck.resize(20, 21);
	} else if (select == decks.size() + 1)
		return renderDeck(chr, weirdRand((int)&decks, 3) % (decks.empty() ? 1 : decks.size()), decks, pos, "Any deck");
	else {
		name = decks[select].name;
		deck = {decks[select].cards.begin(), decks[select].cards.end()};
	}

	if (overridingName)
		name = overridingName;

	if (!deck.empty()) {
		for (int i = 0; i < 10; i++) {
			DrawUtils::Sprite &sprite = (deck[i] < 100 ? cardsTextures[SokuLib::CHARACTER_RANDOM][deck[i]] : cardsTextures[chr][deck[i]]);

			sprite.setPosition(pos);
			sprite.setSize({10, 16});
			sprite.rect.top = sprite.rect.width = 0;
			sprite.rect.width = sprite.texture.getSize().x;
			sprite.rect.height = sprite.texture.getSize().y;
			sprite.tint = DrawUtils::DxSokuColor::White;
			sprite.draw();
			pos.x += 10;
		}
		pos.x = base.x;
		pos.y += 16;
		for (int i = 0; i < 10; i++) {
			DrawUtils::Sprite &sprite = (deck[i + 10] < 100 ? cardsTextures[SokuLib::CHARACTER_RANDOM][deck[i + 10]] : cardsTextures[chr][deck[i + 10]]);

			sprite.setPosition(pos);
			sprite.setSize({10, 16});
			sprite.rect.top = sprite.rect.width = 0;
			sprite.rect.width = sprite.texture.getSize().x;
			sprite.rect.height = sprite.texture.getSize().y;
			sprite.tint = DrawUtils::DxSokuColor::White;
			sprite.draw();
			pos.x += 10;
		}
	}
	pos.y = base.y + 32;

	DrawUtils::Sprite sprite;
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
	sprite.tint = DrawUtils::DxSokuColor::White;
	sprite.draw();
}

static int counter = 0;

int renderingCommon(int ret)
{
	initFont();

	std::string name;
	auto stageUp = *reinterpret_cast<unsigned char *>(*(int *)0x8a000c + 0x22C0);
	auto stageDown = *reinterpret_cast<unsigned char *>(*(int *)0x8a000c + 0x22C1);
	auto &randUp = *reinterpret_cast<bool *>(*(int *)0x8a000c + 0x22C2);
	auto &randDown = *reinterpret_cast<bool *>(*(int *)0x8a000c + 0x22C3);

	if (randUp) {
		randUp = false;
		upSelectedDeck = loadedDecks[0][SokuLib::leftChar].size() + 1;
	}
	if (randDown) {
		randDown = false;
		downSelectedDeck = loadedDecks[1][SokuLib::rightChar].size() + 1;
	}
	if (stageUp == 3 && stageDown == 3) {
		if (counter >= 60)
			return ret;
		else
			counter++;
	} else {
		counter = 0;
		generated = false;
	}

	if (stageUp == 1   && SokuLib::leftChar  != SokuLib::CHARACTER_RANDOM && (SokuLib::sceneId == SokuLib::SCENE_SELECT || SokuLib::mainMode == SokuLib::BATTLE_MODE_VSCLIENT))
		renderDeck(SokuLib::leftChar,  upSelectedDeck,   loadedDecks[0][SokuLib::leftChar],  {28, 98});
	if (stageDown == 1 && SokuLib::rightChar != SokuLib::CHARACTER_RANDOM && (SokuLib::sceneId == SokuLib::SCENE_SELECT || SokuLib::mainMode == SokuLib::BATTLE_MODE_VSSERVER))
		renderDeck(SokuLib::rightChar, downSelectedDeck, loadedDecks[1][SokuLib::rightChar], {28, 384});
	return ret;
}

int __fastcall CSelectCL_OnRender(Select *This) {
	return renderingCommon((This->*s_originalSelectCLOnRender)());
}

int __fastcall CSelectSV_OnRender(Select *This) {
	return renderingCommon((This->*s_originalSelectSVOnRender)());
}

int __fastcall CSelect_OnRender(Select *This) {
	return renderingCommon((This->*s_originalSelectOnRender)());
}

static void loadTexture(DrawUtils::Sprite &container, const char *path, DrawUtils::Vector2<unsigned> size)
{
	int text;

	if (!SokuLib::textureMgr.loadTexture(&text, path, nullptr, nullptr) || !text) {
		MessageBoxA(SokuLib::window, ("Cannot load " + std::string(path)).c_str(), "Fatal error", MB_ICONERROR);
		abort();
	}
	printf("%s: %i\n", path, text);
	container.texture.setHandle(text, size);
}

static void loadCardAssets()
{
	char buffer[128];

	for (int i = 0; i <= 20; i++) {
		sprintf(buffer, "data/card/common/card%03i.bmp", i);
		loadTexture(cardsTextures[SokuLib::CHARACTER_RANDOM][i], buffer, {41, 65});
	}
	loadTexture(cardsTextures[SokuLib::CHARACTER_RANDOM][21], "data/battle/cardFaceDown.bmp", {41, 65});
	for (int j = 0; j < 20; j++) {
		for (int i = (j == SokuLib::CHARACTER_PATCHOULI ? 15 : 12); i; i--) {
			sprintf(buffer, "data/card/%s/card%03i.bmp", names[j], 99 + i);
			loadTexture(cardsTextures[j][99 + i], buffer, {41, 65});
		}
		for (auto &card : characterSpellCards.at(static_cast<SokuLib::Character>(j))) {
			sprintf(buffer, "data/card/%s/card%03i.bmp", names[j], card);
			loadTexture(cardsTextures[j][card], buffer, {41, 65});
		}
	}
}

static int selectProcessCommon(int v)
{
	if (counter < 60) {
		if (lastLeft != SokuLib::leftChar)
			upSelectedDeck = 0;
		lastLeft = SokuLib::leftChar;
		if (lastRight != SokuLib::rightChar)
			downSelectedDeck = 0;
		lastRight = SokuLib::rightChar;
	}
	if (v != SokuLib::SCENE_SELECT && v != SokuLib::SCENE_SELECTSV && v != SokuLib::SCENE_SELECTCL) {
		loaded = false;
		return v;
	}
	if (loaded)
		return v;
	if (firstLoad)
		loadCardAssets();
	loaded = true;
	firstLoad = false;
	return v;
}

int __fastcall CSelectCL_OnProcess(Select *This) {
	return selectProcessCommon((This->*s_originalSelectCLOnProcess)());
}

int __fastcall CSelectSV_OnProcess(Select *This) {
	return selectProcessCommon((This->*s_originalSelectSVOnProcess)());
}

int __fastcall CSelect_OnProcess(Select *This) {
	return selectProcessCommon((This->*s_originalSelectOnProcess)());
}

static void handleInput(const SokuLib::KeyInput &inputs, int index)
{
	auto &selectedDeck = (index == 0 ? upSelectedDeck : downSelectedDeck);
	auto &decks = loadedDecks[index][index == 0 ? SokuLib::leftChar : SokuLib::rightChar];

	if (inputs.horizontalAxis < 0) {
		if (selectedDeck == 0) {
			selectedDeck = decks.size() + 1;
		} else {
			selectedDeck--;
		}
	} else {
		if (selectedDeck == decks.size() + 1) {
			selectedDeck = 0;
		} else {
			selectedDeck++;
		}
	}
}

int __fastcall myGetInput(Select *This) {
	int ret = (This->*s_originalInputMgrGet)();
	auto scene = *(int *)0x8a000c;
	SokuLib::KeyManager *keys;
	auto &battle = SokuLib::getBattleMgr();
	int index = 0;

	if (reinterpret_cast<int>(This) == scene + 0x150) {
		index = 0;
		if (SokuLib::sceneId == SokuLib::SCENE_SELECT) {
			*(int *)(scene + 0x15C) = 0;
			*(int *)(scene + 0x160) = 0;
		}
		keys = reinterpret_cast<SokuLib::KeyManager *>(scene + 0x10);
	} else if (reinterpret_cast<int>(This) == scene + 0x178) {
		index = 1;
		if (SokuLib::sceneId == SokuLib::SCENE_SELECT) {
			*(int *)(scene + 0x184) = 0;
			*(int *)(scene + 0x188) = 0;
		}
		keys = reinterpret_cast<SokuLib::KeyManager *>(scene + 0x14);
	}
	if (ret) {
		int a = abs(keys->keymapManager->input.horizontalAxis);

		if (a != 1 && (a < 36 || a % 6 != 0))
			return ret;
		SokuLib::playSEWaveBuffer(0x27);
		handleInput(keys->keymapManager->input, index);
		return 0;
	}
	return ret;
}


extern "C" __declspec(dllexport) bool CheckVersion(const BYTE hash[16]) {
	return true;
}

extern "C" __declspec(dllexport) bool Initialize(HMODULE hMyModule, HMODULE hParentModule) {
	DWORD old;
	FILE *_;

	AllocConsole();
	freopen_s(&_, "CONOUT$", "w", stdout);
	freopen_s(&_, "CONOUT$", "w", stderr);
	puts("Hey !");
	::VirtualProtect((PVOID)RDATA_SECTION_OFFSET, RDATA_SECTION_SIZE, PAGE_EXECUTE_WRITECOPY, &old);
	s_originalSelectCLOnProcess = SokuLib::union_cast<int (Select::*)()>(SokuLib::TamperDword(
		SokuLib::vtbl_CSelectCL + SokuLib::OFFSET_ON_PROCESS,
		reinterpret_cast<DWORD>(CSelectCL_OnProcess)
	));
	s_originalSelectSVOnProcess = SokuLib::union_cast<int (Select::*)()>(SokuLib::TamperDword(
		SokuLib::vtbl_CSelectSV + SokuLib::OFFSET_ON_PROCESS,
		reinterpret_cast<DWORD>(CSelectSV_OnProcess)
	));
	s_originalSelectOnProcess = SokuLib::union_cast<int (Select::*)()>(SokuLib::TamperDword(
		SokuLib::vtbl_CSelect + SokuLib::OFFSET_ON_PROCESS,
		reinterpret_cast<DWORD>(CSelect_OnProcess)
	));
	s_originalSelectCLOnRender = SokuLib::union_cast<int (Select::*)()>(SokuLib::TamperDword(
		SokuLib::vtbl_CSelectCL + SokuLib::OFFSET_ON_RENDER,
		reinterpret_cast<DWORD>(CSelectCL_OnRender)
	));
	s_originalSelectSVOnRender = SokuLib::union_cast<int (Select::*)()>(SokuLib::TamperDword(
		SokuLib::vtbl_CSelectSV + SokuLib::OFFSET_ON_RENDER,
		reinterpret_cast<DWORD>(CSelectSV_OnRender)
	));
	s_originalSelectOnRender = SokuLib::union_cast<int (Select::*)()>(SokuLib::TamperDword(
		SokuLib::vtbl_CSelect + SokuLib::OFFSET_ON_RENDER,
		reinterpret_cast<DWORD>(CSelect_OnRender)
	));
	SokuLib::TamperDword(SENDTO_JUMP_ADDR, reinterpret_cast<DWORD>(mySendTo));
	::VirtualProtect((PVOID)RDATA_SECTION_OFFSET, RDATA_SECTION_SIZE, old, &old);

	::VirtualProtect((PVOID)TEXT_SECTION_OFFSET, TEXT_SECTION_SIZE, PAGE_EXECUTE_WRITECOPY, &old);
	//Force deck icon to be hidden
	*(unsigned char *)0x4210e2 = 0xEB;
	s_originalInputMgrGet = SokuLib::union_cast<int (Select::*)()>(SokuLib::TamperNearJmpOpr(0x4206B3, reinterpret_cast<DWORD>(myGetInput)));
	s_origLoadDeckData = reinterpret_cast<void (__stdcall *)(char *, void *, SokuLib::deckInfo &, int, SokuLib::mVC9Dequeue<short> &)>(
		SokuLib::TamperNearJmpOpr(0x437D23, reinterpret_cast<DWORD>(loadDeckData))
	);
	::VirtualProtect((PVOID)TEXT_SECTION_OFFSET, TEXT_SECTION_SIZE, old, &old);

	profileTrampoline = new SokuLib::Trampoline(0x435377, onProfileChanged, 7);

	::FlushInstructionCache(GetCurrentProcess(), nullptr, 0);
	return true;
}

extern "C" int APIENTRY DllMain(HMODULE hModule, DWORD fdwReason, LPVOID lpReserved) {
	if (fdwReason == DLL_PROCESS_DETACH) {
		if (profileTrampoline) {
			delete profileTrampoline;
			profileTrampoline = nullptr;
		}
	}
	return TRUE;
}