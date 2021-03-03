//
// Created by PinkySmile on 03/03/2021.
//

#include <SokuLib.hpp>
#include <list>
#include "DrawUtils.hpp"
#include "Inputs.hpp"
#include "Gui.hpp"

namespace Practice
{
	struct Input {
		SokuLib::KeyInput input;
		unsigned duration;
		SokuLib::Action action;
	};

	static GradiantRect leftBox;
	static GradiantRect rightBox;
	static std::list<Input> leftInputList;
	static std::list<Input> rightInputList;
	static Texture leftSkillSheet;
	static Texture rightSkillSheet;
	static Texture inputSheet;

	void initInputDisplay(LPCSTR profilePath)
	{
		std::string profile = profilePath;

		leftBox.setPosition({0, 60});
		leftBox.setSize({140, 320});
		leftBox.fillColors[GradiantRect::RECT_TOP_LEFT_CORNER]    = leftBox.fillColors[GradiantRect::RECT_TOP_RIGHT_CORNER]    = DxSokuColor::Black * 0.5;
		leftBox.fillColors[GradiantRect::RECT_BOTTOM_LEFT_CORNER] = leftBox.fillColors[GradiantRect::RECT_BOTTOM_RIGHT_CORNER] = DxSokuColor::Black * 0.9;
		leftBox.borderColors[0] = leftBox.borderColors[1] = leftBox.borderColors[2] = leftBox.borderColors[3] = DxSokuColor::Transparent;
		rightBox = leftBox;
		rightBox.setPosition({500, 60});
		if (!inputSheet.hasTexture())
			Texture::loadFromFile(inputSheet, (profile + "/assets/inputs.png").c_str());
		Texture::loadFromFile(leftSkillSheet, (profile +  + "/assets/skills/" + names[SokuLib::leftChar] + "Skills.png").c_str());
		Texture::loadFromFile(rightSkillSheet, (profile +  + "/assets/skills/" + names[SokuLib::rightChar] + "Skills.png").c_str());
	}

	void updateList(std::list<Input> &list, SokuLib::CharacterManager &character)
	{

	}

	void displayInputs()
	{
		leftBox.draw();
		rightBox.draw();
	}
}