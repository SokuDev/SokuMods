//
// Created by PinkySmile on 10/03/2021.
//

#include <SokuLib.hpp>
#include <cmath>
#include "DrawUtils.hpp"
#include "Hitboxes.hpp"
#include <list>

namespace Practice
{
#define BOXES_ALPHA 0.5
#define GRAZE_SPRITE_OFF       {0,   0}
#define HJC_SPRITE_OFF         {32,  0}
#define GRAB_INVUL_SPRITE_OFF  {64,  0}
#define PROJ_INVUL_SPRITE_OFF  {96,  0}
#define MELEE_INVUL_SPRITE_OFF {128, 0}
#define CAN_BLOCK_SPRITE_OFF   {160, 0}
#define SUPERARMOR_SPRITE_OFF  {192, 0}
#define GRAZABLE_SPRITE_OFF    {0,   32}
#define AUB_SPRITE_OFF         {32,  32}
#define UNBLOCKABLE_SPRITE_OFF {64,  32}
#define GRAB_SPRITE_OFF        {96,  32}
#define MILLENIUM_SPRITE_OFF   {128, 32}
#define DROP_SPRITE_OFF        {160, 32}
#define HEALTH_SPRITE_OFF      {192, 32}

	struct MaxAttributes {
		unsigned short drop;
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

		if (manager.hitBoxCount)
			printf("%x\n", manager.frameData.attackFlags.value);

		for (int i = 0; i < manager.hitBoxCount; i++)
			drawBox(manager.hitBoxes[i], manager.hitBoxesRotation[i], DxSokuColor::Red, DxSokuColor::Red * BOXES_ALPHA);
	}

	static void displayObjectFrameFlags(const SokuLib::ObjectManager &manager)
	{
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
		flagsSprite.setPosition(barPos);
		flagsSprite.draw();
		if (!reverse)
			barPos.x += 32;
	}

	static void displayPlayerBar(Vector2<int> textureOffset, Vector2<int> &barPos, Vector2<int> basePos, unsigned value, unsigned max, bool reverse)
	{
		if (barPos.x != basePos.x || barPos.y != basePos.y) {
			barPos.x = basePos.x;
			barPos.y += 32;
		}
		displayPlayerFrameFlag(textureOffset, barPos, reverse);
		rectangle.setSize({200 * value / max, 24});
		rectangle.setBorderColor(DxSokuColor::Black);
		rectangle.setFillColor(DxSokuColor::Red);
		if (reverse)
			rectangle.setPosition({static_cast<int>(200 * value / max - barPos.x), barPos.y + 4});
		else
			rectangle.setPosition({barPos.x, barPos.y + 4});
		rectangle.draw();
	}

	static unsigned getMaxSuperArmor(const SokuLib::CharacterManager &manager, SokuLib::Character character, MaxAttributes &maxAttributes)
	{
		if (character == SokuLib::CHARACTER_SUIKA) {
			if (manager.objectBase.action == SokuLib::ACTION_DEFAULT_SKILL1_B)
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
			else if (manager.objectBase.action == SokuLib::ACTION_DEFAULT_SKILL1_C)
				switch (manager.skillLevels[0]) {
				case 0:
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

	static void displayPlayerFrameFlags(const SokuLib::CharacterManager &manager, SokuLib::Character character, MaxAttributes &maxAttributes, Vector2<int> barPos, bool reverse)
	{
		auto basePos = barPos;
		auto &base = manager.objectBase;
		auto &flags = base.frameData.frameFlags;
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
		if (manager.noSuperArmor == 0 || flags.superArmor) {
			if (maxSuperArmor)
				displayPlayerBar(SUPERARMOR_SPRITE_OFF, basePos, barPos, maxSuperArmor - manager.objectBase.superarmorDamageTaken, maxSuperArmor, reverse);
			else
				displayPlayerFrameFlag(SUPERARMOR_SPRITE_OFF, barPos, reverse);
		}
		if (manager.dropInvulTimeLeft) {
			maxAttributes.drop = max(maxAttributes.drop, manager.dropInvulTimeLeft);
			displayPlayerBar(SUPERARMOR_SPRITE_OFF, basePos, barPos, manager.dropInvulTimeLeft, maxAttributes.drop, reverse);
		} else
			maxAttributes.drop = 0;
	}

	static void drawPlayerBoxes(const SokuLib::CharacterManager &manager, SokuLib::Character character, MaxAttributes &maxAttributes, Vector2<int> barPos, bool reverse)
	{
		drawCollisionBox(manager.objectBase);
		drawHurtBoxes(manager.objectBase);
		drawHitBoxes(manager.objectBase);
		drawPositionBox(manager.objectBase);
		displayPlayerFrameFlags(manager, character, maxAttributes, barPos, reverse);

		auto array = manager.objects.list.vector();

		for (const auto _elem : array) {
			auto elem = reinterpret_cast<const SokuLib::ProjectileManager *>(_elem);

			if ((elem->isActive && elem->objectBase.hitCount > 0) || elem->objectBase.frameData.attackFlags.value > 0) {
				drawCollisionBox(elem->objectBase);
				drawHurtBoxes(elem->objectBase);
				drawHitBoxes(elem->objectBase);
				drawPositionBox(elem->objectBase);
				displayObjectFrameFlags(elem->objectBase);
			}
		}
	}

	void drawAllBoxes()
	{
		auto &battle = SokuLib::getBattleMgr();

		drawPlayerBoxes(battle.leftCharacterManager,  SokuLib::leftChar,  allMaxAttributes.first,  {20, 70}, false);
		drawPlayerBoxes(battle.rightCharacterManager, SokuLib::rightChar, allMaxAttributes.second, {620, 70}, true);
	}
}