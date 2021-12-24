//
// Created by PinkySmile on 02/08/2021.
//

#include <Shlwapi.h>
#include <memory>
#include "../BattleAnimation.hpp"

#define TRANSLATE_MAX 120
#define YUKARI_START_LOCATION 500
#define CAM_START_LOCATION (-20)
#define BG_START_LOCATION 700
#define HIT_STOP 7
#define PUSH_RATIO 2

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
	"LHDSo now will you tell me what is going on?",
	"RHDYou should come back to your first clue,<br>that will help you I'm sure.",
	"LcDCan't you just explain to me right now?",
	"REEI could,<br>but I believe it would be best to find out by yourself.",
	"LHEIt is fine,<br>your explanations are always too cryptic anyway.",
	"LWEWell I have someone else to ask questions thanks to you now,<br>so I must take my leave.",
	"R CIt seems she is going for the right source...",
	"R CSadly for her it is the flow,<br>and not the spring that matters this time...",
	"R CSadly for her it is the flow,<br>and not the spring that matters this time...<br>isn't it?",
	"R ASadly for her it is the flow,<br>and not the spring that matters this time...<br>isn't it?",
	"R CSadly for her it is the flow,<br>and not the spring that matters this time...<br>isn't it?",
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

		((void (__thiscall *)(int *, bool, SokuLib::PlayerInfo &))0x46da40)(this->buffer, false, this->_playerInfo);
		(*(void (__thiscall **)(SokuLib::CharacterManager *))(*(int *)this->_yukari + 0x44))(this->_yukari);
		*(SokuLib::CharacterManager **)&this->_yukari->objectBase.offset_0x168[8] = &battleMgr.rightCharacterManager;

		SokuLib::camera.scale = 1;
		//SokuLib::camera.translate.x = CAM_START_LOCATION;
		//SokuLib::camera.translate.y = 0;
		//SokuLib::camera.backgroundTranslate.x = BG_START_LOCATION;

		battleMgr.leftCharacterManager.objectBase.position.x = -500;
		battleMgr.leftCharacterManager.objectBase.action = SokuLib::ACTION_DEFAULT_SKILL1_B;

		battleMgr.rightCharacterManager.objectBase.position.y = 0;
		battleMgr.rightCharacterManager.objectBase.action = SokuLib::ACTION_IDLE;
		battleMgr.rightCharacterManager.objectBase.animate();

		this->_yukari->objectBase.position.x = 600;
		this->_yukari->objectBase.position.y = 800;
		this->_yukari->objectBase.action = SokuLib::ACTION_ALT2_SKILL4_C;
		this->_yukari->objectBase.animate();
		//this->_yukari->objectBase.animate2();
		this->_yukari->objectBase.renderInfos.color = SokuLib::Color::Black;
	}

	void stage0()
	{
		auto &battleMgr = SokuLib::getBattleMgr();
		auto color = this->_flashRect.getFillColor();

		battleMgr.leftCharacterManager.objectBase.doAnimation();
		battleMgr.rightCharacterManager.objectBase.doAnimation();
		if (!color.a && !this->_ctr && this->_side) {
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
		if (!this->_side) {
			color.a += 0x11;
			if (color.a == 0xFF) {
				this->_side = true;
				this->_init();
				while (battleMgr.rightCharacterManager.objectBase.actionBlockId != 3 && battleMgr.rightCharacterManager.objectBase.animationCounter != 2)
					battleMgr.rightCharacterManager.objectBase.doAnimation();
			}
		} else if (this->_flashRect.getFillColor().a)
			color.a -= 0x11;
		else if (this->_ctr)
			this->_ctr--;
		this->_flashRect.setFillColor(color);
	}

	void stage1()
	{
		updateSubObjects(*this->_yukari);
		if (this->_yukari->objects.list.size && this->_yukari->objects.list.head->next->val->animationCounter == 14) {
			this->_yukari->playSE(4);
			this->_yukari->playSE(5);
			this->_yukari->objectBase.position.y = 0;
			this->_currentStage++;
		}
	}

	void stage2()
	{
		this->_ctr ^= 1;
		if (this->_ctr)
			this->_yukari->objectBase.doAnimation();
		else if (this->_yukari->objectBase.actionBlockId == 0) {
			this->_yukari->objectBase.action = SokuLib::ACTION_IDLE;
			this->_yukari->objectBase.animate();
			this->_currentStage++;
		}
	}

	void stage3()
	{
		updateSubObjects(*this->_yukari);
		this->_yukari->objectBase.doAnimation();
	}

	std::vector<void (Outro::*)()> anims{
		&Outro::stage0,
		&Outro::stage1,
		&Outro::stage2,
		&Outro::stage3
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
		this->_flashRect.draw();
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