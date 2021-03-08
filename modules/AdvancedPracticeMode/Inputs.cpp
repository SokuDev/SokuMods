//
// Created by PinkySmile on 03/03/2021.
//

#include <SokuLib.hpp>
#include <list>
#include <utility>
#include "DrawUtils.hpp"
#include "Inputs.hpp"
#include "Gui.hpp"
#include "State.hpp"

namespace Practice
{
#define BOX_WIDTH 170
#define BOX_WIDTH2 116
#define BOX_HEIGHT 340
#define MIN_ALPHA 0.5
#define MAX_ALPHA 1.0
#define MIN_BOX_ALPHA 0.1
#define MAX_BOX_ALPHA 0.4
#define SPRITE_SIZE 24
#define MAX_LIST_SIZE 0x100


#define FAKE_ACTION_ORRERIES_REACTIVATE static_cast<SokuLib::Action>(SokuLib::ACTION_AIR_ORRERIES_C + 1)
#define FAKE_ACTION_5AAA6A static_cast<SokuLib::Action>(1000)
#define FAKE_ACTION_FLY1 static_cast<SokuLib::Action>(1001)
#define FAKE_ACTION_FLY2 static_cast<SokuLib::Action>(1002)
#define FAKE_ACTION_FLY3 static_cast<SokuLib::Action>(1003)
#define FAKE_ACTION_FLY4 static_cast<SokuLib::Action>(1004)
#define FAKE_ACTION_FLY6 static_cast<SokuLib::Action>(1005)
#define FAKE_ACTION_FLY7 static_cast<SokuLib::Action>(1006)
#define FAKE_ACTION_FLY8 static_cast<SokuLib::Action>(1007)
#define FAKE_ACTION_FLY9 static_cast<SokuLib::Action>(1008)

//Suwako stuff
#define FAKE_ACTION_j1D static_cast<SokuLib::Action>(1100)
#define FAKE_ACTION_j3D static_cast<SokuLib::Action>(1101)
#define FAKE_ACTION_LILIPAD_NEUTRAL_HIGHJUMP static_cast<SokuLib::Action>(1102)
#define FAKE_ACTION_LILIPAD_FORWARD_HIGHJUMP static_cast<SokuLib::Action>(1103)
#define FAKE_ACTION_LILIPAD_BACKWARD_HIGHJUMP static_cast<SokuLib::Action>(1104)
#define FAKE_ACTION_LILIPAD_A static_cast<SokuLib::Action>(1105)
#define FAKE_ACTION_LILIPAD_3A static_cast<SokuLib::Action>(1106)
#define FAKE_ACTION_LILIPAD_2B static_cast<SokuLib::Action>(1107)
#define FAKE_ACTION_LILIPAD_6B static_cast<SokuLib::Action>(1108)
#define FAKE_ACTION_LILIPAD_2C static_cast<SokuLib::Action>(1109)
#define FAKE_ACTION_LILIPAD_SPAWN static_cast<SokuLib::Action>(1110)
#define FAKE_ACTION_LILIPAD_DESPAWN static_cast<SokuLib::Action>(1111)
#define FAKE_ACTION_LILIPAD_DEFAULT_22B static_cast<SokuLib::Action>(1112)
#define FAKE_ACTION_LILIPAD_DEFAULT_22C static_cast<SokuLib::Action>(1113)
#define FAKE_ACTION_UNDERGROUND_ALT1_22B static_cast<SokuLib::Action>(1114)
#define FAKE_ACTION_UNDERGROUND_ALT1_22C static_cast<SokuLib::Action>(1115)
#define FAKE_ACTION_LILIPAD_DEFAULT_623b static_cast<SokuLib::Action>(1116)
#define FAKE_ACTION_UNDERGROUND_DEFAULT_623b static_cast<SokuLib::Action>(1117)
#define FAKE_ACTION_UNDERGROUND_DEFAULT_623c static_cast<SokuLib::Action>(1118)
#define FAKE_ACTION_LILIPAD_ALT1_623b static_cast<SokuLib::Action>(1119)
#define FAKE_ACTION_LILIPAD_ALT1_623c static_cast<SokuLib::Action>(1120)
#define FAKE_ACTION_UNDERGROUND_DEFAULT_236b static_cast<SokuLib::Action>(1121)
#define FAKE_ACTION_UNDERGROUND_DEFAULT_236c static_cast<SokuLib::Action>(1122)
#define FAKE_ACTION_UNDERGROUND_ALT1_236b static_cast<SokuLib::Action>(1123)
#define FAKE_ACTION_UNDERGROUND_ALT1_236c static_cast<SokuLib::Action>(1124)
#define FAKE_ACTION_UNDERGROUND_ALT2_236b static_cast<SokuLib::Action>(1125)
#define FAKE_ACTION_UNDERGROUND_ALT2_236c static_cast<SokuLib::Action>(1126)
#define FAKE_ACTION_UNDERGROUND_DEFAULT_214b static_cast<SokuLib::Action>(1127)
#define FAKE_ACTION_UNDERGROUND_DEFAULT_214c static_cast<SokuLib::Action>(1128)
#define FAKE_ACTION_LILIPAD_ALT1_214b static_cast<SokuLib::Action>(1129)
#define FAKE_ACTION_LILIPAD_ALT1_214c static_cast<SokuLib::Action>(1130)
#define FAKE_ACTION_LILIPAD_ALT2_214b static_cast<SokuLib::Action>(1131)
#define FAKE_ACTION_LILIPAD_ALT2_214c static_cast<SokuLib::Action>(1132)
#define FAKE_ACTION_lSC201 static_cast<SokuLib::Action>(1133)
#define FAKE_ACTION_jSC201 static_cast<SokuLib::Action>(1134)
#define FAKE_ACTION_uSC202 static_cast<SokuLib::Action>(1135)
#define FAKE_ACTION_jSC203 static_cast<SokuLib::Action>(1136)
#define FAKE_ACTION_lSC203 static_cast<SokuLib::Action>(1137)
#define FAKE_ACTION_jSC204 static_cast<SokuLib::Action>(1138)
#define FAKE_ACTION_lSC205 static_cast<SokuLib::Action>(1139)
#define FAKE_ACTION_uSC205 static_cast<SokuLib::Action>(1140)
#define FAKE_ACTION_lSC206 static_cast<SokuLib::Action>(1141)
#define FAKE_ACTION_jSC207 static_cast<SokuLib::Action>(1142)
#define FAKE_ACTION_lSC209 static_cast<SokuLib::Action>(1143)
#define FAKE_ACTION_jSC209 static_cast<SokuLib::Action>(1144)
#define FAKE_ACTION_uSC212 static_cast<SokuLib::Action>(1145)

#define A_SPRITE_POS           Vector2<int>{0,   28}
#define B_SPRITE_POS           Vector2<int>{32,  28}
#define C_SPRITE_POS           Vector2<int>{64,  28}
#define D_SPRITE_POS           Vector2<int>{96,  28}
#define CH_SPRITE_POS          Vector2<int>{128, 28}
#define SC_SPRITE_POS          Vector2<int>{160, 28}
#define UP_SPRITE_POS          Vector2<int>{0,   -4}
#define DOWN_SPRITE_POS        Vector2<int>{32,  -4}
#define LEFT_SPRITE_POS        Vector2<int>{64,  -4}
#define RIGHT_SPRITE_POS       Vector2<int>{96,  -4}
#define LEFTUP_SPRITE_POS      Vector2<int>{128, -4}
#define RIGHTUP_SPRITE_POS     Vector2<int>{160, -4}
#define RIGHTDOWN_SPRITE_POS   Vector2<int>{192, -4}
#define LEFTDOWN_SPRITE_POS    Vector2<int>{224, -4}
#define BE_SPRITE_POS          Vector2<int>{192, 28}
#define HJ_SPRITE_POS          Vector2<int>{224, 32}
#define AIR_SPRITE_POS         Vector2<int>{256, 0}
#define DASH_SPRITE_POS        Vector2<int>{256, 32}
#define LILIPAD_SPRITE_POS     Vector2<int>{288, 0}
#define FAR_SPRITE_POS         Vector2<int>{288, 32}
#define UNDERGROUND_SPRITE_POS Vector2<int>{320, 0}
#define REIMU_SPRITE_POS       Vector2<int>{320, 32}

	struct MoveState {
		SokuLib::Action action;
		unsigned frames;
		unsigned short animation;
		unsigned short subFrames;
	};

	struct Input {
		SokuLib::KeyInput input;
		unsigned duration;
		SokuLib::Action action;
	};

	struct MoveSpriteDescriptor {
	private:
		bool _isSkill = false;
		std::vector<std::string> _sequences;
		Sprite &_sprite;
		Sprite &_sprite2;
		Vector2<int> _startRect;
		std::vector<Vector2<int>> _rects;
		Vector2<unsigned> _rectSize;
		Vector2<unsigned> _rectSize2;

		static unsigned _getSpriteIndex(SokuLib::Action action, SokuLib::Character character)
		{
			switch (action) {
			case FAKE_ACTION_LILIPAD_DEFAULT_22B:
			case FAKE_ACTION_LILIPAD_DEFAULT_22C:
				return 3;
			case FAKE_ACTION_UNDERGROUND_ALT1_22B:
			case FAKE_ACTION_UNDERGROUND_ALT1_22C:
				return 7;
			case FAKE_ACTION_LILIPAD_DEFAULT_623b:
			case FAKE_ACTION_UNDERGROUND_DEFAULT_623b:
			case FAKE_ACTION_UNDERGROUND_DEFAULT_623c:
				return 1;
			case FAKE_ACTION_LILIPAD_ALT1_623b:
			case FAKE_ACTION_LILIPAD_ALT1_623c:
				return 5;
			case FAKE_ACTION_UNDERGROUND_DEFAULT_236b:
			case FAKE_ACTION_UNDERGROUND_DEFAULT_236c:
				return 2;
			case FAKE_ACTION_UNDERGROUND_ALT1_236b:
			case FAKE_ACTION_UNDERGROUND_ALT1_236c:
				return 6;
			case FAKE_ACTION_UNDERGROUND_ALT2_236b:
			case FAKE_ACTION_UNDERGROUND_ALT2_236c:
				return 10;
			case FAKE_ACTION_UNDERGROUND_DEFAULT_214b:
			case FAKE_ACTION_UNDERGROUND_DEFAULT_214c:
				return 0;
			case FAKE_ACTION_LILIPAD_ALT1_214b:
			case FAKE_ACTION_LILIPAD_ALT1_214c:
				return 4;
			case FAKE_ACTION_LILIPAD_ALT2_214b:
			case FAKE_ACTION_LILIPAD_ALT2_214c:
				return 8;
			default:
				int move = ((action - 500) / 20) * 3 + ((action - 500) % 20 / 5);

				return (move % 3) * (4 + (character == SokuLib::CHARACTER_PATCHOULI)) + move / 3;
			}
		}

		static unsigned _getSpellIndex(SokuLib::Action action)
		{
			switch (action) {
			case FAKE_ACTION_lSC201:
			case FAKE_ACTION_jSC201:
				return 1;
			case FAKE_ACTION_uSC202:
				return 2;
			case FAKE_ACTION_jSC203:
			case FAKE_ACTION_lSC203:
				return 3;
			case FAKE_ACTION_jSC204:
				return 4;
			case FAKE_ACTION_lSC205:
			case FAKE_ACTION_uSC205:
				return 5;
			case FAKE_ACTION_lSC206:
				return 6;
			case FAKE_ACTION_jSC207:
				return 7;
			case FAKE_ACTION_lSC209:
			case FAKE_ACTION_jSC209:
				return 9;
			case FAKE_ACTION_uSC212:
				return 12;
			default:
				if (action >= SokuLib::ACTION_SC_ID_200_ALT_EFFECT)
					return (action - SokuLib::ACTION_SC_ID_200_ALT_EFFECT);
				return (action - SokuLib::ACTION_USING_SC_ID_200);
			}
		}

		void _displayOrreriesSpecialStuff(Sprite &spell, Vector2<int> pos, Vector2<unsigned> size, bool reverse, SokuLib::Action action, float alpha) const
		{
			spell.setSize({41 * size.y / 65, size.y});
			spell.tint = DxSokuColor::White * alpha;
			spell.rect.top = 0;
			spell.rect.left = 15 * 41;
			spell.rect.width = 41;
			spell.rect.height = 65;
			if (reverse) {
				for (unsigned i = this->_rects.size(); i > 0; i--) {
					this->_sprite.setPosition(pos);
					this->_sprite.rect.top = this->_rects[i - 1].y;
					this->_sprite.rect.left = this->_rects[i - 1].x;
					this->_sprite.rect.width = this->_rectSize.x;
					this->_sprite.rect.height = this->_rectSize.y;
					this->_sprite.draw();
					pos.x -= size.x;
				}
				for (int i = action == FAKE_ACTION_ORRERIES_REACTIVATE; i >= 0; i--) {
					spell.setPosition(pos);
					spell.draw();
					pos.x -= spell.getSize().x;
				}
			} else {
				for (int i = action == FAKE_ACTION_ORRERIES_REACTIVATE; i >= 0; i--) {
					spell.setPosition(pos);
					spell.draw();
					pos.x += spell.getSize().x;
				}
				for (const auto &rect : this->_rects) {
					this->_sprite.setPosition(pos);
					this->_sprite.rect.top = rect.y;
					this->_sprite.rect.left = rect.x;
					this->_sprite.rect.width = this->_rectSize.x;
					this->_sprite.rect.height = this->_rectSize.y;
					this->_sprite.draw();
					pos.x += size.x;
				}
			}
		}

		void _drawSpell(Sprite &spell, Vector2<int> pos, Vector2<unsigned> size, bool reverse, SokuLib::Action action, float alpha) const
		{
			unsigned index = MoveSpriteDescriptor::_getSpellIndex(action);

			spell.setSize({41 * size.y / 65, size.y});
			spell.tint = DxSokuColor::White * alpha;
			if (reverse) {
				this->_normalDraw(pos, size, reverse, alpha);
				pos.x -= size.x;
				spell.setPosition(pos);
				spell.rect.top = 0;
				spell.rect.left = index * 41;
				spell.rect.width = 41;
				spell.rect.height = 65;
				spell.draw();
			} else {
				spell.setPosition(pos);
				spell.rect.top = 0;
				spell.rect.left = index * 41;
				spell.rect.width = 41;
				spell.rect.height = 65;
				spell.draw();
				pos.x += size.x;
				this->_normalDraw(pos, size, reverse, alpha);
			}
		}

		void _drawSkill(Sprite &skills, Vector2<int> pos, Vector2<unsigned> size, bool reverse, SokuLib::Character character, SokuLib::Action action) const
		{
			int rectPos = MoveSpriteDescriptor::_getSpriteIndex(action, character);
			std::vector<Vector2<int>> rects;

			for (auto c : this->_sequences[character]) {
				switch (c) {
				case '1':
					rects.push_back(LEFTDOWN_SPRITE_POS);
					break;
				case '2':
					rects.push_back(DOWN_SPRITE_POS);
					break;
				case '3':
					rects.push_back(RIGHTDOWN_SPRITE_POS);
					break;
				case '4':
					rects.push_back(LEFT_SPRITE_POS);
					break;
				case '6':
					rects.push_back(RIGHT_SPRITE_POS);
					break;
				case 'b':
					rects.push_back(B_SPRITE_POS);
					break;
				case 'c':
					rects.push_back(C_SPRITE_POS);
					break;
				case 'j':
					rects.push_back(AIR_SPRITE_POS);
					break;
				case 'l':
					rects.push_back(LILIPAD_SPRITE_POS);
					break;
				case 'u':
					rects.push_back(UNDERGROUND_SPRITE_POS);
					break;
				default:
					abort();
				}
			}
			skills.setSize({size.y, size.y});
			skills.rect.top = 0;
			skills.rect.left = rectPos * 32;
			skills.rect.width = 32;
			skills.rect.height = 32;
			if (reverse) {
				pos.x += size.x;
				for (unsigned i = rects.size(); i > 0; i--) {
					pos.x -= size.x;
					this->_sprite.setPosition(pos);
					this->_sprite.rect.top = rects[i - 1].y;
					this->_sprite.rect.left = rects[i - 1].x;
					this->_sprite.rect.width = this->_rectSize.x;
					this->_sprite.rect.height = this->_rectSize.y;
					this->_sprite.draw();
				}
				pos.x -= size.y;
				skills.setPosition(pos);
				skills.draw();
			} else {
				skills.setPosition(pos);
				skills.draw();
				pos.x += size.y;
				for (const auto &rect : rects) {
					this->_sprite.setPosition(pos);
					this->_sprite.rect.top = rect.y;
					this->_sprite.rect.left = rect.x;
					this->_sprite.rect.width = this->_rectSize.x;
					this->_sprite.rect.height = this->_rectSize.y;
					this->_sprite.draw();
					pos.x += size.x;
				}
			}
		}

		void _normalDraw(Vector2<int> pos, Vector2<unsigned> size, bool reverse, float alpha = 1) const
		{
			if (reverse) {
				for (unsigned i = this->_rects.size(); i > 0; i--) {
					this->_sprite.setPosition(pos);
					this->_sprite.rect.top = this->_rects[i - 1].y;
					this->_sprite.rect.left = this->_rects[i - 1].x;
					this->_sprite.rect.width = this->_rectSize.x;
					this->_sprite.rect.height = this->_rectSize.y;
					this->_sprite.draw();
					pos.x -= size.x;
				}
				this->_sprite2.setPosition(pos);
				this->_sprite2.rect.top = this->_startRect.y;
				this->_sprite2.rect.left = this->_startRect.x;
				this->_sprite2.rect.width = this->_rectSize2.x;
				this->_sprite2.rect.height = this->_rectSize2.y;
				this->_sprite2.draw();
			} else {
				this->_sprite2.setPosition(pos);
				this->_sprite2.rect.top = this->_startRect.y;
				this->_sprite2.rect.left = this->_startRect.x;
				this->_sprite2.rect.width = this->_rectSize2.x;
				this->_sprite2.rect.height = this->_rectSize2.y;
				this->_sprite2.draw();
				for (const auto &rect : this->_rects) {
					pos.x += size.x;
					this->_sprite.setPosition(pos);
					this->_sprite.rect.top = rect.y;
					this->_sprite.rect.left = rect.x;
					this->_sprite.rect.width = this->_rectSize.x;
					this->_sprite.rect.height = this->_rectSize.y;
					this->_sprite.draw();
				}
			}
		}

	public:
		MoveSpriteDescriptor(Sprite &sprite, Vector2<int> rect, Vector2<unsigned> rectSize) :
			_sprite(sprite),
			_sprite2(sprite),
			_startRect(rect),
			_rectSize(rectSize),
			_rectSize2(rectSize)
		{
		}

		MoveSpriteDescriptor(Sprite &sprite, const std::vector<Vector2<int>>& rects, Vector2<unsigned> rectSize) :
			MoveSpriteDescriptor(sprite, sprite, rects, rectSize, rectSize)
		{
		}

		MoveSpriteDescriptor(Sprite &sprite, std::vector<std::string> sequences, Vector2<unsigned> rectSize) :
			_isSkill(true),
			_sequences(std::move(sequences)),
			_sprite(sprite),
			_sprite2(sprite),
			_rectSize(rectSize)
		{
		}

		MoveSpriteDescriptor(Sprite &sprite, Sprite &sprite2, std::vector<Vector2<int>> rects, Vector2<unsigned> rectSize, Vector2<unsigned> rectSize2) :
			_sprite(sprite),
			_sprite2(sprite2),
			_startRect(rects[0]),
			_rects(std::move(rects)),
			_rectSize(rectSize),
			_rectSize2(rectSize2)
		{
			this->_rects.erase(this->_rects.begin());
		}

		//TODO: Pass a struct instead of this never-ending argument list
		void draw(Sprite &skills, Sprite &spells, Vector2<int> pos, Vector2<unsigned> size, bool reverse, SokuLib::Character character, SokuLib::Action action, float alpha = 1) const
		{
			this->_sprite.setSize(size);
			this->_sprite.tint = DxSokuColor::White * alpha;
			this->_sprite2.setSize(size);
			this->_sprite2.tint = (action == FAKE_ACTION_LILIPAD_DESPAWN ? DxSokuColor::Red : DxSokuColor::White) * alpha;
			//TODO: Find a more elegant way to do this
			if (
				(action >= SokuLib::ACTION_USING_SC_ID_200 && action <= SokuLib::ACTION_USING_SC_ID_219) ||
				(action >= FAKE_ACTION_lSC201 && action <= FAKE_ACTION_uSC212) ||
				(action >= SokuLib::ACTION_SC_ID_200_ALT_EFFECT && action <= SokuLib::ACTION_SC_ID_219_ALT_EFFECT)
			)
				return this->_drawSpell(spells, pos, size, reverse, action, alpha);
			else if (action >= SokuLib::ACTION_ORRERIES_B && action <= FAKE_ACTION_ORRERIES_REACTIVATE)
				return this->_displayOrreriesSpecialStuff(spells, pos, size, reverse, action, alpha);
			else if (this->_isSkill)
				return this->_drawSkill(skills, pos, size, reverse, character, action);
			else
				return this->_normalDraw(pos, size, reverse, alpha);
		}
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
	static Sprite leftScSheet;
	static Sprite rightScSheet;
	static Sprite inputSheet;
	static Sprite systemsSheet;
	static MoveState lastLeftMove;
	static MoveState lastRightMove;
	static const std::map<SokuLib::Action, MoveSpriteDescriptor> moveSprites{
		{ SokuLib::ACTION_5A, {
			inputSheet,
			{A_SPRITE_POS},
			{24, 32}
		} },
		{ SokuLib::ACTION_f5A, {
			inputSheet,
			{FAR_SPRITE_POS, A_SPRITE_POS},
			{24, 32}
		} },
		{ SokuLib::ACTION_6A, {
			inputSheet,
			{RIGHT_SPRITE_POS, A_SPRITE_POS},
			{24, 32}
		} },
		{ SokuLib::ACTION_2A, {
			inputSheet,
			{DOWN_SPRITE_POS, A_SPRITE_POS},
			{24, 32}
		} },
		{ SokuLib::ACTION_3A, {
			inputSheet,
			{RIGHTDOWN_SPRITE_POS, A_SPRITE_POS},
			{24, 32}
		} },
		{ SokuLib::ACTION_66A, {
			inputSheet,
			{DASH_SPRITE_POS, A_SPRITE_POS},
			{24, 32}
		} },
		{ SokuLib::ACTION_j5A, {
			inputSheet,
			{AIR_SPRITE_POS, A_SPRITE_POS},
			{24, 32}
		} },
		{ SokuLib::ACTION_j6A, {
			inputSheet,
			{AIR_SPRITE_POS, RIGHT_SPRITE_POS, A_SPRITE_POS},
			{24, 32}
		} },
		{ SokuLib::ACTION_j2A, {
			inputSheet,
			{AIR_SPRITE_POS, DOWN_SPRITE_POS, A_SPRITE_POS},
			{24, 32}
		} },
		{ SokuLib::ACTION_j8A, {
			inputSheet,
			{AIR_SPRITE_POS, UP_SPRITE_POS, A_SPRITE_POS},
			{24, 32}
		} },
		{ SokuLib::ACTION_f2A, {
			inputSheet,
			{FAR_SPRITE_POS, DOWN_SPRITE_POS, A_SPRITE_POS},
			{24, 32}
		} },
		{ SokuLib::ACTION_5AA, {
			inputSheet,
			{A_SPRITE_POS, A_SPRITE_POS},
			{24, 32}
		} },
		{ SokuLib::ACTION_5AAA, {
			inputSheet,
			{A_SPRITE_POS, A_SPRITE_POS, A_SPRITE_POS},
			{24, 32}
		} },
		{ SokuLib::ACTION_5AAAA, {
			inputSheet,
			{A_SPRITE_POS, A_SPRITE_POS, A_SPRITE_POS, A_SPRITE_POS},
 			{24, 32}
		} },
		{ SokuLib::ACTION_5AAAAA, {
			inputSheet,
			{A_SPRITE_POS, A_SPRITE_POS, A_SPRITE_POS, A_SPRITE_POS, A_SPRITE_POS},
  			{24, 32}
		} },
		{ SokuLib::ACTION_j5AA, {
			inputSheet,
			{AIR_SPRITE_POS, A_SPRITE_POS, A_SPRITE_POS},
			{24, 32}
		} },
		{ SokuLib::ACTION_4A, {
			inputSheet,
			{LEFT_SPRITE_POS, A_SPRITE_POS},
			{24, 32}
		} },
		{ SokuLib::ACTION_5B, {
			inputSheet,
			{B_SPRITE_POS},
			{24, 32}
		} },
		{ SokuLib::ACTION_6B, {
			inputSheet,
			{RIGHT_SPRITE_POS, B_SPRITE_POS},
			{24, 32}
		} },
		{ SokuLib::ACTION_2B, {
			inputSheet,
			{DOWN_SPRITE_POS, B_SPRITE_POS},
			{24, 32}
		} },
		{ SokuLib::ACTION_3B, {
			inputSheet,
			{RIGHTDOWN_SPRITE_POS, B_SPRITE_POS},
			{24, 32}
		} },
		{ SokuLib::ACTION_j5B, {
			inputSheet,
			{AIR_SPRITE_POS, B_SPRITE_POS},
			{24, 32}
		} },
		{ SokuLib::ACTION_j6B, {
			inputSheet,
			{AIR_SPRITE_POS, RIGHT_SPRITE_POS, B_SPRITE_POS},
			{24, 32}
		} },
		{ SokuLib::ACTION_j2B, {
			inputSheet,
			{AIR_SPRITE_POS, DOWN_SPRITE_POS, B_SPRITE_POS},
			{24, 32}
		} },
		{ SokuLib::ACTION_4B, {
			inputSheet,
			{LEFT_SPRITE_POS, B_SPRITE_POS},
			{24, 32}
		} },
		{ SokuLib::ACTION_66B, {
			inputSheet,
			{DASH_SPRITE_POS, B_SPRITE_POS},
			{24, 32}
		} },
		{ SokuLib::ACTION_j4B, {
			inputSheet,
			{AIR_SPRITE_POS, LEFT_SPRITE_POS, B_SPRITE_POS},
			{24, 32}
		} },
		{ SokuLib::ACTION_5C, {
			inputSheet,
			{C_SPRITE_POS},
			{24, 32}
		} },
		{ SokuLib::ACTION_6C, {
			inputSheet,
			{RIGHT_SPRITE_POS, C_SPRITE_POS},
			{24, 32}
		} },
		{ SokuLib::ACTION_2C, {
			inputSheet,
			{DOWN_SPRITE_POS, C_SPRITE_POS},
			{24, 32}
		} },
		{ SokuLib::ACTION_j5C, {
			inputSheet,
			{AIR_SPRITE_POS, C_SPRITE_POS},
			{24, 32}
		} },
		{ SokuLib::ACTION_j6C, {
			inputSheet,
			{AIR_SPRITE_POS, RIGHT_SPRITE_POS, C_SPRITE_POS},
			{24, 32}
		} },
		{ SokuLib::ACTION_j2C, {
			inputSheet,
			{AIR_SPRITE_POS, DOWN_SPRITE_POS, C_SPRITE_POS},
			{24, 32}
		} },
		{ SokuLib::ACTION_j1C, {
			inputSheet,
			{AIR_SPRITE_POS, LEFTDOWN_SPRITE_POS, C_SPRITE_POS},
			{24, 32}
		} },
		{ SokuLib::ACTION_66C, {
			inputSheet,
			{DASH_SPRITE_POS, C_SPRITE_POS},
			{24, 32}
		} },
		{ SokuLib::ACTION_FORWARD_DASH, {
			inputSheet,
			{RIGHT_SPRITE_POS, DASH_SPRITE_POS},
			{24, 32}
		} },
		{ SokuLib::ACTION_BACKDASH, {
			inputSheet,
			{LEFT_SPRITE_POS, DASH_SPRITE_POS},
			{24, 32}
		} },
		{ SokuLib::ACTION_LILIPAD_FORWARD_DASH, {
			inputSheet,
			{LILIPAD_SPRITE_POS, RIGHT_SPRITE_POS, DASH_SPRITE_POS},
			{24, 32}
		} },
		{ SokuLib::ACTION_LILIPAD_BACKDASH, {
			inputSheet,
			{LILIPAD_SPRITE_POS, LEFT_SPRITE_POS, DASH_SPRITE_POS},
			{24, 32}
		} },
		{ SokuLib::ACTION_FORWARD_AIRDASH, {
			inputSheet,
			{AIR_SPRITE_POS, RIGHT_SPRITE_POS, DASH_SPRITE_POS},
			{24, 32}
		} },
		{ SokuLib::ACTION_BACKWARD_AIRDASH, {
			inputSheet,
			{AIR_SPRITE_POS, LEFT_SPRITE_POS, DASH_SPRITE_POS},
			{24, 32}
		} },
		{ SokuLib::ACTION_NEUTRAL_HIGH_JUMP, {
			inputSheet,
			{UP_SPRITE_POS, HJ_SPRITE_POS},
			{24, 32}
		} },
		{ SokuLib::ACTION_FORWARD_HIGH_JUMP, {
			inputSheet,
			{RIGHTUP_SPRITE_POS, HJ_SPRITE_POS},
			{24, 32}
		} },
		{ SokuLib::ACTION_BACKWARD_HIGH_JUMP, {
			inputSheet,
			{LEFTUP_SPRITE_POS, HJ_SPRITE_POS},
			{24, 32}
		} },
		{ SokuLib::ACTION_BE2, {
			inputSheet,
			{DOWN_SPRITE_POS, BE_SPRITE_POS},
			{24, 32}
		} },
		{ SokuLib::ACTION_BE1, {
			inputSheet,
			{LEFTDOWN_SPRITE_POS, BE_SPRITE_POS},
			{24, 32}
		} },
		{ SokuLib::ACTION_BE6, {
			inputSheet,
			{RIGHT_SPRITE_POS, BE_SPRITE_POS},
			{24, 32}
		} },
		{ SokuLib::ACTION_BE4, {
			inputSheet,
			{LEFT_SPRITE_POS, BE_SPRITE_POS},
			{24, 32}
		} },
		{ SokuLib::ACTION_jBE4, {
			inputSheet,
			{AIR_SPRITE_POS, LEFT_SPRITE_POS, BE_SPRITE_POS},
			{24, 32}
		} },
		{ SokuLib::ACTION_jBE6, {
			inputSheet,
			{AIR_SPRITE_POS, RIGHT_SPRITE_POS, BE_SPRITE_POS},
			{24, 32}
		} },
		{ SokuLib::ACTION_FLY, {
			inputSheet,
			{AIR_SPRITE_POS, DASH_SPRITE_POS},
			{24, 32}
		} },
		{ SokuLib::ACTION_NEUTRAL_JUMP, {
			inputSheet,
			{UP_SPRITE_POS},
			{24, 32}
		} },
		{ SokuLib::ACTION_FORWARD_JUMP, {
			inputSheet,
			{RIGHTUP_SPRITE_POS},
			{24, 32}
		} },
		{ SokuLib::ACTION_BACKWARD_JUMP, {
			inputSheet,
			{LEFTUP_SPRITE_POS},
			{24, 32}
		} },
		{ SokuLib::ACTION_WALK_FORWARD, {
			inputSheet,
			{RIGHT_SPRITE_POS},
			{24, 32}
		} },
		{ SokuLib::ACTION_CROUCHING, {
			inputSheet,
			{DOWN_SPRITE_POS},
			{24, 32}
		} },
		{ SokuLib::ACTION_WALK_BACKWARD, {
			inputSheet,
			{LEFT_SPRITE_POS},
			{24, 32}
		} },
		{ SokuLib::ACTION_DEFAULT_SKILL1_B, {
			inputSheet,
			{"236b", "214b", "623b", "236b", "236b", "236b", "236b", "214b", "236b", "236b", "236b", "236b", "236b", "236b", "214b", "236b", "236b", "214b", "623b", "214b"},
			{24, 32}
		} },
		{ SokuLib::ACTION_DEFAULT_SKILL1_C, {
			inputSheet,
			{"236c", "214c", "623c", "236c", "236c", "236c", "236c", "214c", "236c", "236c", "236c", "236c", "236c", "236c", "214c", "236c", "236c", "214c", "623c", "214c"},
			{24, 32}
		} },
		{ SokuLib::ACTION_ALT1_SKILL1_B, {
			inputSheet,
			{"236b", "214b", "623b", "236b", "236b", "236b", "236b", "214b", "236b", "236b", "236b", "236b", "236b", "236b", "214b", "236b", "236b", "214b", "623b", "214b"},
			{24, 32}
		} },
		{ SokuLib::ACTION_ALT1_SKILL1_C, {
			inputSheet,
			{"236c", "214c", "623c", "236c", "236c", "236c", "236c", "214c", "236c", "236c", "236c", "236c", "236c", "236c", "214c", "236c", "236c", "214c", "623c", "214c"},
			{24, 32}
		} },
		{ SokuLib::ACTION_ALT2_SKILL1_B, {
			inputSheet,
			{"236b", "214b", "623b", "236b", "236b", "236b", "236b", "214b", "236b", "236b", "236b", "236b", "236b", "236b", "214b", "236b", "236b", "214b", "623b", "214b"},
			{24, 32}
		} },
		{ SokuLib::ACTION_ALT2_SKILL1_C, {
			inputSheet,
			{"236c", "214c", "623c", "236c", "236c", "236c", "236c", "214c", "236c", "236c", "236c", "236c", "236c", "236c", "214c", "236c", "236c", "214c", "623c", "214c"},
			{24, 32}
		} },
		{ SokuLib::ACTION_DEFAULT_SKILL2_B, {
			inputSheet,
			{"214b", "623b", "214b", "623b", "22b", "623b", "214b", "236b", "623b", "623b", "214b", "214b", "623b", "623b", "22b", "22b", "214b", "623b", "236b", "623b"},
			{24, 32}
		} },
		{ SokuLib::ACTION_DEFAULT_SKILL2_C, {
			inputSheet,
			{"214c", "623c", "214c", "623c", "22c", "623c", "214c", "236c", "623c", "623c", "214c", "214c", "623c", "623c", "22c", "22c", "214c", "623c", "236c", "623c"},
			{24, 32}
		} },
		{ SokuLib::ACTION_ALT1_SKILL2_B, {
			inputSheet,
			{"214b", "623b", "214b", "623b", "22b", "623b", "214b", "236b", "623b", "623b", "214b", "214b", "623b", "623b", "22b", "22b", "214b", "623b", "236b", "623b"},
			{24, 32}
		} },
		{ SokuLib::ACTION_ALT1_SKILL2_C, {
			inputSheet,
			{"214c", "623c", "214c", "623c", "22c", "623c", "214c", "236c", "623c", "623c", "214c", "214c", "623c", "623c", "22c", "22c", "214c", "623c", "236c", "623c"},
			{24, 32}
		} },
		{ SokuLib::ACTION_ALT2_SKILL2_B, {
			inputSheet,
			{"214b", "623b", "214b", "623b", "22b", "623b", "214b", "236b", "623b", "623b", "214b", "214b", "623b", "623b", "22b", "22b", "214b", "623b", "236b", "623b"},
			{24, 32}
		} },
		{ SokuLib::ACTION_ALT2_SKILL2_C, {
			inputSheet,
			{"214c", "623c", "214c", "623c", "22c", "623c", "214c", "236c", "623c", "623c", "214c", "214c", "623c", "623c", "22c", "22c", "214c", "623c", "236c", "623c"},
			{24, 32}
		} },
		{ SokuLib::ACTION_DEFAULT_SKILL3_B, {
			inputSheet,
			{"421b", "22b", "236b", "214b", "623b", "214b", "623b", "421b", "214b", "214b", "623b", "22b", "22b", "22b", "236b", "623b", "22b", "22b", "22b", "236b"},
			{24, 32}
		} },
		{ SokuLib::ACTION_DEFAULT_SKILL3_C, {
			inputSheet,
			{"421c", "22c", "236c", "214c", "623c", "214c", "623c", "421c", "214c", "214c", "623c", "22c", "22c", "22c", "236c", "623c", "22c", "22c", "22c", "236c"},
			{24, 32}
		} },
		{ SokuLib::ACTION_ALT1_SKILL3_B, {
			inputSheet,
			{"421b", "22b", "236b", "214b", "623b", "214b", "623b", "421b", "214b", "214b", "623b", "22b", "22b", "22b", "236b", "623b", "22b", "22b", "22b", "236b"},
			{24, 32}
		} },
		{ SokuLib::ACTION_ALT1_SKILL3_C, {
			inputSheet,
			{"421c", "22c", "236c", "214c", "623c", "214c", "623c", "421c", "214c", "214c", "623c", "22c", "22c", "22c", "236c", "623c", "22c", "22c", "22c", "236c"},
			{24, 32}
		} },
		{ SokuLib::ACTION_ALT2_SKILL3_B, {
			inputSheet,
			{"421b", "22b", "236b", "214b", "623b", "214b", "623b", "421b", "214b", "214b", "623b", "22b", "22b", "22b", "236b", "623b", "22b", "22b", "22b", "236b"},
			{24, 32}
		} },
		{ SokuLib::ACTION_ALT2_SKILL3_C, {
			inputSheet,
			{"421c", "22c", "236c", "214c", "623c", "214c", "623c", "421c", "214c", "214c", "623c", "22c", "22c", "22c", "236c", "623c", "22c", "22c", "22c", "236c"},
			{24, 32}
		} },
		{ SokuLib::ACTION_DEFAULT_SKILL4_B, {
			inputSheet,
			{"623b", "236b", "22b", "22b", "214b", "22b", "22b", "623b", "421b", "22b", "22b", "421b", "214b", "214b", "623b", "214b", "623b", "236b", "214b", "22b"},
			{24, 32}
		} },
		{ SokuLib::ACTION_DEFAULT_SKILL4_C, {
			inputSheet,
			{"623c", "236c", "22c", "22c", "214c", "22c", "22c", "623c", "421c", "22c", "22c", "421c", "214c", "214c", "623c", "214c", "623c", "236c", "214c", "22c"},
			{24, 32}
		} },
		{ SokuLib::ACTION_ALT1_SKILL4_B, {
			inputSheet,
			{"623b", "236b", "22b", "22b", "214b", "22b", "22b", "623b", "421b", "22b", "22b", "421b", "214b", "214b", "623b", "214b", "623b", "236b", "214b", "22b"},
			{24, 32}
		} },
		{ SokuLib::ACTION_ALT1_SKILL4_C, {
			inputSheet,
			{"623c", "236c", "22c", "22c", "214c", "22c", "22c", "623c", "421c", "22c", "22c", "421c", "214c", "214c", "623c", "214c", "623c", "236c", "214c", "22c"},
			{24, 32}
		} },
		{ SokuLib::ACTION_ALT2_SKILL4_B, {
			inputSheet,
			{"623b", "236b", "22b", "22b", "214b", "22b", "22b", "623b", "421b", "22b", "22b", "421b", "214b", "214b", "623b", "214b", "623b", "236b", "214b", "22b"},
			{24, 32}
		} },
		{ SokuLib::ACTION_ALT2_SKILL4_C, {
			inputSheet,
			{"623c", "236c", "22c", "22c", "214c", "22c", "22c", "623c", "421c", "22c", "22c", "421c", "214c", "214c", "623c", "214c", "623c", "236c", "214c", "22c"},
			{24, 32}
		} },
		{ SokuLib::ACTION_DEFAULT_SKILL5_B, {
			inputSheet,
			{"", "", "", "", "421b", "", "", "", "", "", "", "", "", "", "", "", "", "", "", ""},
			{24, 32}
		} },
		{ SokuLib::ACTION_DEFAULT_SKILL5_C, {
			inputSheet,
			{"", "", "", "", "421c", "", "", "", "", "", "", "", "", "", "", "", "", "", "", ""},
			{24, 32}
		} },
		{ SokuLib::ACTION_ALT1_SKILL5_B, {
			inputSheet,
			{"", "", "", "", "421b", "", "", "", "", "", "", "", "", "", "", "", "", "", "", ""},
			{24, 32}
		} },
		{ SokuLib::ACTION_ALT1_SKILL5_C, {
			inputSheet,
			{"", "", "", "", "421c", "", "", "", "", "", "", "", "", "", "", "", "", "", "", ""},
			{24, 32}
		} },
		{ SokuLib::ACTION_ALT2_SKILL5_B, {
			inputSheet,
			{"", "", "", "", "421b", "", "", "", "", "", "", "", "", "", "", "", "", "", "", ""},
			{24, 32}
		} },
		{ SokuLib::ACTION_ALT2_SKILL5_C, {
			inputSheet,
			{"", "", "", "", "421c", "", "", "", "", "", "", "", "", "", "", "", "", "", "", ""},
			{24, 32}
		} },
		{ SokuLib::ACTION_DEFAULT_SKILL1_AIR_B, {
			inputSheet,
			{"j236b", "j214b", "j623b", "j236b", "j236b", "j236b", "j236b", "j214b", "j236b", "j236b", "j236b", "j236b", "j236b", "j236b", "j214b", "j236b", "j236b", "j214b", "j623b", "j214b"},
			{24, 32}
		} },
		{ SokuLib::ACTION_DEFAULT_SKILL1_AIR_C, {
			inputSheet,
			{"j236c", "j214c", "j623c", "j236c", "j236c", "j236c", "j236c", "j214c", "j236c", "j236c", "j236c", "j236c", "j236c", "j236c", "j214c", "j236c", "j236c", "j214c", "j623c", "j214c"},
			{24, 32}
		} },
		{ SokuLib::ACTION_ALT1_SKILL1_AIR_B, {
			inputSheet,
			{"j236b", "j214b", "j623b", "j236b", "j236b", "j236b", "j236b", "j214b", "j236b", "j236b", "j236b", "j236b", "j236b", "j236b", "j214b", "j236b", "j236b", "j214b", "j623b", "j214b"},
			{24, 32}
		} },
		{ SokuLib::ACTION_ALT1_SKILL1_AIR_C, {
			inputSheet,
			{"j236c", "j214c", "j623c", "j236c", "j236c", "j236c", "j236c", "j214c", "j236c", "j236c", "j236c", "j236c", "j236c", "j236c", "j214c", "j236c", "j236c", "j214c", "j623c", "j214c"},
			{24, 32}
		} },
		{ SokuLib::ACTION_ALT2_SKILL1_AIR_B, {
			inputSheet,
			{"j236b", "j214b", "j623b", "j236b", "j236b", "j236b", "j236b", "j214b", "j236b", "j236b", "j236b", "j236b", "j236b", "j236b", "j214b", "j236b", "j236b", "j214b", "j623b", "j214b"},
			{24, 32}
		} },
		{ SokuLib::ACTION_ALT2_SKILL1_AIR_C, {
			inputSheet,
			{"j236c", "j214c", "j623c", "j236c", "j236c", "j236c", "j236c", "j214c", "j236c", "j236c", "j236c", "j236c", "j236c", "j236c", "j214c", "j236c", "j236c", "j214c", "j623c", "j214c"},
			{24, 32}
		} },
		{ SokuLib::ACTION_DEFAULT_SKILL2_AIR_B, {
			inputSheet,
			{"j214b", "j623b", "j214b", "j623b", "j22b", "j623b", "j214b", "j236b", "j623b", "j623b", "j214b", "j214b", "j623b", "j623b", "j22b", "j22b", "j214b", "j623b", "j236b", "j623b"},
			{24, 32}
		} },
		{ SokuLib::ACTION_DEFAULT_SKILL2_AIR_C, {
			inputSheet,
			{"j214c", "j623c", "j214c", "j623c", "j22c", "j623c", "j214c", "j236c", "j623c", "j623c", "j214c", "j214c", "j623c", "j623c", "j22c", "j22c", "j214c", "j623c", "j236c", "j623c"},
			{24, 32}
		} },
		{ SokuLib::ACTION_ALT1_SKILL2_AIR_B, {
			inputSheet,
			{"j214b", "j623b", "j214b", "j623b", "j22b", "j623b", "j214b", "j236b", "j623b", "j623b", "j214b", "j214b", "j623b", "j623b", "j22b", "j22b", "j214b", "j623b", "j236b", "j623b"},
			{24, 32}
		} },
		{ SokuLib::ACTION_ALT1_SKILL2_AIR_C, {
			inputSheet,
			{"j214c", "j623c", "j214c", "j623c", "j22c", "j623c", "j214c", "j236c", "j623c", "j623c", "j214c", "j214c", "j623c", "j623c", "j22c", "j22c", "j214c", "j623c", "j236c", "j623c"},
			{24, 32}
		} },
		{ SokuLib::ACTION_ALT2_SKILL2_AIR_B, {
			inputSheet,
			{"j214b", "j623b", "j214b", "j623b", "j22b", "j623b", "j214b", "j236b", "j623b", "j623b", "j214b", "j214b", "j623b", "j623b", "j22b", "j22b", "j214b", "j623b", "j236b", "j623b"},
			{24, 32}
		} },
		{ SokuLib::ACTION_ALT2_SKILL2_AIR_C, {
			inputSheet,
			{"j214c", "j623c", "j214c", "j623c", "j22c", "j623c", "j214c", "j236c", "j623c", "j623c", "j214c", "j214c", "j623c", "j623c", "j22c", "j22c", "j214c", "j623c", "j236c", "j623c"},
			{24, 32}
		} },
		{ SokuLib::ACTION_DEFAULT_SKILL3_AIR_B, {
			inputSheet,
			{"j421b", "j22b", "j236b", "j214b", "j623b", "j214b", "j623b", "j421b", "j214b", "j214b", "j623b", "j22b", "j22b", "j22b", "j236b", "j623b", "j22b", "j22b", "j22b", "j236b"},
			{24, 32}
		} },
		{ SokuLib::ACTION_DEFAULT_SKILL3_AIR_C, {
			inputSheet,
			{"j421c", "j22c", "j236c", "j214c", "j623c", "j214c", "j623c", "j421c", "j214c", "j214c", "j623c", "j22c", "j22c", "j22c", "j236c", "j623c", "j22c", "j22c", "j22c", "j236c"},
			{24, 32}
		} },
		{ SokuLib::ACTION_ALT1_SKILL3_AIR_B, {
			inputSheet,
			{"j421b", "j22b", "j236b", "j214b", "j623b", "j214b", "j623b", "j421b", "j214b", "j214b", "j623b", "j22b", "j22b", "j22b", "j236b", "j623b", "j22b", "j22b", "j22b", "j236b"},
			{24, 32}
		} },
		{ SokuLib::ACTION_ALT1_SKILL3_AIR_C, {
			inputSheet,
			{"j421c", "j22c", "j236c", "j214c", "j623c", "j214c", "j623c", "j421c", "j214c", "j214c", "j623c", "j22c", "j22c", "j22c", "j236c", "j623c", "j22c", "j22c", "j22c", "j236c"},
			{24, 32}
		} },
		{ SokuLib::ACTION_ALT2_SKILL3_AIR_B, {
			inputSheet,
			{"j421b", "j22b", "j236b", "j214b", "j623b", "j214b", "j623b", "j421b", "j214b", "j214b", "j623b", "j22b", "j22b", "j22b", "j236b", "j623b", "j22b", "j22b", "j22b", "j236b"},
			{24, 32}
		} },
		{ SokuLib::ACTION_ALT2_SKILL3_AIR_C, {
			inputSheet,
			{"j421c", "j22c", "j236c", "j214c", "j623c", "j214c", "j623c", "j421c", "j214c", "j214c", "j623c", "j22c", "j22c", "j22c", "j236c", "j623c", "j22c", "j22c", "j22c", "j236c"},
			{24, 32}
		} },
		{ SokuLib::ACTION_DEFAULT_SKILL4_AIR_B, {
			inputSheet,
			{"j623b", "j236b", "j22b", "j22b", "j214b", "j22b", "j22b", "j623b", "j421b", "j22b", "j22b", "j421b", "j214b", "j214b", "j623b", "j214b", "j623b", "j236b", "j214b", "j22b"},
			{24, 32}
		} },
		{ SokuLib::ACTION_DEFAULT_SKILL4_AIR_C, {
			inputSheet,
			{"j623c", "j236c", "j22c", "j22c", "j214c", "j22c", "j22c", "j623c", "j421c", "j22c", "j22c", "j421c", "j214c", "j214c", "j623c", "j214c", "j623c", "j236c", "j214c", "j22c"},
			{24, 32}
		} },
		{ SokuLib::ACTION_ALT1_SKILL4_AIR_B, {
			inputSheet,
			{"j623b", "j236b", "j22b", "j22b", "j214b", "j22b", "j22b", "j623b", "j421b", "j22b", "j22b", "j421b", "j214b", "j214b", "j623b", "j214b", "j623b", "j236b", "j214b", "j22b"},
			{24, 32}
		} },
		{ SokuLib::ACTION_ALT1_SKILL4_AIR_C, {
			inputSheet,
			{"j623c", "j236c", "j22c", "j22c", "j214c", "j22c", "j22c", "j623c", "j421c", "j22c", "j22c", "j421c", "j214c", "j214c", "j623c", "j214c", "j623c", "j236c", "j214c", "j22c"},
			{24, 32}
		} },
		{ SokuLib::ACTION_ALT2_SKILL4_AIR_B, {
			inputSheet,
			{"j623b", "j236b", "j22b", "j22b", "j214b", "j22b", "j22b", "j623b", "j421b", "j22b", "j22b", "j421b", "j214b", "j214b", "j623b", "j214b", "j623b", "j236b", "j214b", "j22b"},
			{24, 32}
		} },
		{ SokuLib::ACTION_ALT2_SKILL4_AIR_C, {
			inputSheet,
			{"j623c", "j236c", "j22c", "j22c", "j214c", "j22c", "j22c", "j623c", "j421c", "j22c", "j22c", "j421c", "j214c", "j214c", "j623c", "j214c", "j623c", "j236c", "j214c", "j22c"},
			{24, 32}
		} },
		{ SokuLib::ACTION_DEFAULT_SKILL5_AIR_B, {
			inputSheet,
			{"", "", "", "", "j421b", "", "", "", "", "", "", "", "", "", "", "", "", "", "", ""},
			{24, 32}
		} },
		{ SokuLib::ACTION_DEFAULT_SKILL5_AIR_C, {
			inputSheet,
			{"", "", "", "", "j421c", "", "", "", "", "", "", "", "", "", "", "", "", "", "", ""},
			{24, 32}
		} },
		{ SokuLib::ACTION_ALT1_SKILL5_AIR_B, {
			inputSheet,
			{"", "", "", "", "j421b", "", "", "", "", "", "", "", "", "", "", "", "", "", "", ""},
			{24, 32}
		} },
		{ SokuLib::ACTION_ALT1_SKILL5_AIR_C, {
			inputSheet,
			{"", "", "", "", "j421c", "", "", "", "", "", "", "", "", "", "", "", "", "", "", ""},
			{24, 32}
		} },
		{ SokuLib::ACTION_ALT2_SKILL5_AIR_B, {
			inputSheet,
			{"", "", "", "", "j421b", "", "", "", "", "", "", "", "", "", "", "", "", "", "", ""},
			{24, 32}
		} },
		{ SokuLib::ACTION_ALT2_SKILL5_AIR_C, {
			inputSheet,
			{"", "", "", "", "j421c", "", "", "", "", "", "", "", "", "", "", "", "", "", "", ""},
			{24, 32}
		} },
		{ SokuLib::ACTION_USING_SC_ID_200, { inputSheet, SC_SPRITE_POS, {24, 32} } },
		{ SokuLib::ACTION_USING_SC_ID_201, { inputSheet, SC_SPRITE_POS, {24, 32} } },
		{ SokuLib::ACTION_USING_SC_ID_202, { inputSheet, SC_SPRITE_POS, {24, 32} } },
		{ SokuLib::ACTION_USING_SC_ID_203, { inputSheet, SC_SPRITE_POS, {24, 32} } },
		{ SokuLib::ACTION_USING_SC_ID_204, { inputSheet, SC_SPRITE_POS, {24, 32} } },
		{ SokuLib::ACTION_USING_SC_ID_205, { inputSheet, SC_SPRITE_POS, {24, 32} } },
		{ SokuLib::ACTION_USING_SC_ID_206, { inputSheet, SC_SPRITE_POS, {24, 32} } },
		{ SokuLib::ACTION_USING_SC_ID_207, { inputSheet, SC_SPRITE_POS, {24, 32} } },
		{ SokuLib::ACTION_USING_SC_ID_208, { inputSheet, SC_SPRITE_POS, {24, 32} } },
		{ SokuLib::ACTION_USING_SC_ID_209, { inputSheet, SC_SPRITE_POS, {24, 32} } },
		{ SokuLib::ACTION_USING_SC_ID_210, { inputSheet, SC_SPRITE_POS, {24, 32} } },
		{ SokuLib::ACTION_USING_SC_ID_211, { inputSheet, SC_SPRITE_POS, {24, 32} } },
		{ SokuLib::ACTION_USING_SC_ID_212, { inputSheet, SC_SPRITE_POS, {24, 32} } },
		{ SokuLib::ACTION_USING_SC_ID_213, { inputSheet, SC_SPRITE_POS, {24, 32} } },
		{ SokuLib::ACTION_USING_SC_ID_214, { inputSheet, SC_SPRITE_POS, {24, 32} } },
		{ SokuLib::ACTION_USING_SC_ID_215, { inputSheet, SC_SPRITE_POS, {24, 32} } },
		{ SokuLib::ACTION_USING_SC_ID_216, { inputSheet, SC_SPRITE_POS, {24, 32} } },
		{ SokuLib::ACTION_USING_SC_ID_217, { inputSheet, SC_SPRITE_POS, {24, 32} } },
		{ SokuLib::ACTION_USING_SC_ID_218, { inputSheet, SC_SPRITE_POS, {24, 32} } },
		{ SokuLib::ACTION_USING_SC_ID_219, { inputSheet, SC_SPRITE_POS, {24, 32} } },
		{ SokuLib::ACTION_SYSTEM_CARD, {
			inputSheet,
			systemsSheet,
			{Vector2<int>{41 * 21, 0}, SC_SPRITE_POS},
			{24, 32},
			{41, 65}
		} },
		{ SokuLib::ACTION_IBUKI_GOURD, {
			inputSheet,
			systemsSheet,
			{Vector2<int>{41 * 9, 0}, SC_SPRITE_POS},
			{24, 32},
			{41, 65}
		} },
		{ SokuLib::ACTION_RECOVERY_CHARM, {
			inputSheet,
			systemsSheet,
			{Vector2<int>{41 * 15, 0}, SC_SPRITE_POS},
			{24, 32},
			{41, 65}
		} },
		{ SokuLib::ACTION_MAGIC_POTION, {
			inputSheet,
			systemsSheet,
			{Vector2<int>{41 * 1, 0}, SC_SPRITE_POS},
			{24, 32},
			{41, 65}
		} },
		{ SokuLib::ACTION_TALISMAN, {
			inputSheet,
			systemsSheet,
			{Vector2<int>{41 * 0, 0}, SC_SPRITE_POS},
			{24, 32},
			{41, 65}
		} },
		{ SokuLib::ACTION_DIVINE_RAIMENT_OF_THE_DRAGON_FISH, {
			inputSheet,
			systemsSheet,
			{Vector2<int>{41 * 13, 0}, SC_SPRITE_POS},
			{24, 32},
			{41, 65}
		} },
		{ SokuLib::ACTION_LEFT_HANDED_FOLDING_FAN, {
			inputSheet,
			systemsSheet,
			{Vector2<int>{41 * 8, 0}, SC_SPRITE_POS},
			{24, 32},
			{41, 65}
		} },
		{ SokuLib::ACTION_SPELL_BREAKING_DRUG, {
			inputSheet,
			systemsSheet,
			{Vector2<int>{41 * 11, 0}, SC_SPRITE_POS},
			{24, 32},
			{41, 65}
		} },
		{ SokuLib::ACTION_ORRERIES_B, {
			inputSheet,
			{Vector2<int>{41 * 15}, B_SPRITE_POS},
			{24, 32}
		} },
		{ SokuLib::ACTION_ORRERIES_C, {
			inputSheet,
			{Vector2<int>{41 * 15}, C_SPRITE_POS},
			{24, 32}
		} },
		{ SokuLib::ACTION_AIR_ORRERIES_B, {
			inputSheet,
			{Vector2<int>{41 * 15}, AIR_SPRITE_POS, B_SPRITE_POS},
			{24, 32}
		} },
		{ SokuLib::ACTION_AIR_ORRERIES_C, {
			inputSheet,
			{Vector2<int>{41 * 15}, AIR_SPRITE_POS, C_SPRITE_POS},
			{24, 32}
		} },
		{ FAKE_ACTION_ORRERIES_REACTIVATE, { //This is the fake action generated when using the Orreries card when Orreries is already active
			inputSheet,
			{Vector2<int>{41 * 15}, SC_SPRITE_POS},
			{24, 32}
		} },
		{ FAKE_ACTION_5AAA6A, { //This is the fake action generated when Meiling uses 5AAA6A
			inputSheet,
			{A_SPRITE_POS, A_SPRITE_POS, A_SPRITE_POS, RIGHT_SPRITE_POS, A_SPRITE_POS},
			{24, 32}
		} },
		{ SokuLib::ACTION_5AAA3A, {
			inputSheet,
			{A_SPRITE_POS, A_SPRITE_POS, A_SPRITE_POS, RIGHTDOWN_SPRITE_POS, A_SPRITE_POS},
			{24, 32}
		} },
		{ FAKE_ACTION_j1D, {
			inputSheet,
			{AIR_SPRITE_POS, LEFTDOWN_SPRITE_POS, DASH_SPRITE_POS},
			{24, 32}
		} },
		{ FAKE_ACTION_j3D, {
			inputSheet,
			{AIR_SPRITE_POS, RIGHTDOWN_SPRITE_POS, DASH_SPRITE_POS},
			{24, 32}
		} },
		{ FAKE_ACTION_LILIPAD_NEUTRAL_HIGHJUMP, {
			inputSheet,
			{LILIPAD_SPRITE_POS, UP_SPRITE_POS, HJ_SPRITE_POS},
			{24, 32}
		} },
		{ FAKE_ACTION_LILIPAD_FORWARD_HIGHJUMP, {
			inputSheet,
			{LILIPAD_SPRITE_POS, RIGHTUP_SPRITE_POS, HJ_SPRITE_POS},
			{24, 32}
		} },
		{ FAKE_ACTION_LILIPAD_BACKWARD_HIGHJUMP, {
			inputSheet,
			{LILIPAD_SPRITE_POS, LEFTUP_SPRITE_POS, HJ_SPRITE_POS},
			{24, 32}
		} },
		{ FAKE_ACTION_LILIPAD_A, {
			inputSheet,
			{LILIPAD_SPRITE_POS, A_SPRITE_POS},
			{24, 32}
		} },
		{ FAKE_ACTION_LILIPAD_3A, {
			inputSheet,
			{LILIPAD_SPRITE_POS, RIGHTDOWN_SPRITE_POS, A_SPRITE_POS},
			{24, 32}
		} },
		{ FAKE_ACTION_LILIPAD_2B, {
			inputSheet,
			{LILIPAD_SPRITE_POS, DOWN_SPRITE_POS, B_SPRITE_POS},
			{24, 32}
		} },
		{ FAKE_ACTION_LILIPAD_6B, {
			inputSheet,
			{LILIPAD_SPRITE_POS, RIGHT_SPRITE_POS, B_SPRITE_POS},
			{24, 32}
		} },
		{ FAKE_ACTION_LILIPAD_2C, {
			inputSheet,
			{LILIPAD_SPRITE_POS, DOWN_SPRITE_POS, C_SPRITE_POS},
			{24, 32}
		} },
		{ FAKE_ACTION_LILIPAD_SPAWN, {
			inputSheet,
			{LILIPAD_SPRITE_POS},
			{24, 32}
		} },
		{ FAKE_ACTION_LILIPAD_DESPAWN, {
			inputSheet,
			{LILIPAD_SPRITE_POS},
			{24, 32}
		} },
		{ FAKE_ACTION_LILIPAD_DEFAULT_22B, {
			inputSheet,
			{"", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "l22b"},
			{24, 32}
		} },
		{ FAKE_ACTION_LILIPAD_DEFAULT_22C, {
			inputSheet,
			{"", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "l22c"},
			{24, 32}
		} },
		{ FAKE_ACTION_UNDERGROUND_ALT1_22B, {
			inputSheet,
			{"", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "u22b"},
			{24, 32}
		} },
		{ FAKE_ACTION_UNDERGROUND_ALT1_22C, {
			inputSheet,
			{"", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "u22c"},
			{24, 32}
		} },
		{ FAKE_ACTION_LILIPAD_DEFAULT_623b, {
			inputSheet,
			{"", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "l623b"},
			{24, 32}
		} },
		{ FAKE_ACTION_UNDERGROUND_DEFAULT_623b, {
			inputSheet,
			{"", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "u623b"},
			{24, 32}
		} },
		{ FAKE_ACTION_UNDERGROUND_DEFAULT_623c, {
			inputSheet,
			{"", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "u623c"},
			{24, 32}
		} },
		{ FAKE_ACTION_LILIPAD_ALT1_623b, {
			inputSheet,
			{"", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "l623b"},
			{24, 32}
		} },
		{ FAKE_ACTION_LILIPAD_ALT1_623c, {
			inputSheet,
			{"", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "l623c"},
			{24, 32}
		} },
		{ FAKE_ACTION_UNDERGROUND_DEFAULT_236b, {
			inputSheet,
			{"", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "u236b"},
			{24, 32}
		} },
		{ FAKE_ACTION_UNDERGROUND_DEFAULT_236c, {
			inputSheet,
			{"", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "u236c"},
			{24, 32}
		} },
		{ FAKE_ACTION_UNDERGROUND_ALT1_236b, {
			inputSheet,
			{"", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "u236b"},
			{24, 32}
		} },
		{ FAKE_ACTION_UNDERGROUND_ALT1_236c, {
			inputSheet,
			{"", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "u236c"},
			{24, 32}
		} },
		{ FAKE_ACTION_UNDERGROUND_ALT2_236b, {
			inputSheet,
			{"", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "u236b"},
			{24, 32}
		} },
		{ FAKE_ACTION_UNDERGROUND_ALT2_236c, {
			inputSheet,
			{"", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "u236c"},
			{24, 32}
		} },
		{ FAKE_ACTION_UNDERGROUND_DEFAULT_214b, {
			inputSheet,
			{"", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "u214b"},
			{24, 32}
		} },
		{ FAKE_ACTION_UNDERGROUND_DEFAULT_214c, {
			inputSheet,
			{"", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "u214c"},
			{24, 32}
		} },
		{ FAKE_ACTION_LILIPAD_ALT1_214b, {
			inputSheet,
			{"", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "l214b"},
			{24, 32}
		} },
		{ FAKE_ACTION_LILIPAD_ALT1_214c, {
			inputSheet,
			{"", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "l214c"},
			{24, 32}
		} },
		{ FAKE_ACTION_LILIPAD_ALT2_214b, {
			inputSheet,
			{"", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "l214b"},
			{24, 32}
		} },
		{ FAKE_ACTION_LILIPAD_ALT2_214c, {
			inputSheet,
			{"", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "l214c"},
			{24, 32}
		} },
		{ SokuLib::ACTION_SUWAKO_j2D, {
			inputSheet,
			{AIR_SPRITE_POS, DOWN_SPRITE_POS, DASH_SPRITE_POS},
			{24, 32}
		} },
		{ SokuLib::ACTION_SUWAKO_3A, {
			inputSheet,
			{RIGHTDOWN_SPRITE_POS, A_SPRITE_POS},
			{24, 32}
		} },
		{ SokuLib::ACTION_SUWAKO_LILIPAD_6A, {
			inputSheet,
			{LILIPAD_SPRITE_POS, RIGHT_SPRITE_POS, A_SPRITE_POS},
			{24, 32}
		} },
		{ FAKE_ACTION_lSC201, {
			inputSheet,
			{LILIPAD_SPRITE_POS, SC_SPRITE_POS},
			{24, 32}
		} },
		{ FAKE_ACTION_jSC201, {
			inputSheet,
			{AIR_SPRITE_POS, SC_SPRITE_POS},
			{24, 32}
		} },
		{ FAKE_ACTION_uSC202, {
			inputSheet,
			{UNDERGROUND_SPRITE_POS, SC_SPRITE_POS},
			{24, 32}
		} },
		{ FAKE_ACTION_jSC203, {
			inputSheet,
			{AIR_SPRITE_POS, SC_SPRITE_POS},
			{24, 32}
		} },
		{ FAKE_ACTION_lSC203, {
			inputSheet,
			{LILIPAD_SPRITE_POS, SC_SPRITE_POS},
			{24, 32}
		} },
		{ FAKE_ACTION_jSC204, {
			inputSheet,
			{AIR_SPRITE_POS, SC_SPRITE_POS},
			{24, 32}
		} },
		{ FAKE_ACTION_lSC205, {
			inputSheet,
			{LILIPAD_SPRITE_POS, SC_SPRITE_POS},
			{24, 32}
		} },
		{ FAKE_ACTION_uSC205, {
			inputSheet,
			{UNDERGROUND_SPRITE_POS, SC_SPRITE_POS},
			{24, 32}
		} },
		{ FAKE_ACTION_lSC206, {
			inputSheet,
			{LILIPAD_SPRITE_POS, SC_SPRITE_POS},
			{24, 32}
		} },
		{ FAKE_ACTION_jSC207, {
			inputSheet,
			{AIR_SPRITE_POS, SC_SPRITE_POS},
			{24, 32}
		} },
		{ FAKE_ACTION_lSC209, {
			inputSheet,
			{LILIPAD_SPRITE_POS, SC_SPRITE_POS},
			{24, 32}
		} },
		{ FAKE_ACTION_jSC209, {
			inputSheet,
			{AIR_SPRITE_POS, SC_SPRITE_POS},
			{24, 32}
		} },
		{ FAKE_ACTION_uSC212, {
			inputSheet,
			{UNDERGROUND_SPRITE_POS, SC_SPRITE_POS},
			{24, 32}
		} },
		{ SokuLib::ACTION_FANTASY_HEAVEN_ACTIVATE, {
			inputSheet,
			{REIMU_SPRITE_POS},
			{24, 32}
		} }
	};

	void initInputDisplay(LPCSTR profilePath)
	{
		std::string profile = profilePath;

		leftInputList.clear();
		rightInputList.clear();
		leftBox.setPosition({0, 60});
		leftBox.fillColors[GradiantRect::RECT_TOP_LEFT_CORNER]    = leftBox.fillColors[GradiantRect::RECT_TOP_RIGHT_CORNER]    = DxSokuColor::Black * MIN_BOX_ALPHA;
		leftBox.fillColors[GradiantRect::RECT_BOTTOM_LEFT_CORNER] = leftBox.fillColors[GradiantRect::RECT_BOTTOM_RIGHT_CORNER] = DxSokuColor::Black * MAX_BOX_ALPHA;
		leftBox.borderColors[0] = leftBox.borderColors[1] = leftBox.borderColors[2] = leftBox.borderColors[3] = DxSokuColor::Transparent;
		rightBox = leftBox;
		rightBox.setPosition({640 - BOX_WIDTH, 60});
		if (!inputSheet.texture.hasTexture())
			Texture::loadFromFile(inputSheet.texture, (profile + "/assets/inputs.png").c_str());
		if (!systemsSheet.texture.hasTexture())
			Texture::loadFromFile(systemsSheet.texture, (profile + "/assets/cards/systemCards.png").c_str());
		Texture::loadFromFile(leftSkillSheet.texture,  (profile +  + "/assets/skills/" + names[SokuLib::leftChar]  + "Skills.png").c_str());
		Texture::loadFromFile(rightSkillSheet.texture, (profile +  + "/assets/skills/" + names[SokuLib::rightChar] + "Skills.png").c_str());
		Texture::loadFromFile(leftScSheet.texture,     (profile +  + "/assets/cards/"  + names[SokuLib::leftChar]  + "Spells.png").c_str());
		Texture::loadFromFile(rightScSheet.texture,    (profile +  + "/assets/cards/"  + names[SokuLib::rightChar] + "Spells.png").c_str());
	}

	bool isCancelableBy(SokuLib::Action last, SokuLib::Action action)
	{
		if (last == SokuLib::ACTION_66A && action == SokuLib::ACTION_j5A)
			return false;
		if (last == SokuLib::ACTION_ALT2_SKILL3_AIR_B && action == FAKE_ACTION_UNDERGROUND_ALT2_236b)
			return false;
		if (last == SokuLib::ACTION_ALT2_SKILL3_AIR_C && action == FAKE_ACTION_UNDERGROUND_ALT2_236c)
			return false;
		return true;
	}

	SokuLib::Action addCustomActions(SokuLib::CharacterManager &character, SokuLib::Character characterId)
	{
		if (characterId == SokuLib::CHARACTER_MARISA && character.orreriesTimeLeft && character.objectBase.action == SokuLib::ACTION_USING_SC_ID_215)
			return FAKE_ACTION_ORRERIES_REACTIVATE;
		if (characterId == SokuLib::CHARACTER_IKU && (character.objectBase.action >= SokuLib::ACTION_5AAA && character.objectBase.action <= SokuLib::ACTION_5AAAAA))
			return static_cast<SokuLib::Action>(character.objectBase.action - 1);
		if (characterId == SokuLib::CHARACTER_MEILING && character.objectBase.action == SokuLib::ACTION_5AAAAA)
			return FAKE_ACTION_5AAA6A;
		//My mom says I am special !
		if (characterId == SokuLib::CHARACTER_SUWAKO)
			switch (character.objectBase.action) {
			case SokuLib::ACTION_NEUTRAL_HIGH_JUMP:
				if (character.objectBase.actionBlockId == 4)
					return FAKE_ACTION_LILIPAD_NEUTRAL_HIGHJUMP;
				break;
			case SokuLib::ACTION_BACKWARD_HIGH_JUMP:
				if (character.objectBase.actionBlockId == 4)
					return FAKE_ACTION_LILIPAD_BACKWARD_HIGHJUMP;
				break;
			case SokuLib::ACTION_FORWARD_HIGH_JUMP:
				if (character.objectBase.actionBlockId == 4)
					return FAKE_ACTION_LILIPAD_FORWARD_HIGHJUMP;
				break;
			case SokuLib::ACTION_SUWAKO_j1D_j3D:
				printf("%f\n", character.objectBase.speed.x);
				if (copysign(1, character.objectBase.speed.x) == character.objectBase.direction)
					return FAKE_ACTION_j3D;
				return FAKE_ACTION_j1D;
			case SokuLib::ACTION_2A:
				return FAKE_ACTION_LILIPAD_A;
			case SokuLib::ACTION_3A:
				return FAKE_ACTION_LILIPAD_3A;
			case SokuLib::ACTION_2B:
				return FAKE_ACTION_LILIPAD_2B;
			case SokuLib::ACTION_3B:
				return FAKE_ACTION_LILIPAD_6B;
			case SokuLib::ACTION_2C:
				return FAKE_ACTION_LILIPAD_2C;
			case SokuLib::ACTION_CROUCHING:
				return FAKE_ACTION_LILIPAD_SPAWN;
			case SokuLib::ACTION_STANDING_UP:
				return FAKE_ACTION_LILIPAD_DESPAWN;
			case SokuLib::ACTION_DEFAULT_SKILL4_AIR_B:
				return FAKE_ACTION_LILIPAD_DEFAULT_22B;
			case SokuLib::ACTION_DEFAULT_SKILL4_AIR_C:
				return FAKE_ACTION_LILIPAD_DEFAULT_22C;
			case SokuLib::ACTION_ALT1_SKILL4_AIR_B:
				return FAKE_ACTION_UNDERGROUND_ALT1_22B;
			case SokuLib::ACTION_ALT1_SKILL4_AIR_C:
				return FAKE_ACTION_UNDERGROUND_ALT1_22C;
			case SokuLib::ACTION_DEFAULT_SKILL2_AIR_B:
				if (character.objectBase.actionBlockId == 6)
					return FAKE_ACTION_UNDERGROUND_DEFAULT_623c;
				return FAKE_ACTION_LILIPAD_DEFAULT_623b;
			case SokuLib::ACTION_DEFAULT_SKILL2_B:
				if (character.objectBase.actionBlockId == 6)
					return FAKE_ACTION_UNDERGROUND_DEFAULT_623b;
				break;
			case SokuLib::ACTION_ALT1_SKILL2_B:
				return FAKE_ACTION_LILIPAD_ALT1_623b;
			case SokuLib::ACTION_ALT1_SKILL2_C:
				return FAKE_ACTION_LILIPAD_ALT1_623c;
			case SokuLib::ACTION_DEFAULT_SKILL3_B:
				return FAKE_ACTION_UNDERGROUND_DEFAULT_236b;
			case SokuLib::ACTION_DEFAULT_SKILL3_C:
				return FAKE_ACTION_UNDERGROUND_DEFAULT_236c;
			case SokuLib::ACTION_ALT1_SKILL3_B:
				return FAKE_ACTION_UNDERGROUND_ALT1_236b;
			case SokuLib::ACTION_ALT1_SKILL3_C:
				return FAKE_ACTION_UNDERGROUND_ALT1_236c;
			case SokuLib::ACTION_ALT2_SKILL3_B:
				return FAKE_ACTION_UNDERGROUND_ALT2_236b;
			case SokuLib::ACTION_ALT2_SKILL3_C:
				return FAKE_ACTION_UNDERGROUND_ALT2_236c;
			case SokuLib::ACTION_DEFAULT_SKILL1_B:
				return FAKE_ACTION_UNDERGROUND_DEFAULT_214b;
			case SokuLib::ACTION_DEFAULT_SKILL1_C:
				return FAKE_ACTION_UNDERGROUND_DEFAULT_214c;
			case SokuLib::ACTION_ALT1_SKILL1_AIR_B:
				return FAKE_ACTION_LILIPAD_ALT1_214b;
			case SokuLib::ACTION_ALT1_SKILL1_AIR_C:
				return FAKE_ACTION_LILIPAD_ALT1_214c;
			case SokuLib::ACTION_ALT2_SKILL1_AIR_B:
				return FAKE_ACTION_LILIPAD_ALT2_214b;
			case SokuLib::ACTION_ALT2_SKILL1_AIR_C:
				return FAKE_ACTION_LILIPAD_ALT2_214c;
			case SokuLib::ACTION_USING_SC_ID_201:
				return FAKE_ACTION_lSC201;
			case SokuLib::ACTION_SC_ID_201_ALT_EFFECT:
				return FAKE_ACTION_jSC201;
			case SokuLib::ACTION_SC_ID_202_ALT_EFFECT:
				return FAKE_ACTION_uSC202;
			case SokuLib::ACTION_SC_ID_203_ALT_EFFECT:
				return FAKE_ACTION_jSC203;
			case SokuLib::ACTION_USING_SC_ID_203:
				return FAKE_ACTION_lSC203;
			case SokuLib::ACTION_SC_ID_204_ALT_EFFECT:
				return FAKE_ACTION_jSC204;
			case SokuLib::ACTION_USING_SC_ID_205:
				return FAKE_ACTION_lSC205;
			case SokuLib::ACTION_SC_ID_205_ALT_EFFECT:
				return FAKE_ACTION_uSC205;
			case SokuLib::ACTION_SC_ID_206_ALT_EFFECT:
				return FAKE_ACTION_lSC206;
			case SokuLib::ACTION_SC_ID_207_ALT_EFFECT:
				return FAKE_ACTION_jSC207;
			case SokuLib::ACTION_USING_SC_ID_209:
				return FAKE_ACTION_lSC209;
			case SokuLib::ACTION_SC_ID_209_ALT_EFFECT:
				return FAKE_ACTION_jSC209;
			case SokuLib::ACTION_USING_SC_ID_212:
				return FAKE_ACTION_uSC212;
			default:
				break;
			}
		return character.objectBase.action;
	}

	bool isStartOfMove(SokuLib::Action action, const SokuLib::CharacterManager &character, SokuLib::Character characterId)
	{
		if (action == FAKE_ACTION_ORRERIES_REACTIVATE)
			return character.objectBase.frameCount == 0 && character.objectBase.actionBlockId == 1;
		if (characterId == SokuLib::CHARACTER_YUKARI && (action == SokuLib::ACTION_4A || action == SokuLib::ACTION_5A))
			return character.objectBase.frameCount == 1 && character.objectBase.actionBlockId == 0;
		if (characterId == SokuLib::CHARACTER_AYA && action == SokuLib::ACTION_66B)
			return character.objectBase.frameCount == 1 && character.objectBase.actionBlockId == 0;
		if (characterId == SokuLib::CHARACTER_REMILIA && action == SokuLib::ACTION_ALT1_SKILL1_B)
			return character.objectBase.frameCount == 0;
		if (characterId == SokuLib::CHARACTER_YOUMU && action == SokuLib::ACTION_DEFAULT_SKILL3_B)
			return character.objectBase.frameCount == 0 && (character.objectBase.actionBlockId < 3);

		if (characterId == SokuLib::CHARACTER_SUWAKO) {
			if (action == FAKE_ACTION_UNDERGROUND_DEFAULT_623c || action == FAKE_ACTION_UNDERGROUND_DEFAULT_623b)
				return character.objectBase.frameCount == 0 && character.objectBase.actionBlockId == 6;
			if (action >= FAKE_ACTION_LILIPAD_NEUTRAL_HIGHJUMP && action <= FAKE_ACTION_LILIPAD_BACKWARD_HIGHJUMP)
				return character.objectBase.frameCount == 0 && character.objectBase.actionBlockId == 4;
			if (action == FAKE_ACTION_j1D || action == FAKE_ACTION_j3D)
				return character.objectBase.frameCount == 10 && (character.objectBase.actionBlockId == 3 || character.objectBase.actionBlockId == 0);
			if (action == SokuLib::ACTION_SUWAKO_j2D)
				return character.objectBase.frameCount == 0 && (character.objectBase.actionBlockId == 3 || character.objectBase.actionBlockId == 0);
			if (action == SokuLib::ACTION_j6C)
				return character.objectBase.frameCount == 0 && character.objectBase.actionBlockId == 0 && character.objectBase.animationCounter == 0;
			if (action == SokuLib::ACTION_USING_SC_ID_201 || action == SokuLib::ACTION_SC_ID_201_ALT_EFFECT)
				return character.objectBase.frameCount == 0 && character.objectBase.actionBlockId == 0 && character.objectBase.animationCounter == 0;
		}
		return character.objectBase.frameCount == 0 && character.objectBase.actionBlockId == 0;
	}

	void updateList(std::list<Input> &list, SokuLib::CharacterManager &character, MoveState &last, SokuLib::Character characterId)
	{
		if (list.empty() || list.front().input != character.keyMap) {
			list.push_front({character.keyMap, 1, SokuLib::ACTION_IDLE});
			while (list.size() > MAX_LIST_SIZE)
				list.pop_back();
		}

		auto realAction = addCustomActions(character, characterId);
		auto *front = &list.front();

		if (moveSprites.find(realAction) != moveSprites.end()) {
			printf("%i: action %i|block %i|frame %i|subanim %i|subframe %i\n", realAction, character.objectBase.action, character.objectBase.actionBlockId, character.objectBase.frameCount, character.objectBase.animationCounter, character.objectBase.animationSubFrame);
			if (isCancelableBy(last.action, realAction) && isStartOfMove(realAction, character, characterId)) {
				if (front->action) {
					list.push_front(list.front());
					while (list.size() > MAX_LIST_SIZE)
						list.pop_back();
					front = &list.front();
					front->duration = 0;
				}
				last.action = front->action = realAction;
			}
			last.animation = character.objectBase.animationCounter;
			last.subFrames = character.objectBase.animationSubFrame;
			last.frames = character.objectBase.frameCount;
		} else {
			last.action = SokuLib::ACTION_IDLE;
			if (realAction >= 200)
				printf("Unknown action %i\n", realAction);
		}
		front->duration++;
	}

	void updateInputLists()
	{
		updateList(leftInputList,  SokuLib::getBattleMgr().leftCharacterManager,  lastLeftMove,  SokuLib::leftChar);
		updateList(rightInputList, SokuLib::getBattleMgr().rightCharacterManager, lastRightMove, SokuLib::rightChar);
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

	//TODO: Pass a struct instead of this never-ending argument list
	void drawInputList(Sprite &skills, Sprite &spells, const std::list<Input> &list, Vector2<int> offset, bool reversed, SokuLib::Character character)
	{
		int baseX = offset.x;
		int border = offset.x;
		float alpha = MAX_ALPHA;
		int i = 0;
		SokuLib::KeyInput empty;

		memset(&empty, 0, sizeof(empty));
		if (reversed)
			baseX += BOX_WIDTH - SPRITE_SIZE;
		else
			border += BOX_WIDTH - SPRITE_SIZE;
		offset.y += BOX_HEIGHT + (SPRITE_SIZE / 6);
		for (auto &input : list) {
			if (memcmp(&input.input, &empty, sizeof(empty)) == 0 && !settings.showEmptyInputs && !input.action)
				continue;
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

			auto it = moveSprites.find(input.action);

			if (it != moveSprites.end())
				it->second.draw(skills, spells, {border, offset.y}, {SPRITE_SIZE - (SPRITE_SIZE / 4), SPRITE_SIZE}, !reversed, character, input.action, alpha);
			alpha -= (MAX_ALPHA - MIN_ALPHA) / 10;
			i++;
			if (i > (BOX_HEIGHT / (SPRITE_SIZE + (SPRITE_SIZE / 3))))
				break;
		}
	}

	void drawInputListOnlyMoves(Sprite &skills, Sprite &spells, const std::list<Input> &list, Vector2<int> offset, bool reversed, SokuLib::Character character)
	{
		int baseX = offset.x;
		float alpha = MAX_ALPHA;
		int i = 0;
		SokuLib::KeyInput empty;

		memset(&empty, 0, sizeof(empty));
		if (reversed)
			baseX += BOX_WIDTH2 - (SPRITE_SIZE - (SPRITE_SIZE / 4));
		offset.y += BOX_HEIGHT + (SPRITE_SIZE / 6);
		for (auto &input : list) {
			if (!input.action)
				continue;

			auto it = moveSprites.find(input.action);

			if (it == moveSprites.end())
				continue;

			offset.y -= SPRITE_SIZE + (SPRITE_SIZE / 3);
			offset.x = baseX;

			it->second.draw(skills, spells, offset, {SPRITE_SIZE - (SPRITE_SIZE / 4), SPRITE_SIZE}, reversed, character, input.action, alpha);
			alpha -= (MAX_ALPHA - MIN_ALPHA) / 10;

			i++;
			if (i > (BOX_HEIGHT / (SPRITE_SIZE + (SPRITE_SIZE / 3))))
				break;
		}
	}

	void displayInputs()
	{
		if (settings.showLeftInputBox) {
			if (settings.showRawInputs) {
				leftBox.setSize({BOX_WIDTH, BOX_HEIGHT});
				leftBox.draw();
				drawInputList(leftSkillSheet, leftScSheet, leftInputList, {0, 60}, false, SokuLib::leftChar);
			} else {
				leftBox.setSize({BOX_WIDTH2, BOX_HEIGHT});
				leftBox.draw();
				drawInputListOnlyMoves(leftSkillSheet, leftScSheet, leftInputList, {0, 60}, false, SokuLib::leftChar);
			}
		}
		if (settings.showRightInputBox) {
			if (settings.showRawInputs) {
				rightBox.setSize({BOX_WIDTH, BOX_HEIGHT});
				rightBox.setPosition({640 - BOX_WIDTH, 60});
				rightBox.draw();
				drawInputList(rightSkillSheet, rightScSheet, rightInputList, {640 - BOX_WIDTH, 60}, true, SokuLib::rightChar);
			} else {
				rightBox.setSize({BOX_WIDTH2, BOX_HEIGHT});
				rightBox.setPosition({640 - BOX_WIDTH2, 60});
				rightBox.draw();
				drawInputListOnlyMoves(rightSkillSheet, rightScSheet, rightInputList, {640 - BOX_WIDTH2, 60}, true, SokuLib::rightChar);
			}
		}
	}
}