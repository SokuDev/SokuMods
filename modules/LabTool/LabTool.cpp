#include "stdafx.h"

// Personal header
#include "functions.h"
#include "joystick.h"
#include "labtool_manager.h"

/* freopen is obsolete */
#pragma warning(disable : 4996)

// A couple of thoughts about the battle manager virtual functions table and its memory offsets (by Alexey-Ul)
// 0x00 - destructor
// 0x04 - after select arena start, best place for initialize girls void*(void* int int)
// 0x08 - After exit on selection screen or menu
// 0x0c - Progress??
// 0x10 - announcement say start round
// 0x14 - After load black scree
// 0x18 - Progress???
// 0x1c - After round
// 0x20 - After battle (KO)
// 0x24 - After battle girls talk
// 0x28 - ??????????
// 0x2c - before last hit in round  void*(void* void* int)
// 0x30 - after stat round and before round end pure battle()
// 0x34 - at the start before "start round" logo int is 1 and it becomes 2 after logo and becomes 3 after the end of round
//       and becomes 5 after final round before logo and and 6 after final and 7 after girls talks states void*(void* void* int)
// 0x38 - After select arena render or draw()
// 0x3c - After select arena render????
// 0x40 - After select arena render???? Last function???

#define OFFSET_BTLMGR_DESTR 0x00
#define OFFSET_BTLMGR_INIT 0x04
#define OFFSET_BTLMGR_DEINIT 0x08
#define OFFSET_BTLMGR_BATTLE 0x30
#define OFFSET_BTLMGR_RENDER 0x38

#define CBattleManager_Create(p) Ccall(p, s_origCBattleManager_OnCreate, void *, ())()
#define CBattleManager_Render(p) Ccall(p, s_origCBattleManager_OnRender, void, ())()
#define CBattleManager_Process(p) Ccall(p, s_origCBattleManager_OnProcess, int, ())()
#define CBattleManager_Destruct(p, dyn) Ccall(p, s_origCBattleManager_OnDestruct, void *, (int))(dyn)

#define CBattleManager_Initialize(p, dyn) Ccall(p, s_origCBattleManager_Initialize, void *, (int))(dyn)
#define CBattleManager_DeInitialize(p) Ccall(p, s_origCBattleManager_DeInitialize, int, ())()

static unsigned long s_origCBattleManager_OnCreate;
static unsigned long s_origCBattleManager_OnDestruct;
static unsigned long s_origCBattleManager_OnRender;
static unsigned long s_origCBattleManager_OnProcess;
static unsigned long s_origCBattleManager_Initialize;
static unsigned long s_origCBattleManager_DeInitialize;
static DWORD s_origCBattleManager_Size;

#define ADDR_BMGR_P1 0x0C
#define ADDR_BMGR_P2 0x10

static char s_profilePath[1024 + MAX_PATH];

Toggle_key toggle_keys = {false};
Held_key held_keys = {false};
Misc_state misc_states;

Joystick joystick;
Button buttons;
Player p1;
Player p2;

void *__fastcall CBattleManager_OnInitialize(void *This, void *mystery, int dyn) {
	auto ret = CBattleManager_Initialize(This, dyn);
	return ret;
}

int __fastcall CBattleManager_OnDeInitialize(void *This) {
	SetConsoleTitle(std::string("LabTool 1.1.0").c_str());
	auto ret = CBattleManager_DeInitialize(This);
	return CBattleManager_DeInitialize(This);
}

void *__fastcall CBattleManager_OnCreate(void *This) {
	CBattleManager_Create(This);
	LabToolManager::getInstance().create();

	joystick.CreateDIObject();
	joystick.getDIJoypad();

	/* .INI */
	savestate_keys.reset_pos = ::GetPrivateProfileInt("KEYS", "reset_pos", 0, s_profilePath);
	savestate_keys.save_pos = ::GetPrivateProfileInt("KEYS", "save_pos", 0, s_profilePath);
	savestate_keys.reset_skills = ::GetPrivateProfileInt("KEYS", "reset_skills", 0, s_profilePath);
	savestate_keys.display_states = ::GetPrivateProfileInt("KEYS", "display_states", 0, s_profilePath);
	buttons.reset_pos = ::GetPrivateProfileInt("KEYS", "JOYPADreset_pos", 0, s_profilePath);
	buttons.save_pos = ::GetPrivateProfileInt("KEYS", "JOYPADsave_pos", 0, s_profilePath);
	buttons.reset_skills = ::GetPrivateProfileInt("KEYS", "JOYPADreset_skills", 0, s_profilePath);
	buttons.display_states = ::GetPrivateProfileInt("KEYS", "JOYPADdisplay_states", 0, s_profilePath);

	held_keys = {false};
	toggle_keys.display_states = false;

	misc_states.frame_advantage = 0;
	misc_states.blockstring = false;
	misc_states.hjc_advantage = 0;
	misc_states.hjc_blockstring = false;
	misc_states.isIdle = -1;
	misc_states.untight_nextframe = false;
	misc_states.wakeup_count_p1 = 0;
	misc_states.wakeup_count_p2 = 0;

	return This;
}

void toggleFunction(UINT *key, bool *functionBool, bool *held_key) {
	if (GetAsyncKeyState(*key) & 0x8000) {
		if (!*held_key) {
			*functionBool = !*functionBool;
			*held_key = true;
		}
	} else {
		*held_key = false;
	}
}

void applyKeyboardInputs() {
	if (LabToolManager::isHisoutensokuOnTop()) {
		toggleFunction(&savestate_keys.reset_pos, &toggle_keys.reset_pos, &held_keys.reset_pos);
		toggleFunction(&savestate_keys.save_pos, &toggle_keys.save_pos, &held_keys.save_pos);
		toggleFunction(&savestate_keys.reset_skills, &toggle_keys.reset_skills, &held_keys.reset_skills);
		toggleFunction(&savestate_keys.display_states, &toggle_keys.display_states, &held_keys.display_states);
	}
}

void joypadToggleFunction(int button, bool *functionBool, bool *held_key) {
	if (button > -1) {
		if (joystick.joypadBuffer.rgbButtons[button] == 0x80) {
			if (!*held_key) {
				*functionBool = !*functionBool;
				*held_key = true;
			}
		} else {
			*held_key = false;
		}
	}
}

void applyJoypadInputs() {
	if (LabToolManager::isHisoutensokuOnTop()) {
		joypadToggleFunction(buttons.reset_pos, &toggle_keys.reset_pos, &held_keys.JOYPADreset_pos);
		joypadToggleFunction(buttons.save_pos, &toggle_keys.save_pos, &held_keys.JOYPADsave_pos);
		joypadToggleFunction(buttons.reset_skills, &toggle_keys.reset_skills, &held_keys.JOYPADreset_skills);
		joypadToggleFunction(buttons.display_states, &toggle_keys.display_states, &held_keys.JOYPADdisplay_states);
	}
}

void __fastcall CBattleManager_OnRender(void *This) {
	auto &labToolMgr = LabToolManager::getInstance();
	if (labToolMgr.isValidMode()) {
		applyKeyboardInputs();
		if (joystick.lpDIJoypad) {
			joystick.getJoypadInputs();
			applyJoypadInputs();
		}
	}
	CBattleManager_Render(This);
}

void rewriteConsoleTitle() {
	std::string str
		= std::to_string(p1.health) + "HP (" + std::to_string(p1.spirit) + "SP) [VS]" + std::to_string(p2.health) + "HP (" + std::to_string(p2.spirit) + "SP)";
	SetConsoleTitle((LPCSTR)str.c_str());
}

int __fastcall CBattleManager_OnProcess(void *This) {
	auto &labToolMgr = LabToolManager::getInstance();

	if (labToolMgr.isValidMode()) {
		update_playerinfo(&p1, ADDR_BMGR_P1);
		update_playerinfo(&p2, ADDR_BMGR_P2);

		position_management(&p1, &p2);
		gap_count(&p2);
		hjcadvantage_count(&p1, &p2);
		frameadvantage_count(&p1, &p2);
		is_tight(&p2);

		reset_skills(&p1);
		reset_skills(&p2);
		state_display(&p1);
		state_display(&p2);

		rewriteConsoleTitle();
	}

	auto ret = CBattleManager_Process(This);
	int battleManager = ACCESS_INT(ADDR_BATTLE_MANAGER, 0);
	return ret;
}

void *__fastcall CBattleManager_OnDestruct(void *This, int mystery, int dyn) {
	LabToolManager::getInstance().destruct();
	auto ret = CBattleManager_Destruct(This, dyn);
	return ret;
}

extern "C" {
__declspec(dllexport) bool CheckVersion(const BYTE hash[16]) {
	return ::memcmp(TARGET_HASH, hash, sizeof TARGET_HASH) == 0;
}

__declspec(dllexport) bool Initialize(HMODULE hMyModule, HMODULE hParentModule) {
	DWORD old;

	/* .ini */
	GetModuleFileName(hMyModule, s_profilePath, 1024);
	PathRemoveFileSpec(s_profilePath);
	PathAppend(s_profilePath, "LabTool.ini");

	if (!LoadLibrary("dinput8.dll")) {
		return true;
	}

	if (!VirtualProtect((PVOID)text_Offset, text_Size, PAGE_EXECUTE_WRITECOPY, &old)) {
		return false;
	}

	s_origCBattleManager_OnCreate = TamperNearJmpOpr(CBattleManager_Creater, union_cast<DWORD>(CBattleManager_OnCreate));

	if (!VirtualProtect((PVOID)text_Offset, text_Size, old, &old)) {
		return false;
	}

	if (!VirtualProtect((PVOID)rdata_Offset, rdata_Size, PAGE_WRITECOPY, &old)) {
		return false;
	}

	s_origCBattleManager_OnDestruct = TamperDword(vtbl_CBattleManager + OFFSET_BTLMGR_DESTR, union_cast<DWORD>(CBattleManager_OnDestruct));

	s_origCBattleManager_Initialize = TamperDword(vtbl_CBattleManager + OFFSET_BTLMGR_INIT, union_cast<DWORD>(CBattleManager_OnInitialize));

	s_origCBattleManager_DeInitialize = TamperDword(vtbl_CBattleManager + OFFSET_BTLMGR_DEINIT, union_cast<DWORD>(CBattleManager_OnDeInitialize));

	s_origCBattleManager_OnProcess = TamperDword(vtbl_CBattleManager + OFFSET_BTLMGR_BATTLE, union_cast<DWORD>(CBattleManager_OnProcess));

	s_origCBattleManager_OnRender = TamperDword(vtbl_CBattleManager + OFFSET_BTLMGR_RENDER, union_cast<DWORD>(CBattleManager_OnRender));

	if (!VirtualProtect((PVOID)rdata_Offset, rdata_Size, old, &old)) {
		return false;
	}

	if (!FlushInstructionCache(GetCurrentProcess(), nullptr, 0)) {
		return false;
	}

	return true;
}
}

// https://stackoverflow.com/a/32999084
// https://stackoverflow.com/a/33001454
// https://stackoverflow.com/a/8145349
// Libraries: shlwapi.lib user32.lib dinput8.lib dxguid.lib
