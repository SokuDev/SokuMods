//
// Created by PinkySmile on 18/02/2021.
//

#include <SokuLib.hpp>
#include "Gui.hpp"
#include "Moves.hpp"
#include "State.hpp"
#include "Dummy.hpp"

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
		settings = Settings(settings.activated);
		if (!skillsTextures.empty())
			return;
		skillsTextures.resize(SokuLib::CHARACTER_RANDOM);
		for (int i = 0; i < SokuLib::CHARACTER_RANDOM; i++) {
			printf("Loading file %s\n", (std::string(profilePath) + "/assets/skills/" + names[i] + "Skills.png").c_str());
			skillsTextures[i].loadFromFile(std::string(profilePath) + "/assets/skills/" + names[i] + "Skills.png");
		}
	}

	static bool makeFakeCard(SokuLib::CharacterManager &manager, unsigned short id)
	{
		printf("Insert card %i\n", id);
		auto card = manager.addCard(id);

		if (card) {
			card->cost = 1;
			return true;
		}
		printf("Cannot insert card %i !", id);
		return false;
	}

	static void populateCharacterPanel(const std::string &profilePath, tgui::Panel::Ptr panel, SokuLib::CharacterManager &manager, SokuLib::Character character, CharacterState &state)
	{
		char nbSkills = 4;

		panel->loadWidgetsFromFile(profilePath + "/assets/chr.gui");
		if (character == SokuLib::CHARACTER_PATCHOULI) {
			nbSkills = 5;
			panel->get<tgui::Widget>("Skill4Lvl")->setVisible(true);
			panel->get<tgui::Widget>("Skill4Img")->setVisible(true);
			panel->get<tgui::Widget>("Skill4Id")->setVisible(true);
		}
		if (character == SokuLib::CHARACTER_REISEN) {
			panel->get<tgui::Widget>("SpecialPic")->setVisible(true);
			panel->get<tgui::Widget>("SpecialLevel")->setVisible(true);
			panel->get<tgui::Picture>("SpecialPic")->getRenderer()->setTexture(profilePath + "/assets/stats/elixir.png");
			panel->get<tgui::ComboBox>("SpecialLevel")->removeItem("MAX");
			panel->get<tgui::ComboBox>("SpecialLevel")->setSelectedItemByIndex(0);
		}
		if (character == SokuLib::CHARACTER_YUYUKO) {
			panel->get<tgui::Widget>("SpecialPic")->setVisible(true);
			panel->get<tgui::Widget>("SpecialLevel")->setVisible(true);
			panel->get<tgui::Picture>("SpecialPic")->getRenderer()->setTexture(profilePath + "/assets/stats/butterflies.png");
			panel->get<tgui::ComboBox>("SpecialLevel")->setSelectedItemByIndex(0);
		}
		panel->get<tgui::Picture>("DollPic")->getRenderer()->setTexture(profilePath + "/assets/stats/doll.png");
		panel->get<tgui::Picture>("DropPic")->getRenderer()->setTexture(profilePath + "/assets/stats/drop.png");
		panel->get<tgui::Picture>("RodPic")->getRenderer()->setTexture(profilePath + "/assets/stats/rod.png");
		panel->get<tgui::Picture>("GrimPic")->getRenderer()->setTexture(profilePath + "/assets/stats/grimoire.png");
		panel->get<tgui::Picture>("FanPic")->getRenderer()->setTexture(profilePath + "/assets/stats/fan.png");

		for (int i = 0; i < nbSkills; i++) {
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
		panel->get<tgui::Slider>("HP")->connect("ValueChanged", [&state](float newValue){
			state.hp = newValue;
		});
		panel->get<tgui::Button>("Button1")->connect("Clicked", [&manager, character]{
			unsigned last = 100 + 3 * (4 + (character == SokuLib::CHARACTER_PATCHOULI));
			const char *brokenNames[] = {
				"reimu", "marisa", "sakuya", "alice", "patchouli", "youmu", "remilia", "yuyuko", "yukari",
				"suika", "udonge", "aya", "komachi", "iku", "tenshi", "sanae", "chirno", "meirin", "utsuho", "suwako"
			};
			std::vector<unsigned short> cards;

			for (int i = 0; i < 21; i++)
				cards.push_back(i);
			for (int i = 100; i < last; i++)
				cards.push_back(i);

			auto &entry = characterSpellCards[brokenNames[character]];

			for (auto &card : entry)
				cards.push_back(card);
			makeFakeCard(manager, cards[rand() % cards.size()]);
		});
	}

	static void displaySkillsPanel(const std::string &profile)
	{
		panel->loadWidgetsFromFile(profile + "/assets/skills.gui");

		tgui::Panel::Ptr leftPanel = panel->get<tgui::Panel>("Left");
		tgui::Panel::Ptr rightPanel = panel->get<tgui::Panel>("Right");

		populateCharacterPanel(profile, leftPanel,  SokuLib::getBattleMgr().leftCharacterManager,  SokuLib::leftChar,  settings.leftState);
		populateCharacterPanel(profile, rightPanel, SokuLib::getBattleMgr().rightCharacterManager, SokuLib::rightChar, settings.rightState);
		updateGuiState();
	}

	static void updateDummyPanel()
	{
		settings.controlDummy = panel->get<tgui::CheckBox>("Control")->isChecked();
	}

	static void displayDummyPanel(const std::string &profile)
	{
		panel->loadWidgetsFromFile(profile + "/assets/dummy.gui");
		panel->get<tgui::CheckBox>("Control")->setChecked(settings.controlDummy);

		auto airtech = panel->get<tgui::ComboBox>("Airtech");
		auto tech = panel->get<tgui::ComboBox>("Tech");
		auto breaking = panel->get<tgui::Panel>("Breaking");
		auto normal = panel->get<tgui::CheckBox>("Normal");
		auto x = panel->get<tgui::Slider>("X");
		auto y = panel->get<tgui::Slider>("Y");
		auto xen = panel->get<tgui::CheckBox>("XEnabled");
		auto fct = [breaking]{
			for (auto &widget : breaking->getWidgets())
				widget->setEnabled(settings.activated);
		};

		panel->get<tgui::Button>("Button1")->connect("Clicked", []{
			addInputSequence("623b");
		});

		x->setValue(settings.posX != 0 ? settings.posX : 40);
		x->setEnabled(settings.posX != 0);
		xen->connect("Changed", [x](bool b){
			x->setEnabled(b);
			settings.posX = b ? x->getValue() : 0;
		});
		xen->setChecked(settings.posX != 0);

		y->setValue(settings.posY != 0 ? settings.posY : 0);
		y->setEnabled(settings.posY != 0);

		x->connect("ValueChanged", [](float v){
			settings.posX = v;
		});
		y->connect("ValueChanged", [](float v){
			settings.posY = v;
		});

		airtech->setSelectedItemByIndex(settings.airtech);
		tech->setSelectedItemByIndex(settings.tech);
		airtech->connect("ItemSelected", [](int item){
			settings.airtech = static_cast<AirTechDirection>(item);
		});
		tech->connect("ItemSelected", [](int item){
			settings.tech = static_cast<TechDirection>(item);
		});
		fct();
		normal->setChecked(!settings.activated);
		normal->connect("Changed", [fct](bool b){
			if (b)
				removeHooks();
			else
				placeHooks();
			fct();
		});
	}

	static void updateStatePanel()
	{
	}

	static void setNewWeather(SokuLib::Weather weather)
	{
		if (weather == SokuLib::WEATHER_CLEAR) {
			settings.weatherResetRequest = true;
			return;
		}
		SokuLib::displayedWeather = weather;
		SokuLib::activeWeather = SokuLib::WEATHER_CLEAR;
		SokuLib::weatherCounter = 999;
	}

	static void displayStatePanel(const std::string &profile)
	{
		panel->loadWidgetsFromFile(profile + "/assets/state.gui");

		auto force = panel->get<tgui::CheckBox>("ForceWeather");
		auto weather = panel->get<tgui::ComboBox>("Weather");

		force->setChecked(settings.forceWeather);
		force->connect("Changed", [weather](bool newValue){
			settings.forceWeather = newValue;
			weather->setEnabled(newValue);
			if (newValue)
				setNewWeather(static_cast<SokuLib::Weather>(weather->getSelectedItemIndex()));
		});
		weather->setEnabled(settings.forceWeather);
		if (settings.forceWeather && SokuLib::activeWeather != SokuLib::WEATHER_CLEAR)
			weather->setSelectedItemByIndex(SokuLib::displayedWeather);
		else
			weather->setSelectedItemByIndex(SokuLib::WEATHER_CLEAR);
		weather->connect("ItemSelected", [](int item){
			setNewWeather(static_cast<SokuLib::Weather>(item));
		});
	}

	void loadAllGuiElements(LPCSTR profilePath)
	{
		std::string profile = profilePath;

		gui.loadWidgetsFromFile(profile + "/assets/main.gui");
		panel = gui.get<tgui::Panel>("Panel");
		tab = gui.get<tgui::Tabs>("Tabs");
		tab->connect("TabSelected", [profile]{
			switch (tab->getSelectedIndex()) {
			case 0:
				return displaySkillsPanel(profile);
			case 1:
				return displayDummyPanel(profile);
			case 2:
				return displayStatePanel(profile);
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

		//TODO: Manager should be const but is modified here
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
		manager.drops = min(drop->getSelectedItemIndex(), 2);
		if (drop->getSelectedItemIndex() == 3)
			manager.dropInvulTimeLeft = 2;
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
		case 2:
			updateStatePanel();
			break;
		default:
			break;
		}
	}
}