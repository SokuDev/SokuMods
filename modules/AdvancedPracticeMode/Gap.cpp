//
// Created by PinkySmile on 05/04/2021.
//

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
	static bool loaded = false;
	static SokuLib::SWRFont font;
	static RectangleShape rect;
	static std::pair<int, int> gaps;
	static std::pair<unsigned, unsigned> timers = {400, 400};

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

	void initFont()
	{
		SokuLib::FontDescription desc;

		if (loaded)
			return;
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

	static void drawSprite(int texid, float x, float y, float cx, float cy) {
		const struct {
			float x, y, z;
			float rhw;
			unsigned color;
			float u, v;
		} vertices[] = {
			{x, y, 0.0f, 1.0f, 0xffffffff, 0.0f, 0.0f},
			{x + cx, y, 0.0f, 1.0f, 0xffffffff, 1.0f, 0.0f},
			{x + cx, y + cy, 0.0f, 1.0f, 0xffffffff, 1.0f, 1.0f},
			{x, y + cy, 0.0f, 1.0f, 0xffffffff, 0.0f, 1.0f},
		};

		SokuLib::textureMgr.setTexture(texid, 0);
		SokuLib::pd3dDev->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, 2, vertices, sizeof(*vertices));
	}

	void showGap(int val, int x, float alpha)
	{
		int text;
		char buffer[30];
		Sprite sprite;

		if (alpha == 0)
			return;
		sprintf(buffer, "Adv: %+i", val);
		SokuLib::textureMgr.createTextTexture(&text, buffer, font, TEXTURE_SIZE, FONT_HEIGHT + 18, nullptr, nullptr);
		sprite.texture.setHandle(text, {TEXTURE_SIZE, FONT_HEIGHT + 18});
		rect.setBorderColor(DxSokuColor::Black * alpha);
		rect.setFillColor((DxSokuColor::Black + DxSokuColor::White) * alpha);
		sprite.setPosition({x, 416});
		rect.setPosition({x, 416});
		sprite.setSize({TEXTURE_SIZE, FONT_HEIGHT + 18});
		rect.setSize({88, 18});
		sprite.tint = (val < 0 ? DxSokuColor::Red : DxSokuColor::Green) * alpha;
		sprite.rect = {
			0, 0, TEXTURE_SIZE, FONT_HEIGHT + 18
		};
		rect.draw();
		sprite.draw();
	}

	void displayFrameStuff()
	{
		static BlockingState left;
		static BlockingState right;
		auto &battle = SokuLib::getBattleMgr();
		auto padv = getFrameAdvantage(battle.leftCharacterManager, battle.rightCharacterManager, left);
		auto dadv = getFrameAdvantage(battle.rightCharacterManager, battle.leftCharacterManager, right);

		initFont();
		if (padv) {
			timers.first = 0;
			gaps.first = *padv;
		}
		if (dadv) {
			timers.second = 0;
			gaps.second = *dadv;
		}
		showGap(gaps.first,  348, 1 - max(0, min(1, (timers.first - 120.f) / 60)));
		showGap(gaps.second, 202, 1 - max(0, min(1, (timers.second - 120.f) / 60)));
		timers.first++;
		timers.second++;
	}
}
