//
// Created by PinkySmile on 02/08/2021.
//

#include <Shlwapi.h>

#include <memory>
#include "../BattleAnimation.hpp"

char profilePath[1024];
static const std::vector<std::string> dialogs{
	"lc Another boring day. Maybe I'll go duel Reimu<br>tonight? If I destroy her shrine, she<br>might get motivated a bit.",
	//Clic
	"rh*That should make a good cover. As for the title...*",
	"lWOh my, an intruder. You are on time!",
	"rHDon't mind me. I'm not here to invade your<br>privacy or anything.<br>Just here to take a few picture in secret...",
	"rEWell so long for the secret part...",
	"lWPatchouli told me she needs a new tengu feather,<br>might as well help her out!",
	//one screaming crow and a feather later
	"lHI'm sure even the vegetative Patchouli<br>shall be grateful.",
	"rDMy beautiful feather...",
	"lCAnyway what were you doing here?",
	"rhI was investigating a rumor that..",
	"lASurely You are too tough to talk aren't you?",
	"rANo it's not a se...",
	"lhRoughing you up shall make you talk.",
	//Battle here
};
static const std::vector<std::string> outroDialogs{
	"rWDWhy being a journalist is so hard?",
	"lWI was still pretty soft you know?",
	"lCNow talk!",
	"rDI was just investigating the rumor that someone<br>had infiltrated the scarlet manor.",
	"lcWell it's not anything new...",
	"rDI meant neither marisa nor me.",
	"lSNow that is surprising!<br>Sakuya must know something..."
};

class Intro : public BattleAnimation {
private:
	SokuLib::DrawUtils::RectangleShape _flashRect;
	SokuLib::DrawUtils::Sprite _stageBg;
	SokuLib::DrawUtils::Sprite _stageBottom;
	unsigned _ctr = 240;
	unsigned _currentStage = 0;
	std::unique_ptr<Dialog> _dialog;
	bool _dialogHidden = true;
	bool _keyPressed = false;

	void stage0()
	{
		auto &battleMgr = SokuLib::getBattleMgr();

		if (this->_ctr == 240) {
			//int local_24;
			//int local_20 [3];
			//int local_14 [4];

			//local_24 = ((int (*)(const char *, int))0x409b50)("data/bgm/ta01.ogg",0x11);
			//printf("%i\n", local_24);
			//((void (__thiscall *)(int, int *, int *))0x430450)(0x899FA4, local_20, &local_24);
			((void (*)(const char *))0x43ff10)("data/bgm/ta01.ogg");
		}
		SokuLib::camera.translate.y = 820;
		SokuLib::camera.backgroundTranslate.y = -400;
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
			this->_currentStage++;
			battleMgr.leftCharacterManager.objectBase.position.x = 400;
			battleMgr.leftCharacterManager.objectBase.actionBlockId = 0;
			battleMgr.leftCharacterManager.objectBase.animationCounter = 0;
			battleMgr.leftCharacterManager.objectBase.animationSubFrame = 0;
			battleMgr.leftCharacterManager.objectBase.action = SokuLib::ACTION_WALK_FORWARD;
			battleMgr.leftCharacterManager.objectBase.animate();
			battleMgr.rightCharacterManager.objectBase.position.x = 1200;
			this->_ctr = 3;
		}
	}

	void stage1()
	{
		auto &battleMgr = SokuLib::getBattleMgr();

		battleMgr.leftCharacterManager.objectBase.doAnimation();
		if (SokuLib::camera.backgroundTranslate.y) {
			SokuLib::camera.backgroundTranslate.y++;
			SokuLib::camera.translate.y--;
		}
		battleMgr.leftCharacterManager.objectBase.position.x += 5 * battleMgr.leftCharacterManager.objectBase.direction;
		if (battleMgr.leftCharacterManager.objectBase.direction == SokuLib::RIGHT && battleMgr.leftCharacterManager.objectBase.position.x > 800) {
			battleMgr.leftCharacterManager.objectBase.direction = SokuLib::LEFT;
		} else if (battleMgr.leftCharacterManager.objectBase.direction == SokuLib::LEFT && battleMgr.leftCharacterManager.objectBase.position.x < 400) {
			battleMgr.leftCharacterManager.objectBase.direction = SokuLib::RIGHT;
			this->_ctr--;
		}
		if (!this->_ctr) {
			battleMgr.leftCharacterManager.objectBase.action = SokuLib::ACTION_IDLE;
			battleMgr.leftCharacterManager.objectBase.animate();
			this->_currentStage++;
			this->_dialogHidden = false;
			this->_dialog->onKeyPress();
		}
	}

	void stage2()
	{
		auto &battleMgr = SokuLib::getBattleMgr();

		battleMgr.leftCharacterManager.objectBase.doAnimation();
		if (this->_dialog->getCurrentDialog() == dialogs.size() - 2) {
			this->_currentStage++;
			this->_dialogHidden = true;
			battleMgr.rightCharacterManager.objectBase.position.x = 800;
			battleMgr.rightCharacterManager.objectBase.actionBlockId = 0;
			battleMgr.rightCharacterManager.objectBase.animationCounter = 0;
			battleMgr.rightCharacterManager.objectBase.animationSubFrame = 0;
			battleMgr.rightCharacterManager.objectBase.action = SokuLib::ACTION_IDLE;
			battleMgr.rightCharacterManager.objectBase.animate();
			battleMgr.leftCharacterManager.objectBase.actionBlockId = 0;
			battleMgr.leftCharacterManager.objectBase.animationCounter = 0;
			battleMgr.leftCharacterManager.objectBase.animationSubFrame = 0;
			battleMgr.leftCharacterManager.objectBase.action = SokuLib::ACTION_STAND_GROUND_HIT_HUGE_HITSTUN;
			battleMgr.leftCharacterManager.objectBase.animate();
			this->_flashRect.setFillColor(SokuLib::DrawUtils::DxSokuColor::White);
			SokuLib::playSEWaveBuffer(12);
		}
	}

	void stage3()
	{
		auto &battleMgr = SokuLib::getBattleMgr();
		auto color = this->_flashRect.getFillColor();

		battleMgr.rightCharacterManager.objectBase.doAnimation();
		if (color.a > 1) {
			color.a -= 0x2;
			this->_flashRect.setFillColor(color);
		} else {
			this->_flashRect.setFillColor(SokuLib::DrawUtils::DxSokuColor::Transparent);
			this->_currentStage++;
			battleMgr.leftCharacterManager.objectBase.actionBlockId = 0;
			battleMgr.leftCharacterManager.objectBase.animationCounter = 0;
			battleMgr.leftCharacterManager.objectBase.animationSubFrame = 0;
			battleMgr.leftCharacterManager.objectBase.action = SokuLib::ACTION_IDLE;
			battleMgr.leftCharacterManager.objectBase.animate();
			this->_ctr = 12;
		}
	}

	void stage4()
	{
		auto &battleMgr = SokuLib::getBattleMgr();

		battleMgr.leftCharacterManager.objectBase.doAnimation();
		battleMgr.rightCharacterManager.objectBase.doAnimation();
		if (this->_ctr)
			this->_ctr--;
		else
			this->_dialogHidden = false;
		if (this->_dialog->getCurrentDialog() == dialogs.size() - 14)
			this->_currentStage++;
	}

	std::vector<void (Intro::*)()> anims{
		&Intro::stage0,
		&Intro::stage1,
		&Intro::stage2,
		&Intro::stage3,
		&Intro::stage4
	};

public:
	Intro()
	{
		puts("Init intro.");

		this->_stageBg.texture.loadFromFile((profilePath + std::string("\\bnbmid1intro.png")).c_str());
		this->_stageBg.setSize(this->_stageBg.texture.getSize());
		this->_stageBg.setPosition({
			static_cast<int>(320 - this->_stageBg.texture.getSize().x / 2),
			-50
		});
		this->_stageBg.rect.width  = this->_stageBg.texture.getSize().x;
		this->_stageBg.rect.height = this->_stageBg.texture.getSize().y;
		this->_stageBg.tint.a = 0;

		this->_stageBottom.texture.loadFromGame("data/scenario/effect/Stage1.png");
		this->_stageBottom.setSize(this->_stageBottom.texture.getSize());
		this->_stageBottom.setPosition({
			static_cast<int>(320 - this->_stageBg.texture.getSize().x / 2),
			static_cast<int>(-50 + this->_stageBg.texture.getSize().y)
		});
		this->_stageBottom.rect.width  = this->_stageBottom.texture.getSize().x;
		this->_stageBottom.rect.height = this->_stageBottom.texture.getSize().y;
		this->_stageBottom.tint.a = 0;

		this->_flashRect.setSize({640, 480});
		this->_flashRect.setBorderColor(SokuLib::DrawUtils::DxSokuColor::Transparent);
		this->_flashRect.setFillColor(SokuLib::DrawUtils::DxSokuColor::Transparent);

		this->_dialog = std::make_unique<SokuStand>(dialogs);
	}

	bool update() override
	{
		if (!this->_dialogHidden) {
			this->_dialog->update();
			if (this->_keyPressed) {
				if (!this->_dialog->onKeyPress()) {
					((void (*)(const char *))0x43ff10)("data/bgm/st17.ogg");
					return false;
				}
				this->_keyPressed = false;
			}
		}
		if (this->_currentStage >= this->anims.size())
			return false;
		(this->*this->anims[this->_currentStage])();
		return this->_currentStage < this->anims.size();
	}

	void render() const override
	{
		if (this->_ctr) {
			this->_stageBg.draw();
			this->_stageBottom.draw();
		}
		this->_flashRect.draw();
		if (!this->_dialogHidden)
			this->_dialog->render();
	}

	void onKeyPressed() override
	{
		if (!this->_dialogHidden)
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
		this->_dialog->onKeyPress();

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