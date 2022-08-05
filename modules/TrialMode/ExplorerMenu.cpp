//
// Created by Gegel85 on 26/05/2022.
//

#include "ExplorerMenu.hpp"
#include "Menu.hpp"
#include "Socket.hpp"
#include "version.h"
#include "Exceptions.hpp"
#include "Crc32.h"
#include <zip.h>
#include <fstream>
#include <filesystem>
#include <dinput.h>
#include <direct.h>

#define FILTER_TEXT_SIZE 120

extern char serverHost[1024];
extern unsigned short serverPort;

ExplorerMenu::ExplorerMenu() :
	_guides{
		{myModule, MAKEINTRESOURCE(292)},
		{myModule, MAKEINTRESOURCE(296)}
	}
{
	this->_guides[0].active = true;
	this->_title.texture.loadFromResource(myModule, MAKEINTRESOURCE(488));
	this->_title.setSize(this->_title.texture.getSize());
	this->_title.rect.width = this->_title.texture.getSize().x;
	this->_title.rect.height = this->_title.texture.getSize().y;
	this->_loadingThread = std::thread([this]{
		this->_loadPacks();
	});

	this->_nameFilterText.texture.createFromText( "Any name",  defaultFont12, {300, 20}, &this->_nameFilterSize);
	this->_nameFilterText.setSize({
		this->_nameFilterText.texture.getSize().x,
		this->_nameFilterText.texture.getSize().y
	});
	this->_nameFilterText.rect.width = this->_nameFilterText.texture.getSize().x;
	this->_nameFilterText.rect.height = this->_nameFilterText.texture.getSize().y;

	this->_modeFilterText.texture.createFromText( "Any mode",  defaultFont12, {300, 20}, &this->_modeFilterSize);
	this->_modeFilterText.setSize({
		this->_modeFilterText.texture.getSize().x,
		this->_modeFilterText.texture.getSize().y
	});
	this->_modeFilterText.rect.width = this->_modeFilterText.texture.getSize().x;
	this->_modeFilterText.rect.height = this->_modeFilterText.texture.getSize().y;

	this->_topicFilterText.texture.createFromText("Any topic", defaultFont12, {300, 20}, &this->_topicFilterSize);
	this->_topicFilterText.setSize({
		this->_topicFilterText.texture.getSize().x,
		this->_topicFilterText.texture.getSize().y
	});
	this->_topicFilterText.rect.width = this->_topicFilterText.texture.getSize().x;
	this->_topicFilterText.rect.height = this->_topicFilterText.texture.getSize().y;

	this->_loadingText[0].texture.createFromText(("Fetching trial list from<br><color AAAAFF>http://" + std::string(serverHost) + (serverPort == 80 ? "" : ":" + std::to_string(serverPort)) + "</color>").c_str(), defaultFont16, {300, 74});
	for (int i = 0; i < MAX_STEP + 1; i++) {
		this->_loadingText[i].setSize({
			this->_loadingText[0].texture.getSize().x,
			this->_loadingText[0].texture.getSize().y
		});
		this->_loadingText[i].rect.width = this->_loadingText[0].texture.getSize().x;
		this->_loadingText[i].rect.height = this->_loadingText[0].texture.getSize().y;
		this->_loadingText[i].setPosition({174, 218});
	}

	this->_messageBox.texture.loadFromGame("data/menu/21_Base.cv2");
	this->_messageBox.setSize({
		this->_messageBox.texture.getSize().x,
		this->_messageBox.texture.getSize().y
	});
	this->_messageBox.rect.width = this->_messageBox.texture.getSize().x;
	this->_messageBox.rect.height = this->_messageBox.texture.getSize().y;
	this->_messageBox.setPosition({155, 203});

	this->_lockedText.setSize({300, 150});
	this->_lockedText.rect.width = 300;
	this->_lockedText.rect.height = 150;
	this->_lockedText.setPosition({356, 286});
}

ExplorerMenu::~ExplorerMenu()
{
	this->_loadingThread.join();
}

void ExplorerMenu::_renderOnePackBack(Pack &pack, SokuLib::Vector2<float> &pos, bool deployed)
{
	if (this->_nameFilter != -1 && pack.nameStr != uniqueNames[this->_nameFilter])
		return;
	if (this->_modeFilter != -1 && std::find(pack.modes.begin(), pack.modes.end(), uniqueModes[this->_modeFilter]) == pack.modes.end())
		return;
	if (this->_topicFilter != -1 && pack.category != uniqueCategories[this->_topicFilter])
		return;

	if (pos.y >= 100) {
		packContainer.setPosition({
			static_cast<int>(pos.x),
			static_cast<int>(pos.y)
		});
		packContainer.draw();
	}
	pos.y += 35;
	if (deployed && this->_expended) {
		for (int i = this->_entryStart; i < pack.scenarios.size(); i++) {
			pos.y += 15;
			if (pos.y > 379)
				break;
		}
	} else
		pos.y += 5;
}


void ExplorerMenu::_renderOnePack(Pack &pack, SokuLib::Vector2<float> &pos, bool deployed)
{
	int i;
	auto p = pos;
	auto &sprite = pack.error.texture.hasTexture() ? pack.error : pack.author;

	if (this->_nameFilter != -1 && pack.nameStr != uniqueNames[this->_nameFilter])
		return;
	if (this->_modeFilter != -1 && std::find(pack.modes.begin(), pack.modes.end(), uniqueModes[this->_modeFilter]) == pack.modes.end())
		return;
	if (this->_topicFilter != -1 && pack.category != uniqueCategories[this->_topicFilter])
		return;

	//100 <= y <= 406
	if (pos.y >= 100) {
		if (!pack.requirement.empty());
		else if (pack.icon) {
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
		if (!pack.requirement.empty()) {
			lock.setPosition({
				static_cast<int>(pos.x + 18),
				static_cast<int>(pos.y - 14)
			});
			lock.setSize({64, 64});
			lock.draw();
		}

		sprite.setPosition({
			static_cast<int>(pos.x + 75),
			static_cast<int>(pos.y + 17)
		});
		sprite.draw();
	}

	if (this->_currentEntry != -1) {
		p.x += 25;
		p.y += (this->_currentEntry - this->_entryStart) * 15 + 33;
	}
	if (deployed)
		displaySokuCursor(
			{static_cast<int>(p.x + 70), static_cast<int>(p.y + 1)},
			{300, 16}
		);

	if (pos.y >= 100 && pos.y <= 406) {
		pack.name.setPosition({static_cast<int>(pos.x + 74), static_cast<int>(pos.y + 2)});
		pack.name.draw();
	}
	pos.y += 35;

	if (!deployed || !this->_expended) {
		pos.y += 5;
		return;
	}

	for (i = this->_entryStart; i < pack.scenarios.size(); i++) {
		auto curr = i;
		auto &scenario = pack.scenarios[curr];

		if (pos.y >= 100) {
			if (scenario->extra && scenario->score == -1) {
				extraImg.setPosition({static_cast<int>(pos.x + 271), static_cast<int>(pos.y - 5)});
				extraImg.draw();
			}
			if (this->_isLocked(i)) {
				lock.setPosition({static_cast<int>(pos.x + 271), static_cast<int>(pos.y - 10)});
				lock.setSize({32, 32});
				lock.draw();
				if (scenario->nameHiddenIfLocked) {
					questionMarks.setPosition({static_cast<int>(pos.x + 100), static_cast<int>(pos.y)});
					for (int j = 0; j < 4; j++)
						questionMarks.fillColors[j] = scenario->name.fillColors[j];
					questionMarks.draw();
				} else {
					scenario->name.setPosition({static_cast<int>(pos.x + 100), static_cast<int>(pos.y)});
					if (!editorMode)
						scenario->name.tint = SokuLib::DrawUtils::DxSokuColor{0x80, 0x80, 0x80};
					else
						scenario->name.tint = SokuLib::DrawUtils::DxSokuColor::White;
					scenario->name.draw();
				}
			} else {
				scenario->name.setPosition({static_cast<int>(pos.x + 100), static_cast<int>(pos.y)});
				scenario->name.tint = SokuLib::DrawUtils::DxSokuColor::White;
				scenario->name.draw();
				if (scenario->score != -1) {
					scenario->scoreSprite.setPosition({static_cast<int>(pos.x + 271), static_cast<int>(pos.y - 10)});
					scenario->scoreSprite.draw();
				}
			}
		}
		pos.y += 15;
		if (pos.y > 379)
			break;
	}
	if (this->_entryStart) {
		upArrow.setPosition({72, 148});
		upArrow.setSize({16, 16});
		upArrow.draw();
	}
	if (i < pack.scenarios.size() - 1) {
		downArrow.setPosition({72, 372});
		downArrow.setSize({16, 16});
		downArrow.draw();
	}
}

void ExplorerMenu::_()
{
	puts("_ !");
	*(int *)0x882a94 = 0x16;
}

void ExplorerMenu::_checkScrollDown()
{
	if (this->_currentPack < 0) {
		this->_packStart = 0;
		this->_entryStart = 0;
		return;
	}
	if (this->_currentPack >= 0 && this->_currentEntry == -1) {
		if (this->_expended) {
			auto newStart = max(0, min(this->_currentPack, 1.f * this->_currentPack - static_cast<int>(264 - (this->_currentPack == this->_loadedPacks.size() - 1 ? 0 : 20) - 25 - (this->_expended ? 15.f * this->_loadedPacks[this->_currentPack]->scenarios.size() : 0)) / 35));

			this->_packStart = max(this->_packStart, newStart);
		} else if (this->_currentPack - this->_packStart > 6)
			this->_packStart = this->_currentPack - 6;
		this->_entryStart = 0;
		return;
	}
	if (this->_currentEntry - this->_entryStart > 15)
		this->_entryStart = this->_currentEntry - 15;
}

void ExplorerMenu::_checkScrollUp()
{
	if (this->_currentPack < 0) {
		this->_entryStart = 0;
		return;
	}
	if (this->_currentEntry == -1) {
		if (this->_currentPack < this->_packStart)
			this->_packStart = this->_currentPack;
		else if (this->_currentPack - this->_packStart > 6)
			this->_packStart = this->_currentPack - 6;
		return;
	}
	if (this->_currentEntry == this->_loadedPacks[this->_currentPack]->scenarios.size() - 1) {
		this->_packStart = max(0, min(this->_currentPack, 1.f * this->_currentPack - static_cast<int>(264 - (this->_currentPack == this->_loadedPacks.size() - 1 ? 0 : 20) - 35 - 15.f * this->_loadedPacks[this->_currentPack]->scenarios.size()) / 35));
		if (this->_currentEntry > 15)
			this->_entryStart = this->_currentEntry - 15;
		return;
	}
	if (this->_currentEntry < this->_entryStart)
		this->_entryStart = this->_currentEntry;
}

void ExplorerMenu::_handleGoLeft()
{
	if (this->_currentPack >= 0)
		return;
	SokuLib::playSEWaveBuffer(0x27);
	switch (this->_currentPack) {
	case -3:
		this->_nameFilter--;
		if (this->_nameFilter == -2)
			this->_nameFilter = this->_uniqueNames.size() - 1;
		this->_nameFilterText.texture.createFromText(this->_nameFilter == -1  ? "Any name" : this->_uniqueNames[this->_nameFilter].c_str(),  defaultFont12, {300, 20}, &this->_nameFilterSize);
		return;
	case -2:
		this->_modeFilter--;
		if (this->_modeFilter == -2)
			this->_modeFilter = this->_uniqueModes.size() - 1;
		this->_modeFilterText.texture.createFromText(this->_modeFilter == -1  ? "Any mode" : this->_uniqueModes[this->_modeFilter].c_str(),  defaultFont12, {300, 20}, &this->_modeFilterSize);
		return;
	case -1:
		this->_topicFilter--;
		if (this->_topicFilter == -2)
			this->_topicFilter = this->_uniqueCategories.size() - 1;
		this->_topicFilterText.texture.createFromText(this->_topicFilter == -1 ? "Any topic" : this->_uniqueCategories[this->_topicFilter].c_str(), defaultFont12, {300, 20}, &this->_topicFilterSize);
		return;
	default:
		return;
	}
}

void ExplorerMenu::_handleGoRight()
{
	if (this->_currentPack >= 0)
		return;
	SokuLib::playSEWaveBuffer(0x27);
	switch (this->_currentPack) {
	case -3:
		this->_nameFilter++;
		if (this->_nameFilter == this->_uniqueNames.size())
			this->_nameFilter = -1;
		this->_nameFilterText.texture.createFromText(this->_nameFilter == -1  ? "Any name" : this->_uniqueNames[this->_nameFilter].c_str(),  defaultFont12, {300, 20}, &this->_nameFilterSize);
		return;
	case -2:
		this->_modeFilter++;
		if (this->_modeFilter == this->_uniqueModes.size())
			this->_modeFilter = -1;
		this->_modeFilterText.texture.createFromText(this->_modeFilter == -1  ? "Any mode" : this->_uniqueModes[this->_modeFilter].c_str(),  defaultFont12, {300, 20}, &this->_modeFilterSize);
		return;
	case -1:
		this->_topicFilter++;
		if (this->_topicFilter == this->_uniqueCategories.size())
			this->_topicFilter = -1;
		this->_topicFilterText.texture.createFromText(this->_topicFilter == -1 ? "Any topic" : this->_uniqueCategories[this->_topicFilter].c_str(), defaultFont12, {300, 20}, &this->_topicFilterSize);
		return;
	default:
		return;
	}
}

void ExplorerMenu::_handleGoUp()
{
	SokuLib::playSEWaveBuffer(0x27);
	if (this->_currentEntry == -1) {
		do {
			this->_currentEntry = -1;
			this->_currentPack--;
			if (this->_currentPack == -4)
				this->_currentPack += this->_loadedPacks.size() + 3;
			if (this->_currentPack >= 0) {
				//currentEntry += loadedPacks[currentPack]->scenarios.size();
				this->_shownPack = this->_currentPack;
			}
			if (this->_currentPack < 0)
				break;

			auto &pack = this->_loadedPacks[this->_currentPack];

			if (this->_nameFilter != -1 && pack->nameStr != this->_uniqueNames[this->_nameFilter])
				continue;
			if (this->_modeFilter != -1 && std::find(pack->modes.begin(), pack->modes.end(), this->_uniqueModes[this->_modeFilter]) == pack->modes.end())
				continue;
			if (this->_topicFilter != -1 && pack->category != this->_uniqueCategories[this->_topicFilter])
				continue;
			break;
		} while (true);
	} else {
		auto oldEntry = this->_currentEntry;

		this->_currentEntry--;
		if (this->_currentEntry == -1)
			this->_currentEntry = this->_loadedPacks[this->_currentPack]->scenarios.size() - 1;
		this->_loadedPacks[this->_currentPack]->scenarios[oldEntry]->loaded = false;
		if (!this->_loadedPacks[this->_currentPack]->scenarios[oldEntry]->loading && this->_loadedPacks[this->_currentPack]->scenarios[oldEntry]->preview)
			this->_loadedPacks[this->_currentPack]->scenarios[oldEntry]->preview.reset();
		this->_loadedPacks[this->_currentPack]->scenarios[this->_currentEntry]->loaded = true;
		this->_loadedPacks[this->_currentPack]->scenarios[this->_currentEntry]->loadPreview();
	}
	this->_checkScrollUp();
	printf("Pack: %i, Entry %i, Shown %i\n", this->_currentPack, this->_currentEntry, this->_shownPack);
	if (this->_currentEntry != -1 && this->_isLocked(this->_currentEntry)) {
		auto &other = this->_loadedPacks[this->_currentPack]->scenarios[this->_currentEntry - 1];

		this->_lockedText.texture.createFromText(("Unlocked by completing " + (this->_isLocked(this->_currentEntry - 1) && other->nameHiddenIfLocked ? std::string("????????????????") : other->nameStr)).c_str(), defaultFont12, {300, 150});
	} else if (this->_currentEntry == -1 && !this->_loadedPacks[this->_shownPack]->requirement.empty())
		this->_lockedText.texture.createFromText(("Unlocked by completing " + this->_loadedPacks[this->_shownPack]->requirement + "'s<br>episode").c_str(), defaultFont12, {300, 150});
}

void ExplorerMenu::_handleGoDown()
{
	SokuLib::playSEWaveBuffer(0x27);
	if (this->_currentPack < 0 || !this->_expended) {
		do {
			this->_currentPack++;
			if (this->_currentPack == this->_loadedPacks.size())
				this->_currentPack = -3;
			this->_shownPack = max(0, this->_currentPack);
			this->_currentEntry = -1;
			if (this->_currentPack < 0)
				break;

			auto &pack = this->_loadedPacks[this->_currentPack];

			if (this->_nameFilter != -1 && pack->nameStr != this->_uniqueNames[this->_nameFilter])
				continue;
			if (this->_modeFilter != -1 && std::find(pack->modes.begin(), pack->modes.end(), this->_uniqueModes[this->_modeFilter]) == pack->modes.end())
				continue;
			if (this->_topicFilter != -1 && pack->category != this->_uniqueCategories[this->_topicFilter])
				continue;
			break;
		} while (true);
	} else {
		auto oldEntry = this->_currentEntry;

		this->_currentEntry++;
		if (this->_currentEntry == this->_loadedPacks[this->_currentPack]->scenarios.size()) {
			this->_currentEntry = 0;
			if (this->_expended) {
				auto newStart = max(0, min(this->_currentPack, 1.f * this->_currentPack - static_cast<int>(264 - (this->_currentPack == this->_loadedPacks.size() - 1 ? 0 : 20) - 25 - (this->_expended ? 15.f * this->_loadedPacks[this->_currentPack]->scenarios.size() : 0)) / 35));

				this->_packStart = max(this->_packStart, newStart);
			} else if (this->_currentPack - this->_packStart > 6)
				this->_packStart = this->_currentPack - 6;
			this->_entryStart = 0;
		}
		this->_loadedPacks[this->_currentPack]->scenarios[oldEntry]->loaded = false;
		if (!this->_loadedPacks[this->_currentPack]->scenarios[oldEntry]->loading && this->_loadedPacks[this->_currentPack]->scenarios[oldEntry]->preview)
			this->_loadedPacks[this->_currentPack]->scenarios[oldEntry]->preview.reset();
		this->_loadedPacks[this->_currentPack]->scenarios[this->_currentEntry]->loaded = true;
		this->_loadedPacks[this->_currentPack]->scenarios[this->_currentEntry]->loadPreview();
	}
	this->_checkScrollDown();
	printf("Pack: %i, Entry %i, Shown %i\n", this->_currentPack, this->_currentEntry, this->_shownPack);
	if (this->_currentEntry != -1 && this->_isLocked(this->_currentEntry)) {
		auto &other = this->_loadedPacks[this->_currentPack]->scenarios[this->_currentEntry - 1];

		this->_lockedText.texture.createFromText(("Unlocked by completing " + (this->_isLocked(this->_currentEntry - 1) && other->nameHiddenIfLocked ? std::string("????????????????") : other->nameStr)).c_str(), defaultFont12, {300, 150});
	} else if (this->_currentEntry == -1 && !this->_loadedPacks[this->_shownPack]->requirement.empty())
		this->_lockedText.texture.createFromText(("Unlocked by completing " + this->_loadedPacks[this->_shownPack]->requirement + "'s<br>episode").c_str(), defaultFont12, {300, 150});
}

void ExplorerMenu::_handlePlayerInputs(const SokuLib::KeyInput &input)
{
	if (input.changeCard == 1 && this->_currentPack >= 0) {
		auto &pack = this->_loadedPacks[this->_currentPack];
		auto start = 7 + strlen(serverHost) + (serverPort != 80) * (2 + (serverPort / 10000 != 0) + (serverPort / 1000 != 0) + (serverPort / 100 != 0) + (serverPort / 10 % 10 != 0));
		auto link = pack->path.substr(start, pack->path.size() - 1 - start) + ".zip";

		SokuLib::playSEWaveBuffer(0x28);
		this->_loading = true;
		if (this->_loadingThread.joinable())
			this->_loadingThread.join();
		this->_linkDownload = "<color AAAAFF>" + link + "</color>";
		this->_loadingStep = 0;
		this->_loadingText[0].texture.createFromText((this->_linkDownload + "<br>Step 1/" str(MAX_STEP) ": Downloading").c_str(), defaultFont16, {300, 74});
		this->_loadingText[1].texture.createFromText((this->_linkDownload + "<br>Step 2/" str(MAX_STEP) ": Verifying file integrity").c_str(), defaultFont16, {300, 74});
		this->_loadingText[2].texture.createFromText((this->_linkDownload + "<br>Step 3/" str(MAX_STEP) ": Extracting files").c_str(), defaultFont16, {300, 74});
		this->_loadingText[3].texture.createFromText((this->_linkDownload + "<br>Step 4/" str(MAX_STEP) ": Reloading packs").c_str(), defaultFont16, {300, 74});
		this->_loadingText[4].texture.createFromText(("Successfully downloaded<br>" + this->_linkDownload).c_str(), defaultFont16, {300, 74});
		this->_loadingThread = std::thread([this, link]{
			this->_downloadPack(link);
			this->_loading = false;
		});
		return;
	}
	if (input.a == 1 && SokuLib::newSceneId == SokuLib::SCENE_TITLE && this->_currentPack >= 0) {
		if (this->_loadedPacks[this->_currentPack]->scenarios.empty())
			SokuLib::playSEWaveBuffer(0x29);
		else if (this->_currentEntry == -1) {
			if (!this->_loadedPacks[this->_currentPack]->requirement.empty())
				return SokuLib::playSEWaveBuffer(0x29);
			SokuLib::playSEWaveBuffer(0x28);
			this->_expended = true;
			this->_currentEntry = 0;
			this->_packStart = max(0, min(this->_currentPack, 1.f * this->_currentPack - static_cast<int>(264 - (this->_currentPack == this->_loadedPacks.size() - 1 ? 0 : 20) - 35 - 15.f * this->_loadedPacks[this->_currentPack]->scenarios.size()) / 35));
			if (this->_currentEntry > 15)
				this->_entryStart = this->_currentEntry - 15;
		}
	}
	if (input.verticalAxis == -1 || (input.verticalAxis <= -36 && input.verticalAxis % 6 == 0))
		this->_handleGoUp();
	else if (input.verticalAxis == 1 || (input.verticalAxis >= 36 && input.verticalAxis % 6 == 0))
		this->_handleGoDown();
	if (input.horizontalAxis == -1 || (input.horizontalAxis <= -36 && input.horizontalAxis % 6 == 0))
		this->_handleGoLeft();
	else if (input.horizontalAxis == 1 || (input.horizontalAxis >= 36 && input.horizontalAxis % 6 == 0))
		this->_handleGoRight();
}

void ExplorerMenu::_displayFilters()
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

	if (this->_currentPack == -1)
		displaySokuCursor(
			{540 - FILTER_TEXT_SIZE / 2, static_cast<int>(90 + arrowSprite.getSize().y / 2 - 7)},
			{FILTER_TEXT_SIZE + FILTER_TEXT_SIZE / 2, 16}
		);
	else if (this->_currentPack == -2)
		displaySokuCursor(
			{320 - FILTER_TEXT_SIZE / 2, static_cast<int>(80 + arrowSprite.getSize().y / 2 - 7)},
			{FILTER_TEXT_SIZE + FILTER_TEXT_SIZE / 2, 16}
		);
	else if (this->_currentPack == -3)
		displaySokuCursor(
			{100 - FILTER_TEXT_SIZE / 2, static_cast<int>(70 + arrowSprite.getSize().y / 2 - 7)},
			{FILTER_TEXT_SIZE + FILTER_TEXT_SIZE / 2, 16}
		);

	this->_nameFilterText.setPosition({
		static_cast<int>(100 - this->_nameFilterSize.x / 2),
		static_cast<int>(70 + arrowSprite.getSize().y / 2 - this->_nameFilterSize.y / 2)
	});
	this->_nameFilterText.draw();
	this->_modeFilterText.setPosition({
		static_cast<int>(320 - this->_modeFilterSize.x / 2),
		static_cast<int>(80 + arrowSprite.getSize().y / 2 - this->_modeFilterSize.y / 2)
	});
	this->_modeFilterText.draw();
	this->_topicFilterText.setPosition({
		static_cast<int>(540 - this->_topicFilterSize.x / 2),
		static_cast<int>(90 + arrowSprite.getSize().y / 2 - this->_topicFilterSize.y / 2)
	});
	this->_topicFilterText.draw();
}

int ExplorerMenu::onProcess()
{
	if (this->_message) {
		this->_guides[0].active = !this->_expended;
		this->_guides[1].active = this->_expended;
		this->_guides[0].update();
		this->_guides[1].update();
		this->_message = SokuLib::inputMgrs.input.a != 1;
		if (!this->_message)
			SokuLib::playSEWaveBuffer(0x28);
		return true;
	}
	if (this->_error) {
		SokuLib::playSEWaveBuffer(0x29);
		return false;
	}
	if (this->_loading) {
		loadingGear.setRotation(loadingGear.getRotation() + 0.1);
		return true;
	}
	this->_guides[0].active = !this->_expended;
	this->_guides[1].active = this->_expended;
	this->_guides[0].update();
	this->_guides[1].update();
	updateNoiseTexture();
	updateBandTexture();
	if (this->_currentEntry >= 0) {
		auto curr = this->_currentEntry;

		if (!this->_loadedPacks[this->_currentPack]->scenarios[curr]->loading && this->_loadedPacks[this->_currentPack]->scenarios[curr]->preview)
			this->_loadedPacks[this->_shownPack]->scenarios[curr]->preview->update();
		else {
			this->_loadedPacks[this->_currentPack]->scenarios[curr]->loaded = true;
			if (!this->_loadedPacks[this->_currentPack]->scenarios[curr]->loading)
				this->_loadedPacks[this->_currentPack]->scenarios[curr]->loadPreview();
			loadingGear.setRotation(loadingGear.getRotation() + 0.1);
		}
	}
	if (SokuLib::inputMgrs.input.b == 1 && this->_expended) {
		SokuLib::playSEWaveBuffer(0x29);
		this->_expended = false;
		this->_currentEntry = -1;
	} else if (SokuLib::inputMgrs.input.b == 1 || SokuLib::checkKeyOneshot(DIK_ESCAPE, 0, 0, 0)) {
		SokuLib::playSEWaveBuffer(0x29);
		return false;
	}
	this->_handlePlayerInputs(SokuLib::inputMgrs.input);
	return true;
}

int ExplorerMenu::onRender()
{
	SokuLib::Vector2<float> pos = {16, 116};

	this->_title.draw();
	if (this->_message) {
		this->_messageBox.draw();
		this->_loadingText[this->_loadingStep].draw();
		this->_guides[0].render();
		this->_guides[1].render();
		return 0;
	}
	if (this->_error) {
		this->_guides[0].render();
		this->_guides[1].render();
		return 0;
	}
	if (this->_loading) {
		this->_messageBox.draw();
		this->_downloadMutex.lock();
		this->_loadingText[this->_loadingStep].draw();
		this->_downloadMutex.unlock();
		loadingGear.setRotation(-loadingGear.getRotation());
		loadingGear.setPosition({412, 227});
		loadingGear.draw();
		loadingGear.setRotation(-loadingGear.getRotation());
		loadingGear.setPosition({412 + 23, 227 - 18});
		loadingGear.draw();
		return 0;
	}

	this->_displayFilters();
	for (unsigned i = this->_packStart; i < this->_loadedPacks.size(); i++) {
		// 100 <= y <= 364
		this->_renderOnePackBack(*this->_loadedPacks[i], pos, i == this->_currentPack);
		if (pos.y > 394)
			break;
	}
	pos = {16, 116};
	for (unsigned i = this->_packStart; i < this->_loadedPacks.size(); i++) {
		// 100 <= y <= 364
		this->_renderOnePack(*this->_loadedPacks[i], pos, i == this->_currentPack);
		if (pos.y > 394)
			break;
	}

	//Guide *guide = nullptr;

	//for (auto &g : noEditorGuides)
	//	if (g) {
	//		g->render();
	//		if (g->active)
	//			guide = &*g;
	//	}
	//for (auto &g : editorGuides)
	//	if (g) {
	//		g->render();
	//		if (g->active)
	//			guide = &*g;
	//	}

	previewContainer.draw();
	if (this->_loadedPacks.empty()) {
		this->_guides[0].render();
		this->_guides[1].render();
		return 0;
	}

	if (this->_currentEntry < 0) {
		if (this->_loadedPacks[this->_shownPack]->preview.texture.hasTexture())
			this->_loadedPacks[this->_shownPack]->preview.draw();
		if (!this->_loadedPacks[this->_shownPack]->requirement.empty()) {
			this->_lockedText.draw();
			lockedImg.draw();
		} else if (this->_loadedPacks[this->_shownPack]->description.texture.hasTexture())
			this->_loadedPacks[this->_shownPack]->description.draw();
		this->_guides[0].render();
		this->_guides[1].render();
		return 0;
	}
	if (!this->_isLocked(this->_currentEntry)) {
		auto curr = this->_currentEntry;

		if (!this->_loadedPacks[this->_shownPack]->scenarios[curr]->loading) {
			if (this->_loadedPacks[this->_shownPack]->scenarios[curr]->preview && this->_loadedPacks[this->_shownPack]->scenarios[curr]->preview->isValid())
				this->_loadedPacks[this->_shownPack]->scenarios[curr]->preview->render();
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
		if (this->_loadedPacks[this->_shownPack]->scenarios[curr]->description.texture.hasTexture())
			this->_loadedPacks[this->_shownPack]->scenarios[curr]->description.draw();
		CRTBands.draw();
	} else {
		lockedNoise.draw();
		CRTBands.draw();
		if (this->_loadedPacks[this->_shownPack]->scenarios[this->_currentEntry]->extra)
			extraText.draw();
		else
			this->_lockedText.draw();
		lockedImg.draw();
	}
	frame.draw();
	this->_guides[0].render();
	this->_guides[1].render();
	return 0;
}

std::string ExplorerMenu::_downloadPackList()
{
	Socket sock;
	Socket::HttpRequest req;
	nlohmann::json json;

	req.host = serverHost;
	req.header["User-Agent"] = "TrialMode " VERSION_STR;
	req.header["Accept"] = "application/json";
	req.httpVer = "HTTP/1.1";
	req.method = "GET";
	req.portno = serverPort;
	req.path = "/packs";

	auto res = sock.makeHttpRequest(req);

	if (res.returnCode >= 300)
		throw HTTPErrorException(res);
	return res.body;
}

void ExplorerMenu::_loadPacks()
{
	nlohmann::json list;

	try {
		list = nlohmann::json::parse(this->_downloadPackList());
	} catch (HTTPErrorException &e) {
		MessageBox(
			SokuLib::window,
			("Pack list download failed: " + std::string(e.what())).c_str(),
			"Trial pack loading error",
			MB_ICONERROR
		);
		this->_loading = false;
		this->_error = true;
		return;
	} catch (nlohmann::json::exception &e) {
		MessageBox(
			SokuLib::window,
			("Pack list download from " + std::string(serverHost) + " on port " + std::to_string(serverPort) + " returned an invalid json: " + e.what() + ".").c_str(),
			"Trial pack loading error",
			MB_ICONERROR
		);
		this->_loading = false;
		this->_error = true;
		return;
	} catch (std::exception &e) {
		MessageBox(
			SokuLib::window,
			("Pack list download from " + std::string(serverHost) + " on port " + std::to_string(serverPort) + " failed: " + e.what() + ".").c_str(),
			"Trial pack loading error",
			MB_ICONERROR
		);
		this->_loading = false;
		this->_error = true;
		return;
	}

	for (auto &val : list) {
		if (!val.contains("link") || !val["link"].is_string())
			continue;

		auto pack = new Pack(val["link"], val);

		if (!pack->requirement.empty()) {
			pack->name.fillColors[SokuLib::DrawUtils::GradiantRect::RECT_BOTTOM_LEFT_CORNER] = SokuLib::Color{0x40, 0x40, 0x40};
			pack->name.fillColors[SokuLib::DrawUtils::GradiantRect::RECT_BOTTOM_RIGHT_CORNER] = SokuLib::Color{0x40, 0x40, 0x40};
		}
		this->_loadedPacks.emplace_back(pack);
		this->_packsByName[pack->nameStr].push_back(this->_loadedPacks.back());
		this->_packsByCategory[pack->category].push_back(this->_loadedPacks.back());
		if (std::find(this->_uniqueCategories.begin(), this->_uniqueCategories.end(), pack->category) == this->_uniqueCategories.end())
			this->_uniqueCategories.push_back(pack->category);
		if (std::find(this->_uniqueNames.begin(), this->_uniqueNames.end(), pack->nameStr) == this->_uniqueNames.end())
			this->_uniqueNames.push_back(pack->nameStr);
		for (auto &mode : pack->modes) {
			this->_packsByName[mode].push_back(this->_loadedPacks.back());
			if (std::find(this->_uniqueModes.begin(), this->_uniqueModes.end(), mode) == this->_uniqueModes.end())
				this->_uniqueModes.push_back(mode);
		}
	}
	std::sort(this->_loadedPacks.begin(), this->_loadedPacks.end(), [](std::shared_ptr<Pack> pack1, std::shared_ptr<Pack> pack2){
		if (pack1->error.texture.hasTexture() != pack2->error.texture.hasTexture())
			return pack2->error.texture.hasTexture();
		if (pack1->nameStr != pack2->nameStr)
			return pack1->nameStr < pack2->nameStr;
		return pack1->category < pack2->category;
	});
	std::sort(this->_uniqueCategories.begin(), this->_uniqueCategories.end());
	std::sort(this->_uniqueNames.begin(), this->_uniqueNames.end());
	std::sort(this->_uniqueModes.begin(), this->_uniqueModes.end());
	currentPack = -loadedPacks.empty();
	this->_loading = false;
}

bool ExplorerMenu::_isLocked(int entry)
{
	if (entry <= 0)
		return false;
	return this->_loadedPacks[this->_currentPack]->scenarios[entry]->canBeLocked;
}

void ExplorerMenu::_downloadPack(const std::string &link)
{
	Socket socket;
	Socket::HttpRequest req;
	Socket::HttpResponse res;
	std::string folder = link.substr(link.find_last_of('/') + 1);

	folder = std::string(packsLocation, packsLocation + strlen(packsLocation) - 2) + "/" + folder.substr(0, folder.size() - 4);
	req.host = serverHost;
	req.header["User-Agent"] = "TrialMode " VERSION_STR;
	req.header["Accept"] = "application/json";
	req.httpVer = "HTTP/1.1";
	req.method = "GET";
	req.portno = serverPort;
	req.path = link;
	try {
		res = socket.makeHttpRequest(req);
	} catch (HTTPErrorException &e) {
		MessageBox(
			SokuLib::window,
			("Pack download failed: " + std::string(e.what())).c_str(),
			"Trial pack loading error",
			MB_ICONERROR
		);
		return;
	} catch (std::exception &e) {
		MessageBox(
			SokuLib::window,
			("Pack download " + link + " failed: " + e.what() + ".").c_str(),
			"Trial pack loading error",
			MB_ICONERROR
		);
		return;
	}

	this->_downloadMutex.lock();
	this->_loadingStep++;
	this->_downloadMutex.unlock();

	zip_source_t *src;
	zip_error_t error;
	zip_int64_t nb;
	zip_t *za;
	zip_stat_t statObj;

	zip_error_init(&error);
	/* create source from buffer */
	if ((src = zip_source_buffer_create(res.body.c_str(), res.body.size(), 0, &error)) == nullptr) {
		MessageBox(
			SokuLib::window,
			("Pack download " + link + " failed: Can't create zip source: " + zip_error_strerror(&error)).c_str(),
			"Trial pack loading error",
			MB_ICONERROR
		);
		zip_error_fini(&error);
		return;
	}

	/* open zip archive from source */
	if ((za = zip_open_from_source(src, ZIP_RDONLY, &error)) == nullptr) {
		MessageBox(
			SokuLib::window,
			("Pack download " + link + " failed: Can't open zip from source: " + zip_error_strerror(&error)).c_str(),
			"Trial pack loading error",
			MB_ICONERROR
		);
		zip_source_free(src);
		zip_error_fini(&error);
		return;
	}
	nb = zip_get_num_entries(za, 0);

	for (zip_int64_t i = 0; i < nb; i++) {
		zip_stat_index(za, i, 0, &statObj);

		if (!(statObj.valid & ZIP_STAT_NAME))
			continue;
		if (statObj.valid & ZIP_STAT_CRC) {
			zip_file_t *file = zip_fopen_index(za, i, ZIP_FL_COMPRESSED);
			zip_int64_t bytes;
			char buffer[4096];
			uint32_t crc = 0;

			do {
				bytes = zip_fread(file, buffer, sizeof(buffer));
				if (bytes == 0)
					break;
				if (bytes == -1) {
					MessageBox(
						SokuLib::window,
						("Pack download " + link + " failed: Error when reading zip file " + statObj.name + ": " + zip_file_strerror(file)).c_str(),
						"Trial pack loading error",
						MB_ICONERROR
					);
					zip_fclose(file);
					zip_close(za);
					zip_source_free(src);
					zip_error_fini(&error);
					return;
				}
				crc = crc32_fast(buffer, bytes, crc);
			} while (true);
			zip_fclose(file);
			if (statObj.crc != crc) {
				printf("CRC mismatch for file %s: Expected %X but got %X", statObj.name, statObj.crc, crc);
				//MessageBox(
				//	SokuLib::window,
				//	("Pack download " + link + " failed: Error when reading zip file " + statObj.name + ": CRC mismatch (Computed CRC: " + std::to_string(crc) + " vs ZIP CRC " + std::to_string(statObj.crc) + ")").c_str(),
				//	"Trial pack loading error",
				//	MB_ICONERROR
				//);
				//zip_close(za);
				//zip_source_free(src);
				//zip_error_fini(&error);
				//return;
			}
		} else
			printf("Warning: File %s doesn't contain a CRC\n", statObj.name);
		if (
			strcmp(statObj.name, "..") == 0 ||
			strncmp(statObj.name, "../", 3) == 0 ||
			(strlen(statObj.name) > 3 && strcmp(statObj.name + strlen(statObj.name) - 3, "/..") == 0) ||
			strstr(statObj.name, "/../")
		) {
			MessageBox(
				SokuLib::window,
				("Pack download " + link + " failed: Zip contains a relative path: " + statObj.name).c_str(),
				"Trial pack loading error",
				MB_ICONERROR
			);
			zip_close(za);
			zip_source_free(src);
			zip_error_fini(&error);
			return;
		}
		puts(statObj.name);
	}
	zip_error_fini(&error);

	this->_downloadMutex.lock();
	this->_loadingStep++;
	this->_downloadMutex.unlock();
	rename(folder.c_str(), (folder + ".bck").c_str());
	_mkdir(folder.c_str());
	for (zip_int64_t i = 0; i < nb; i++) {
		char *name = strdup(zip_get_name(za, i, 0));
		char *base = name;
		zip_file_t *file = zip_fopen_index(za, i, 0);
		zip_int64_t bytes;
		char buffer[4096];
		std::ofstream stream;

		for (char *ptr = strchr(name, '/'); ptr; ptr = strchr(name, '/')) {
			*ptr = 0;
			_mkdir((folder + "/" + base).c_str());
			*ptr = '/';
			name = ptr + 1;
		}
		stream.open(folder + "/" + base, std::ifstream::binary);
		if (stream.fail()) {
			MessageBox(
				SokuLib::window,
				("Pack download " + link + " failed: Error opening file " + folder + "/" + base + " for writing: " + strerror(errno)).c_str(),
				"Trial pack loading error",
				MB_ICONERROR
			);
			zip_fclose(file);
			zip_close(za);
			zip_source_free(src);
			return;
		}
		do {
			bytes = zip_fread(file, buffer, sizeof(buffer));
			if (bytes == 0)
				break;
			if (bytes == -1) {
				MessageBox(
					SokuLib::window,
					("Pack download " + link + " failed: Error when reading zip file " + statObj.name + ": " + zip_file_strerror(file)).c_str(),
					"Trial pack loading error",
					MB_ICONERROR
				);
				zip_fclose(file);
				zip_close(za);
				zip_source_free(src);
				return;
			}
			stream.write(buffer, bytes);
		} while (true);
		free((void *)base);
	}
	rename((folder + ".bck/score.dat").c_str(), (folder + "/score.dat").c_str());
	std::filesystem::remove_all(folder + ".bck");
	zip_close(za);
	zip_source_free(src);

	this->_downloadMutex.lock();
	this->_loadingStep++;
	this->_downloadMutex.unlock();
	loadedPacks.clear();
	uniqueNames.clear();
	uniqueCategories.clear();
	uniqueModes.clear();
	packsByName.clear();
	packsByCategory.clear();
	loadPacks();

	this->_downloadMutex.lock();
	this->_loadingStep++;
	this->_downloadMutex.unlock();
	this->_message = true;
	SokuLib::playSEWaveBuffer(0x28);
}
