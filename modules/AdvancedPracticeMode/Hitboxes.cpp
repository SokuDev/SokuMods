//
// Created by PinkySmile on 10/03/2021.
//

#include <SokuLib.hpp>
#include <cmath>
#include "DrawUtils.hpp"
#include "Hitboxes.hpp"

namespace Practice
{
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
		rectangle.setFillColor(DxSokuColor::Yellow * 0.5);
		rectangle.setBorderColor(DxSokuColor::Yellow);
		rectangle.draw();
	}

	static void drawHurtBoxes(const SokuLib::ObjectManager &manager)
	{
		if (manager.hurtBoxCount > 5)
			return;
		for (int i = 0; i < manager.hurtBoxCount; i++)
			drawBox(manager.hurtBoxes[i], manager.hurtBoxesRotation[i], DxSokuColor::Green, DxSokuColor::Green * 0.5);
	}

	static void drawHitBoxes(const SokuLib::ObjectManager &manager)
	{
		if (manager.hitBoxCount > 5)
			return;
		for (int i = 0; i < manager.hitBoxCount && i < 5; i++)
			drawBox(manager.hitBoxes[i], manager.hitBoxesRotation[i], DxSokuColor::Red, DxSokuColor::Red * 0.5);
	}

	static void drawPlayerBoxes(const SokuLib::CharacterManager &manager)
	{
		drawCollisionBox(manager.objectBase);
		drawHurtBoxes(manager.objectBase);
		drawHitBoxes(manager.objectBase);

		auto array = manager.objects.list.vector();

		for (const auto elem : array) {
			//if ((elem->isActive && elem->hitCount > 0) || elem->frameData.attackFlags.value > 0) {
				drawHurtBoxes(*elem);
				drawHitBoxes(*elem);
			//}
		}
	}

	void drawAllBoxes()
	{
		auto &battle = SokuLib::getBattleMgr();

		drawPlayerBoxes(battle.leftCharacterManager);
		drawPlayerBoxes(battle.rightCharacterManager);
	}
}