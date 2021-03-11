//
// Created by PinkySmile on 10/03/2021.
//

#include <SokuLib.hpp>
#include <cmath>
#include "DrawUtils.hpp"
#include "Hitboxes.hpp"

namespace Practice
{
#define BOXES_ALPHA 0.5

	static RectangleShape rectangle;

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
		FloatRect rect{};

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

		std::pair<DxSokuColor, DxSokuColor> colors = {DxSokuColor::Green, DxSokuColor::Green * BOXES_ALPHA};

		if (manager.frameData.frameFlags.chOnHit)
			colors.second = DxSokuColor::Cyan * BOXES_ALPHA;
		if (manager.frameData.frameFlags.meleeInvincible)
			colors.second = DxSokuColor::Transparent;
		if (manager.frameData.frameFlags.grabInvincible)
			colors.second = {0xA0, 0xFF, 0xA0, 0xFF};
		if (manager.frameData.frameFlags.graze)
			colors.second *= 0.5;

		for (int i = 0; i < manager.hurtBoxCount; i++)
			drawBox(manager.hurtBoxes[i], manager.hurtBoxesRotation[i], colors.first, colors.second);
	}

	static void drawHitBoxes(const SokuLib::ObjectManager &manager)
	{
		if (manager.hitBoxCount > 5)
			return;

		std::pair<DxSokuColor, DxSokuColor> colors = {DxSokuColor::Red, DxSokuColor::Red * BOXES_ALPHA};

		if (manager.frameData.attackFlags.grazable)
			colors.second = DxSokuColor::Magenta * BOXES_ALPHA;
		if (!manager.frameData.attackFlags.airBlockable)
			colors.second = DxSokuColor::Yellow * BOXES_ALPHA;

		if (manager.hitBoxCount)
			printf("%x\n", manager.frameData.attackFlags.value);

		for (int i = 0; i < manager.hitBoxCount; i++)
			drawBox(manager.hitBoxes[i], manager.hitBoxesRotation[i], colors.first, colors.second);
	}

	static void drawPlayerBoxes(const SokuLib::CharacterManager &manager)
	{
		drawCollisionBox(manager.objectBase);
		drawHurtBoxes(manager.objectBase);
		drawHitBoxes(manager.objectBase);
		drawPositionBox(manager.objectBase);

		auto array = manager.objects.list.vector();

		for (const auto elem : array) {
			if ((elem->isActive && elem->hitCount > 0) || elem->frameData.attackFlags.value > 0) {
				drawCollisionBox(*elem);
				drawHurtBoxes(*elem);
				drawHitBoxes(*elem);
				drawPositionBox(*elem);
			}
		}
	}

	void drawAllBoxes()
	{
		auto &battle = SokuLib::getBattleMgr();

		drawPlayerBoxes(battle.leftCharacterManager);
		drawPlayerBoxes(battle.rightCharacterManager);
	}
}