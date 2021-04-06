//
// Created by PinkySmile on 05/04/2021.
//

#include <list>
#include "Gap.hpp"
#include "DrawUtils.hpp"

#define FONT_HEIGHT 14
#define TEXTURE_SIZE 0x100

#define SMALL_RIGHT_BLOCK_TIME 10
#define MEDIUM_RIGHT_BLOCK_TIME 16
#define BIG_RIGHT_BLOCK_TIME 22
#define HUGE_RIGHT_BLOCK_TIME 28

#define SMALL_WRONG_BLOCK_TIME 12
#define MEDIUM_WRONG_BLOCK_TIME 20
#define BIG_WRONG_BLOCK_TIME 28
#define HUGE_WRONG_BLOCK_TIME 32

#define AIR_BLOCK_TIME 20

namespace Practice
{
	struct GapElem {
		int timer;
		int gap;
		Sprite sprite;

		GapElem() = default;
		GapElem(GapElem &other) :
			timer(other.timer),
			gap(other.gap)
		{
			this->sprite.texture.swap(other.sprite.texture);
		}
		GapElem(int timer, int gap): timer(timer), gap(gap) {};
	};
	static bool loaded = false;
	static SokuLib::SWRFont font;
	static RectangleShape rect;
	static std::pair<int, int> fas;
	static Sprite leftFass;
	static Sprite rightFass;
	static std::pair<unsigned, unsigned> timers = {400, 400};
	static std::pair<std::list<GapElem>, std::list<GapElem>> gaps;

	std::map<SokuLib::Action, unsigned char> blockStun{
		{ SokuLib::ACTION_AIR_GUARD,                        AIR_BLOCK_TIME },

		{ SokuLib::ACTION_RIGHTBLOCK_LOW_SMALL_BLOCKSTUN,   SMALL_RIGHT_BLOCK_TIME },
		{ SokuLib::ACTION_RIGHTBLOCK_HIGH_SMALL_BLOCKSTUN,  SMALL_RIGHT_BLOCK_TIME },
		{ SokuLib::ACTION_WRONGBLOCK_HIGH_SMALL_BLOCKSTUN,  SMALL_WRONG_BLOCK_TIME },
		{ SokuLib::ACTION_WRONGBLOCK_LOW_SMALL_BLOCKSTUN,   SMALL_WRONG_BLOCK_TIME },

		{ SokuLib::ACTION_RIGHTBLOCK_HIGH_MEDIUM_BLOCKSTUN, MEDIUM_RIGHT_BLOCK_TIME },
		{ SokuLib::ACTION_RIGHTBLOCK_LOW_MEDIUM_BLOCKSTUN,  MEDIUM_RIGHT_BLOCK_TIME },
		{ SokuLib::ACTION_WRONGBLOCK_HIGH_MEDIUM_BLOCKSTUN, MEDIUM_WRONG_BLOCK_TIME },
		{ SokuLib::ACTION_WRONGBLOCK_LOW_MEDIUM_BLOCKSTUN,  MEDIUM_WRONG_BLOCK_TIME },

		{ SokuLib::ACTION_RIGHTBLOCK_HIGH_BIG_BLOCKSTUN,    BIG_RIGHT_BLOCK_TIME },
		{ SokuLib::ACTION_RIGHTBLOCK_LOW_BIG_BLOCKSTUN,     BIG_RIGHT_BLOCK_TIME },
		{ SokuLib::ACTION_WRONGBLOCK_HIGH_BIG_BLOCKSTUN,    BIG_WRONG_BLOCK_TIME },
		{ SokuLib::ACTION_WRONGBLOCK_LOW_BIG_BLOCKSTUN,     BIG_WRONG_BLOCK_TIME },

		{ SokuLib::ACTION_RIGHTBLOCK_HIGH_HUGE_BLOCKSTUN,   HUGE_RIGHT_BLOCK_TIME },
		{ SokuLib::ACTION_RIGHTBLOCK_LOW_HUGE_BLOCKSTUN,    HUGE_RIGHT_BLOCK_TIME },
		{ SokuLib::ACTION_WRONGBLOCK_HIGH_HUGE_BLOCKSTUN,   HUGE_WRONG_BLOCK_TIME },
		{ SokuLib::ACTION_WRONGBLOCK_LOW_HUGE_BLOCKSTUN,    HUGE_WRONG_BLOCK_TIME }
	};

	static bool isAttacking(const SokuLib::CharacterManager &character)
	{
		return character.objectBase.action >= SokuLib::ACTION_5A;
	}

	static bool isBlocking(const SokuLib::CharacterManager &character)
	{
		return (
			character.objectBase.action >= SokuLib::ACTION_GROUND_HIT_SMALL_HITSTUN &&
			character.objectBase.action <= SokuLib::ACTION_NEUTRAL_TECH
		) && (
			!character.objectBase.frameData.frameFlags.guardAvailable ||
			character.objectBase.frameData.frameFlags.guarding
		);
	}

	std::optional<int> getFrameAdvantage(const SokuLib::CharacterManager &attacker, const SokuLib::CharacterManager &blocker, BlockingState &state)
	{
		bool attacking = isAttacking(attacker);
		bool blocking = isBlocking(blocker);
		std::optional<int> val;

		if (blocking && attacking) {
			state.started = true;
			state.timer = 0;
		}
		if (!blocking && !attacking && state.started) {
			state.started = false;
			val = state.wasBlocking ? state.timer : -state.timer;
		}
		if (!attacking || !blocking)
			state.timer++;
		state.wasAttacking = attacking;
		state.wasBlocking = blocking;
		return val;
	}

	static bool canMash(const SokuLib::CharacterManager &manager)
	{
		auto it = blockStun.find(manager.objectBase.action);

		if (it == blockStun.end())
			return false;
		return it->second - 1 <= manager.objectBase.frameCount;
	}

	std::optional<int> getGap(const SokuLib::CharacterManager &attacker, const SokuLib::CharacterManager &blocker, BlockingState &state)
	{
		std::optional<int> val;

		if (state.wasBlocking) {
			if (state.gapCounter >= 0 && state.gapCounter <= 30)
				val = state.gapCounter;
			state.gapCounter = -1;
		}
		if (!state.wasBlocking || canMash(blocker))
			state.gapCounter++;
		return val;
	}

	void initFont()
	{
		SokuLib::FontDescription desc;

		if (loaded)
			return;
		rect.setSize({102, 18});
		loaded = true;
		desc.r1 = 205;
		desc.r2 = 205;
		desc.g1 = 205;
		desc.g2 = 205;
		desc.b1 = 205;
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

	void showFrameAdvantageOrGapBox(Sprite &sprite, int x, int &y, float alpha, DxSokuColor color)
	{
		if (alpha == 0)
			return;
		rect.setBorderColor(DxSokuColor::Black * alpha);
		rect.setFillColor((DxSokuColor::Black + DxSokuColor::White) * alpha);
		sprite.setPosition({x, y});
		rect.setPosition({x, y});
		y -= 20;
		sprite.tint = color * alpha;
		rect.draw();
		sprite.draw();
	}

	static float getAlpha(unsigned timer)
	{
		if (timer < 5)
			return timer / 5.f;
		if (timer > 180)
			return 1 - (timer - 180) / 60.f;
		return 1.f;
	}

	void drawGap(GapElem &gap, int x, int &y)
	{
		DxSokuColor color;

		if (gap.gap < 7)
			color = DxSokuColor::Green;
		else if (gap.gap < 12)
			color = DxSokuColor::Yellow;
		else if (gap.gap < 16)
			color = DxSokuColor{0xFF, 0x80, 0x00, 0xFF};
		else
			color = DxSokuColor::Red;
		showFrameAdvantageOrGapBox(gap.sprite, x, y, getAlpha(gap.timer), color);
	}

	void displayFrameStuff()
	{
		int yLeft = 416;
		int yRight = 416;

		if (timers.first < 240)
			showFrameAdvantageOrGapBox(leftFass,  348, yLeft,  getAlpha(timers.first),  (fas.first  < 0 ? DxSokuColor::Red : DxSokuColor::Green));
		if (timers.second < 240)
			showFrameAdvantageOrGapBox(rightFass, 202, yRight, getAlpha(timers.second), (fas.second < 0 ? DxSokuColor::Red : DxSokuColor::Green));

		for (auto &gap : gaps.first)
			drawGap(gap, 348, yLeft);
		for (auto &gap : gaps.second)
			drawGap(gap, 202, yRight);
	}

	static void generateTextSprite(Sprite &sprite, const char *buffer)
	{
		int text;

		SokuLib::textureMgr.createTextTexture(&text, buffer, font, TEXTURE_SIZE, FONT_HEIGHT + 18, nullptr, nullptr);
		sprite.texture.setHandle(text, {TEXTURE_SIZE, FONT_HEIGHT + 18});
		sprite.setSize({TEXTURE_SIZE, FONT_HEIGHT + 18});
		sprite.rect = {
			0, 0, TEXTURE_SIZE, FONT_HEIGHT + 18
		};
	}

	static void addGap(int val, std::list<GapElem> &gaps)
	{
		char buffer[30];

		if (!val)
			sprintf(buffer, "Gap: %iF (mash.)", val);
		else
			sprintf(buffer, "Gap: %iF", val);
		gaps.push_front({0, val});
		generateTextSprite(gaps.front().sprite, buffer);
	}

	void updatedFrameStuff()
	{
		static BlockingState left;
		static BlockingState right;
		auto &battle = SokuLib::getBattleMgr();
		auto padv = getFrameAdvantage(battle.leftCharacterManager, battle.rightCharacterManager, left);
		auto dadv = getFrameAdvantage(battle.rightCharacterManager, battle.leftCharacterManager, right);
		auto pgap = getGap(battle.leftCharacterManager, battle.rightCharacterManager, left);
		auto dgap = getGap(battle.rightCharacterManager, battle.leftCharacterManager, right);
		char buffer[30];

		initFont();
		if (padv) {
			timers.first = 0;
			fas.first = *padv;
			sprintf(buffer, "Adv: %+i", *padv);
			generateTextSprite(leftFass, buffer);
		}
		if (dadv) {
			timers.second = 0;
			fas.second = *dadv;
			sprintf(buffer, "Adv: %+i", *dadv);
			generateTextSprite(rightFass, buffer);
		}

		timers.first++;
		timers.second++;
		if (pgap)
			addGap(*pgap, gaps.first);
		if (dgap)
			addGap(*dgap, gaps.second);
		while (!gaps.first.empty() && gaps.first.back().timer >= 240)
			gaps.first.pop_back();
		while (!gaps.second.empty() && gaps.second.back().timer >= 240)
			gaps.second.pop_back();
		for (auto &gap : gaps.first)
			gap.timer++;
		for (auto &gap : gaps.second)
			gap.timer++;
	}
}
