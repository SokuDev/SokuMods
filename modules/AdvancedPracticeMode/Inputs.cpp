//
// Created by PinkySmile on 03/03/2021.
//

#include <SokuLib.hpp>
#include <list>
#include "DrawUtils.hpp"
#include "Inputs.hpp"
#include "Gui.hpp"
#include "State.hpp"

namespace Practice
{
#define BOX_WIDTH 140
#define BOX_HEIGHT 340
#define MIN_ALPHA 0.5
#define MAX_ALPHA 0.9
#define SPRITE_SIZE 15

	struct Input {
		SokuLib::KeyInput input;
		unsigned duration;
		SokuLib::Action action;
	};

	struct MoveState {
		SokuLib::Action action;
		unsigned short frames;
		unsigned animation;
		unsigned short subFrames;
	};

	bool operator==(const SokuLib::KeyInput &i1, const SokuLib::KeyInput &i2)
	{
		return !!i1.a == !!i2.a &&
			!!i1.b == !!i2.b &&
			!!i1.c == !!i2.c &&
			!!i1.d == !!i2.d &&
			!!i1.changeCard == !!i2.changeCard &&
			!!i1.spellcard == !!i2.spellcard &&
			(i1.horizontalAxis == 0) == (i2.horizontalAxis == 0) &&
			(i1.horizontalAxis < 0) == (i2.horizontalAxis < 0) &&
			(i1.horizontalAxis > 0) == (i2.horizontalAxis > 0) &&
			(i1.verticalAxis == 0) == (i2.verticalAxis == 0) &&
			(i1.verticalAxis < 0) == (i2.verticalAxis < 0) &&
			(i1.verticalAxis > 0) == (i2.verticalAxis > 0);
	}

	bool operator!=(const SokuLib::KeyInput &i1, const SokuLib::KeyInput &i2)
	{
		return !(i1 == i2);
	}

	static const Vector2<int> dirSheetOffset[9]{
		{32 * 7, 0},
		{32 * 1, 0},
		{32 * 6, 0},
		{32 * 2, 0},
		{   -32, 0},
		{32 * 3, 0},
		{32 * 4, 0},
		{32 * 0, 0},
		{32 * 5, 0},
	};
	static GradiantRect leftBox;
	static GradiantRect rightBox;
	static std::list<Input> leftInputList;
	static std::list<Input> rightInputList;
	static Sprite leftSkillSheet;
	static Sprite rightSkillSheet;
	static Sprite inputSheet;

	void initInputDisplay(LPCSTR profilePath)
	{
		std::string profile = profilePath;

		leftInputList.clear();
		rightInputList.clear();
		leftBox.setPosition({0, 60});
		leftBox.setSize({BOX_WIDTH, BOX_HEIGHT});
		leftBox.fillColors[GradiantRect::RECT_TOP_LEFT_CORNER]    = leftBox.fillColors[GradiantRect::RECT_TOP_RIGHT_CORNER]    = DxSokuColor::Black * MIN_ALPHA;
		leftBox.fillColors[GradiantRect::RECT_BOTTOM_LEFT_CORNER] = leftBox.fillColors[GradiantRect::RECT_BOTTOM_RIGHT_CORNER] = DxSokuColor::Black * MAX_ALPHA;
		leftBox.borderColors[0] = leftBox.borderColors[1] = leftBox.borderColors[2] = leftBox.borderColors[3] = DxSokuColor::Transparent;
		rightBox = leftBox;
		rightBox.setPosition({640 - BOX_WIDTH, 60});
		if (!inputSheet.texture.hasTexture())
			Texture::loadFromFile(inputSheet.texture, (profile + "/assets/inputs.png").c_str());
		Texture::loadFromFile(leftSkillSheet.texture, (profile +  + "/assets/skills/" + names[SokuLib::leftChar] + "Skills.png").c_str());
		Texture::loadFromFile(rightSkillSheet.texture, (profile +  + "/assets/skills/" + names[SokuLib::rightChar] + "Skills.png").c_str());
	}

	void updateList(std::list<Input> &list, SokuLib::CharacterManager &character)
	{
		if (list.empty() || list.front().input != character.keyMap) {
			list.push_front({character.keyMap, 1, SokuLib::ACTION_IDLE});
			while (list.size() > (BOX_HEIGHT / (SPRITE_SIZE + (SPRITE_SIZE / 3))))
				list.pop_back();
			return;
		}

		//auto realAction = character.objectBase.action;

		//if (realAction != list.front().action) {
		//	if (list.front().duration == 1) {
		//		if (list.size() >= 2) {
		//			auto input = list.front();

		//			list.pop_front();
		//			if (list.front().action != realAction)
		//				input.action = realAction;
		//			list.push_front(input);
		//		}
		//	}
		//}
		list.front().duration++;
	}

	void updateInputLists()
	{
		updateList(leftInputList, SokuLib::getBattleMgr().leftCharacterManager);
		updateList(rightInputList, SokuLib::getBattleMgr().rightCharacterManager);
	}

	void showInput(int value, Vector2<int> &pos, Vector2<int> sheetPos, bool goLeft)
	{
		if (!value)
			return;
		inputSheet.setPosition(pos);
		inputSheet.setSize({SPRITE_SIZE, SPRITE_SIZE});
		inputSheet.rect.top = sheetPos.y;
		inputSheet.rect.left = sheetPos.x;
		inputSheet.rect.width = inputSheet.rect.height = 24;
		inputSheet.draw();
		if (goLeft)
			pos.x -= SPRITE_SIZE;
		else
			pos.x += SPRITE_SIZE;
	}

	void drawInputList(const std::list<Input> &list, Vector2<int> offset, bool reversed)
	{
		int baseX = offset.x;
		float alpha = MAX_ALPHA;

		if (reversed)
			baseX += BOX_WIDTH - SPRITE_SIZE;
		offset.y += BOX_HEIGHT + (SPRITE_SIZE / 6);
		for (auto &input : list) {
			offset.y -= SPRITE_SIZE + (SPRITE_SIZE / 3);
			offset.x = baseX;

			int dir = 5 + (input.input.horizontalAxis > 0) - (input.input.horizontalAxis < 0) + ((input.input.verticalAxis < 0) - (input.input.verticalAxis > 0)) * 3;

			inputSheet.tint = DxSokuColor::White * alpha;
			showInput(dir != 5, offset, dirSheetOffset[dir - 1], reversed);
			showInput(input.input.a, offset, {0, 32}, reversed);
			showInput(input.input.b, offset, {32, 32}, reversed);
			showInput(input.input.c, offset, {64, 32}, reversed);
			showInput(input.input.d, offset, {96, 32}, reversed);
			showInput(input.input.changeCard, offset, {128, 32}, reversed);
			showInput(input.input.spellcard, offset, {160, 32}, reversed);
			alpha -= (MAX_ALPHA - MIN_ALPHA) / 10;
		}
	}

	void displayInputs()
	{
		if (settings.showLeftInputBox) {
			leftBox.draw();
			drawInputList(leftInputList, {0, 60}, false);
		}
		if (settings.showRightInputBox) {
			rightBox.draw();
			drawInputList(rightInputList, {640 - BOX_WIDTH, 60}, true);
		}
	}
}