//
// Created by PinkySmile on 05/04/2021.
//

#include <list>
#include "Gap.hpp"
#include "DrawUtils.hpp"
#include "State.hpp"

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

#define SMALL_HIT_TIME 11
#define MEDIUM_HIT_TIME 18
#define BIG_HIT_TIME 25
#define HUGE_HIT_TIME 35

#define AIR_BLOCK_TIME 20

namespace Practice
{
	struct GapElem {
		int timer;
		int gap;

		GapElem() = default;
		//GapElem(GapElem &other) :
		//	timer(other.timer),
		//	gap(other.gap)
		//{
		//	this->sprite.texture.swap(other.sprite.texture);
		//}
		GapElem(int timer, int gap): timer(timer), gap(gap) {};
	};
	static bool loaded = false;
	static SokuLib::SWRFont font;
	static RectangleShape rect;
	static RectangleShape bar;
	static std::pair<int, int> fas;
	static BlockingState left;
	static BlockingState right;
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
	std::map<SokuLib::Action, unsigned char> hitStun{
		/*  50 */ { SokuLib::ACTION_STAND_GROUND_HIT_SMALL_HITSTUN,   SMALL_HIT_TIME },
		/*  51 */ { SokuLib::ACTION_STAND_GROUND_HIT_MEDIUM_HITSTUN,  MEDIUM_HIT_TIME },
		/*  52 */ { SokuLib::ACTION_STAND_GROUND_HIT_BIG_HITSTUN,     BIG_HIT_TIME },
		/*  53 */ { SokuLib::ACTION_STAND_GROUND_HIT_HUGE_HITSTUN,    HUGE_HIT_TIME },
		/*  56 */ { SokuLib::ACTION_STAND_GROUND_HIT_SMALL_HITSTUN_2, SMALL_HIT_TIME },
		/*  57 */ { SokuLib::ACTION_STAND_GROUND_HIT_MEDIUM_HITSTUN_2,MEDIUM_HIT_TIME },
		/*  58 */ { SokuLib::ACTION_STAND_GROUND_HIT_BIG_HITSTUN_2,   BIG_HIT_TIME },
		/*  62 */ { SokuLib::ACTION_CROUCH_GROUND_HIT_SMALL_HITSTUN,  SMALL_HIT_TIME },
		/*  63 */ { SokuLib::ACTION_CROUCH_GROUND_HIT_MEDIUM_HITSTUN, MEDIUM_HIT_TIME },
		/*  64 */ { SokuLib::ACTION_CROUCH_GROUND_HIT_BIG_HITSTUN,    BIG_HIT_TIME },
	};

	static bool isAttacking(const SokuLib::CharacterManager &character)
	{
		return character.objectBase.action >= SokuLib::ACTION_5A;
	}

	static bool isBlocking(const SokuLib::CharacterManager &character)
	{
		return (
			character.objectBase.action >= SokuLib::ACTION_STAND_GROUND_HIT_SMALL_HITSTUN &&
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

	void showFrameAdvantageOrGapBox(const char *fmt, int v, int x, int &y, float alpha, DxSokuColor color)
	{
		Sprite sprite;
		char buffer[32];
		int text;

		if (alpha == 0)
			return;

		rect.setBorderColor(DxSokuColor::Black * alpha);
		rect.setFillColor((DxSokuColor::Black + DxSokuColor::White) * alpha);
		rect.setPosition({x, y});
		rect.draw();

		sprintf(buffer, fmt, v);
		if (!SokuLib::textureMgr.createTextTexture(&text, buffer, font, TEXTURE_SIZE, FONT_HEIGHT + 18, nullptr, nullptr)) {
			puts("C'est vraiment pas de chance");
			y -= 20;
			return;
		}

		sprite.setPosition({x, y});
		sprite.texture.setHandle(text, {TEXTURE_SIZE, FONT_HEIGHT + 18});
		sprite.setSize({TEXTURE_SIZE, FONT_HEIGHT + 18});
		sprite.rect = {
			0, 0, TEXTURE_SIZE, FONT_HEIGHT + 18
		};
		sprite.tint = color * alpha;
		sprite.draw();

		y -= 20;
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
		if (gap.gap)
			showFrameAdvantageOrGapBox("Gap: %iF", gap.gap, x, y, getAlpha(gap.timer), color);
		else
			showFrameAdvantageOrGapBox("Gap: %iF (mash.)", gap.gap, x, y, getAlpha(gap.timer), color);
	}

	static void drawBars(const SokuLib::CharacterManager &manager, const BlockingState &state)
	{
		auto it = hitStun.find(manager.objectBase.action);
		auto it2 = blockStun.find(manager.objectBase.action);
		unsigned actual;

		if (it != hitStun.end()) {
			if (!settings.showHitstun)
				return;
			if (manager.objectBase.action == SokuLib::ACTION_STAND_GROUND_HIT_HUGE_HITSTUN)
				actual = state.hitTimer;
			else
				actual = manager.objectBase.frameCount;
			actual = it->second - actual;
			bar.setPosition({
				static_cast<int>(SokuLib::camera.scale * (manager.objectBase.position.x - actual + SokuLib::camera.translate.x)),
				static_cast<int>(SokuLib::camera.scale * (manager.objectBase.position.y + 5 + SokuLib::camera.translate.y))
			});
			bar.setSize({
				actual * 2 + 1,
				5
			});
			bar.setBorderColor(DxSokuColor::Black);
			bar.setFillColor(DxSokuColor::Red);
			bar.draw();
		} else if (it2 != blockStun.end()) {
			if (!settings.showBlockstun)
				return;
			actual = it2->second - manager.objectBase.frameCount;
			bar.setPosition({
				static_cast<int>(SokuLib::camera.scale * (manager.objectBase.position.x - actual + SokuLib::camera.translate.x)),
				static_cast<int>(SokuLib::camera.scale * (manager.objectBase.position.y + 5 + SokuLib::camera.translate.y))
			});
			bar.setSize({
				actual * 2 + 1,
				5
			});
			bar.setBorderColor(DxSokuColor::Black);
			bar.setFillColor(DxSokuColor::White);
			bar.draw();
		} else if (
			!manager.damageLimited &&
			manager.untech &&
			manager.objectBase.action >= SokuLib::ACTION_STAND_GROUND_HIT_SMALL_HITSTUN &&
			manager.objectBase.action < SokuLib::ACTION_RIGHTBLOCK_HIGH_SMALL_BLOCKSTUN &&
			manager.objectBase.action != SokuLib::ACTION_KNOCKED_DOWN &&
			manager.objectBase.action != SokuLib::ACTION_KNOCKED_DOWN_STATIC
		) {
			if (!settings.showUntech)
				return;
			bar.setPosition({
				static_cast<int>(SokuLib::camera.scale * (manager.objectBase.position.x - manager.untech / 2 + SokuLib::camera.translate.x)),
				static_cast<int>(SokuLib::camera.scale * (-manager.objectBase.position.y + 5 + SokuLib::camera.translate.y))
			});
			bar.setSize({
				 static_cast<unsigned int>(manager.untech + 1),
				5
			});
			bar.setBorderColor(DxSokuColor::Black);
			bar.setFillColor(DxSokuColor{0xFF, 0x80, 0x00, 0xFF});
			bar.draw();
		}
	}

	void displayFrameStuff()
	{
		auto &battle = SokuLib::getBattleMgr();
		int yLeft = 416;
		int yRight = 416;

		if (!settings.nonSaved.enabled && settings.showFrameAdvantage) {
			if (timers.first < 240)
				showFrameAdvantageOrGapBox("Adv: %+i", fas.first,  348, yLeft,  getAlpha(timers.first),  (fas.first  < 0 ? DxSokuColor::Red : DxSokuColor::Green));
			if (timers.second < 240)
				showFrameAdvantageOrGapBox("Adv: %+i", fas.second, 202, yRight, getAlpha(timers.second), (fas.second < 0 ? DxSokuColor::Red : DxSokuColor::Green));
		}

		if (!settings.nonSaved.enabled && settings.showGaps) {
			for (auto &gap : gaps.first)
				drawGap(gap, 348, yLeft);
			for (auto &gap : gaps.second)
				drawGap(gap, 202, yRight);
		}
		drawBars(battle.leftCharacterManager,  left);
		drawBars(battle.rightCharacterManager, right);
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

		//if (!val)
		//	sprintf(buffer, "Gap: %iF (mash.)", val);
		//else
		//	sprintf(buffer, "Gap: %iF", val);
		gaps.push_front({0, val});
		//generateTextSprite(gaps.front().sprite, buffer);
	}

	static void updateBars(const SokuLib::CharacterManager &manager, BlockingState &state)
	{
		if (
			manager.objectBase.action == SokuLib::ACTION_STAND_GROUND_HIT_HUGE_HITSTUN && (
				state.lastFrame != manager.objectBase.frameCount ||
				manager.objectBase.action != state.lastAction
			)
		)
			state.hitTimer++;
		else
			state.hitTimer = 0;
		state.lastAction = manager.objectBase.action;
		state.lastFrame = manager.objectBase.frameCount;
	}

	void updatedFrameStuff()
	{
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
			//sprintf(buffer, "Adv: %+i", *padv);
			//generateTextSprite(leftFass, buffer);
		}
		if (dadv) {
			timers.second = 0;
			fas.second = *dadv;
			//sprintf(buffer, "Adv: %+i", *dadv);
			//generateTextSprite(rightFass, buffer);
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
		updateBars(battle.leftCharacterManager,  left);
		updateBars(battle.rightCharacterManager, right);
	}
}
