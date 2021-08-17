//
// Created by PinkySmile on 02/08/2021.
//

#include <Shlwapi.h>

#include <memory>
#include "../BattleAnimation.hpp"

char profilePath[1024];
static const std::vector<std::string> outroDialogs{
	"rWDWhy being a journalist is so hard?",
	"lWDI was still pretty soft you know?",
	"lCDNow talk!",
	"rCDI was just investigating the rumor that someone<br>had infiltrated the scarlet manor.",
	"lcDWell it's not anything new...",
	"rcDI meant neither Marisa nor me.",
	"lSDNow that is surprising!<br>Sakuya must know something..."
};

class Outro : public BattleAnimation {
private:
	std::unique_ptr<Dialog> _dialog;
	bool _pressed = false;
	bool _side = false;
	unsigned _ctr = 60;
	SokuLib::DrawUtils::RectangleShape _flashRect;

public:
	Outro()
	{
		this->_dialog = std::make_unique<SokuStand>(outroDialogs);
		this->_dialog->setHidden(false);

		this->_flashRect.setFillColor(SokuLib::DrawUtils::DxSokuColor{0, 0, 0, 0x0});
		this->_flashRect.setBorderColor(SokuLib::DrawUtils::DxSokuColor{0, 0, 0, 0});
		this->_flashRect.setSize({640, 480});
	}

	bool update() override
	{
		auto &battleMgr = SokuLib::getBattleMgr();
		auto color = this->_flashRect.getFillColor();

		battleMgr.leftCharacterManager.objectBase.doAnimation();
		if (!color.a && !this->_ctr && this->_side) {
			this->_dialog->update();
			if (this->_pressed) {
				if (!this->_dialog->onKeyPress())
					return false;
				this->_pressed = false;
			}
		}
		if (!this->_side) {
			color.a += 0x11;
			if (color.a == 0xFF) {
				this->_side = true;
				SokuLib::camera.translate.x = -320;
				SokuLib::camera.translate.y = 420;
				SokuLib::camera.backgroundTranslate.x = 640;
				SokuLib::camera.backgroundTranslate.y = 0;

				battleMgr.leftCharacterManager.objectBase.position.x = 400;
				battleMgr.leftCharacterManager.objectBase.position.y = 0;
				battleMgr.leftCharacterManager.objectBase.actionBlockId = 0;
				battleMgr.leftCharacterManager.objectBase.animationCounter = 0;
				battleMgr.leftCharacterManager.objectBase.animationSubFrame = 0;
				battleMgr.leftCharacterManager.objectBase.action = SokuLib::ACTION_IDLE;
				battleMgr.leftCharacterManager.objectBase.animate();

				battleMgr.rightCharacterManager.objectBase.position.x = 800;
				battleMgr.rightCharacterManager.objectBase.position.y = -50;
				battleMgr.rightCharacterManager.objectBase.animationSubFrame = 0;
				battleMgr.rightCharacterManager.objectBase.action = SokuLib::ACTION_KNOCKED_DOWN;
				battleMgr.rightCharacterManager.objectBase.animate();
				while (battleMgr.rightCharacterManager.objectBase.actionBlockId != 3 && battleMgr.rightCharacterManager.objectBase.animationCounter != 2)
					battleMgr.rightCharacterManager.objectBase.doAnimation();
			}
		} else if (this->_flashRect.getFillColor().a)
			color.a -= 0x11;
		else if (this->_ctr)
			this->_ctr--;
		this->_flashRect.setFillColor(color);
		return true;
	}

	void render() const override
	{
		this->_flashRect.draw();
		if (!this->_flashRect.getFillColor().a && !this->_ctr && this->_side)
			this->_dialog->render();
	}

	void onKeyPressed() override
	{
		if (!this->_flashRect.getFillColor().a && !this->_ctr && this->_side)
			this->_pressed = true;
	}
};

extern "C"
{
	__declspec(dllexport) BattleAnimation *getOutro()
	{
		return new Outro();
	}

	int APIENTRY DllMain(HMODULE hModule, DWORD fdwReason, LPVOID lpReserved)
	{
		if (fdwReason != DLL_PROCESS_ATTACH)
			return TRUE;
		GetModuleFileName(hModule, profilePath, 1024);
		PathRemoveFileSpec(profilePath);
		return TRUE;
	}
}












//struct Node {
//	Node *higher;
//	Node *head;
//	Node *lower;
//	int a;
//	char _0x10[0x5];
//	bool isBottom;
//};
//
//struct BinTree {
//	Node *array2;
//	Node *array;
//};
//
//Node *** __thiscall FUN_0043f2f0(BinTree *This, Node ***param_1, int *search)
//{
//	Node **ppiVar1;
//	Node *currentNode;
//	Node *nextNode;
//	Node ***ppiVar4;
//	Node **local_10;
//	Node *firstNode;
//	Node **local_8;
//	Node *local_4;
//
//	firstNode = This->array->higher;
//	if (!firstNode->lower->isBottom) {
//		currentNode = firstNode->head;
//		do {
//			if (currentNode->a < *search) {
//				nextNode = currentNode->lower;
//			}
//			else {
//				nextNode = currentNode->higher;
//				firstNode = currentNode;
//			}
//			currentNode = nextNode;
//		} while (nextNode->isBottom);
//	}
//	local_10 = &This->array2;
//	if (firstNode == This->array->higher || *search < firstNode->a) {
//		local_8 = &This->array2;
//		local_4 = This->array;
//		ppiVar4 = &local_8;
//	}
//	else {
//		ppiVar4 = &local_10;
//	}
//	ppiVar1 = ppiVar4[1];
//	param_1[0] = *ppiVar4;
//	param_1[1] = ppiVar1;
//	return param_1;
//}