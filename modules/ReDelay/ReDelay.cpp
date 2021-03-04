#include <Shlwapi.h>
#include <SokuLib.hpp>
#include <d3d9.h>
#include <list>

static SokuLib::BattleManager *(SokuLib::BattleManager::*s_origCBattleManager_Create)();
static int (__thiscall SokuLib::BattleManager::*s_origCBattleManager_Render)();
static void *(__thiscall SokuLib::BattleManager::*s_origCBattleManager_Destruct)(int);
static bool activated = false;
static bool created = false;
static void (SokuLib::KeymapManager::*s_origKeymapManager_SetInputs)();
static std::list<SokuLib::KeyInput> lastInputsLeft;
static std::list<SokuLib::KeyInput> lastInputsRight;
static std::pair<SokuLib::KeyInput, SokuLib::KeyInput> lastInputs;
static SokuLib::SWRFont font;
static char increaseDelay;
static char decreaseDelay;
static bool wasMorePressed = false;
static bool wasLessPressed = false;

#define FONT_HEIGHT 14
#define TEXTURE_SIZE 0x100
#define PAYLOAD_ADDRESS_GET_INPUTS 0x40A45D

static void drawSprite(int texid, float x, float y, float cx, float cy) {
	const struct {
		float x, y, z;
		float rhw;
		unsigned color;
		float u, v;
	} vertices[] = {
		{x, y, 0.0f, 1.0f, 0xffffffff, 0.0f, 0.0f},
		{x + cx, y, 0.0f, 1.0f, 0xffffffff, 1.0f, 0.0f},
		{x + cx, y + cy, 0.0f, 1.0f, 0xffffffff, 1.0f, 1.0f},
		{x, y + cy, 0.0f, 1.0f, 0xffffffff, 0.0f, 1.0f},
	};

	SokuLib::textureMgr.setTexture(texid, 0);
	SokuLib::pd3dDev->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, 2, vertices, sizeof(*vertices));
}

void updateInput(SokuLib::KeyInput &old, const SokuLib::KeyInput &n) {
	old.a = n.a ? (old.a + 1) : 0;
	old.b = n.b ? (old.b + 1) : 0;
	old.c = n.c ? (old.c + 1) : 0;
	old.d = n.d ? (old.d + 1) : 0;
	old.changeCard = n.changeCard ? (old.changeCard + 1) : 0;
	old.spellcard = n.spellcard ? (old.spellcard + 1) : 0;

	if (n.horizontalAxis == 0)
		old.horizontalAxis = 0;
	else if (n.horizontalAxis > 0)
		old.horizontalAxis = max(0, old.horizontalAxis) + 1;
	else
		old.horizontalAxis = min(0, old.horizontalAxis) - 1;

	if (n.verticalAxis == 0)
		old.verticalAxis = 0;
	else if (n.verticalAxis > 0)
		old.verticalAxis = max(0, old.verticalAxis) + 1;
	else
		old.verticalAxis = min(0, old.verticalAxis) - 1;
}

void handleInput(SokuLib::KeymapManager *base) {
	if (SokuLib::sceneId != SokuLib::SCENE_BATTLE)
		return;

	auto &mgr = SokuLib::getBattleMgr();

	if (base == mgr.leftCharacterManager.keyManager->keymapManager) {
		lastInputsLeft.push_back(base->input);
		updateInput(lastInputs.first, lastInputsLeft.front());
		memcpy(&base->input, &lastInputs.first, sizeof(base->input));
		lastInputsLeft.pop_front();
	} else if (mgr.rightCharacterManager.keyManager && base == mgr.rightCharacterManager.keyManager->keymapManager) {
		lastInputsRight.push_back(base->input);
		updateInput(lastInputs.second, lastInputsRight.front());
		memcpy(&base->input, &lastInputs.second, sizeof(base->input));
		lastInputsRight.pop_front();
	} else
		return;

	if (GetForegroundWindow() != SokuLib::window)
		return;

	if ((GetKeyState(increaseDelay) & 0x8000) && !wasMorePressed) {
		lastInputsLeft.push_front({0});
		lastInputsRight.push_front({0});
	}
	if ((GetKeyState(decreaseDelay) & 0x8000) && !lastInputsLeft.empty() && !wasLessPressed) {
		lastInputsLeft.pop_back();
		lastInputsRight.pop_back();
	}
	wasMorePressed = (GetKeyState(increaseDelay) & 0x8000);
	wasLessPressed = (GetKeyState(decreaseDelay) & 0x8000);
}

void __fastcall KeymapManagerSetInputs(SokuLib::KeymapManager *This) {
	(This->*s_origKeymapManager_SetInputs)();
	handleInput(This);
}

void initFont() {
	SokuLib::FontDescription desc;

	if (created)
		return;
	created = true;
	desc.r1 = 205;
	desc.r2 = 205;
	desc.g1 = 205;
	desc.g2 = 205;
	desc.b1 = 205;
	desc.height = FONT_HEIGHT;
	desc.weight = FW_BOLD;
	desc.italic = 0;
	desc.shadow = 2;
	desc.bufferSize = 1000000;
	desc.charSpaceX = 0;
	desc.charSpaceY = 0;
	desc.offsetX = 0;
	desc.offsetY = 0;
	desc.useOffset = 0;
	strcpy(desc.faceName, "Tahoma");
	font.create();
	font.setIndirect(desc);
}

SokuLib::BattleManager *__fastcall CBattleManager_Create(SokuLib::BattleManager *This) {
	(This->*s_origCBattleManager_Create)();

	if (SokuLib::subMode == SokuLib::BATTLE_SUBMODE_REPLAY)
		return This;

	DWORD old;

	switch (SokuLib::mainMode) {
	case SokuLib::BATTLE_MODE_VSPLAYER:
	case SokuLib::BATTLE_MODE_VSCOM:
	case SokuLib::BATTLE_MODE_PRACTICE:
		activated = true;
		VirtualProtect((PVOID)TEXT_SECTION_OFFSET, TEXT_SECTION_SIZE, PAGE_EXECUTE_WRITECOPY, &old);
		s_origKeymapManager_SetInputs
			= SokuLib::union_cast<void (SokuLib::KeymapManager::*)()>(SokuLib::TamperNearJmpOpr(PAYLOAD_ADDRESS_GET_INPUTS, (int)KeymapManagerSetInputs));
		VirtualProtect((PVOID)TEXT_SECTION_OFFSET, TEXT_SECTION_SIZE, PAGE_EXECUTE_WRITECOPY, &old);
		initFont();
		lastInputsLeft.clear();
	default:
		return This;
	}
}

int __fastcall CBattleManager_Render(SokuLib::BattleManager *This) {
	// super
	int ret = (This->*s_origCBattleManager_Render)();

	if (activated) {
		int texture;

		SokuLib::textureMgr.createTextTexture(&texture, std::to_string(lastInputsLeft.size()).c_str(), font, TEXTURE_SIZE, FONT_HEIGHT + 18, nullptr, nullptr);
		drawSprite(texture, 3, 462, TEXTURE_SIZE, FONT_HEIGHT + 18.0f);
		SokuLib::textureMgr.remove(texture);
	}
	return ret;
}

void *__fastcall CBattleManager_Destruct(SokuLib::BattleManager *This, int, int dyn) {
	// super
	void *ret = (This->*s_origCBattleManager_Destruct)(dyn);

	if (activated) {
		DWORD old;

		VirtualProtect((PVOID)PAYLOAD_ADDRESS_GET_INPUTS, 4, PAGE_EXECUTE_WRITECOPY, &old);
		SokuLib::TamperNearJmpOpr(PAYLOAD_ADDRESS_GET_INPUTS, SokuLib::union_cast<DWORD>(s_origKeymapManager_SetInputs));
		VirtualProtect((PVOID)PAYLOAD_ADDRESS_GET_INPUTS, 4, old, &old);
	}
	activated = false;
	return ret;
}

void hookFunctions() {
	DWORD old;

	// Setup hooks
	VirtualProtect((PVOID)RDATA_SECTION_OFFSET, RDATA_SECTION_SIZE, PAGE_EXECUTE_WRITECOPY, &old);
	s_origCBattleManager_Destruct = SokuLib::union_cast<void *(SokuLib::BattleManager::*)(int)>(
		SokuLib::TamperDword(SokuLib::vtbl_CBattleManager + SokuLib::BATTLE_MGR_OFFSET_ON_DESTRUCT, reinterpret_cast<DWORD>(CBattleManager_Destruct)));
	s_origCBattleManager_Render = SokuLib::union_cast<int (SokuLib::BattleManager::*)()>(
		SokuLib::TamperDword(SokuLib::vtbl_CBattleManager + SokuLib::BATTLE_MGR_OFFSET_ON_RENDER, reinterpret_cast<DWORD>(CBattleManager_Render)));
	VirtualProtect((PVOID)RDATA_SECTION_OFFSET, RDATA_SECTION_SIZE, old, &old);

	::VirtualProtect((PVOID)TEXT_SECTION_OFFSET, TEXT_SECTION_SIZE, PAGE_EXECUTE_WRITECOPY, &old);
	s_origCBattleManager_Create = SokuLib::union_cast<SokuLib::BattleManager *(SokuLib::BattleManager::*)()>(
		SokuLib::TamperNearJmpOpr(SokuLib::ADDR_BATTLE_MANAGER_CREATER, reinterpret_cast<DWORD>(CBattleManager_Create)));
	::VirtualProtect((PVOID)TEXT_SECTION_OFFSET, TEXT_SECTION_SIZE, old, &old);

	::FlushInstructionCache(GetCurrentProcess(), nullptr, 0);
}

extern "C" __declspec(dllexport) bool CheckVersion(const BYTE hash[16]) {
	return ::memcmp(SokuLib::targetHash, hash, sizeof(SokuLib::targetHash)) == 0;
}

extern "C" __declspec(dllexport) bool Initialize(HMODULE hMyModule, HMODULE hParentModule) {
	char profilePath[1024 + MAX_PATH];
	GetModuleFileName(hMyModule, profilePath, 1024);
	PathRemoveFileSpec(profilePath);
	PathAppend(profilePath, "ReDelay.ini");
	increaseDelay = GetPrivateProfileIntA("Keys", "IncreaseDelay", 0x6B, profilePath);
	decreaseDelay = GetPrivateProfileIntA("Keys", "DecreaseDelay", 0x6D, profilePath);
	hookFunctions();

	return true;
}

extern "C" int APIENTRY DllMain(HMODULE hModule, DWORD fdwReason, LPVOID lpReserved) {
	return TRUE;
}
