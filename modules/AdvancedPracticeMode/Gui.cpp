//
// Created by PinkySmile on 18/02/2021.
//

#include <fstream>
#include <SokuLib.hpp>
#include "Gui.hpp"
#include "Moves.hpp"
#include "State.hpp"
#include "Dummy.hpp"
#include "Logic.hpp"
#include "Network.hpp"

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
	tgui::Panel::Ptr panel;
	tgui::Tabs::Ptr tab;

	inline void getSkillMap(SokuLib::Skill *skillMap, char (&skills)[5], SokuLib::Character character)
	{
		char nbSkills = 4 + (character == SokuLib::CHARACTER_PATCHOULI);

		memset(skills, 0, sizeof(skills));
		for (char i = nbSkills; i < 3 * nbSkills; i++) {
			if (!skillMap[i].notUsed)
				skills[i % nbSkills] = i / nbSkills;
		}
	}

	void init(LPCSTR profilePath)
	{
#ifdef NDEBUG
		if (!skillsTextures.empty())
#endif
			return;
		skillsTextures.resize(SokuLib::CHARACTER_RANDOM);
		for (int i = 0; i < SokuLib::CHARACTER_RANDOM; i++) {
			printf("Loading file %s\n", (std::string(profilePath) + "/assets/skills/" + names[i] + "Skills.png").c_str());
			skillsTextures[i].loadFromFile(std::string(profilePath) + "/assets/skills/" + names[i] + "Skills.png");
		}
		puts("Init done");
	}

	static void exportMacrosToFile(const MacroManager &macros, const std::string &file)
	{
		if (!macros.save(file))
			MessageBoxA(SokuLib::window, ("Cannot save macros to file " + file).c_str(), "Saving error", MB_ICONERROR);
	}

	static void exportMacrosToFile(const std::vector<Macro> &macros, const std::string &file)
	{
		std::ofstream stream{file};
		unsigned magic = MAGIC_NUMBER_MACRO_WHOLE_CHARACTER;
		int i = 0;
		unsigned short length = macros.size();

		if (stream.fail()) {
			MessageBoxA(
				SokuLib::window,
				(
					"Cannot save macros to file " + file +
					"\n\nPlease check permissions and disk space.\n\n" +
					file + ": " + strerror(errno)
				).c_str(),
				"Saving error",
				MB_ICONERROR
			);
			return;
		}

		stream.write(reinterpret_cast<char *>(&magic), sizeof(magic));
		stream.write(reinterpret_cast<char *>(&length), sizeof(length));
		for (const auto &macro : macros)
			stream << macro;

		if (stream.fail())
			MessageBoxA(SokuLib::window, ("Cannot save macros to file " + file + "\n\nPlease check permissions and disk space.").c_str(), "Saving error", MB_ICONERROR);
	}

	static void exportMacroToFile(const Macro &macro, const std::string &file)
	{
		std::ofstream stream{file};
		unsigned magic = MAGIC_NUMBER_MACRO_SINGLE_MACRO;
		int i = 0;

		if (stream.fail()) {
			MessageBoxA(
				SokuLib::window,
				(
					"Cannot save macros to file " + file +
					"\n\nPlease check permissions and disk space.\n\n" +
					file + ": " + strerror(errno)
				).c_str(),
				"Saving error",
				MB_ICONERROR
			);
			return;
		}
		stream.write(reinterpret_cast<char *>(&magic), sizeof(magic));
		stream << macro;

		if (stream.fail())
			MessageBoxA(SokuLib::window, ("Cannot save macro to file " + file + "\n\nPlease check permissions and disk space.").c_str(), "Saving error", MB_ICONERROR);
	}

	static std::string exportMacroToString(const Macro &macro)
	{
		std::stringstream stream;
		bool invalid = macro.name.find('\n') != std::string::npos;

		stream << (invalid ? "Invalid macro name" : macro.name) << '\n';
		for (auto &elem : macro.macroElems) {
			auto &input = elem.first;
			auto dir = 5 + (input.horizontalAxis > 0) - (input.horizontalAxis < 0) + ((input.verticalAxis < 0) - (input.verticalAxis > 0)) * 3;

			if (dir != 5 || (!input.a && !input.b && !input.c && !input.d && !input.spellcard && !input.changeCard))
				stream << dir;
			if (input.a)
				stream << "A";
			if (input.b)
				stream << "B";
			if (input.c)
				stream << "C";
			if (input.d)
				stream << "D";
			if (input.spellcard)
				stream << "S";
			if (input.changeCard)
				stream << "s";
			if (elem.second != 1)
				stream << ' ' << elem.second << '\n';
		}
		return stream.str();
	}

	static Macro importMacroFromString(const std::string &str)
	{
		Macro macro;
		std::stringstream buffer{str};
		std::string line;

		std::getline(buffer, macro.name);
		while (std::getline(buffer, line)) {
			std::string command;
			std::stringstream buffer2{line};
			std::pair<SokuLib::KeyInput, unsigned> elem;

			buffer2 >> command;
			if (buffer2.fail() || command.empty())
				continue;

			buffer2 >> elem.second;
			if (buffer2.fail())
				elem.second = 1;

			if (isdigit(command[0])) {
				char v = command[0] - '1';

				elem.first.verticalAxis = -(v / 3 - 1);
				elem.first.horizontalAxis = v % 3 - 1;
			}
			while (!command.empty() && isdigit(command[0]))
				command.erase(command.begin());

			for (auto c : command) {
				switch (c) {
				case 'A':
				case 'a':
					elem.first.a = 1;
					break;
				case 'B':
				case 'b':
					elem.first.b = 1;
					break;
				case 'C':
				case 'c':
					elem.first.c = 1;
					break;
				case 'D':
				case 'd':
					elem.first.d = 1;
					break;
				case 'S':
					elem.first.spellcard = 1;
					break;
				case 's':
					elem.first.changeCard = 1;
					break;
				default:
					break;
				}
			}
			macro.macroElems.push_back(elem);
		}
		return macro;
	}

	static void importMacrosFromFile(std::array<std::vector<Macro>, 20> &buffer, int selectedCharacter, const std::string &file)
	{
		std::ifstream stream{file};
		unsigned magic;
		unsigned short length;
		MacroManager alternate;
		Macro temp;

		if (stream.fail()) {
			MessageBoxA(
				SokuLib::window,
				(
					"Cannot load macro(s) from file " + file +
					"\n\nPlease check permissions and file path.\n\n" +
					file + ": " + strerror(errno)
				).c_str(),
				"Loading error",
				MB_ICONERROR
			);
			return;
		}

		stream.read(reinterpret_cast<char *>(&magic), sizeof(magic));
		switch (magic) {
		case MAGIC_NUMBER_MACRO_WHOLE_CHARACTER:
			stream.read(reinterpret_cast<char *>(&length), sizeof(length));
			for (int i = 0; i < length; i++) {
				stream >> temp;
				buffer[selectedCharacter].push_back(temp);
			}
			break;
		case MAGIC_NUMBER_MACRO_SINGLE_MACRO:
			stream >> temp;
			buffer[selectedCharacter].push_back(temp);
			break;
		case MAGIC_NUMBER_MACRO:
			if (!alternate.load(stream, file, false)) {
				MessageBoxA(SokuLib::window, (
					"Cannot load macro(s) from file " + file +
					"\n\nTbh, it is my bad. Please report this error and attach the file you are trying to load." +
					file + ": " + strerror(errno)
				).c_str(), "Loading error", MB_ICONERROR);
				return;
			}
			settings.nonSaved.macros.import(alternate);
			break;
		default:
			MessageBoxA(
				SokuLib::window,
				(
					"Cannot load macro(s) from file " + file +
					"\n\nPlease check file path." +
					file + ": The magic number doesn't match any known exported macro file type."
				).c_str(),
				"Loading error",
				MB_ICONERROR
			);
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

	static void populateCharacterPanel(const std::string &profilePath, tgui::Panel::Ptr pan, SokuLib::CharacterManager &manager, SokuLib::Character character, CharacterState &state)
	{
		// Levels
		char nbSkills = 4;

		pan->loadWidgetsFromFile(profilePath + "/assets/chr.gui");

		auto spePic = pan->get<tgui::Picture>("SpecialPic");
		auto speLvl = pan->get<tgui::ComboBox>("SpecialLevel");

		auto dollPic = pan->get<tgui::Picture>("DollPic");
		auto dollLvl = pan->get<tgui::ComboBox>("DollLvl");

		auto dropPic = pan->get<tgui::Picture>("DropPic");
		auto dropLvl = pan->get<tgui::ComboBox>("DropLvl");

		auto rodPic = pan->get<tgui::Picture>("RodPic");
		auto rodLvl = pan->get<tgui::ComboBox>("RodLvl");

		auto grimPic = pan->get<tgui::Picture>("GrimPic");
		auto grimLvl = pan->get<tgui::ComboBox>("GrimLvl");

		auto fanPic = pan->get<tgui::Picture>("FanPic");
		auto fanLvl = pan->get<tgui::ComboBox>("FanLvl");

		char skills[5];

		getSkillMap(state.skillMap, skills, character);
		if (character == SokuLib::CHARACTER_PATCHOULI) {
			nbSkills = 5;
			pan->get<tgui::Widget>("Skill4Lvl")->setVisible(true);
			pan->get<tgui::Widget>("Skill4Img")->setVisible(true);
			pan->get<tgui::Widget>("Skill4Id")->setVisible(true);
		}
		if (character == SokuLib::CHARACTER_REISEN) {
			spePic->setVisible(true);
			speLvl->setVisible(true);
			spePic->getRenderer()->setTexture(profilePath + "/assets/stats/elixir.png");
			speLvl->removeItem("MAX");
		}
		if (character == SokuLib::CHARACTER_YUYUKO) {
			spePic->setVisible(true);
			speLvl->setVisible(true);
			spePic->getRenderer()->setTexture(profilePath + "/assets/stats/butterflies.png");
		}

		dollPic->getRenderer()->setTexture(profilePath + "/assets/stats/doll.png");
		dropPic->getRenderer()->setTexture(profilePath + "/assets/stats/drop.png");
		rodPic->getRenderer()->setTexture(profilePath + "/assets/stats/rod.png");
		grimPic->getRenderer()->setTexture(profilePath + "/assets/stats/grimoire.png");
		fanPic->getRenderer()->setTexture(profilePath + "/assets/stats/fan.png");
		dollLvl->setSelectedItemByIndex(state.dollLevel);
		dropLvl->setSelectedItemByIndex(state.dropLevel);
		rodLvl->setSelectedItemByIndex(state.rodLevel);
		grimLvl->setSelectedItemByIndex(state.grimoireLevel);
		fanLvl->setSelectedItemByIndex(state.fanLevel);
		speLvl->setSelectedItemByIndex(state.specialValue);
		dollLvl->connect("ItemSelected", [&state](int item){
			state.dollLevel = item;
		});
		dropLvl->connect("ItemSelected", [&state](int item){
			state.dropLevel = item;
		});
		rodLvl->connect("ItemSelected", [&state](int item){
			state.rodLevel = item;
		});
		grimLvl->connect("ItemSelected", [&state](int item){
			state.grimoireLevel = item;
		});
		fanLvl->connect("ItemSelected", [&state](int item){
			state.fanLevel = item;
		});
		speLvl->connect("ItemSelected", [&state](int item){
			state.specialValue = item;
		});

		for (int i = 0; i < nbSkills; i++) {
			auto img = pan->get<tgui::Picture>("Skill" + std::to_string(i) + "Img");
			auto lvl = pan->get<tgui::ComboBox>("Skill" + std::to_string(i) + "Lvl");
			auto id = pan->get<tgui::ComboBox>("Skill" + std::to_string(i) + "Id");
			auto callback = std::make_shared<unsigned>(0);

			lvl->removeAllItems();
			for (int j = skills[i] != 0; j < 5; j++)
				lvl->addItem(j == 4 ? "MAX" : std::to_string(j));
			lvl->setSelectedItemByIndex(state.skillMap[skills[i] * nbSkills + i].level - (skills[i] != 0));
			*callback = lvl->connect("ItemSelected", [&state, i, nbSkills](std::weak_ptr<tgui::ComboBox> skill, int item){
				auto index = skill.lock()->getSelectedItemIndex();

				for (int j = 0; j < 3; j++) {
					if (j == index) {
						state.skillMap[j * nbSkills + i].notUsed = false;
						state.skillMap[j * nbSkills + i].level = item + (index != 0);
					} else {
						state.skillMap[j * nbSkills + i].notUsed = true;
						state.skillMap[j * nbSkills + i].level = 0x7F;
					}
				}
			}, std::weak_ptr<tgui::ComboBox>(id));

			for (int j = 0; j < 3; j++)
				id->addItem(movesNames[character][i + j * nbSkills]);
			id->setSelectedItemByIndex(skills[i]);
			id->connect("ItemSelected", [lvl, img, &state, nbSkills, i, character, callback](std::weak_ptr<tgui::ComboBox> skill, int item){
				lvl->disconnect(*callback);
				lvl->removeAllItems();
				for (int j = item != 0; j < 5; j++)
					lvl->addItem(j == 4 ? "MAX" : std::to_string(j));
				*callback = lvl->connect("ItemSelected", [&state, i, nbSkills, skill](int item){
					auto index = skill.lock()->getSelectedItemIndex();

					for (int j = 0; j < 3; j++) {
						if (j == index) {
							state.skillMap[j * nbSkills + i].notUsed = false;
							state.skillMap[j * nbSkills + i].level = item + (index != 0);
						} else {
							state.skillMap[j * nbSkills + i].notUsed = true;
							state.skillMap[j * nbSkills + i].level = 0x7F;
						}
					}
				});
				lvl->setSelectedItemByIndex(0);

				img->getRenderer()->setTexture(
					tgui::Texture(
						skillsTextures[character],
						sf::IntRect((item * nbSkills + i) * 32, 0, 32, 32)
					)
				);
			}, std::weak_ptr<tgui::ComboBox>(id));

			img->getRenderer()->setTexture(
				tgui::Texture(
					skillsTextures[character],
					sf::IntRect((skills[i] * nbSkills + i) * 32, 0, 32, 32)
				)
			);
		}

		pan->get<tgui::Button>("Button1")->connect("Clicked", [&manager, character]{
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

		// HP and SP
		auto HP = pan->get<tgui::Slider>("HP");
		auto SP = pan->get<tgui::Slider>("SP");
		auto brokenOrbs = pan->get<tgui::Slider>("BrokenOrbs");
		HP->connect("ValueChanged", [&state](float newValue) { state.hp = newValue; });
		SP->connect("ValueChanged", [&state](float newValue) { state.maxCurrentSpirit = newValue; });
		brokenOrbs->connect("ValueChanged", [&state](float newValue) { state.brokenOrbs = newValue; });
		HP->setValue(state.hp);
		SP->setValue(state.maxCurrentSpirit);
		brokenOrbs->setValue(state.brokenOrbs);

		pan->get<tgui::CheckBox>("HPInstantRegen")->setChecked(state.HPInstantRegen);
		pan->get<tgui::CheckBox>("SPInstantRegen")->setChecked(state.SPInstantRegen);
	}

	static void displaySkillsPanel(const std::string &profile)
	{
		panel->loadWidgetsFromFile(profile + "/assets/skills.gui");

		tgui::Panel::Ptr leftPanel = panel->get<tgui::Panel>("Left");
		tgui::Panel::Ptr rightPanel = panel->get<tgui::Panel>("Right");

		populateCharacterPanel(profile, leftPanel,  SokuLib::getBattleMgr().leftCharacterManager,  SokuLib::leftChar,  settings.nonSaved.leftState);
		populateCharacterPanel(profile, rightPanel, SokuLib::getBattleMgr().rightCharacterManager, SokuLib::rightChar, settings.nonSaved.rightState);
		updateGuiState();
	}

	static void updateDummyPanel()
	{
		settings.nonSaved.controlDummy = panel->get<tgui::CheckBox>("Control")->isChecked();
	}

	static void displayDummyPanel(const std::string &profile)
	{
		panel->loadWidgetsFromFile(profile + "/assets/dummy.gui");
		panel->get<tgui::CheckBox>("Control")->setChecked(settings.nonSaved.controlDummy);

		auto block = panel->get<tgui::ComboBox>("Block");
		auto airtech = panel->get<tgui::ComboBox>("Airtech");
		auto tech = panel->get<tgui::ComboBox>("Tech");
		auto breaking = panel->get<tgui::Panel>("Breaking");
		auto normal = panel->get<tgui::CheckBox>("Normal");
		auto x = panel->get<tgui::Slider>("X");
		auto y = panel->get<tgui::Slider>("Y");
		auto xen = panel->get<tgui::CheckBox>("XEnabled");
		auto fct = [breaking]{
			for (auto &widget : breaking->getWidgets())
				widget->setEnabled(settings.nonSaved.activated);
		};

		panel->get<tgui::Button>("Button1")->connect("Clicked", []{
			//static int i = 0;
			//static const char *s[] = {"d 2d", "d 1d", "d 4d", "d 6d"};

			//addInputSequence(s[i]);
			//i = (i + 1) % 4;
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

		block->setSelectedItemByIndex(settings.block);
		airtech->setSelectedItemByIndex(settings.airtech);
		tech->setSelectedItemByIndex(settings.tech);
		block->connect("ItemSelected", [](int item){
			settings.block = static_cast<BlockingSide>(item);
		});
		airtech->connect("ItemSelected", [](int item){
			settings.airtech = static_cast<AirTechDirection>(item);
		});
		tech->connect("ItemSelected", [](int item){
			settings.tech = static_cast<TechDirection>(item);
		});
		fct();
		normal->setChecked(!settings.nonSaved.activated);
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
		auto editTimer = panel->get<tgui::EditBox>("EditTimer");
		auto counter = panel->get<tgui::ComboBox>("Counter");
		auto frameRate = panel->get<tgui::EditBox>("FPS");
		auto frameStep = panel->get<tgui::Button>("Step");
		auto leftHurtBoxes        = panel->get<tgui::CheckBox>("LeftHurtBoxes");
		auto leftHitBoxes         = panel->get<tgui::CheckBox>("LeftHitBoxes");
		auto leftPosBox           = panel->get<tgui::CheckBox>("LeftPosBox");
		auto leftCollisionBox     = panel->get<tgui::CheckBox>("LeftCollisionBox");
		auto leftBuff             = panel->get<tgui::CheckBox>("LeftBuff");
		auto leftHitProperties    = panel->get<tgui::CheckBox>("LeftHitProperties");
		auto leftSubObjHurtBoxes  = panel->get<tgui::CheckBox>("LeftSubObjHurtBoxes");
		auto leftSubObjHitBoxes   = panel->get<tgui::CheckBox>("LeftSubObjHitBoxes");
		auto leftSubObjPosBox     = panel->get<tgui::CheckBox>("LeftSubObjPosBox");
		auto leftSubObjProperties = panel->get<tgui::CheckBox>("LeftSubObjProperties");
		auto rightHurtBoxes       = panel->get<tgui::CheckBox>("RightHurtBoxes");
		auto rightHitBoxes        = panel->get<tgui::CheckBox>("RightHitBoxes");
		auto rightPosBox          = panel->get<tgui::CheckBox>("RightPosBox");
		auto rightCollisionBox    = panel->get<tgui::CheckBox>("RightCollisionBox");
		auto rightBuff            = panel->get<tgui::CheckBox>("RightBuff");
		auto rightHitProperties   = panel->get<tgui::CheckBox>("RightHitProperties");
		auto rightSubObjHurtBoxes = panel->get<tgui::CheckBox>("RightSubObjHurtBoxes");
		auto rightSubObjHitBoxes  = panel->get<tgui::CheckBox>("RightSubObjHitBoxes");
		auto rightSubObjPosBox    = panel->get<tgui::CheckBox>("RightSubObjPosBox");
		auto rightSubObjProperties= panel->get<tgui::CheckBox>("RightSubObjProperties");

		leftHurtBoxes->setChecked(settings.leftHitboxSettings.showHurtboxes);
		leftHitBoxes->setChecked(settings.leftHitboxSettings.showHitboxes);
		leftPosBox->setChecked(settings.leftHitboxSettings.showPosition);
		leftCollisionBox->setChecked(settings.leftHitboxSettings.showCollisionBox);
		leftBuff->setChecked(settings.leftHitboxSettings.showBuffProperties);
		leftHitProperties->setChecked(settings.leftHitboxSettings.showHitProperties);
		leftSubObjHurtBoxes->setChecked(settings.leftHitboxSettings.showSubObjectHurtboxes);
		leftSubObjHitBoxes->setChecked(settings.leftHitboxSettings.showSubObjectHitboxes);
		leftSubObjPosBox->setChecked(settings.leftHitboxSettings.showSubObjectPosition);
		leftSubObjProperties->setChecked(settings.leftHitboxSettings.showSubObjectProperties);
		rightHurtBoxes->setChecked(settings.rightHitboxSettings.showHurtboxes);
		rightHitBoxes->setChecked(settings.rightHitboxSettings.showHitboxes);
		rightPosBox->setChecked(settings.rightHitboxSettings.showPosition);
		rightCollisionBox->setChecked(settings.rightHitboxSettings.showCollisionBox);
		rightBuff->setChecked(settings.rightHitboxSettings.showBuffProperties);
		rightHitProperties->setChecked(settings.rightHitboxSettings.showHitProperties);
		rightSubObjHurtBoxes->setChecked(settings.rightHitboxSettings.showSubObjectHurtboxes);
		rightSubObjHitBoxes->setChecked(settings.rightHitboxSettings.showSubObjectHitboxes);
		rightSubObjPosBox->setChecked(settings.rightHitboxSettings.showSubObjectPosition);
		rightSubObjProperties->setChecked(settings.rightHitboxSettings.showSubObjectProperties);

		leftHurtBoxes->connect("Changed", [](bool b){
			settings.leftHitboxSettings.showHurtboxes = b;
		});
		leftHitBoxes->connect("Changed", [](bool b){
			settings.leftHitboxSettings.showHitboxes = b;
		});
		leftPosBox->connect("Changed", [](bool b){
			settings.leftHitboxSettings.showPosition = b;
		});
		leftCollisionBox->connect("Changed", [](bool b){
			settings.leftHitboxSettings.showCollisionBox = b;
		});
		leftBuff->connect("Changed", [](bool b){
			settings.leftHitboxSettings.showBuffProperties = b;
		});
		leftHitProperties->connect("Changed", [](bool b){
			settings.leftHitboxSettings.showHitProperties = b;
		});
		leftSubObjHurtBoxes->connect("Changed", [](bool b){
			settings.leftHitboxSettings.showSubObjectHurtboxes = b;
		});
		leftSubObjHitBoxes->connect("Changed", [](bool b){
			settings.leftHitboxSettings.showSubObjectHitboxes = b;
		});
		leftSubObjPosBox->connect("Changed", [](bool b){
			settings.leftHitboxSettings.showSubObjectPosition = b;
		});
		leftSubObjProperties->connect("Changed", [](bool b){
			settings.leftHitboxSettings.showSubObjectProperties = b;
		});
		rightHurtBoxes->connect("Changed", [](bool b){
			settings.rightHitboxSettings.showHurtboxes = b;
		});
		rightHitBoxes->connect("Changed", [](bool b){
			settings.rightHitboxSettings.showHitboxes = b;
		});
		rightPosBox->connect("Changed", [](bool b){
			settings.rightHitboxSettings.showPosition = b;
		});
		rightCollisionBox->connect("Changed", [](bool b){
			settings.rightHitboxSettings.showCollisionBox = b;
		});
		rightBuff->connect("Changed", [](bool b){
			settings.rightHitboxSettings.showBuffProperties = b;
		});
		rightHitProperties->connect("Changed", [](bool b){
			settings.rightHitboxSettings.showHitProperties = b;
		});
		rightSubObjHurtBoxes->connect("Changed", [](bool b){
			settings.rightHitboxSettings.showSubObjectHurtboxes = b;
		});
		rightSubObjHitBoxes->connect("Changed", [](bool b){
			settings.rightHitboxSettings.showSubObjectHitboxes = b;
		});
		rightSubObjPosBox->connect("Changed", [](bool b){
			settings.rightHitboxSettings.showSubObjectPosition = b;
		});
		rightSubObjProperties->connect("Changed", [](bool b){
			settings.rightHitboxSettings.showSubObjectProperties = b;
		});

		frameRate->setText(std::to_string(settings.requestedFrameRate));
		auto fct = [](std::weak_ptr<tgui::EditBox> boxWeak){
			auto box = boxWeak.lock();

			if (box->getText().isEmpty())
				return;
			settings.requestedFrameRate = std::stoul(box->getText().toAnsiString());
		};
		frameRate->connect("TextChanged", fct, std::weak_ptr<tgui::EditBox>(frameRate));
		frameRate->connect("ReturnKeyPressed", fct, std::weak_ptr<tgui::EditBox>(frameRate));
		frameStep->connect("Clicked", [frameRate]{
			extern float frameCounter;
			if (frameRate->getText() != "0")
				frameRate->setText("0");
			else
				frameCounter++;
		});

		std::string editTimerStr = std::to_string(settings.weatherTime / 10) + "." + std::to_string(settings.weatherTime % 10);
		editTimer->setText(editTimerStr);

		editTimer->connect("TextChanged", [](std::string time) {
			settings.weatherTime = !time.empty() ? atof(time.c_str()) * 10 : 999;
		});

		counter->setSelectedItemByIndex(settings.counter);
		counter->connect("ItemSelected", [](int item){
			settings.counter = static_cast<Counter>(item);
		});

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

	static void displayMiscPanel(const std::string &profile)
	{
		panel->loadWidgetsFromFile(profile + "/assets/misc.gui");

		auto leftBox = panel->get<tgui::CheckBox>("LeftInputs");
		auto rightBox = panel->get<tgui::CheckBox>("RightInputs");
		auto leftJoypad = panel->get<tgui::CheckBox>("LeftJoypad");
		auto rightJoypad = panel->get<tgui::CheckBox>("RightJoypad");
		auto rawBox = panel->get<tgui::CheckBox>("RawInputs");
		auto emptyBox = panel->get<tgui::CheckBox>("Empty");
		auto untech = panel->get<tgui::CheckBox>("Untech");
		auto gap = panel->get<tgui::CheckBox>("Gaps");
		auto frameadv = panel->get<tgui::CheckBox>("FrameAdvantage");
		auto blockstun = panel->get<tgui::CheckBox>("Blockstun");
		auto hitstun = panel->get<tgui::CheckBox>("Hitstun");

		leftBox->setChecked(settings.showLeftInputBox);
		leftBox->connect("Changed", [](bool b){
			settings.showLeftInputBox = b;
		});

		rightBox->setChecked(settings.showRightInputBox);
		rightBox->connect("Changed", [](bool b){
			settings.showRightInputBox = b;
		});

		leftJoypad->setChecked(settings.showLeftJoypad);
		leftJoypad->connect("Changed", [](bool b){
			settings.showLeftJoypad = b;
		});

		rightJoypad->setChecked(settings.showRightJoypad);
		rightJoypad->connect("Changed", [](bool b){
			settings.showRightJoypad = b;
		});

		rawBox->setChecked(settings.showRawInputs);
		rawBox->connect("Changed", [](bool b){
			settings.showRawInputs = b;
		});

		emptyBox->setChecked(settings.showEmptyInputs);
		emptyBox->connect("Changed", [](bool b){
			settings.showEmptyInputs = b;
		});

		untech->setChecked(settings.showUntech);
		untech->connect("Changed", [](bool b){
			settings.showUntech = b;
		});

		gap->setChecked(settings.showGaps);
		gap->connect("Changed", [](bool b){
			settings.showGaps = b;
		});

		frameadv->setChecked(settings.showFrameAdvantage);
		frameadv->connect("Changed", [](bool b){
			settings.showFrameAdvantage = b;
		});

		blockstun->setChecked(settings.showBlockstun);
		blockstun->connect("Changed", [](bool b){
			settings.showBlockstun = b;
		});

		hitstun->setChecked(settings.showHitstun);
		hitstun->connect("Changed", [](bool b){
			settings.showHitstun = b;
		});
	}

	static void showMacroInputs(const std::string &profile, std::weak_ptr<tgui::ComboBox> macrosWeak, tgui::EditBox::Ptr name, tgui::ScrollablePanel::Ptr inputPanel, tgui::ComboBox::Ptr character)
	{
		auto macros = macrosWeak.lock();
		auto &allMacros = settings.nonSaved.macros.macros[character->getSelectedItemIndex()];

		inputPanel->removeAllWidgets();
		if (macros->getSelectedItemIndex() >= allMacros.size())
			macros->setSelectedItemByIndex(allMacros.size() - 1);
		if (macros->getSelectedItemIndex() < 0)
			return;

		auto &selected = allMacros[macros->getSelectedItemIndex()];
		auto &elems = selected.macroElems;
		int i = 0;

		name->setText(selected.name);
		for (auto &elem : elems) {
			auto pan = tgui::Panel::create({615, 22});

			pan->loadWidgetsFromFile(profile + "/assets/macroElem.gui");

			auto A = pan->get<tgui::CheckBox>("A");
			auto B = pan->get<tgui::CheckBox>("B");
			auto C = pan->get<tgui::CheckBox>("C");
			auto D = pan->get<tgui::CheckBox>("D");
			auto SC = pan->get<tgui::CheckBox>("SC");
			auto CH = pan->get<tgui::CheckBox>("CH");
			auto H = pan->get<tgui::Slider>("H");
			auto V = pan->get<tgui::Slider>("V");
			auto duration = pan->get<tgui::EditBox>("Duration");
			auto remove = pan->get<tgui::Button>("Remove");

			A->setChecked(elem.first.a);
			B->setChecked(elem.first.b);
			C->setChecked(elem.first.c);
			D->setChecked(elem.first.d);
			SC->setChecked(elem.first.spellcard);
			CH->setChecked(elem.first.changeCard);
			H->setValue(elem.first.horizontalAxis);
			V->setValue(elem.first.verticalAxis);
			duration->setText(std::to_string(elem.second));

			A->connect("Changed", [&elem](bool b){
				elem.first.a = b;
			});
			B->connect("Changed", [&elem](bool b){
				elem.first.b = b;
			});
			C->connect("Changed", [&elem](bool b){
				elem.first.c = b;
			});
			D->connect("Changed", [&elem](bool b){
				elem.first.d = b;
			});
			SC->connect("Changed", [&elem](bool b){
				elem.first.spellcard = b;
			});
			CH->connect("Changed", [&elem](bool b){
				elem.first.changeCard = b;
			});
			H->connect("ValueChanged", [&elem](float v){
				elem.first.horizontalAxis = v;
			});
			V->connect("ValueChanged", [&elem](float v){
				elem.first.verticalAxis = v;
			});
			duration->connect("TextChanged", [&elem](const std::string &text){
				if (text.empty())
					return;
				try {
					elem.second = std::stoul(text);
				} catch (...) {}
			});
			remove->connect("Clicked", [profile, macros, name, inputPanel, character, i, &elems]{
				elems.erase(elems.begin() + i);
				showMacroInputs(profile, macros, name, inputPanel, character);
			});

			pan->setPosition(0, i * 30);
			i++;
			inputPanel->add(pan);
			if (i >= 50) {
				MessageBoxA(
					SokuLib::window,
					("List has been trimmed because this macro has more than 50 input entries (" +
					std::to_string(elems.size()) + ") entries.\nIf you think this macro is corrupted, please remove it.").c_str(),
					"Corrupted macro ?",
					MB_ICONWARNING
				);
				break;
			}
		}
		auto lab = tgui::Label::create();

		lab->setSize({20, 20});
		lab->setPosition(0, i * 30);
		inputPanel->add(lab);
	}

	static std::string exploreFile(const std::string &title, const std::string &basePath, bool save)
	{
		OPENFILENAMEA ofn;
		char buffer[MAX_PATH];

		std::memset(buffer, 0, sizeof(buffer));
		std::memset(&ofn, 0, sizeof(ofn));
		ofn.lStructSize = sizeof(OPENFILENAMEA);
		ofn.hwndOwner = SokuLib::window;
		ofn.lpstrFile = buffer;
		ofn.nMaxFile = MAX_PATH;
		ofn.Flags = save ? (OFN_OVERWRITEPROMPT | OFN_NOREADONLYRETURN) : OFN_FILEMUSTEXIST;
		ofn.lpstrFilter = "Macro files\0*.macros\0\0";
		ofn.lpstrCustomFilter = nullptr;
		ofn.nFilterIndex = 0;
		ofn.lpstrFileTitle = nullptr;
		ofn.lpstrInitialDir = basePath.c_str();
		ofn.lpstrTitle = title.c_str();
		ofn.lpstrDefExt = "macros";
		save ? GetSaveFileNameA(&ofn) : GetOpenFileNameA(&ofn);
		return buffer;
	}

	void displayMacroPanel(const std::string &profile)
	{
		panel->loadWidgetsFromFile(profile + "/assets/macro.gui");

		auto macros = panel->get<tgui::ComboBox>("Macros");
		auto character = panel->get<tgui::ComboBox>("Character");
		auto name = panel->get<tgui::EditBox>("Name");
		auto inputPanel = panel->get<tgui::ScrollablePanel>("InputPanel");
		auto newMacro = panel->get<tgui::Button>("New");
		auto deleteMacro = panel->get<tgui::Button>("Delete");
		auto play = panel->get<tgui::Button>("Play");
		auto addInput = panel->get<tgui::Button>("AddInput");
		auto record1 = panel->get<tgui::Button>("Record1");
		auto record2 = panel->get<tgui::Button>("Record2");
		auto exportButton = panel->get<tgui::Button>("Export");
		auto import = panel->get<tgui::Button>("Import");
		auto loop = panel->get<tgui::CheckBox>("Loop");
		auto recorderCommon = [deleteMacro, macros, newMacro, character, addInput, exportButton, import, profile, name, inputPanel](std::weak_ptr<tgui::Button> me, std::weak_ptr<tgui::Button> other, bool recordAsDummy){
			const auto &allMacros = settings.nonSaved.macros.macros[character->getSelectedItemIndex()];
			auto isEmpty = allMacros.empty();
			SokuLib::KeyInput empty;

			if (settings.nonSaved.recordingMacro) {
				memset(&empty, 0, sizeof(empty));
				settings.nonSaved.recordingMacro = false;
				if (!settings.nonSaved.recordBuffer->empty() && compareKeyInputs(empty, settings.nonSaved.recordBuffer->front().first))
					settings.nonSaved.recordBuffer->erase(settings.nonSaved.recordBuffer->begin());
				character->setEnabled(true);
				deleteMacro->setEnabled(true);
				macros->setEnabled(true);
				me.lock()->setEnabled(!isEmpty);
				other.lock()->setEnabled(true);
				newMacro->setEnabled(true);
				addInput->setEnabled(true);
				import->setEnabled(true);
				exportButton->setEnabled(true);
				showMacroInputs(profile, macros, name, inputPanel, character);
			} else if (settings.nonSaved.playingMacroBuffer.macroElems.empty() && settings.nonSaved.playList.empty()) {
				settings.nonSaved.recordingMacro = true;
				settings.nonSaved.recordForDummy = recordAsDummy;
				settings.nonSaved.recordingStarted = false;
				character->setEnabled(false);
				other.lock()->setEnabled(false);
				deleteMacro->setEnabled(false);
				newMacro->setEnabled(false);
				macros->setEnabled(false);
				addInput->setEnabled(false);
				import->setEnabled(false);
				exportButton->setEnabled(false);
				me.lock()->setText("Stop recording");
				settings.nonSaved.recordBuffer = &settings.nonSaved.macros.macros[character->getSelectedItemIndex()][macros->getSelectedItemIndex()].macroElems;
			} else
				return false;
			return true;
		};
		auto createEmptyMacro = [name, inputPanel, macros, character, deleteMacro, play, record1, addInput, record2, exportButton]{
			auto &allMacros = settings.nonSaved.macros.macros[character->getSelectedItemIndex()];
			auto macroName = "Untitled macro " + std::to_string(allMacros.size() + 1);

			allMacros.push_back({macroName, {}});
			macros->addItem(macroName);
			macros->setSelectedItemByIndex(allMacros.size() - 1);
			name->setText(macroName);
			name->setEnabled(true);
			deleteMacro->setEnabled(true);
			play->setEnabled(true);
			record1->setEnabled(true);
			addInput->setEnabled(true);
			record2->setEnabled(true);
			exportButton->setEnabled(true);
			inputPanel->removeAllWidgets();
		};
		auto loadMacros = [macros, name, deleteMacro, inputPanel, play, record1, addInput, record2, exportButton](int character){
			const auto &allMacros = settings.nonSaved.macros.macros[character];
			auto isEmpty = allMacros.empty();

			macros->removeAllItems();
			for (auto &macro : allMacros)
				macros->addItem(macro.name);
			inputPanel->removeAllWidgets();
			macros->setSelectedItemByIndex(0);
			if (!isEmpty)
				name->setText(allMacros.front().name);
			name->setEnabled(!isEmpty);
			deleteMacro->setEnabled(!isEmpty);
			play->setEnabled(!isEmpty);
			record1->setEnabled(!isEmpty);
			addInput->setEnabled(!isEmpty);
			record2->setEnabled(!isEmpty);
			exportButton->setEnabled(!isEmpty);
		};

		character->setSelectedItemByIndex(SokuLib::rightChar);
		character->connect("ItemSelected", loadMacros);
		loadMacros(SokuLib::rightChar);
		showMacroInputs(profile, macros, name, inputPanel, character);
		if (!settings.nonSaved.playingMacroBuffer.macroElems.empty() && settings.nonSaved.playList.empty()){
			record1->setText("Pause macro");
			record2->setText("Stop macro");
		}

		loop->connect("Changed", [](bool b){
			settings.nonSaved.loopMacros = b;
		});
		macros->connect("ItemSelected", showMacroInputs, profile, std::weak_ptr<tgui::ComboBox>(macros), name, inputPanel, character);
		newMacro->connect("Clicked", createEmptyMacro);
		name->connect("TextChanged", [character, macros, profile, name, inputPanel](std::string newName){
			auto &allMacros = settings.nonSaved.macros.macros[character->getSelectedItemIndex()];
			auto selected = macros->getSelectedItemIndex();
			auto &changedMacro = allMacros[selected];

			changedMacro.name.swap(newName);
			macros->disconnectAll();
			macros->removeAllItems();
			for (auto &macro : allMacros)
				macros->addItem(macro.name);
			macros->setSelectedItemByIndex(selected);
			macros->connect("ItemSelected", showMacroInputs, profile, std::weak_ptr<tgui::ComboBox>(macros), name, inputPanel, character);
		});
		deleteMacro->connect("Clicked", [inputPanel, macros, character, name, play, record1, addInput, record2, exportButton](std::weak_ptr<tgui::Button> self){
			auto &allMacros = settings.nonSaved.macros.macros[character->getSelectedItemIndex()];
			auto index = macros->getSelectedItemIndex();
			auto isEmpty = allMacros.size() == 1;

			name->setEnabled(!isEmpty);
			self.lock()->setEnabled(!isEmpty);
			play->setEnabled(!isEmpty);
			record1->setEnabled(!isEmpty);
			addInput->setEnabled(!isEmpty);
			record2->setEnabled(!isEmpty);
			exportButton->setEnabled(!isEmpty);

			allMacros.erase(allMacros.begin() + index);
			macros->removeItemByIndex(index);
			if (index < allMacros.size())
				macros->setSelectedItemByIndex(index);
			else if (index)
				macros->setSelectedItemByIndex(index - 1);
			if (allMacros.empty())
				inputPanel->removeAllWidgets();
		}, std::weak_ptr<tgui::Button>(deleteMacro));
		play->connect("Clicked", [character, macros, record1, record2]{
			auto &macro = settings.nonSaved.macros.macros[character->getSelectedItemIndex()][macros->getSelectedItemIndex()];

			if (settings.nonSaved.playingMacroBuffer.macroElems.empty() && settings.nonSaved.playList.empty())
				settings.nonSaved.playingMacro = true;
			settings.nonSaved.playList.push_back(macro);
			record1->setText("Pause macro");
			record2->setText("Stop macro");
		});
		record1->connect("Clicked", [recorderCommon](std::weak_ptr<tgui::Button> me, std::weak_ptr<tgui::Button> other){
			if (!recorderCommon(std::move(me), std::move(other), true))
				settings.nonSaved.playingMacro = !settings.nonSaved.playingMacro;
		}, std::weak_ptr<tgui::Button>(record1), std::weak_ptr<tgui::Button>(record2));
		record2->connect("Clicked", [recorderCommon](std::weak_ptr<tgui::Button> me, std::weak_ptr<tgui::Button> other){
			if (!recorderCommon(std::move(me), std::move(other), false)) {
				settings.nonSaved.playingMacro = false;
				settings.nonSaved.playList.clear();
				settings.nonSaved.playingMacroBuffer.name.clear();
				settings.nonSaved.playingMacroBuffer.macroElems.clear();
			}
		}, std::weak_ptr<tgui::Button>(record2), std::weak_ptr<tgui::Button>(record1));
		addInput->connect("Clicked", [character, profile, macros, name, inputPanel]{
			auto &macro = settings.nonSaved.macros.macros[character->getSelectedItemIndex()][macros->getSelectedItemIndex()];

			if (macro.macroElems.empty())
				macro.macroElems.emplace_back();
			else
				macro.macroElems.push_back(macro.macroElems.back());
			macro.macroElems.back().second = 1;
			showMacroInputs(profile, macros, name, inputPanel, character);
		});
		exportButton->connect("Clicked", [character, macros, profile](std::weak_ptr<tgui::Button> button, tgui::Vector2f pos){
			auto fakePanel = tgui::Panel::create({"100%", "100%"});
			auto pan = tgui::Panel::create({150, 90});
			auto close = [pan](std::weak_ptr<tgui::Panel> fakePanel){
				gui.remove(pan);
				gui.remove(fakePanel.lock());
			};

			pan->loadWidgetsFromFile(profile + "/assets/context_export.gui");

			auto exportStr = pan->get<tgui::Button>("ThisStr");
			auto exportThisToFile = pan->get<tgui::Button>("ThisFile");
			auto exportChrToFile = pan->get<tgui::Button>("ChrFile");
			auto exportAllToFile = pan->get<tgui::Button>("AllFile");

			exportThisToFile->connect("Clicked", [character, macros, profile]{
				auto &macro = settings.nonSaved.macros.macros[character->getSelectedItemIndex()][macros->getSelectedItemIndex()];
				std::string path = exploreFile("Save macro file", profile, true);

				if (path.empty())
					return;
				exportMacroToFile(macro, path);
			});
			exportChrToFile->connect("Clicked", [character, profile]{
				auto &macros = settings.nonSaved.macros.macros[character->getSelectedItemIndex()];
				std::string path = exploreFile("Save macro file", profile, true);

				if (path.empty())
					return;
				exportMacrosToFile(macros, path);
			});
			exportAllToFile->connect("Clicked", [profile]{
				std::string path = exploreFile("Save macro file", profile, true);

				if (path.empty())
					return;
				exportMacrosToFile(settings.nonSaved.macros, path);
			});
			exportStr->connect("Clicked", [character, macros, close]{
				auto &macro = settings.nonSaved.macros.macros[character->getSelectedItemIndex()][macros->getSelectedItemIndex()];
				auto fakePanel = tgui::Panel::create({"100%", "100%"});
				auto text = tgui::TextBox::create();
				auto close = [text](std::weak_ptr<tgui::Panel> fakePanel){
					gui.remove(text);
					gui.remove(fakePanel.lock());
				};

				text->setReadOnly();
				text->setSize({"&.w / 2 - 100", "&.h / 2 - 100"});
				text->setPosition({"(&.w - w) / 2", "(&.h - h) / 2"});
				text->setText(exportMacroToString(macro));
				fakePanel->getRenderer()->setBackgroundColor({0x00, 0x00, 0x00, 0x80});
				fakePanel->connect("Clicked", close, std::weak_ptr<tgui::Panel>(fakePanel));
				gui.add(fakePanel);
				gui.add(text);
			});

			pan->getRenderer()->setBackgroundColor({0xAA, 0xAA, 0xAA});
			fakePanel->getRenderer()->setBackgroundColor({0, 0, 0, 0});
			fakePanel->connect("Clicked", close, std::weak_ptr<tgui::Panel>(fakePanel));
			for (auto &wid : pan->getWidgets())
				wid->connect("Pressed", close, std::weak_ptr<tgui::Panel>(fakePanel));
			pos.x += button.lock()->getPosition().x + panel->getPosition().x - 150;
			pos.y += button.lock()->getPosition().y + panel->getPosition().y;
			pan->setPosition(pos);
			gui.add(fakePanel);
			gui.add(pan);
		}, std::weak_ptr<tgui::Button>(exportButton));
		import->connect("Clicked", [profile, character, macros, name, deleteMacro, play, record1, addInput, record2, exportButton, inputPanel](std::weak_ptr<tgui::Button> button, tgui::Vector2f pos){
			auto fakePanel = tgui::Panel::create({"100%", "100%"});
			auto pan = tgui::Panel::create({110, 40});
			auto close = [pan](std::weak_ptr<tgui::Panel> fakePanel){
				gui.remove(pan);
				gui.remove(fakePanel.lock());
			};

			pan->loadWidgetsFromFile(profile + "/assets/context_import.gui");

			auto str = pan->get<tgui::Button>("Str");
			auto file = pan->get<tgui::Button>("File");

			file->connect("Clicked", [character, macros, profile]{
				std::string path = exploreFile("Load macro file", profile, false);

				if (path.empty())
					return;
				importMacrosFromFile(settings.nonSaved.macros.macros, character->getSelectedItemIndex(), path);
				displayMacroPanel(profile);

				panel->get<tgui::ComboBox>("Character")->setSelectedItemByIndex(character->getSelectedItemIndex());
				panel->get<tgui::ComboBox>("Macros")->setSelectedItemByIndex(macros->getItemCount() + 1);
			});
			str->connect("Clicked", [profile, character, macros, name, deleteMacro, play, record1, addInput, record2, exportButton, inputPanel]{
				auto fakePanel = tgui::Panel::create({"100%", "100%"});
				auto text = tgui::TextBox::create();
				auto ok = tgui::Button::create("OK");
				auto cancel = tgui::Button::create("Cancel");
				auto close = [text, ok, cancel](std::weak_ptr<tgui::Panel> fakePanel){
					gui.remove(ok);
					gui.remove(text);
					gui.remove(cancel);
					gui.remove(fakePanel.lock());
				};

				ok->connect("Clicked", [profile, text, character, macros, name, deleteMacro, play, record1, addInput, record2, exportButton, inputPanel]{
					auto &allMacros = settings.nonSaved.macros.macros[character->getSelectedItemIndex()];
					auto macro = importMacroFromString(text->getText());

					allMacros.push_back(macro);
					macros->addItem(macro.name);
					macros->setSelectedItemByIndex(allMacros.size() - 1);
					name->setText(macro.name);
					name->setEnabled(true);
					deleteMacro->setEnabled(true);
					play->setEnabled(true);
					record1->setEnabled(true);
					addInput->setEnabled(true);
					record2->setEnabled(true);
					exportButton->setEnabled(true);
					showMacroInputs(profile, macros, name, inputPanel, character);
				});
				ok->connect("Clicked", close, std::weak_ptr<tgui::Panel>(fakePanel));
				cancel->connect("Clicked", close, std::weak_ptr<tgui::Panel>(fakePanel));

				ok->setPosition("Text.x + Text.w - w", "Text.y + Text.h + 2");
				cancel->setPosition("Text.x + Text.w - OKButton.w - w - 10", "Text.y + Text.h + 2");
				text->setSize({"&.w / 2 - 100", "&.h / 2 - 100"});
				text->setDefaultText("Exported string");
				text->setPosition({"(&.w - w) / 2", "(&.h - h) / 2"});
				fakePanel->getRenderer()->setBackgroundColor({0x00, 0x00, 0x00, 0x80});
				fakePanel->connect("Clicked", close, std::weak_ptr<tgui::Panel>(fakePanel));
				gui.add(fakePanel);
				gui.add(text, "Text");
				gui.add(ok, "OKButton");
				gui.add(cancel, "CancelButton");
			});

			pan->getRenderer()->setBackgroundColor({0xAA, 0xAA, 0xAA});
			fakePanel->getRenderer()->setBackgroundColor({0, 0, 0, 0});
			fakePanel->connect("Clicked", close, std::weak_ptr<tgui::Panel>(fakePanel));
			for (auto &wid : pan->getWidgets())
				wid->connect("Pressed", close, std::weak_ptr<tgui::Panel>(fakePanel));
			pos.x += button.lock()->getPosition().x + panel->getPosition().x - 110;
			pos.y += button.lock()->getPosition().y + panel->getPosition().y;
			pan->setPosition(pos);
			gui.add(fakePanel);
			gui.add(pan);
		}, std::weak_ptr<tgui::Button>(import));

		if (settings.nonSaved.recordingMacro) {
			for (int i = 0; i < settings.nonSaved.macros.macros.size(); i++)
				for (int j = 0; j < settings.nonSaved.macros.macros[i].size(); j++)
					if (settings.nonSaved.recordBuffer == &settings.nonSaved.macros.macros[i][j].macroElems) {
						character->setSelectedItemByIndex(i);
						macros->setSelectedItemByIndex(j);
					}

			(settings.nonSaved.recordForDummy ? record2 : record1)->setEnabled(false);
			(settings.nonSaved.recordForDummy ? record1 : record2)->setEnabled(true);
			(settings.nonSaved.recordForDummy ? record1 : record2)->setText("Stop recording");
			deleteMacro->setEnabled(false);
			newMacro->setEnabled(false);
			macros->setEnabled(false);
			character->setEnabled(false);
			addInput->setEnabled(false);
			import->setEnabled(false);
			exportButton->setEnabled(false);
		}
	}
	int yolo = 0;

	static void updateCharacterPanel(tgui::Panel::Ptr pan, SokuLib::CharacterManager &manager, SokuLib::Character character, CharacterState &state)
	{
		state.HPInstantRegen = panel->get<tgui::CheckBox>("HPInstantRegen")->isChecked();
		state.SPInstantRegen = panel->get<tgui::CheckBox>("SPInstantRegen")->isChecked();
	}

	static void updateMiscPanel()
	{
	}

	static void updateMacroPanel()
	{
		auto queue = panel->get<tgui::TextBox>("Queue");
		auto record1 = panel->get<tgui::Button>("Record1");
		auto record2 = panel->get<tgui::Button>("Record2");

		if (settings.nonSaved.playingMacroBuffer.macroElems.empty() && settings.nonSaved.playList.empty() && !settings.nonSaved.recordingMacro) {
			record1->setText("Record macro from dummy");
			record2->setText("Record macro from player");
		}
		if (settings.nonSaved.playingMacroBuffer.macroElems.empty() && settings.nonSaved.playList.empty()) {
			queue->setText("");
		} else {
			size_t size = 3 * settings.nonSaved.playList.size();
			std::string text;

			for (auto &elem : settings.nonSaved.playList)
				size += elem.name.size();
			text.reserve(size + settings.nonSaved.playingMacroBuffer.name.size());
			text = settings.nonSaved.playingMacroBuffer.name;
			for (auto &elem : settings.nonSaved.playList) {
				text += " -> ";
				text += elem.name;
			}
			queue->setText(text);
		}
	}

	static int tabChangeRequest = -1;
	static const std::map<std::string, std::function<void (const std::string &)>> creaters{
		{ "Characters", displaySkillsPanel },
		{ "Macros",  displayMacroPanel },
		{ "Dummy",  displayDummyPanel },
		{ "State",  displayStatePanel },
		{ "Misc",   displayMiscPanel }
	};
	static const std::map<std::string, std::function<void ()>> updaters{
		{ "Characters", []{
			updateCharacterPanel(panel->get<tgui::Panel>("Left"),  SokuLib::getBattleMgr().leftCharacterManager,  SokuLib::leftChar,  settings.nonSaved.leftState);
			updateCharacterPanel(panel->get<tgui::Panel>("Right"), SokuLib::getBattleMgr().rightCharacterManager, SokuLib::rightChar, settings.nonSaved.rightState);
		} },
		{ "Macros",  updateMacroPanel },
		{ "Dummy",  updateDummyPanel },
		{ "State",  updateStatePanel },
		{ "Misc",   updateMiscPanel }
	};
	static const std::map<SokuLib::Scene, std::string> files{
		{ SokuLib::SCENE_BATTLE, "/assets/main.gui" },
		{ SokuLib::SCENE_BATTLECL, "/assets/networkMain.gui" },
		{ SokuLib::SCENE_BATTLESV, "/assets/networkMain.gui" },
	};

	void connectOnChangeCallback(const tgui::Widget::Ptr &widget, const std::string &base = "")
	{
		std::string name = (base.empty() ? "" : base + ".") + widget->getWidgetName();
		auto asButton = widget->cast<tgui::Button>();
		auto asEdit = widget->cast<tgui::EditBox>();
		auto asCombo = widget->cast<tgui::ComboBox>();
		auto asCheck = widget->cast<tgui::CheckBox>();
		auto asSlider = widget->cast<tgui::Slider>();
		auto asPanel = widget->cast<tgui::Panel>();

		if (name.empty()) {
			printf("Warning: Element without name found of type %s\n", widget->getWidgetType().c_str());
			return;
		}

		if (asButton)
			asButton->connect("Clicked", [name]{
				onElemChanged(name, 0);
			});
		else if (asEdit)
			asEdit->connect("TextChanged", [name](const std::string &text){
				onElemChanged(name, text);
			});
		else if (asCombo)
			asCombo->connect("ItemSelected", [name](std::weak_ptr<tgui::ComboBox> me, int index){
				onElemChanged(name, me.lock()->getSelectedItemIndex());
			}, std::weak_ptr<tgui::ComboBox>(asCombo));
		else if (asCheck)
			asCheck->connect("Changed", [name](bool b){
				onElemChanged(name, b);
			});
		else if (asSlider)
			asSlider->connect("ValueChanged", [name](float v){
				onElemChanged(name, v);
			});
		else if (asPanel)
			for (auto &wid : asPanel->getWidgets())
				connectOnChangeCallback(wid, name);
	}

	void loadAllGuiElements(LPCSTR profilePath)
	{
		puts("Loading GUI...");

		std::string profile = profilePath;

		gui.loadWidgetsFromFile(profile + files.at(SokuLib::sceneId));
		panel = gui.get<tgui::Panel>("Panel");
		tab = gui.get<tgui::Tabs>("Tabs");
		tab->connect("TabSelected", [profile]{
			auto it = creaters.find(tab->getSelected());

			if (it != creaters.end())
				it->second(profile);
			else
				panel->removeAllWidgets();
			if (settings.nonSaved.enabled)
				for (auto &widget : panel->getWidgets()) {
					if (SokuLib::mainMode == SokuLib::BATTLE_MODE_VSSERVER)
						widget->setEnabled(false);
					else
						connectOnChangeCallback(widget);
				}
		});
		puts("Display skills");
		displaySkillsPanel(profile);
		if (settings.nonSaved.enabled) {
			for (auto &widget : panel->getWidgets()) {
				if (SokuLib::mainMode == SokuLib::BATTLE_MODE_VSSERVER)
					widget->setEnabled(false);
				else
					connectOnChangeCallback(widget);
			}
			if (SokuLib::mainMode == SokuLib::BATTLE_MODE_VSCLIENT)
				tab->connect("TabSelected", []{
					onElemChanged("", tab->getSelectedIndex());
				});
		}
		tab->setEnabled(SokuLib::mainMode != SokuLib::BATTLE_MODE_VSSERVER);
		printf("%p\n", panel->get<tgui::Button>("Left.Button1").get());
		puts("GUI loaded");
	}

	void updateGuiState()
	{
		auto it = updaters.find(tab->getSelected());

		if (it != updaters.end())
			it->second();
		if (tabChangeRequest != -1) {
			bool enabled = tab->isEnabled();

			tab->setEnabled(true);
			tab->select(tabChangeRequest);
			tab->setEnabled(enabled);
			tabChangeRequest = -1;
		}
	}

	tgui::Widget::Ptr fetchWidget(const std::string &name, const tgui::Panel::Ptr &basePanel)
	{
		auto pos = name.find('.');

		if (pos != std::string::npos)
			return fetchWidget(name.substr(pos + 1), basePanel->get<tgui::Panel>(name.substr(0, pos)));
		return basePanel->get<tgui::Widget>(name);
	}

	void setElem(const std::string &elem, const SokuLib::ElemProperty &property)
	{
		if (elem.empty()) {
			tabChangeRequest = property.selected;
			return;
		}

		auto widget = fetchWidget(elem, panel);

		if (!widget) {
			MessageBoxA(SokuLib::window, ("Cannot find widget " + elem + ".").c_str(), "Invalid widget", MB_ICONERROR);
			exit(1);
		}

		auto asButton = widget->cast<tgui::Button>();
		auto asEdit = widget->cast<tgui::EditBox>();
		auto asCombo = widget->cast<tgui::ComboBox>();
		auto asCheck = widget->cast<tgui::CheckBox>();
		auto asSlider = widget->cast<tgui::Slider>();

		if (asButton)
			asButton->onClick.emit(asButton.get(), {0, 0});
		else if (asEdit) {
			char buffer[5];

			strncpy(buffer, property.text, 4);
			buffer[4] = 0;
			asEdit->setText(buffer);
		} else if (asCombo)
			asCombo->setSelectedItemByIndex(property.selected);
		else if (asCheck)
			asCheck->setChecked(property.checked);
		else if (asSlider)
			asSlider->setValue(property.value);
	}
}