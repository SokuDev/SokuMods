//
// Created by Gegel85 on 26/05/2022.
//

#include "ExplorerMenu.hpp"
#include "Menu.hpp"
#include <dinput.h>

ExplorerMenu::ExplorerMenu()
{
	this->_title.texture.loadFromResource(myModule, MAKEINTRESOURCE(456));
	this->_title.setSize(this->_title.texture.getSize());
	this->_title.rect.width = this->_title.texture.getSize().x;
	this->_title.rect.height = this->_title.texture.getSize().y;
	this->_loadingThread = std::thread([this]{
		this->_loadPacks();
	});
}

ExplorerMenu::~ExplorerMenu()
{
	this->_loadingThread.join();
}

void ExplorerMenu::_()
{
	puts("_ !");
	*(int *)0x882a94 = 0x16;
}

int ExplorerMenu::onProcess()
{
	if (this->_loading)
		return true;
	if (SokuLib::inputMgrs.input.b == 1 || SokuLib::checkKeyOneshot(DIK_ESCAPE, 0, 0, 0)) {
		SokuLib::playSEWaveBuffer(0x29);
		return false;
	}
	return true;
}

int ExplorerMenu::onRender()
{
	SokuLib::Vector2<float> pos = {16, 116};

	//displayFilters();
	this->_title.draw();
	if (this->_loading)
		return 0;

	for (unsigned i = this->_packStart; i < this->_loadedPacks.size(); i++) {
		// 100 <= y <= 364
	//	renderOnePackBack(*this->_loadedPacks[i], pos, i == this->_currentPack);
		if (pos.y > 394)
			break;
	}
	pos = {16, 116};
	for (unsigned i = this->_packStart; i < this->_loadedPacks.size(); i++) {
		// 100 <= y <= 364
	//	renderOnePack(*this->_loadedPacks[i], pos, i == this->_currentPack);
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
	if (this->_loadedPacks.empty())
		return 0;

	if (currentEntry < 0) {
		if (this->_loadedPacks[this->_shownPack]->preview.texture.hasTexture())
			this->_loadedPacks[this->_shownPack]->preview.draw();
		if (this->_loadedPacks[this->_shownPack]->description.texture.hasTexture())
			this->_loadedPacks[this->_shownPack]->description.draw();
		return 0;
	}
	if (!this->_isLocked(currentEntry)) {
		auto curr = currentEntry;

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
		if (this->_loadedPacks[this->_shownPack]->scenarios[currentEntry]->extra)
			extraText.draw();
		else
			this->_lockedText.draw();
		lockedImg.draw();
	}
	frame.draw();
	return 0;
}

void ExplorerMenu::_loadPacks()
{
	nlohmann::json list;
	std::string result = R"([{
	"author": "PinkySmile",
		"category": "Remilia Scarlet",
		"characters": [
	"remilia",
		"aya",
		"sakuya",
		"meirin",
		"chirno",
		"patchouli",
		"flandre",
		"marisa",
		"reimu",
		"udonge"
	],
	"description": "Another boring day for Remilia...<br>Or so she thought, until half of Gensokyo get<br>into her own mansion after hearing a strange<br>rumor. But who is this mysterious intruder<br>everyone is talking about?",
		"icon": {
		"isPath": false,
			"offset": {
			"x": 2.0,
				"y": 0.0
		},
		"path": "data/character/remilia/face/face000.cv2",
			"rect": {
			"height": 54,
				"left": 0,
				"top": 0,
				"width": 133
		},
		"scale": 0.510000467300415,
			"xMirror": true,
			"yMirror": false
	},
	"min_version": "alpha 0.4.0",
		"modes": [
	"Combo",
		"Story"
	],
	"name": "Babylone's Aurora",
		"outro": "outro.lua",
		"scenarios": [
	{
		"description": "2a 5c hj9 j5c j66 j8a | Works everywhere<br><br>",
			"extra": false,
			"file": "bnbmid1.json",
			"may_be_locked": true,
			"name": "The Soul Capturing Intruder",
			"name_hidden_when_locked": false,
			"preview": "bnbmid1.gif"
	},
	{
		"description": "d6 j5aa d6 j5aa d6 j5aa    | Works everywhere<br>d6 j5aa d6 j5aa d6 j5a j8a | Optimized version<br><br>",
			"extra": false,
			"file": "bnbmid2.json",
			"may_be_locked": true,
			"name": "A Crimson Maid Day-to-Day",
			"name_hidden_when_locked": true,
			"preview": "bnbmid2.gif"
	},
	{
		"description": "5aaaa 4SC(Gungnir) hj9 j8a<br>Ender only midscreen, near corner and corner<br><br>",
			"extra": false,
			"file": "bnbmid3.json",
			"may_be_locked": true,
			"name": "A Long Lullaby",
			"name_hidden_when_locked": true,
			"preview": "bnbmid3.gif"
	},
	{
		"description": "j2a CH 2c hj9 j5c j66 j8a | Works everywhere<br><br>",
			"extra": false,
			"file": "bnbmid4.json",
			"may_be_locked": true,
			"name": "Her Voynich Enigma",
			"name_hidden_when_locked": true,
			"preview": "bnbmid4.gif"
	},
	{
		"description": "3a 2a 5c d236b | Tip of 3a only<br><br>",
			"extra": false,
			"file": "bnbmid5.json",
			"may_be_locked": true,
			"name": "Salem's trick",
			"name_hidden_when_locked": true,
			"preview": "bnbmid5.gif"
	},
	{
		"description": "5aaaa 6a 5c | Corner only<br><br>",
			"extra": false,
			"file": "bnbcorn1.json",
			"may_be_locked": true,
			"name": "The Iron Masked Girl",
			"name_hidden_when_locked": true,
			"preview": "bnbcorn1.gif"
	},
	{
		"description": "Rising j5aa j6a j5b | Works everywhere<br><br>",
			"extra": false,
			"file": "bnbcorn2.json",
			"may_be_locked": true,
			"name": "A Tomboyish Maid",
			"name_hidden_when_locked": true,
			"preview": "bnbcorn2.gif"
	},
	{
		"description": "d6 j5aa d6 j5aa 2c hj9 j5aa | Corner only<br><br>",
			"extra": false,
			"file": "bnbcorn3.json",
			"may_be_locked": true,
			"name": "The Curtain Fall?",
			"name_hidden_when_locked": true,
			"preview": "bnbcorn3.gif"
	},
	{
		"description": "4a 4a 4a 4a 5SC(Cradle) 2a 2c hj8 j2c j66 j8a<br>Monsoon only combo<br>",
			"extra": true,
			"file": "bnbany1.json",
			"may_be_locked": true,
			"name": "Curtain Rise On Mr.Norton",
			"name_hidden_when_locked": true,
			"preview": "extra.gif"
	}
	],
	"stand": {
		"isPath": true,
			"path": "title.png"
	}
}])"; //this->_downloadPackList();

	try {
		list = nlohmann::json::parse(result);
	} catch (std::exception &e) {
		MessageBox(
			SokuLib::window,
			("Pack list download from " + this->_link + " returned an invalid json: " + e.what() + ".\n").c_str(),
			"Trial pack loading error",
			MB_ICONERROR
		);
		return;
	}

	for (auto &val : list) {
		if (!val.contains("link") || !val["link"].is_string())
			continue;

		auto pack = new Pack(val["link"], val);

		loadedPacks.emplace_back(pack);

		this->_packsByName[pack->nameStr].push_back(this->_loadedPacks.back());
		this->_packsByCategory[pack->category].push_back(this->_loadedPacks.back());
		if (std::find(this->_uniqueCategories.begin(), uniqueCategories.end(), pack->category) == this->_uniqueCategories.end())
			this->_uniqueCategories.push_back(pack->category);
		if (std::find(this->_uniqueNames.begin(), uniqueNames.end(), pack->nameStr) == this->_uniqueNames.end())
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
		return pack1->category < pack2->category;
	});
	std::sort(this->_uniqueCategories.begin(), this->_uniqueCategories.end());
	std::sort(this->_uniqueNames.begin(), this->_uniqueNames.end());
	std::sort(this->_uniqueModes.begin(), this->_uniqueModes.end());
	this->_loading = false;
}

bool ExplorerMenu::_isLocked(int entry)
{
	if (entry <= 0)
		return false;
	if (!this->_loadedPacks[this->_currentPack]->scenarios[entry]->canBeLocked)
		return false;
	if (!this->_loadedPacks[this->_currentPack]->scenarios[entry]->extra)
		return entry == 0;
	for (auto &scenario : this->_loadedPacks[this->_currentPack]->scenarios)
		if (scenario->score != 3 && !scenario->extra)
			return true;
	return false;
}
