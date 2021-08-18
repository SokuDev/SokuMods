//
// Created by PinkySmile on 02/08/2021.
//

#include <Shlwapi.h>

#include <memory>
#include "../BattleAnimation.hpp"

char profilePath[1024];
static const std::vector<std::string> dialogs{
	"lh Sakuya?",
	"rhHYes, mistress?",
	"lCHDo you know something about a mysterious intruder?<br>One that has been hidding in the mansion.",
	"rCSMysterious? No not that I know...<br>Still, considering the state of our \"gate keeper\"<br>anyone could have come...",
	"lASYou too are in charge of security you know?",
	"lhSIt seems I need to teach you how to do it."
	//Battle here
};
static const std::vector<std::string> outroDialogs{
	"lHDNow, you know how to deal with uninvited guests.",
	"rHDI'll be sure to remember...",
	"lhDOh, by the way could you give that to Patchouli?",
	"rhDA feather? I will give it to her if I see her.",
	"lhDGood, I'm sure she'll find some use to it."
};

class Intro : public BattleAnimation {
private:
	SokuLib::DrawUtils::Sprite _stageBg;
	SokuLib::DrawUtils::Sprite _stageBottom;
	unsigned _ctr = 240;
	unsigned _currentStage = 0;
	std::unique_ptr<SokuStand> _dialog;
	bool _keyPressed = false;
	bool _stop = false;

	void stage0()
	{
		auto &battleMgr = SokuLib::getBattleMgr();

		if (this->_ctr == 240) {
			battleMgr.leftCharacterManager.objectBase.actionBlockId = 0;
			battleMgr.leftCharacterManager.objectBase.animationCounter = 0;
			battleMgr.leftCharacterManager.objectBase.animationSubFrame = 0;
			battleMgr.leftCharacterManager.objectBase.action = SokuLib::ACTION_IDLE;
			battleMgr.leftCharacterManager.objectBase.animate();
			battleMgr.rightCharacterManager.objectBase.position.x = 1200;
			((void (*)(const char *))0x43ff10)("data/bgm/ta00.ogg");
		}

		battleMgr.leftCharacterManager.objectBase.doAnimation();
		if (this->_ctr < 60) {
			if (this->_stageBg.tint.a)
				this->_stageBg.tint.a -= 0xF;
			if (this->_stageBottom.tint.a)
				this->_stageBottom.tint.a -= 0xF;
		} else {
			if (this->_stageBg.tint.a != 0xFF)
				this->_stageBg.tint.a += 0xF;
			if (this->_stageBottom.tint.a != 0xFF)
				this->_stageBottom.tint.a += 0xF;
		}
		if (this->_ctr % 2)
			this->_stageBottom.setPosition(this->_stageBottom.getPosition() + SokuLib::Vector2i{1, 0});
		this->_ctr--;
		if (!this->_ctr) {
			this->_dialog->setHidden(false);
			this->_currentStage++;
		}
	}

	void stage1()
	{
		auto &battleMgr = SokuLib::getBattleMgr();

		battleMgr.leftCharacterManager.objectBase.doAnimation();
		battleMgr.rightCharacterManager.objectBase.doAnimation();
		if (this->_keyPressed) {
			battleMgr.rightCharacterManager.objectBase.actionBlockId = 0;
			battleMgr.rightCharacterManager.objectBase.animationCounter = 0;
			battleMgr.rightCharacterManager.objectBase.animationSubFrame = 0;
			battleMgr.rightCharacterManager.objectBase.action = SokuLib::ACTION_FORWARD_JUMP;
			battleMgr.rightCharacterManager.objectBase.animate();
			battleMgr.rightCharacterManager.objectBase.speed.x = -10;
			battleMgr.rightCharacterManager.objectBase.speed.y = 15;
			battleMgr.rightCharacterManager.objectBase.position.x = 1240;
			this->_currentStage++;
			this->_keyPressed = false;
		}
	}

	void stage2()
	{
		auto &battleMgr = SokuLib::getBattleMgr();

		this->_keyPressed = false;
		battleMgr.leftCharacterManager.objectBase.doAnimation();
		battleMgr.rightCharacterManager.objectBase.doAnimation();
		battleMgr.rightCharacterManager.objectBase.position.x += battleMgr.rightCharacterManager.objectBase.speed.x;
		battleMgr.rightCharacterManager.objectBase.position.y += battleMgr.rightCharacterManager.objectBase.speed.y;
		battleMgr.rightCharacterManager.objectBase.speed.y -= 0.7;
		if (battleMgr.rightCharacterManager.objectBase.position.y <= 0 && battleMgr.rightCharacterManager.objectBase.speed.y < 0) {
			battleMgr.rightCharacterManager.objectBase.position.y = 0;
			battleMgr.rightCharacterManager.objectBase.speed.x = 0;
			battleMgr.rightCharacterManager.objectBase.speed.y = 0;
			SokuLib::playSEWaveBuffer(30);
			battleMgr.rightCharacterManager.objectBase.actionBlockId = 0;
			battleMgr.rightCharacterManager.objectBase.animationCounter = 0;
			battleMgr.rightCharacterManager.objectBase.animationSubFrame = 0;
			battleMgr.rightCharacterManager.objectBase.action = SokuLib::ACTION_LANDING;
			battleMgr.rightCharacterManager.objectBase.animate();
			this->_currentStage++;
			this->_dialog->onKeyPress();
		}
	}

	void stage3()
	{
		auto &battleMgr = SokuLib::getBattleMgr();

		battleMgr.leftCharacterManager.objectBase.doAnimation();
		battleMgr.rightCharacterManager.objectBase.doAnimation();
		if (
			battleMgr.rightCharacterManager.objectBase.action == SokuLib::ACTION_LANDING &&
			battleMgr.rightCharacterManager.objectBase.actionBlockId == 0 &&
			battleMgr.rightCharacterManager.objectBase.animationCounter == 0 &&
			battleMgr.rightCharacterManager.objectBase.animationSubFrame == 0
		) {
			battleMgr.rightCharacterManager.objectBase.actionBlockId = 0;
			battleMgr.rightCharacterManager.objectBase.animationCounter = 0;
			battleMgr.rightCharacterManager.objectBase.animationSubFrame = 0;
			battleMgr.rightCharacterManager.objectBase.action = SokuLib::ACTION_IDLE;
			battleMgr.rightCharacterManager.objectBase.animate();
		}
	}

	std::vector<void (Intro::*)()> anims{
		&Intro::stage0,
		&Intro::stage1,
		&Intro::stage2,
		&Intro::stage3
	};

public:
	Intro()
	{
		puts("Init intro.");

		this->_stageBg.texture.loadFromFile((profilePath + std::string("\\bnbmid2intro.png")).c_str());
		this->_stageBg.setSize(this->_stageBg.texture.getSize());
		this->_stageBg.setPosition({
			static_cast<int>(320 - this->_stageBg.texture.getSize().x / 2),
			-50
		});
		this->_stageBg.rect.width  = this->_stageBg.texture.getSize().x;
		this->_stageBg.rect.height = this->_stageBg.texture.getSize().y;
		this->_stageBg.tint.a = 0;

		this->_stageBottom.texture.loadFromGame("data/scenario/effect/Stage2.png");
		this->_stageBottom.setSize(this->_stageBottom.texture.getSize());
		this->_stageBottom.setPosition({
			static_cast<int>(320 - this->_stageBg.texture.getSize().x / 2),
			static_cast<int>(-50 + this->_stageBg.texture.getSize().y)
		});
		this->_stageBottom.rect.width  = this->_stageBottom.texture.getSize().x;
		this->_stageBottom.rect.height = this->_stageBottom.texture.getSize().y;
		this->_stageBottom.tint.a = 0;

		this->_dialog = std::make_unique<SokuStand>(dialogs);
	}

	bool update() override
	{
		this->_dialog->update();
		if (this->_currentStage < this->anims.size())
			(this->*this->anims[this->_currentStage])();
		if (this->_keyPressed) {
			this->_stop |= !this->_dialog->onKeyPress();
			puts(this->_stop ? "Stop" : "Not stop");
			if (this->_stop)
				this->_dialog->setHidden(true);
			this->_keyPressed = false;
		}
		return !this->_stop || !this->_dialog->isAnimationFinished();
	}

	void render() const override
	{
		if (this->_ctr) {
			this->_stageBg.draw();
			this->_stageBottom.draw();
		}
		this->_dialog->render();
	}

	void onKeyPressed() override
	{
		this->_keyPressed = true;
	}
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
		battleMgr.rightCharacterManager.objectBase.doAnimation();
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

				battleMgr.leftCharacterManager.objectBase.position.x = 450;
				battleMgr.leftCharacterManager.objectBase.position.y = 0;
				battleMgr.leftCharacterManager.objectBase.actionBlockId = 0;
				battleMgr.leftCharacterManager.objectBase.animationCounter = 0;
				battleMgr.leftCharacterManager.objectBase.animationSubFrame = 0;
				battleMgr.leftCharacterManager.objectBase.action = SokuLib::ACTION_IDLE;
				battleMgr.leftCharacterManager.objectBase.animate();

				battleMgr.rightCharacterManager.objectBase.position.x = 800;
				battleMgr.rightCharacterManager.objectBase.position.y = 0;
				battleMgr.rightCharacterManager.objectBase.animationSubFrame = 0;
				battleMgr.rightCharacterManager.objectBase.action = SokuLib::ACTION_IDLE;
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
	__declspec(dllexport) BattleAnimation *getIntro()
	{
		return new Intro();
	}

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