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
		printf("Rotation is at %p\n", rotation);
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
		printf("%f:%f %f:%f -> %i:%i %u:%u\n", rect.x1, rect.y1, rect.x2, rect.y2, rectangle.getPosition().x, rectangle.getPosition().y, rectangle.getSize().x, rectangle.getSize().y);
		if (rectangle.getSize().y >= 480)
			system("pause");
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

			typedef struct {
				int alloc;
				void *head;
				int size;
			} OBJ_LIST;

			typedef struct {
				void *next;
				void *prev;
				void *val;
			} NODE;

			OBJ_LIST objProjList;
			NODE objProjIter;

			char *objProjMgr = *(char **)((char*)&manager + 0x6F8);

			//(std::list<CharacterObject *>)
			objProjMgr = objProjMgr + 4;
			/*if(!ReadProcessMemory(ph, objProjMgr + ADDR_OBJPROJOFS, &objProjList, sizeof(objProjList))) {
				return;
			}*/
			memcpy_s(&objProjList, sizeof(objProjList), objProjMgr + 0x54, sizeof(objProjList));
			/*if(!ReadProcessMemory(ph, objProjList.head, &objProjIter, sizeof(objProjIter))) {
				return;
			}*/
			memcpy_s(&objProjIter, sizeof(objProjIter), objProjList.head, sizeof(objProjIter));


			printf("Check elem %p\n", elem);
			if ((elem->isActive && elem->hitCount > 0) || elem->frameData.attackFlags.value > 0) {
				printf("Showing object %p\n", elem);
				drawHurtBoxes(*elem);
				printf("Showing object %p (2)\n", elem);
				drawHitBoxes(*elem);
				printf("Shown object %p\n", elem);
			}
		}
	}

	void drawAllBoxes()
	{
		auto &battle = SokuLib::getBattleMgr();

		puts("Left character");
		drawPlayerBoxes(battle.leftCharacterManager);
		puts("Right character");
		drawPlayerBoxes(battle.rightCharacterManager);
	}
}