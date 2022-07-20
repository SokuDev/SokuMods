//
// Created by PinkySmile on 01/08/2021.
// Edited by PC_volt
//

#include <SokuLib.hpp>
#include <dinput.h>
#include <fstream>
static void (SokuLib::BattleManager::*og_BattleManagerOnRender)();
static int (SokuLib::BattleManager::*og_BattleManagerOnProcess)();
static int (SokuLib::Loading::*og_LoadingOnProcess)();
static int (SokuLib::LoadingClient::*og_LoadingCLOnProcess)();
static int (SokuLib::LoadingServer::*og_LoadingSVOnProcess)();
static int (SokuLib::CharacterManager::*original_onHit)(int param);
#ifndef _DEBUG
#define puts(...)
#define printf(...)
#endif

static bool loaded = false;
static SokuLib::DrawUtils::Sprite bePunish;
static SokuLib::DrawUtils::Sprite jumpPunish;
static SokuLib::DrawUtils::Sprite dashPunish;
static SokuLib::DrawUtils::Sprite flightPunish;
static SokuLib::DrawUtils::Sprite hardlandPunish;
static SokuLib::DrawUtils::Sprite attackPunish;
static SokuLib::DrawUtils::Sprite punish;
static SokuLib::DrawUtils::Sprite crossup;
static SokuLib::SWRFont font;

struct ChrState {
	SokuLib::DrawUtils::Sprite *punishText;
	unsigned crossupCounter = 30;
	unsigned displayCounter = 1000;
	float x = 0;
};

struct State {
	ChrState P1;
	ChrState P2;
	unsigned lastFrame = 0;
} state;

std::list<State> rollbackBuffer;

SokuLib::DrawUtils::Sprite* associatePunishSprite(SokuLib::CharacterManager &character, unsigned ctr)
{
	if (character.objectBase.frameData->frameFlags.guardAvailable) {
		if (ctr >= 30)
			return nullptr;
		if (!character.keyManager)
			return nullptr;
		if (!character.keyManager->keymapManager)
			return nullptr;
		if (std::copysign(1, character.keyManager->keymapManager->input.horizontalAxis) != character.objectBase.direction)
			return nullptr;
		// if the character is holding the same direction as they are facing

		return &crossup;
	}
	if (
		(character.objectBase.action >= SokuLib::ACTION_NEUTRAL_HIGH_JUMP && character.objectBase.action <= SokuLib::ACTION_FORWARD_HIGH_JUMP_FROM_GROUND_DASH) ||
		(character.objectBase.action >= SokuLib::ACTION_NEUTRAL_JUMP && character.objectBase.action <= SokuLib::ACTION_BACKWARD_JUMP)
	)
		return &jumpPunish;
	else if (character.objectBase.action >= SokuLib::ACTION_FORWARD_DASH && character.objectBase.action <= SokuLib::ACTION_LILYPAD_BACKDASH)
		return &dashPunish;
	else if (character.objectBase.action == SokuLib::ACTION_HARDLAND)
		return &hardlandPunish;
	else if (character.objectBase.action >= SokuLib::ACTION_FLY && character.objectBase.action <= SokuLib::ACTION_SUWAKO_j1D_j3D)
		return &flightPunish;
	else if (character.objectBase.action >= SokuLib::ACTION_BE2 && character.objectBase.action <= SokuLib::ACTION_jBE6)
		return &bePunish;
	else if (character.objectBase.action >= SokuLib::ACTION_5A)
		return &attackPunish;
	else
		return &punish;
}

void handleHit(SokuLib::CharacterManager &chr, ChrState &s)
{
	auto ptr = associatePunishSprite(chr, s.crossupCounter);

	if (!ptr)
		return;
	s.punishText = ptr;
	s.displayCounter = 0;
}

int __fastcall isHit(SokuLib::CharacterManager &character, int, int param)
{
	if (character.objectBase.action < SokuLib::ACTION_STAND_GROUND_HIT_SMALL_HITSTUN || character.objectBase.action > SokuLib::ACTION_NEUTRAL_TECH)
	{
		if (&character == &SokuLib::getBattleMgr().leftCharacterManager)
			handleHit(character, state.P1);
		else if (&character == &SokuLib::getBattleMgr().rightCharacterManager)
			handleHit(character, state.P2);
	}

	return (character.*original_onHit)(param);
}

void createPunishTextSprite(SokuLib::DrawUtils::Sprite &punishText, const char *punishMessage)
{
	SokuLib::Vector2<int> realSize;

	if (!punishText.texture.createFromText(punishMessage, font, {0x1000, 50}, &realSize)) {
		puts("Create from text failed");
		return;
	}
	punishText.setSize(realSize.to<unsigned>());
	punishText.rect.width = realSize.x;
	punishText.rect.height = realSize.y;
}

void createSprites()
{
	if (loaded)
		return;

	loaded = true;
	SokuLib::FontDescription desc;

	desc.r1 = 255;
	desc.r2 = 255;
	desc.g1 = 255;
	desc.g2 = 255;
	desc.b1 = 255;
	desc.b2 = 255;
	desc.height = 24;
	desc.weight = FW_BOLD;
	desc.italic = 0;
	desc.shadow = 4;
	desc.bufferSize = 1000000;
	desc.charSpaceX = 0;
	desc.charSpaceY = 0;
	desc.offsetX = 0;
	desc.offsetY = 0;
	desc.useOffset = 0;
	strcpy(desc.faceName, "MonoSpatialModSWR");
	font.create();
	font.setIndirect(desc);

	createPunishTextSprite(jumpPunish, "Jump Punish!");
	createPunishTextSprite(bePunish, "BE Punish!");
	createPunishTextSprite(flightPunish, "Flight Punish!");
	createPunishTextSprite(hardlandPunish, "Hardland Punish!");
	createPunishTextSprite(dashPunish, "Dash Punish!");
	createPunishTextSprite(attackPunish, "Attack Punish!");
	createPunishTextSprite(crossup, "Crossup!");
	createPunishTextSprite(punish, "Punish!");
	state = State();
}

void unloadSprites()
{
	if (!loaded)
		return;

	loaded = false;
	jumpPunish.texture.destroy();
	bePunish.texture.destroy();
	dashPunish.texture.destroy();
	flightPunish.texture.destroy();
	hardlandPunish.texture.destroy();
	attackPunish.texture.destroy();
	crossup.texture.destroy();
	punish.texture.destroy();
	font.destruct();
}

void updateChrState(ChrState &s, ChrState &op, ChrState *oldState, ChrState *opOld)
{
	if (s.displayCounter < 1000)
		s.displayCounter++;
	if (s.crossupCounter < 30)
		s.crossupCounter++;
	if (!oldState)
		return;
	if ((s.x <= op.x) != (oldState->x <= opOld->x))
		s.crossupCounter = 0;
}

void rollbackState(int count)
{
	if (count > 0)
		return;
	//printf("Rolling back %i frames\n", count);
	while (count < 0) {
		if (rollbackBuffer.empty())
			break;
		rollbackBuffer.pop_back();
		count++;
	}
	state = rollbackBuffer.empty() ? State() : rollbackBuffer.back();
}

void advanceState(int newFrame, int oldFrame)
{
	int count = newFrame - oldFrame;
	//printf("Advancing %i frames (from frame %i to frame %i)\n", count, newFrame, oldFrame);
	if (!count)
		return;
	rollbackState(count);

	auto &mgr = SokuLib::getBattleMgr();
	auto old = rollbackBuffer.empty() ? nullptr : &rollbackBuffer.back();

	state.P1.x = mgr.leftCharacterManager.objectBase.position.x;
	state.P2.x = mgr.rightCharacterManager.objectBase.position.x;
	updateChrState(state.P1, state.P2, old ? &old->P1 : nullptr, old ? &old->P2 : nullptr);
	updateChrState(state.P2, state.P1, old ? &old->P2 : nullptr, old ? &old->P1 : nullptr);
}

int __fastcall BattleOnProcess(SokuLib::BattleManager *This)
{
	int ret = (This->*og_BattleManagerOnProcess)();
	auto &mgr = SokuLib::getBattleMgr();

	advanceState(mgr.frameCount, state.lastFrame);
	state.lastFrame = mgr.frameCount;
	rollbackBuffer.push_back(state);
	while (rollbackBuffer.size() > 10)
		rollbackBuffer.pop_front();
	return ret;
}

int __fastcall LoadingOnProcess(SokuLib::Loading *This)
{
	int ret = (This->*og_LoadingOnProcess)();

	if (state.lastFrame)
		unloadSprites();
	createSprites();
	return ret;
}

int __fastcall LoadingCLOnProcess(SokuLib::LoadingClient *This)
{
	int ret = (This->*og_LoadingCLOnProcess)();

	if (state.lastFrame)
		unloadSprites();
	createSprites();
	return ret;
}

int __fastcall LoadingSVOnProcess(SokuLib::LoadingServer *This)
{
	int ret = (This->*og_LoadingSVOnProcess)();

	if (state.lastFrame)
		unloadSprites();
	createSprites();
	return ret;
}

void renderChrState(ChrState &s, int pos)
{
	if (s.displayCounter >= 1000)
		return;

	s.punishText->setPosition({pos, 65});
	s.punishText->tint.a = min(255, max(0, (240 - static_cast<int>(s.displayCounter * 4) + 255)));
	s.punishText->draw();
}

void __fastcall BattleOnRender(SokuLib::BattleManager *This)
{
	(This->*og_BattleManagerOnRender)();
	//if (SokuLib::mainMode == SokuLib::BATTLE_MODE_VSCLIENT || SokuLib::mainMode == SokuLib::BATTLE_MODE_VSSERVER)
	//	return;
	if (state.P1.punishText)
		renderChrState(state.P1, 640 - 13 - state.P1.punishText->rect.width);

	renderChrState(state.P2, 13);
}

extern "C" __declspec(dllexport) bool CheckVersion(const BYTE hash[16]) {
	return memcmp(hash, SokuLib::targetHash, 16) == 0;
}

extern "C" __declspec(dllexport) bool Initialize(HMODULE hMyModule, HMODULE hParentModule)
{
	DWORD old;

#ifdef _DEBUG
	FILE *_;

	AllocConsole();
	freopen_s(&_, "CONOUT$", "w", stdout);
	freopen_s(&_, "CONOUT$", "w", stderr);
#endif

	VirtualProtect((PVOID)RDATA_SECTION_OFFSET, RDATA_SECTION_SIZE, PAGE_EXECUTE_WRITECOPY, &old);
	og_BattleManagerOnProcess = SokuLib::TamperDword(&SokuLib::VTable_BattleManager.onProcess, BattleOnProcess);
	og_BattleManagerOnRender  = SokuLib::TamperDword(&SokuLib::VTable_BattleManager.onRender,  BattleOnRender);
	og_LoadingOnProcess       = SokuLib::TamperDword(&SokuLib::VTable_Loading.onProcess,       LoadingOnProcess);
	og_LoadingCLOnProcess     = SokuLib::TamperDword(&SokuLib::VTable_LoadingClient.onProcess, LoadingCLOnProcess);
	og_LoadingSVOnProcess     = SokuLib::TamperDword(&SokuLib::VTable_LoadingServer.onProcess, LoadingSVOnProcess);
	VirtualProtect((PVOID)RDATA_SECTION_OFFSET, RDATA_SECTION_SIZE, old, &old);

	VirtualProtect((PVOID)TEXT_SECTION_OFFSET, TEXT_SECTION_SIZE, PAGE_EXECUTE_WRITECOPY, &old);
	original_onHit = SokuLib::union_cast<int (SokuLib::CharacterManager::*)(int)>(SokuLib::TamperNearJmpOpr(0x47c5a9, reinterpret_cast<DWORD>(isHit)));
	VirtualProtect((PVOID)TEXT_SECTION_OFFSET, TEXT_SECTION_SIZE, old, &old);

	FlushInstructionCache(GetCurrentProcess(), nullptr, 0);
	return true;
}

extern "C" int APIENTRY DllMain(HMODULE hModule, DWORD fdwReason, LPVOID lpReserved)
{
	return TRUE;
}