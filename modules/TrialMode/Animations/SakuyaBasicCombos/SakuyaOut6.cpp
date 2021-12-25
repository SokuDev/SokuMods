//
// Created by PinkySmile on 02/08/2021.
//

#include <Shlwapi.h>
#include <memory>
#include "../BattleAnimation.hpp"

#define lobj battleMgr.leftCharacterManager.objectBase
#define robj battleMgr.rightCharacterManager.objectBase
#define TRANSLATE_MAX 120
#define YUKARI_START_LOCATION 500

#define spawnSubObject(chr, id, x, y) do {        \
	float something[3];                        \
	                                           \
	memset(something, 0, sizeof(something));   \
	something[2] = 1;                          \
	((void (__thiscall *)(SokuLib::CharacterManager &, int, float, float, char, int, float *, int))0x46EB30)(chr, id, x, y, 1, 1, something, 3); \
} while (0)
#define updateSubObjects(chr) ((void (__thiscall *)(SokuLib::ObjListManager &))0x633ce0)((chr).objects)

char profilePath[1024];
static const std::vector<std::string> dialogs{
	"LHDNow... Will you tell me what is going on?",
	"RHDYou should come back to your first clue,<br>that will help you I'm sure.",
	"LcDCan't you just explain to me right now?",
	"REEI could, but I believe it would<br>be best to find out by yourself.",
	"LHEIt is fine, your explanations<br>are always too cryptic anyway.",
	"LWEWell. I have someone else to ask questions thanks<br>to you now, so I must take my leave.",
	"R CIt seems she is going for the right source...",
	"R hSadly for her, it is the flow,<br>not the spring that matters this time...",
	"R CIsn't it?",
	"   Here you are!"
};

class Outro : public BattleAnimation {
private:
	SokuLib::DrawUtils::RectangleShape _flashRect;
	SokuLib::DrawUtils::Sprite _stageBg;
	SokuLib::DrawUtils::Sprite _stageBottom;
	bool _pressed = false;
	bool _side = false;
	unsigned _ctr = 60;
	unsigned _currentStage = 0;
	std::unique_ptr<SokuStand> _dialog;
	bool _keyPressed = false;
	bool _stop = false;
	SokuLib::PlayerInfo _playerInfo;
	int buffer[0xA];
	SokuLib::CharacterManager *_yukari = nullptr;

	void _init()
	{
		puts("Init!");
		auto &battleMgr = SokuLib::getBattleMgr();

		this->_playerInfo.character = SokuLib::CHARACTER_YUKARI;
		this->_playerInfo.isRight = false;
		this->_playerInfo.palette = 0;
		this->_playerInfo.padding2 = 0;
		this->_playerInfo.deck = 0;
		this->_playerInfo.effectiveDeck.unknown1 = 0;
		this->_playerInfo.effectiveDeck.data = nullptr;
		this->_playerInfo.effectiveDeck.chunkSize = 0;
		this->_playerInfo.effectiveDeck.counter = 0;
		this->_playerInfo.effectiveDeck.size = 0;
		this->_playerInfo.keyManager = nullptr;

		puts("Create Yukari");
		((void (__thiscall *)(int *, bool, SokuLib::PlayerInfo &))0x46da40)(this->buffer, false, this->_playerInfo);
		puts("Init Yukari");
		(*(void (__thiscall **)(SokuLib::CharacterManager *))(*(int *)this->_yukari + 0x44))(this->_yukari);
		*(SokuLib::CharacterManager **)&this->_yukari->objectBase.offset_0x168[8] = &battleMgr.rightCharacterManager;

		SokuLib::camera.translate.x = -320;
		SokuLib::camera.translate.y = 420;
		SokuLib::camera.backgroundTranslate.x = 640;
		SokuLib::camera.backgroundTranslate.y = 0;

		battleMgr.leftCharacterManager.objectBase.position.x = 420;
		battleMgr.leftCharacterManager.objectBase.action = SokuLib::ACTION_IDLE;
		battleMgr.leftCharacterManager.objectBase.animate();

		battleMgr.rightCharacterManager.objectBase.position.x = 800;
		battleMgr.rightCharacterManager.objectBase.position.y = 0;
		battleMgr.rightCharacterManager.objectBase.action = SokuLib::ACTION_KNOCKED_DOWN_STATIC;
		battleMgr.rightCharacterManager.objectBase.animate();

		this->_yukari->objectBase.position.x = YUKARI_START_LOCATION;
		this->_yukari->objectBase.position.y = 800;
		this->_yukari->objectBase.action = SokuLib::ACTION_ALT2_SKILL4_C;
		this->_yukari->objectBase.animate();
		//this->_yukari->objectBase.animate2();
		this->_yukari->objectBase.renderInfos.color = SokuLib::Color::Black;
		puts("Init done");
	}

	void stage0()
	{
		auto &battleMgr = SokuLib::getBattleMgr();
		auto color = this->_flashRect.getFillColor();

		battleMgr.leftCharacterManager.objectBase.doAnimation();
		battleMgr.rightCharacterManager.objectBase.doAnimation();
		if (!color.a && !this->_ctr && this->_side) {
			this->_dialog->setHidden(false);
			this->_currentStage++;
		}
		if (!this->_side) {
			color.a += 0x11;
			if (color.a == 0xFF) {
				this->_side = true;
				this->_init();
			}
		} else if (this->_flashRect.getFillColor().a)
			color.a -= 0x11;
		else if (this->_ctr)
			this->_ctr--;
		this->_flashRect.setFillColor(color);
	}

	void stage1()
	{
		auto &battleMgr = SokuLib::getBattleMgr();

		lobj.doAnimation();
		if (this->_keyPressed && this->_dialog->getCurrentDialog() == 4) {
			this->_keyPressed = false;
			this->_dialog->setHidden(true);
			lobj.action = SokuLib::ACTION_DEFAULT_SKILL4_B;
			lobj.animate();
			this->_currentStage++;
		}
	}

	void stage2()
	{
		auto &battleMgr = SokuLib::getBattleMgr();

		lobj.doAnimation();
		updateSubObjects(battleMgr.leftCharacterManager);
		if (lobj.actionBlockId == 1 && lobj.animationCounter == 0 && lobj.animationSubFrame == 0 && lobj.frameCount == 0) {
			spawnSubObject(battleMgr.leftCharacterManager, 0x32D, lobj.position.x - 8, 115);
			spawnSubObject(battleMgr.leftCharacterManager, 0x32D, lobj.position.x - 8, 115);
			spawnSubObject(battleMgr.leftCharacterManager, 0x32D, lobj.position.x - 8, 115);
			spawnSubObject(battleMgr.leftCharacterManager, 0x32D, lobj.position.x - 8, 115);
			spawnSubObject(battleMgr.leftCharacterManager, 0x32D, lobj.position.x - 8, 115);
			spawnSubObject(battleMgr.leftCharacterManager, 0x32D, lobj.position.x - 8, 115);
			spawnSubObject(battleMgr.leftCharacterManager, 0x32D, lobj.position.x - 8, 115);

			auto objs = battleMgr.leftCharacterManager.objects.list.vector();
			auto start = battleMgr.leftCharacterManager.objects.list.size - 7;

			for (int i = 6; i; i--) {
				printf("To %i\n", i-1);
				while (objs[start + i]->actionBlockId != i - 1)
					objs[start + i]->animate2();
			}
		}
		if (lobj.actionBlockId == 1 && lobj.animationCounter == 1 && lobj.animationSubFrame == 1 && lobj.frameCount == 10) {
			spawnSubObject(battleMgr.leftCharacterManager, 0x32D, lobj.position.x, 0);
			spawnSubObject(battleMgr.leftCharacterManager, 0x32D, lobj.position.x, 0);

			auto objs = battleMgr.leftCharacterManager.objects.list.vector();
			auto start = battleMgr.leftCharacterManager.objects.list.size - 3;

			for (int i = 2; i; i--) {
				printf("To %i\n", i+5);
				objs[start + i]->renderInfos.xRotation = 0;
				objs[start + i]->renderInfos.yRotation = 0;
				objs[start + i]->renderInfos.zRotation = 0;
				while (objs[start + i]->actionBlockId != i + 5)
					objs[start + i]->animate2();
			}
			battleMgr.leftCharacterManager.playSE(13);
			lobj.position.x = -500;
			this->_currentStage++;
		}
		this->_keyPressed = false;
	}

	void stage3()
	{
		auto &battleMgr = SokuLib::getBattleMgr();

		updateSubObjects(battleMgr.leftCharacterManager);
		if (battleMgr.leftCharacterManager.objects.list.size == 0) {
			robj.action = SokuLib::ACTION_NEUTRAL_TECH;
			robj.animate();
			this->_currentStage++;
		}
	}

	void stage4()
	{
		auto &battleMgr = SokuLib::getBattleMgr();

		robj.doAnimation();
		if (this->_ctr)
			this->_ctr--;
		if (robj.frameCount == 0) {
			robj.action = SokuLib::ACTION_IDLE;
			robj.animate();
			this->_ctr = 30;
		}
		if (robj.action == SokuLib::ACTION_IDLE && this->_ctr == 0) {
			this->_dialog->setHidden(false);
			this->_dialog->onKeyPress();
			this->_currentStage++;
		}
	}

	void stage5()
	{
		SokuLib::getBattleMgr().rightCharacterManager.objectBase.doAnimation();
		if (this->_dialog->getCurrentDialog() == 1 && this->_keyPressed) {
			this->_keyPressed = false;
			this->_dialog->setHidden(true);

			// Yukari arrives init
			this->_yukari->objectBase.position.y = 250;
			this->_yukari->playSE(1);
			spawnSubObject(*this->_yukari, 0x33B, 200, 500);

			auto obj = this->_yukari->objects.list.vector().back();

			while (obj->actionBlockId != 3)
				obj->animate2();
			this->_yukari->objectBase.doAnimation();
			updateSubObjects(*this->_yukari);
			this->_yukari->objectBase.position.y = 800;
			this->_yukari->objectBase.animate2();
			this->_currentStage++;
		}
	}

	void stagey1()
	{
		auto &battleMgr = SokuLib::getBattleMgr();

		robj.doAnimation();
		if (!this->_ctr)
			updateSubObjects(*this->_yukari);
		if (this->_yukari->objects.list.size && this->_yukari->objects.list.head->next->val->animationCounter == 14) {
			if (this->_ctr == 0)
				this->_ctr = 30;
			else if (this->_ctr == 1) {
				this->_yukari->playSE(4);
				this->_yukari->playSE(5);
				this->_yukari->objectBase.position.y = 0;
				this->_currentStage++;
				this->_ctr = 0;
			} else
				this->_ctr--;
		}
	}

	void stagey2()
	{
		SokuLib::getBattleMgr().rightCharacterManager.objectBase.doAnimation();
		this->_ctr ^= 1;
		if (!this->_ctr)
			this->_yukari->objectBase.doAnimation();
		else if (this->_yukari->objectBase.actionBlockId == 0) {
			this->_yukari->objectBase.action = SokuLib::ACTION_IDLE;
			this->_yukari->objectBase.animate();
			this->_dialog->setHidden(false);
			this->_dialog->onKeyPress();
			this->_currentStage++;
		}
	}

	void stagey3()
	{
		SokuLib::getBattleMgr().rightCharacterManager.objectBase.doAnimation();
		updateSubObjects(*this->_yukari);
		this->_yukari->objectBase.doAnimation();
	}

	std::vector<void (Outro::*)()> anims{
		&Outro::stage0,
		&Outro::stage1,
		&Outro::stage2,
		&Outro::stage3,
		&Outro::stage4,
		&Outro::stage5,
		&Outro::stagey1,
		&Outro::stagey2,
		&Outro::stagey3
	};

public:
	Outro()
	{
		puts("Init intro.");

		this->_flashRect.setFillColor(SokuLib::DrawUtils::DxSokuColor{0, 0, 0, 0x0});
		this->_flashRect.setBorderColor(SokuLib::DrawUtils::DxSokuColor{0, 0, 0, 0});
		this->_flashRect.setSize({640, 480});

		this->_stageBg.texture.loadFromFile((profilePath + std::string("\\Lore6Outro.png")).c_str());
		this->_stageBg.setSize(this->_stageBg.texture.getSize());
		this->_stageBg.setPosition({
			static_cast<int>(320 - this->_stageBg.texture.getSize().x / 2),
			-50
		});
		this->_stageBg.rect.width  = this->_stageBg.texture.getSize().x;
		this->_stageBg.rect.height = this->_stageBg.texture.getSize().y;
		this->_stageBg.tint.a = 0;

		this->_stageBottom.texture.loadFromGame("data/scenario/effect/Stage6.png");
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

	~Outro()
	{
		if (this->_yukari) {
			(*(void (__thiscall **)(SokuLib::CharacterManager *, char))this->_yukari->objectBase.vtable)(this->_yukari, 0);
			SokuLib::Delete(this->_yukari);
		}
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
		if (this->_yukari) {
			// Display the CharacterManager subobjects
			((void (__thiscall *)(SokuLib::ObjListManager &, int))0x59be00)(this->_yukari->objects, -2);
			((void (__thiscall *)(SokuLib::ObjListManager &, int))0x59be00)(this->_yukari->objects, -1);

			// Display the CharacterManager
			((void (__thiscall *)(SokuLib::CharacterManager &))0x438d20)(*this->_yukari);

			((void (__thiscall *)(SokuLib::ObjListManager &, int))0x59be00)(this->_yukari->objects, 1);
			((void (__thiscall *)(SokuLib::ObjListManager &, int))0x59be00)(this->_yukari->objects, 2);

			// We redraw Remilia because Reisen's subobjects mess up the DirectX context and this will clean it up
			((void (__thiscall *)(SokuLib::CharacterManager &))0x438d20)(SokuLib::getBattleMgr().leftCharacterManager);
		}
		if (this->_ctr) {
			this->_stageBg.draw();
			this->_stageBottom.draw();
		}
		this->_dialog->render();
		this->_flashRect.draw();
	}

	void onKeyPressed() override
	{
		this->_keyPressed = true;
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