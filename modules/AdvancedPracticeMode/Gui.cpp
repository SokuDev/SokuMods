//
// Created by PinkySmile on 18/02/2021.
//

#include <SokuLib.hpp>
#include "Gui.hpp"
#include "Moves.hpp"
#include "State.hpp"

namespace Practice
{
	tgui::Gui gui;
	std::array<std::string, SokuLib::CHARACTER_RANDOM> names{
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
		"utsuho",
		"suwako",
	};
	std::vector<sf::Texture> skillsTextures;
	static tgui::Panel::Ptr panel;
	static tgui::Tabs::Ptr tab;

	inline void getSkillMap(SokuLib::CharacterManager &manager, char (&skills)[5], SokuLib::Character character)
	{
		char nbSkills = 4 + (character == SokuLib::CHARACTER_PATCHOULI);

		for (char i = nbSkills; i < 3 * nbSkills; i++) {
			if (!manager.skillMap[i].notUsed)
				skills[i % nbSkills] = i / nbSkills;
		}
	}

	void init(LPCSTR profilePath)
	{
		if (!skillsTextures.empty())
			return;
		skillsTextures.resize(SokuLib::CHARACTER_RANDOM);
		for (int i = 0; i < SokuLib::CHARACTER_RANDOM; i++) {
			printf("Loading file %s\n", (std::string(profilePath) + "/assets/skills/" + names[i] + "Skills.png").c_str());
			skillsTextures[i].loadFromFile(std::string(profilePath) + "/assets/skills/" + names[i] + "Skills.png");
		}
		puts("All done");
	}

	static void makeFakeCard(SokuLib::CharacterManager &manager, unsigned short id)
	{
		auto card = manager.addCard(id);

		if (card)
			card->cost = 1;
		else {
			SokuLib::Card card{id, 1};

			puts("No sprite though");
			manager.deckInfos.hand.pushCard(card);
			manager.deckInfos.cardCount = manager.deckInfos.hand.size;
		}
	}

	static void populateCharacterPanel(const std::string &profilePath, tgui::Panel::Ptr panel, SokuLib::CharacterManager &manager, SokuLib::Character character)
	{
		char nbSkills = 4;

		puts("Load chr GUI file");
		panel->loadWidgetsFromFile(profilePath + "/assets/chr.gui");
		if (character == SokuLib::CHARACTER_PATCHOULI) {
			puts("Character is Patchy !");
			nbSkills = 5;
			panel->get<tgui::Widget>("Skill4Lvl")->setVisible(true);
			panel->get<tgui::Widget>("Skill4Img")->setVisible(true);
			panel->get<tgui::Widget>("Skill4Id")->setVisible(true);
		}
		if (character == SokuLib::CHARACTER_REISEN) {
			puts("Character is Reisen !");
			panel->get<tgui::Widget>("SpecialPic")->setVisible(true);
			panel->get<tgui::Widget>("SpecialLevel")->setVisible(true);
			panel->get<tgui::Picture>("SpecialPic")->getRenderer()->setTexture(profilePath + "/assets/stats/elixir.png");
			panel->get<tgui::ComboBox>("SpecialLevel")->removeItem("MAX");
			panel->get<tgui::ComboBox>("SpecialLevel")->setSelectedItemByIndex(0);
		}
		if (character == SokuLib::CHARACTER_YUYUKO) {
			puts("Character is Yuyuko !");
			panel->get<tgui::Widget>("SpecialPic")->setVisible(true);
			panel->get<tgui::Widget>("SpecialLevel")->setVisible(true);
			panel->get<tgui::Picture>("SpecialPic")->getRenderer()->setTexture(profilePath + "/assets/stats/butterflies.png");
			panel->get<tgui::ComboBox>("SpecialLevel")->setSelectedItemByIndex(0);
		}
		puts("Load all textures");
		panel->get<tgui::Picture>("DollPic")->getRenderer()->setTexture(profilePath + "/assets/stats/doll.png");
		panel->get<tgui::Picture>("DropPic")->getRenderer()->setTexture(profilePath + "/assets/stats/drop.png");
		panel->get<tgui::Picture>("RodPic")->getRenderer()->setTexture(profilePath + "/assets/stats/rod.png");
		panel->get<tgui::Picture>("GrimPic")->getRenderer()->setTexture(profilePath + "/assets/stats/grimoire.png");
		panel->get<tgui::Picture>("FanPic")->getRenderer()->setTexture(profilePath + "/assets/stats/fan.png");

		for (int i = 0; i < nbSkills; i++) {
			puts("Set skill thing");

			auto img = panel->get<tgui::Picture>("Skill" + std::to_string(i) + "Img");
			auto lvl = panel->get<tgui::ComboBox>("Skill" + std::to_string(i) + "Lvl");
			auto id = panel->get<tgui::ComboBox>("Skill" + std::to_string(i) + "Id");

			for (int j = 0; j < 3; j++)
				id->addItem(movesNames[character][i + j * nbSkills]);
			id->setSelectedItemByIndex(0);
			lvl->removeAllItems();
			for (int j = 0; j < 5; j++)
				lvl->addItem(j == 4 ? "MAX" : std::to_string(j));
			lvl->setSelectedItemByIndex(0);
			img->getRenderer()->setTexture(
				tgui::Texture(
					skillsTextures[character],
					sf::IntRect(i * 32, 0, 32, 32)
				)
			);
		}
		panel->get<tgui::Button>("Button1")->connect("Clicked", [&manager]{
			makeFakeCard(manager, 200);
		});
	}

	static void displaySkillsPanel(const std::string &profile)
	{
		panel->loadWidgetsFromFile(profile + "/assets/skills.gui");

		tgui::Panel::Ptr leftPanel = panel->get<tgui::Panel>("Left");
		tgui::Panel::Ptr rightPanel = panel->get<tgui::Panel>("Right");

		puts("Populate left GUI");
		populateCharacterPanel(profile, leftPanel,  SokuLib::getBattleMgr().leftCharacterManager,  SokuLib::leftChar);
		puts("Populate Right GUI");
		populateCharacterPanel(profile, rightPanel, SokuLib::getBattleMgr().rightCharacterManager, SokuLib::rightChar);
		puts("Update state");
		updateGuiState();
		puts("All good !");
	}

	static void updateDummyPanel()
	{
		settings.controlDummy = panel->get<tgui::CheckBox>("control")->isChecked();
	}

	static void displayDummyPanel(const std::string &profile)
	{
		panel->loadWidgetsFromFile(profile + "/assets/dummy.gui");

	}

	void loadAllGuiElements(LPCSTR profilePath)
	{
		std::string profile = profilePath;

		puts("Loading GUI file");
		gui.loadWidgetsFromFile(profile + "/assets/main.gui");
		panel = gui.get<tgui::Panel>("Panel");
		tab = gui.get<tgui::Tabs>("Tabs");
		tab->connect("TabSelected", [profile]{
			switch (tab->getSelectedIndex()) {
			case 0:
				return displaySkillsPanel(profile);
			case 1:
				return displayDummyPanel(profile);
			default:
				return panel->removeAllWidgets();
			}
		});
		displaySkillsPanel(profile);
	}

	static void updateCharacterPanel(tgui::Panel::Ptr panel, SokuLib::CharacterManager &manager, SokuLib::Character character)
	{
		char nbSkills = 4 + (character == SokuLib::CHARACTER_PATCHOULI);
		char skills[5] = {0, 0, 0, 0, 0};
		auto doll = panel->get<tgui::ComboBox>("DollLvl");
		auto rod = panel->get<tgui::ComboBox>("RodLvl");
		auto fan = panel->get<tgui::ComboBox>("FanLvl");
		auto grimoire = panel->get<tgui::ComboBox>("GrimLvl");
		auto drop = panel->get<tgui::ComboBox>("DropLvl");
		auto special = panel->get<tgui::ComboBox>("SpecialLevel");

		getSkillMap(manager, skills, character);
		for (int i = 0; i < nbSkills; i++) {
			auto id = panel->get<tgui::ComboBox>("Skill" + std::to_string(i) + "Id");
			auto lvl = panel->get<tgui::ComboBox>("Skill" + std::to_string(i) + "Lvl");
			auto expectedLevel = lvl->getSelectedItemIndex() + (skills[i] != 0);
			int index = id->getSelectedItemIndex();

			if (index != skills[i]) {
				auto img = panel->get<tgui::Picture>("Skill" + std::to_string(i) + "Img");

				for (int j = 0; j < 3; j++) {
					manager.skillMap[i + j * nbSkills].notUsed = true;
					manager.skillMap[i + j * nbSkills].level = -1;
				}
				lvl->removeAllItems();
				for (int j = (index != 0); j < 5; j++)
					lvl->addItem(j == 4 ? "MAX" : std::to_string(j));
				lvl->setSelectedItemByIndex(0);
				manager.skillMap[i + index * nbSkills].notUsed = false;
				manager.skillMap[i + index * nbSkills].level = 1;
				img->getRenderer()->setTexture(
					tgui::Texture(
						skillsTextures[character],
						sf::IntRect((index * nbSkills + i) * 32, 0, 32, 32)
					)
				);
			} else {
				for (int j = 0; j < 3; j++) {
					manager.skillMap[i + j * nbSkills].notUsed = j != skills[i];
					manager.skillMap[i + j * nbSkills].level = j != skills[i] ? -1 : expectedLevel;
				}
			}
		}
		manager.controlRod = rod->getSelectedItemIndex();
		manager.sacrificialDolls = doll->getSelectedItemIndex();
		manager.tenguFans = fan->getSelectedItemIndex();
		manager.drops = drop->getSelectedItemIndex();
		manager.grimoires = grimoire->getSelectedItemIndex();
		if (character == SokuLib::CHARACTER_REISEN)
			manager.elixirUsed = special->getSelectedItemIndex();
		else if (character == SokuLib::CHARACTER_YUYUKO)
			manager.resurrectionButterfliesUsed = special->getSelectedItemIndex();
	}

	void updateGuiState()
	{
		switch (tab->getSelectedIndex()) {
		case 0:
			updateCharacterPanel(panel->get<tgui::Panel>("Left"),  SokuLib::getBattleMgr().leftCharacterManager,  SokuLib::leftChar);
			updateCharacterPanel(panel->get<tgui::Panel>("Right"), SokuLib::getBattleMgr().rightCharacterManager, SokuLib::rightChar);
			break;
		case 1:
			updateDummyPanel();
			break;
		default:
			break;
		}
	}
}