//
// Created by PinkySmile on 10/03/2021.
//

#include <SokuLib.hpp>
#include <cmath>
#include "DrawUtils.hpp"
#include "Hitboxes.hpp"
#include "State.hpp"
#include <list>

namespace Practice
{
#define BOXES_ALPHA 0.5
#define GRAZE_SPRITE_OFF          {0,   0}
#define HJC_SPRITE_OFF            {32,  0}
#define GRAB_INVUL_SPRITE_OFF     {64,  0}
#define PROJ_INVUL_SPRITE_OFF     {96,  0}
#define MELEE_INVUL_SPRITE_OFF    {128, 0}
#define CAN_BLOCK_SPRITE_OFF      {160, 0}
#define SUPERARMOR_SPRITE_OFF     {192, 0}
#define GRAZABLE_SPRITE_OFF       {0,   32}
#define AUB_SPRITE_OFF            {32,  32}
#define UNBLOCKABLE_SPRITE_OFF    {64,  32}
#define GRAB_SPRITE_OFF           {96,  32}
#define MILLENIUM_SPRITE_OFF      {128, 32}
#define DROP_SPRITE_OFF           {160, 32}
#define HEALTH_SPRITE_OFF         {192, 32}
#define FANTASY_HEAVEN_SPRITE_OFF {224, 0}
#define PHILOSOPHERS_STONE_OFF    {224, 32}
#define TIME_SPRITE_OFF           {256, 0}
#define CLONES_SPRITE_OFF         {256, 32}
#define HEALING_CHARM_SPRITE_OFF  {288, 0}
#define STOP_WATCH_SPRITE_OFF     {288, 32}
#define MAGIC_POTION_SPRITE_OFF   {320, 0}
#define TIME_STOP_SPRITE_OFF      {320, 32}
#define _SPRITE_OFF           {352, 0}
#define _SPRITE_OFF           {352, 32}

	struct MaxAttributes {
		unsigned short drop;
		unsigned short star;
		unsigned short clones;
		unsigned short stopWatch;
	};

	static RectangleShape rectangle;
	static Sprite flagsSprite;
	static std::pair<MaxAttributes, MaxAttributes> allMaxAttributes;

	void initBoxDisplay(LPCSTR profilePath)
	{
		if (flagsSprite.texture.hasTexture())
			return;

		std::string profile = profilePath;

		Texture::loadFromFile(flagsSprite.texture, (profile + "/assets/flags.png").c_str());
		flagsSprite.setSize({32, 32});
		flagsSprite.rect.height = 32;
	}

	static void drawBox(const SokuLib::Box &box, const SokuLib::RotationBox *rotation, DxSokuColor borderColor, DxSokuColor fillColor)
	{
		if (!rotation) {
			FloatRect rect{};

			rect.x1 = SokuLib::camera.scale * (SokuLib::camera.translate.x + box.left);
			rect.x2 = SokuLib::camera.scale * (SokuLib::camera.translate.x + box.right);
			rect.y1 = SokuLib::camera.scale * (SokuLib::camera.translate.y + box.top);
			rect.y2 = SokuLib::camera.scale * (SokuLib::camera.translate.y + box.bottom);
			rectangle.setRect(rect);
		} else {
			Rect<Vector2<float>> rect;

			rect.x1.x = SokuLib::camera.scale * (SokuLib::camera.translate.x + box.left);
			rect.x1.y = SokuLib::camera.scale * (SokuLib::camera.translate.y + box.top);

			rect.y1.x = SokuLib::camera.scale * (SokuLib::camera.translate.x + box.left + rotation->pt1.x);
			rect.y1.y = SokuLib::camera.scale * (SokuLib::camera.translate.y + box.top + rotation->pt1.y);

			rect.x2.x = SokuLib::camera.scale * (SokuLib::camera.translate.x + box.left + rotation->pt1.x + rotation->pt2.x);
			rect.x2.y = SokuLib::camera.scale * (SokuLib::camera.translate.y + box.top + rotation->pt1.y + rotation->pt2.y);

			rect.y2.x = SokuLib::camera.scale * (SokuLib::camera.translate.x + box.left + rotation->pt2.x);
			rect.y2.y = SokuLib::camera.scale * (SokuLib::camera.translate.y + box.top + rotation->pt2.y);
			rectangle.rawSetRect(rect);
		}

		rectangle.setFillColor(fillColor);
		rectangle.setBorderColor(borderColor);
		rectangle.draw();
	}

	static void drawCollisionBox(const SokuLib::ObjectManager &manager)
	{
		FloatRect rect{};
		const SokuLib::Box &box = *manager.frameData.collisionBox;

		if (!manager.frameData.collisionBox)
			return;

		rect.x1 = SokuLib::camera.scale * (std::ceil(manager.position.x) + manager.direction * box.left + SokuLib::camera.translate.x);
		rect.x2 = SokuLib::camera.scale * (std::ceil(manager.position.x) + manager.direction * box.right + SokuLib::camera.translate.x);
		rect.y1 = SokuLib::camera.scale * (box.top - std::ceil(manager.position.y) + SokuLib::camera.translate.y);
		rect.y2 = SokuLib::camera.scale * (box.bottom - std::ceil(manager.position.y) + SokuLib::camera.translate.y);

		rectangle.setRect(rect);
		rectangle.setFillColor(DxSokuColor::Yellow * BOXES_ALPHA);
		rectangle.setBorderColor(DxSokuColor::Yellow);
		rectangle.draw();
	}

	static void drawPositionBox(const SokuLib::ObjectManager &manager)
	{
		rectangle.setPosition({
			static_cast<int>(SokuLib::camera.scale * (manager.position.x - 2 + SokuLib::camera.translate.x)),
			static_cast<int>(SokuLib::camera.scale * (-manager.position.y - 2 + SokuLib::camera.translate.y))
		});
		rectangle.setSize({
			static_cast<unsigned int>(SokuLib::camera.scale * 5),
			static_cast<unsigned int>(SokuLib::camera.scale * 5)
		});
		rectangle.setFillColor(DxSokuColor::White);
		rectangle.setBorderColor(DxSokuColor::White + DxSokuColor::Black);
		rectangle.draw();
	}

	static void drawHurtBoxes(const SokuLib::ObjectManager &manager)
	{
		if (manager.hurtBoxCount > 5)
			return;

		for (int i = 0; i < manager.hurtBoxCount; i++)
			drawBox(
				manager.hurtBoxes[i],
				manager.hurtBoxesRotation[i],
				DxSokuColor::Green,
				(manager.frameData.frameFlags.chOnHit ? DxSokuColor::Cyan : DxSokuColor::Green) * BOXES_ALPHA
			);
	}

	static void drawHitBoxes(const SokuLib::ObjectManager &manager)
	{
		if (manager.hitBoxCount > 5)
			return;

		for (int i = 0; i < manager.hitBoxCount; i++)
			drawBox(manager.hitBoxes[i], manager.hitBoxesRotation[i], DxSokuColor::Red, DxSokuColor::Red * BOXES_ALPHA);
	}

	static void displayPlayerFrameFlag(Vector2<int> textureOffset, Vector2<int> &barPos, bool reverse)
	{
		if (reverse) {
			barPos.x -= 32;
			flagsSprite.rect.width = -32;
			flagsSprite.rect.left = textureOffset.x + 32;
		} else {
			flagsSprite.rect.width = 32;
			flagsSprite.rect.left = textureOffset.x;
		}
		flagsSprite.rect.top = textureOffset.y;
		rectangle.setPosition(barPos);
		rectangle.setSize({32, 32});
		rectangle.setBorderColor(DxSokuColor::Transparent);
		rectangle.setFillColor(DxSokuColor::White * 0.5);
		rectangle.draw();
		flagsSprite.setSize({32, 32});
		flagsSprite.setPosition(barPos);
		flagsSprite.draw();
		if (!reverse)
			barPos.x += 32;
	}

	static void displayPlayerBar(Vector2<int> textureOffset, Vector2<int> basePos, Vector2<int> &barPos, unsigned value, unsigned max, bool reverse, DxSokuColor color)
	{
		if (barPos.x != basePos.x || barPos.y != basePos.y) {
			barPos.x = basePos.x;
			barPos.y += 32;
		}
		displayPlayerFrameFlag(textureOffset, barPos, reverse);

		rectangle.setSize({200, 24});
		rectangle.setBorderColor(DxSokuColor::Black);
		rectangle.setFillColor({0x88, 0x88, 0x88, 0xFF});
		if (reverse)
			rectangle.setPosition({barPos.x - 201, barPos.y + 4});
		else
			rectangle.setPosition({barPos.x, barPos.y + 4});
		rectangle.draw();

		rectangle.setSize({200 * value / max, 24});
		rectangle.setBorderColor(DxSokuColor::Black);
		rectangle.setFillColor(color);
		if (reverse)
			rectangle.setPosition({static_cast<int>(barPos.x - 200 * value / max) - 1, barPos.y + 4});
		else
			rectangle.setPosition({barPos.x, barPos.y + 4});
		rectangle.draw();
	}

	static void displayObjectFrameFlag(Vector2<int> textureOffset, Vector2<int> &barPos)
	{
		auto scaled = 32 * SokuLib::camera.scale;

		flagsSprite.rect.width = 32;
		flagsSprite.rect.left = textureOffset.x;
		flagsSprite.rect.top = textureOffset.y;
		rectangle.setPosition(barPos);
		rectangle.setSize({
			static_cast<unsigned int>(scaled),
			static_cast<unsigned int>(scaled)
		});
		rectangle.setBorderColor(DxSokuColor::Transparent);
		rectangle.setFillColor(DxSokuColor::White * 0.5);
		rectangle.draw();
		flagsSprite.setSize({
			static_cast<unsigned int>(scaled),
			static_cast<unsigned int>(scaled)
		 });
		flagsSprite.setPosition(barPos);
		flagsSprite.draw();
		barPos.x += scaled;
	}

	static void displayObjectBar(Vector2<int> textureOffset, Vector2<int> basePos, Vector2<int> &barPos, unsigned value, unsigned max, DxSokuColor color)
	{
		if (barPos.x != basePos.x || barPos.y != basePos.y) {
			barPos.x = basePos.x;
			barPos.y += 32 * SokuLib::camera.scale;
		}
		displayObjectFrameFlag(textureOffset, barPos);
		rectangle.setSize({
			static_cast<unsigned int>(100 * SokuLib::camera.scale * value / max),
			static_cast<unsigned int>(24 * SokuLib::camera.scale)
		});
		rectangle.setBorderColor(DxSokuColor::Black);
		rectangle.setFillColor(color);
		rectangle.setPosition({
			barPos.x,
			barPos.y + static_cast<int>(4 * SokuLib::camera.scale)
		});
		rectangle.draw();
	}

	static float getSuperArmorRatio(const SokuLib::CharacterManager &manager, SokuLib::Character character, MaxAttributes &maxAttributes)
	{
		if (character == SokuLib::CHARACTER_PATCHOULI) {
			if (manager.diamondHardEffectLeft)
				return manager.diamondHardEffectLeft / 720.f;
		}
		if (manager.dragonStarTimeLeft) {
			maxAttributes.star = max(maxAttributes.star, manager.dragonStarTimeLeft);
			return manager.dragonStarTimeLeft / static_cast<float>(maxAttributes.star);
		} else
			maxAttributes.star = 0;
		return 0;
	}

	static unsigned getMaxSuperArmor(const SokuLib::CharacterManager &manager, SokuLib::Character character, MaxAttributes &maxAttributes)
	{
		if (character == SokuLib::CHARACTER_SUIKA) {
			if (manager.objectBase.action == SokuLib::ACTION_DEFAULT_SKILL1_B || manager.objectBase.action == SokuLib::ACTION_DEFAULT_SKILL1_C)
				switch (manager.skillLevels[0]) {
				case 0:
					return 0;
				case 1:
				case 2:
					return 250;
				case 3:
					return 500;
				default:
					return 750;
				}
		}
		if (character == SokuLib::CHARACTER_SANAE)
			if (manager.objectBase.action == SokuLib::ACTION_USING_SC_ID_210)
				return 2500;
		return 0;
	}

	static void displayPlayerStatusFlags(const SokuLib::CharacterManager &manager, SokuLib::Character character, MaxAttributes &maxAttributes, Vector2<int> barPos, bool reverse)
	{
		auto basePos = barPos;
		auto &base = manager.objectBase;
		auto &flags = base.frameData.frameFlags;
		auto superArmorRatio = getSuperArmorRatio(manager, character, maxAttributes);
		auto maxSuperArmor = getMaxSuperArmor(manager, character, maxAttributes);

		if (flags.graze)
			displayPlayerFrameFlag(GRAZE_SPRITE_OFF, barPos, reverse);
		if (flags.highJumpCancellable)
			displayPlayerFrameFlag(HJC_SPRITE_OFF, barPos, reverse);
		if (flags.grabInvincible || manager.grabInvulTimer)
			displayPlayerFrameFlag(GRAB_INVUL_SPRITE_OFF, barPos, reverse);
		if (flags.projectileInvincible || manager.projectileInvulTimer)
			displayPlayerFrameFlag(PROJ_INVUL_SPRITE_OFF, barPos, reverse);
		if (flags.meleeInvincible || manager.meleeInvulTimer)
			displayPlayerFrameFlag(MELEE_INVUL_SPRITE_OFF, barPos, reverse);
		if (flags.guardAvailable)
			displayPlayerFrameFlag(CAN_BLOCK_SPRITE_OFF, barPos, reverse);
		if (!maxAttributes.stopWatch && manager.timeStop && (character != SokuLib::CHARACTER_SAKUYA || !manager.sakuyasWorldTime))
			displayPlayerFrameFlag(STOP_WATCH_SPRITE_OFF, barPos, reverse);
		if (superArmorRatio || manager.noSuperArmor == 0 || flags.superArmor) {
			if (superArmorRatio > 0)
				displayPlayerBar(SUPERARMOR_SPRITE_OFF, basePos, barPos, 10000 * superArmorRatio, 10000, reverse, DxSokuColor::Blue);
			else if (maxSuperArmor)
				displayPlayerBar(SUPERARMOR_SPRITE_OFF, basePos, barPos, maxSuperArmor - manager.objectBase.superarmorDamageTaken, maxSuperArmor, reverse, DxSokuColor::Red);
			else
				displayPlayerFrameFlag(SUPERARMOR_SPRITE_OFF, barPos, reverse);
		}
		if (manager.dropInvulTimeLeft) {
			maxAttributes.drop = max(maxAttributes.drop, manager.dropInvulTimeLeft);
			displayPlayerBar(DROP_SPRITE_OFF, basePos, barPos, manager.dropInvulTimeLeft, maxAttributes.drop, reverse, DxSokuColor::Red);
		} else
			maxAttributes.drop = 0;
		if (manager.magicPotionTimeLeft != 0)
			displayPlayerBar(MAGIC_POTION_SPRITE_OFF, basePos, barPos, manager.magicPotionTimeLeft, 360, reverse, DxSokuColor::Blue);
		if (manager.healingCharmTimeLeft)
			displayPlayerBar(HEALING_CHARM_SPRITE_OFF, basePos, barPos, manager.healingCharmTimeLeft, 250, reverse, DxSokuColor::Cyan);
		if (character == SokuLib::CHARACTER_SAKUYA && manager.sakuyasWorldTime)
			displayPlayerBar(STOP_WATCH_SPRITE_OFF, basePos, barPos, manager.sakuyasWorldTime, 300, reverse, DxSokuColor{0x44, 0x44, 0x44, 0xFF});
		else if (maxAttributes.stopWatch)
			displayPlayerBar(STOP_WATCH_SPRITE_OFF, basePos, barPos, 51 - maxAttributes.stopWatch, 50, reverse, DxSokuColor{0x44, 0x44, 0x44, 0xFF});

		switch (character) {
		case SokuLib::CHARACTER_PATCHOULI:
			if (manager.philosophersStoneTime)
				displayPlayerBar(PHILOSOPHERS_STONE_OFF, basePos, barPos, manager.philosophersStoneTime, 1200, reverse, DxSokuColor::Magenta);
			break;
		case SokuLib::CHARACTER_REMILIA:
			if (manager.milleniumVampireTime)
				displayPlayerBar(MILLENIUM_SPRITE_OFF, basePos, barPos, manager.milleniumVampireTime, 600, reverse, DxSokuColor::Red);
			break;
		case SokuLib::CHARACTER_YOUMU:
			if (manager.youmuCloneTimeLeft) {
				maxAttributes.clones = max(maxAttributes.clones, manager.youmuCloneTimeLeft);
				displayPlayerBar(CLONES_SPRITE_OFF, basePos, barPos, manager.youmuCloneTimeLeft, maxAttributes.clones, reverse, DxSokuColor{0xAA, 0xAA, 0xAA, 0xFF});
			} else
				maxAttributes.clones = 0;
			break;
		case SokuLib::CHARACTER_REIMU:
			if (manager.fantasyHeavenTimeLeft)
				displayPlayerBar(FANTASY_HEAVEN_SPRITE_OFF, basePos, barPos, manager.fantasyHeavenTimeLeft, 900, reverse, DxSokuColor::Red);
			break;
		default:
			break;
		}
	}

	static void displayHitProperties(SokuLib::Character baseCharacter, const SokuLib::ObjectManager &manager, bool showHp)
	{
		int maxX = 640 - (32 + 50) * SokuLib::camera.scale;
		int maxY = 480 - 32 * SokuLib::camera.scale;
		Vector2<int> leftBound = {maxX, 0};
		Vector2<int> base;
		int rightBound = 0;
		auto &attack = manager.frameData.attackFlags;
		Rect<Vector2<float>> rect;

		for (int i = 0; i < manager.hurtBoxCount; i++) {
			auto &box = manager.hurtBoxes[i];
			auto rotation = manager.hurtBoxesRotation[i];

			rect.x1.x = SokuLib::camera.scale * (SokuLib::camera.translate.x + box.left);
			rect.x1.y = SokuLib::camera.scale * (SokuLib::camera.translate.y + box.top);
			if (!rotation) {
				rect.x2.x = SokuLib::camera.scale * (SokuLib::camera.translate.x + box.right);
				rect.x2.y = SokuLib::camera.scale * (SokuLib::camera.translate.y + box.top);

				rect.y1.x = SokuLib::camera.scale * (SokuLib::camera.translate.x + box.left);
				rect.y1.y = SokuLib::camera.scale * (SokuLib::camera.translate.y + box.bottom);

				rect.y2.x = SokuLib::camera.scale * (SokuLib::camera.translate.x + box.right);
				rect.y2.y = SokuLib::camera.scale * (SokuLib::camera.translate.y + box.bottom);
			} else {
				rect.y1.x = SokuLib::camera.scale * (SokuLib::camera.translate.x + box.left + rotation->pt1.x);
				rect.y1.y = SokuLib::camera.scale * (SokuLib::camera.translate.y + box.top +  rotation->pt1.y);

				rect.x2.x = SokuLib::camera.scale * (SokuLib::camera.translate.x + box.left + rotation->pt1.x + rotation->pt2.x);
				rect.x2.y = SokuLib::camera.scale * (SokuLib::camera.translate.y + box.top +  rotation->pt1.y + rotation->pt2.y);

				rect.y2.x = SokuLib::camera.scale * (SokuLib::camera.translate.x + box.left + rotation->pt2.x);
				rect.y2.y = SokuLib::camera.scale * (SokuLib::camera.translate.y + box.top +  rotation->pt2.y);
			}
			rightBound  = min(maxX, max(rightBound, max(rect.x1.x, max(rect.x2.x, max(rect.y1.x, rect.x2.x)))));
			leftBound.x = max(0, min(leftBound.x, min(rect.x1.x, min(rect.x2.x, min(rect.y1.x, rect.x2.x)))));
			leftBound.y = min(maxY, max(leftBound.y, max(rect.x1.y, max(rect.x2.y, max(rect.y1.y, rect.x2.y)))));
		}

		for (int i = 0; i < manager.hitBoxCount; i++) {
			auto &box = manager.hitBoxes[i];
			auto rotation = manager.hitBoxesRotation[i];

			rect.x1.x = SokuLib::camera.scale * (SokuLib::camera.translate.x + box.left);
			rect.x1.y = SokuLib::camera.scale * (SokuLib::camera.translate.y + box.top);
			if (!rotation) {
				rect.x2.x = SokuLib::camera.scale * (SokuLib::camera.translate.x + box.right);
				rect.x2.y = SokuLib::camera.scale * (SokuLib::camera.translate.y + box.top);

				rect.y1.x = SokuLib::camera.scale * (SokuLib::camera.translate.x + box.left);
				rect.y1.y = SokuLib::camera.scale * (SokuLib::camera.translate.y + box.bottom);

				rect.y2.x = SokuLib::camera.scale * (SokuLib::camera.translate.x + box.right);
				rect.y2.y = SokuLib::camera.scale * (SokuLib::camera.translate.y + box.bottom);
			} else {
				rect.y1.x = SokuLib::camera.scale * (SokuLib::camera.translate.x + box.left + rotation->pt1.x);
				rect.y1.y = SokuLib::camera.scale * (SokuLib::camera.translate.y + box.top +  rotation->pt1.y);

				rect.x2.x = SokuLib::camera.scale * (SokuLib::camera.translate.x + box.left + rotation->pt1.x + rotation->pt2.x);
				rect.x2.y = SokuLib::camera.scale * (SokuLib::camera.translate.y + box.top +  rotation->pt1.y + rotation->pt2.y);

				rect.y2.x = SokuLib::camera.scale * (SokuLib::camera.translate.x + box.left + rotation->pt2.x);
				rect.y2.y = SokuLib::camera.scale * (SokuLib::camera.translate.y + box.top +  rotation->pt2.y);
			}
			rightBound = min(maxX, max(rightBound, max(rect.x1.x, max(rect.x2.x, max(rect.y1.x, rect.x2.x)))));
			leftBound.x = max(0, min(leftBound.x, min(rect.x1.x, min(rect.x2.x, min(rect.y1.x, rect.x2.x)))));
			leftBound.y = min(maxY, max(leftBound.y, max(rect.x1.y, max(rect.x2.y, max(rect.y1.y, rect.x2.y)))));
		}

		if (!manager.hurtBoxCount && !manager.hitBoxCount)
			leftBound = Vector2<int>{
				static_cast<int>(manager.position.x),
				static_cast<int>(manager.position.y)
			};

		base = leftBound;
		if (manager.hitBoxCount) {
			if (attack.grab)
				displayObjectFrameFlag(GRAB_SPRITE_OFF, base);
			else if (attack.unblockable)
				displayObjectFrameFlag(UNBLOCKABLE_SPRITE_OFF, base);
			else if (!attack.airBlockable)
				displayObjectFrameFlag(AUB_SPRITE_OFF, base);
			if (attack.grazable)
				displayObjectFrameFlag(GRAZABLE_SPRITE_OFF, base);
		}
		if (manager.hurtBoxCount) {
			if (showHp) {
				auto realHp = manager.hp;
				auto maxHp = 10000;

				if ( //This is Patchouli's bubble
					baseCharacter == SokuLib::CHARACTER_PATCHOULI &&
					manager.action == 0x358 &&
					(manager.image.number == 0x154 || manager.image.number == 0x1B3)
				) {
					displayObjectBar(TIME_SPRITE_OFF, leftBound, base, 360 - manager.frameCount, 360, DxSokuColor::Green);
					realHp += 3000;
					maxHp = 3000;
				}

				if ( //This is Alice's doll (C)
					baseCharacter == SokuLib::CHARACTER_ALICE &&
					manager.action == 805 &&
					(manager.image.number >= 304 && manager.image.number <= 313)
				) {
					realHp += 600;
					maxHp = 600;
				}

				if ( //This is Alice's doll (d22)
					baseCharacter == SokuLib::CHARACTER_ALICE &&
					manager.action == 825 &&
					(manager.image.number >= 322 && manager.image.number <= 325)
				) {
					realHp += 700;
					maxHp = 700;
				}

				if (realHp && maxHp > 1)
					displayObjectBar(HEALTH_SPRITE_OFF, leftBound, base, realHp, maxHp, DxSokuColor::Green);
			}
		}
	}

	static void drawPlayerBoxes(const SokuLib::CharacterManager &manager, SokuLib::Character character, MaxAttributes &maxAttributes, Vector2<int> barPos, bool reverse, const HitBoxParams &params)
	{
		if (params.showCollisionBox)
			drawCollisionBox(manager.objectBase);
		if (params.showHurtboxes)
			drawHurtBoxes(manager.objectBase);
		if (params.showHitboxes)
			drawHitBoxes(manager.objectBase);
		if (params.showPosition)
			drawPositionBox(manager.objectBase);
		if (params.showBuffProperties)
			displayPlayerStatusFlags(manager, character, maxAttributes, barPos, reverse);
		if (params.showHitProperties)
			displayHitProperties(character, manager.objectBase, false);

		auto array = manager.objects.list.vector();

		for (const auto _elem : array) {
			auto elem = reinterpret_cast<const SokuLib::ProjectileManager *>(_elem);

			if ((elem->isActive && elem->objectBase.hitCount > 0) || elem->objectBase.frameData.attackFlags.value > 0) {
				if (params.showSubObjectHurtboxes)
					drawHurtBoxes(elem->objectBase);
				if (params.showSubObjectHitboxes)
					drawHitBoxes(elem->objectBase);
				if (params.showSubObjectPosition)
					drawPositionBox(elem->objectBase);
				if (params.showSubObjectProperties)
					displayHitProperties(character, elem->objectBase, true);
			}
		}
	}

	void drawAllBoxes()
	{
		auto &battle = SokuLib::getBattleMgr();

		drawPlayerBoxes(battle.leftCharacterManager,  SokuLib::leftChar,  allMaxAttributes.first,  {20, 70}, false, settings.leftHitboxSettings);
		drawPlayerBoxes(battle.rightCharacterManager, SokuLib::rightChar, allMaxAttributes.second, {620, 70}, true, settings.rightHitboxSettings);
	}

	void updateCharacter(const SokuLib::CharacterManager &manager, MaxAttributes &maxAttributes)
	{
		if (manager.timeStop && (manager.objectBase.action < SokuLib::ACTION_USING_SC_ID_200 || manager.objectBase.action > SokuLib::ACTION_SC_ID_219_ALT_EFFECT))
			maxAttributes.stopWatch++;
		else
			maxAttributes.stopWatch = 0;
	}

	void onUpdate()
	{
		updateCharacter(SokuLib::getBattleMgr().leftCharacterManager, allMaxAttributes.first);
		updateCharacter(SokuLib::getBattleMgr().rightCharacterManager, allMaxAttributes.second);
	}
}