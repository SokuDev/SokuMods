// TODO: BEtter notification that doesn't block
#include <windows.h>
#include <initguid.h>
#include "common.h"
#include <d3d9.h>
#include <detours.h>
#include <dinput.h>
#include <iostream>
#include <shlwapi.h>

#define SWRS_USES_HASH
#include "swrs.h"

// Places to hold all the old function pointers we replace with ours with the DLL injection.
#define CBattleManager_Create(p) Ccall(p, s_origCBattleManager_OnCreate, void *, ())()
#define CBattleManager_Render(p) Ccall(p, s_origCBattleManager_OnRender, void, ())()
#define CBattleManager_Process(p) Ccall(p, s_origCBattleManager_OnProcess, int, ())()
#define CBattleManager_Destruct(p, dyn) Ccall(p, s_origCBattleManager_OnDestruct, void *, (int))(dyn)
#define CBattle_Process(p) Ccall(p, s_origCBattle_OnProcess, int, ())()
#define CBattle_Destruct(p, dyn) Ccall(p, s_origCBattle_OnDestruct, void *, (int))(dyn)

#define KBDATASIZE 256
#define PADDATASIZE 80
#define MAXDELAYFRAMES 10
#define MAXPADS 5
#define NODELAY 1
#define INITIALDELAY 5

static DWORD s_origCBattleManager_OnCreate;
static DWORD s_origCBattleManager_OnDestruct;
static DWORD s_origCBattleManager_OnRender;
static DWORD s_origCBattleManager_OnProcess;
static DWORD s_origCBattleManager_Size;

static char s_profilePath[1024 + MAX_PATH];
static char s_msg[256];
static HMODULE s_hDllModule;

// Buffer to hold text for delay indicator/debug output
static char TextDrawBuffer[1024];

// The ReDelay state struct. Holds all the global flags and counters needed to run the mod.
static struct {
	int RunningIndex; // Increments by 1 each frame. Used to calculate OldRunningIndex.
	int OldRunningIndex; // Dictates delay ring buffer read and write indexes.
	int NumberOfPads;
	int ModeFlag; // Set to 1 if inside suitable game mode for delay
	int EnablePadDelay;
	int ShowDebugInfo; // Show extended debug info. Lags a lot. Requires game reset if switched to 0 in SokuEngine.
	int DelayFrames = NODELAY; // Dictates delay ring buffer usage depth. Actual delay = DelayFrames - 1.
} rd_state;

// Control keys set in the .ini file for delay control.
static struct {
	BYTE LowerDelay;
	BYTE RaiseDelay;
} control_keys;

// Arrays to manipulate the 256 byte long DI DIK input arrays.
static BYTE DelayKbs[MAXDELAYFRAMES][KBDATASIZE];

// Pointer to incoming DI inputs from windows to the game.
static BYTE *KbInputs; // KbInputs doesn't have to be global, but it is for debugging purposes.

// Arrays to manipulate the 80 byte long DI gamepad input structs.
static DIJOYSTATE DelayPads[MAXPADS][MAXDELAYFRAMES];

// Array to hold the keys we want to delay. Redefine these in ReDelay.ini.
static BYTE DIKInputs[10] = {
	DIK_Z, // Attack
	DIK_X, // Weak bullet
	DIK_C, // Strong bullet
	DIK_A, // Dash
	DIK_S, // Card switch
	DIK_D, // Card use
	DIK_UP, // Up
	DIK_LEFT, // Left
	DIK_DOWN, // Down
	DIK_RIGHT // Right
};

// Bitmasks to separate the buttons we want to delay from the ones we don't.
static BYTE KbInputMask[KBDATASIZE];
static BYTE InverseMask[KBDATASIZE];

// Not sure what this does. It'll stay for now.
/*
#ifndef _DEBUG
extern "C" int _fltused = 1;
#endif
*/

// Macros to change what buffer the debug view shows if full message is uncommented. Lags a lot.
//#define SHOW_KB(i) ((int*)KbInputMask)[i]
#define SHOW_KB(i) ((int *)KbInputs)[i]
//#define SHOW_KB(i) ((int*)BypassKb)[i]
//#define SHOW_KB(i) ((int*)InverseMask)[i]

// Constructor for soku battlemanager. Seems to fire when entering practice/vs/etc. from title screen.
void *__fastcall CBattleManager_OnCreate(void *This) {
	CBattleManager_Create(This);
	// Same modes as RIV, except not replay.
	if ((g_mainMode == SWRSMODE_PRACTICE || g_mainMode == SWRSMODE_VSCOM || g_mainMode == SWRSMODE_VSPLAYER) && g_subMode != SWRSSUBMODE_REPLAY) {
		rd_state.ModeFlag = 1;
	} else {
		// Story, Replay, Netplay, Arcade.
		rd_state.ModeFlag = 0;
	}
	if (rd_state.ModeFlag) {
		sizeof(DIJOYSTATE);
		text::LoadSettings(s_profilePath, "Debug");
		text::OnCreate(This);
		// Initialize ini options.
		control_keys.LowerDelay = ::GetPrivateProfileInt("Keys", "lowerdelay", 0, s_profilePath);
		control_keys.RaiseDelay = ::GetPrivateProfileInt("Keys", "raisedelay", 0, s_profilePath);
		rd_state.ShowDebugInfo = ::GetPrivateProfileInt("Debug", "Enabled", 0, s_profilePath);

		rd_state.EnablePadDelay = ::GetPrivateProfileInt("Debug", "EnablePadDelay", 0, s_profilePath);
		if (rd_state.NumberOfPads > MAXPADS && rd_state.EnablePadDelay) {
			char WhineBuffer[256];
			sprintf_s(WhineBuffer, sizeof(WhineBuffer), "You have %d gamepads connected.\n\rReDelay supports only up to 5 pads plugged in.\n\rPad delay disabled.",
				rd_state.NumberOfPads);
			MessageBox(0, WhineBuffer, 0, 0x00040000L | 0x00010000L);
			rd_state.EnablePadDelay = 0;
		}

		rd_state.DelayFrames = INITIALDELAY;
		DIKInputs[0] = ::GetPrivateProfileInt("DelayButtons", "Attack", 0, s_profilePath);
		DIKInputs[1] = ::GetPrivateProfileInt("DelayButtons", "BBullet", 0, s_profilePath);
		DIKInputs[2] = ::GetPrivateProfileInt("DelayButtons", "CBullet", 0, s_profilePath);
		DIKInputs[3] = ::GetPrivateProfileInt("DelayButtons", "Dash", 0, s_profilePath);
		DIKInputs[4] = ::GetPrivateProfileInt("DelayButtons", "SwitchSpell", 0, s_profilePath);
		DIKInputs[5] = ::GetPrivateProfileInt("DelayButtons", "UseSpell", 0, s_profilePath);
		DIKInputs[6] = ::GetPrivateProfileInt("DelayButtons", "Up", 0, s_profilePath);
		DIKInputs[7] = ::GetPrivateProfileInt("DelayButtons", "Left", 0, s_profilePath);
		DIKInputs[8] = ::GetPrivateProfileInt("DelayButtons", "Down", 0, s_profilePath);
		DIKInputs[9] = ::GetPrivateProfileInt("DelayButtons", "Right", 0, s_profilePath);
		// Set the global keymasks according to button layout.
		memset(KbInputMask, 255, KBDATASIZE);
		for (BYTE i = 0; i < 10; i++) {
			KbInputMask[DIKInputs[i]] = 0;
		}
		memset(InverseMask, 0, KBDATASIZE);
		for (BYTE i = 0; i < 10; i++) {
			InverseMask[DIKInputs[i]] = 255;
		}
	}
	return This;
}
// Function that updates the game state once a frame according to animation information.
// Does not process inputs, sadly.
int __fastcall CBattleManager_OnProcess(void *This) {
	// Hacky code to reset RunningIndex value that keeps increasing too much when window not active.
	if (rd_state.RunningIndex > rd_state.OldRunningIndex + rd_state.DelayFrames) {
		rd_state.RunningIndex = rd_state.OldRunningIndex + (rd_state.RunningIndex - rd_state.OldRunningIndex) % rd_state.DelayFrames;
	}
	// Increase our frame count. This means we can advance the ring buffer.
	rd_state.RunningIndex += 1;

	int ret = CBattleManager_Process(This);
	return ret;
}
// Function to draw the delay display, or debug data if that's enabled.
// Lags if fully uncommented, but useful for debugging.
static void draw_debug_info(void *This) {
	int FrameDiff = rd_state.RunningIndex - rd_state.OldRunningIndex;
	if (!rd_state.ShowDebugInfo) {
		sprintf_s(TextDrawBuffer, sizeof(TextDrawBuffer), "%1d", (rd_state.DelayFrames - 1));
	} else {
		sprintf_s(TextDrawBuffer, sizeof(TextDrawBuffer),

			"Rdx %06d\n"
			"Odx %06d\n"

			"%08x %08x %08x %08x %08x %08x %08x %08x \n"
			"%08x %08x %08x %08x %08x %08x %08x %08x \n"
			"%08x %08x %08x %08x %08x %08x %08x %08x \n"
			"%08x %08x %08x %08x %08x %08x %08x %08x \n"
			"%08x %08x %08x %08x %08x %08x %08x %08x \n"
			"%08x %08x %08x %08x %08x %08x %08x %08x \n"
			"%08x %08x %08x %08x %08x %08x %08x %08x \n"
			"%08x %08x %08x %08x %08x %08x %08x %08x \n"

			"%d    %04d",
			rd_state.RunningIndex, rd_state.OldRunningIndex,

			SHOW_KB(0), SHOW_KB(1), SHOW_KB(2), SHOW_KB(3), SHOW_KB(4), SHOW_KB(5), SHOW_KB(6), SHOW_KB(7), SHOW_KB(8), SHOW_KB(9), SHOW_KB(10), SHOW_KB(11),
			SHOW_KB(12), SHOW_KB(13), SHOW_KB(14), SHOW_KB(15),

			SHOW_KB(16), SHOW_KB(17), SHOW_KB(18), SHOW_KB(19), SHOW_KB(20), SHOW_KB(21), SHOW_KB(22), SHOW_KB(23), SHOW_KB(24), SHOW_KB(25), SHOW_KB(26),
			SHOW_KB(27), SHOW_KB(28), SHOW_KB(29), SHOW_KB(30), SHOW_KB(31),

			SHOW_KB(32), SHOW_KB(33), SHOW_KB(34), SHOW_KB(35), SHOW_KB(36), SHOW_KB(37), SHOW_KB(38), SHOW_KB(39), SHOW_KB(40), SHOW_KB(41), SHOW_KB(42),
			SHOW_KB(43), SHOW_KB(44), SHOW_KB(45), SHOW_KB(46), SHOW_KB(47),

			SHOW_KB(48), SHOW_KB(49), SHOW_KB(50), SHOW_KB(51), SHOW_KB(52), SHOW_KB(53), SHOW_KB(54), SHOW_KB(55), SHOW_KB(56), SHOW_KB(57), SHOW_KB(58),
			SHOW_KB(59), SHOW_KB(60), SHOW_KB(61), SHOW_KB(62), SHOW_KB(63),

			rd_state.DelayFrames - 1, FrameDiff);
	}
	text::SetText(TextDrawBuffer);
	text::OnRender(This);
}
// Called once a frame to render stuff. We want to render our debug info after the original soku function is done.
void __fastcall CBattleManager_OnRender(void *This) {
	CBattleManager_Render(This);
	if (rd_state.ModeFlag) {
		draw_debug_info(This);
	}
}
// Battlemanager destructor. Fires when a new mode is selected after returning to title screen, but not
// when entering the title screen! Careful.
void *__fastcall CBattleManager_OnDestruct(void *This, int mystery, int dyn) {
	if (rd_state.ModeFlag) {
		text::OnDestruct(This, mystery, dyn);
	}
	rd_state.ModeFlag = 0;
	rd_state.DelayFrames = NODELAY;
	rd_state.RunningIndex = 0;
	rd_state.OldRunningIndex = 0;
	return CBattleManager_Destruct(This, dyn);
}

// Function to flush the ring buffer
void ClearDelayBuffers(void) {
	for (int i = 0; i < MAXDELAYFRAMES; i++) {
		memset(DelayKbs[i], 0, KBDATASIZE);
	}
	for (int i = 0; i < MAXPADS; i++) {
		for (int j = 0; j < MAXDELAYFRAMES; j++) {
			memset(&DelayPads[i][j], 0, PADDATASIZE);
		}
	}
}

// Function to sort out ESC and delay control buttons. If the old state is not checked, these rapid fire.
void PreProcessControlKeys(BYTE *DIKkeys) {
	static int OldEscState = 0;
	static int OldRaiseDelayState = 0;
	static int OldLowerDelayState = 0;

	if (DIKkeys[DIK_ESCAPE] & 0x80) {
		if (DIKkeys[DIK_ESCAPE] != OldEscState) {
			// We pressed escape. No delay needs to be calculated for now.
			ClearDelayBuffers();
		}
	}
	OldEscState = DIKkeys[DIK_ESCAPE];

	if (DIKkeys[control_keys.RaiseDelay] & 0x80) {
		if (DIKkeys[control_keys.RaiseDelay] != OldRaiseDelayState) {
			if (rd_state.DelayFrames >= MAXDELAYFRAMES) {
				rd_state.DelayFrames = MAXDELAYFRAMES;
			} else {
				rd_state.DelayFrames += 1;
				ClearDelayBuffers();
			}
		}
	}
	OldRaiseDelayState = DIKkeys[control_keys.RaiseDelay];

	if (DIKkeys[control_keys.LowerDelay] & 0x80) {
		if (DIKkeys[control_keys.LowerDelay] != OldLowerDelayState) {
			if (rd_state.DelayFrames <= 1) {
				rd_state.DelayFrames = 1;
			} else {
				rd_state.DelayFrames -= 1;
				ClearDelayBuffers();
			}
		}
	}
	OldLowerDelayState = DIKkeys[control_keys.LowerDelay];
}

HRESULT(WINAPI *oldGetDeviceState)(LPVOID IDirectInputDevice8W, DWORD cbData, LPVOID lpvData) = NULL;
// This fires everytime soku asks for use input. Seems to happen once a frame?
HRESULT WINAPI myGetDeviceState(LPVOID IDirectInputDevice8W, DWORD cbData, LPVOID lpvData) {
	static int PadIndex = 0;

	HRESULT retValue = oldGetDeviceState(IDirectInputDevice8W, cbData, lpvData);
	if (retValue != DI_OK) {
		return retValue;
	}
	// DI delay processing. We want to do this only when there is right type of a battle going on.
	if (g_sceneId == SWRSSCENE_BATTLE && rd_state.ModeFlag) {
		// Check if we have a new frame. If yes, we update the current frame counter to catch up.
		if (rd_state.RunningIndex > rd_state.OldRunningIndex) {
			rd_state.OldRunningIndex += 1;
			PadIndex = 0;
		} else if (rd_state.RunningIndex < rd_state.OldRunningIndex) {
			// Index flowed over. This happens after a really long time.
			rd_state.OldRunningIndex = 0;
			rd_state.RunningIndex = 0;
		}
		// Calculate access indexes for our delay ring buffers.
		int OldIndex = rd_state.OldRunningIndex % rd_state.DelayFrames;
		int NextIndex = (rd_state.OldRunningIndex + 1) % rd_state.DelayFrames;
		// Process keyboard
		if (cbData == KBDATASIZE) {
			KbInputs = (BYTE *)lpvData; // KbInputs is global for debugging purposes.
			PreProcessControlKeys(KbInputs);

			BYTE BypassKb[KBDATASIZE]; // This will hold the keys we don't want to delay.
			memset(BypassKb, 0, KBDATASIZE);
			// 1) We filter everything but the target delay keys from the new input
			//    and place them to the delay ring buffer to wait for the duration of DelayFrames.

			// 2) We filter the new input for everything but the target keys to get the keys we want out
			//    on the next frame to get the bypass keys.

			// 3) We combine the bypass keys with the input from the past that have waited for the duration
			//	  of DelayFrames and show them to the game.
			for (int i = 0; i < KBDATASIZE / sizeof(int); i++) {
				((int *)DelayKbs[OldIndex])[i] = ((int *)KbInputs)[i] & ((int *)InverseMask)[i];
				((int *)BypassKb)[i] = ((int *)KbInputMask)[i] & ((int *)KbInputs)[i];
				((int *)KbInputs)[i] = ((int *)DelayKbs[NextIndex])[i] | ((int *)BypassKb)[i];
			}
		}
		// Process pads

		else if (cbData == sizeof(DIJOYSTATE) && rd_state.EnablePadDelay) {
			DIJOYSTATE *PadInputs = (DIJOYSTATE *)lpvData;
			// Here we don't do input filtering. We just store entire pad states in a ring buffer.
			// This is a hacky way to deal with multiple pads that use this same function.
			// We assume the pads are checked every frame in the same order, so PadIndex dictates
			// which pad the particular ring buffer belongs to.
			// We end up here NumberOfPads times a frame. Blame Tasofro.
			memcpy(&DelayPads[PadIndex][OldIndex], PadInputs, sizeof(DIJOYSTATE));
			memcpy(PadInputs, &DelayPads[PadIndex][NextIndex], sizeof(DIJOYSTATE));
			PadIndex += 1;
			// Hack to make RIV framestepping work with pad
			if (PadIndex >= rd_state.NumberOfPads) {
				PadIndex = 0;
			}
		}
	}
	return retValue;
}

// This callback checks through all connected gamepads and counts them to get the total number of
// connected pads.
BOOL CALLBACK DIEnumDevicesCallback(LPCDIDEVICEINSTANCE lpddi, LPVOID pvRef) {
	// char buffer[MAX_PATH];
	// sprintf_s(buffer, sizeof(buffer), "%s", lpddi->tszInstanceName);
	// MessageBox(0,buffer,0,0);
	rd_state.NumberOfPads += 1;

	return TRUE;
}

// Magic pointer tricks taken from DPadFix. We essentially steal the function pointer to
// the game's DI Device GetDeviceState function, which the game uses to poll inputs.
void DummyDirectInput() {
	LPDIRECTINPUT8 pDI;
	DirectInput8Create(GetModuleHandle(NULL), DIRECTINPUT_VERSION, IID_IDirectInput8, (void **)&pDI, NULL);

	LPDIRECTINPUTDEVICE8 pKeyboard;
	pDI->CreateDevice(GUID_SysKeyboard, &pKeyboard, NULL);

	LPVOID ptrGetDeviceState = *(((LPVOID *)(*(LPVOID *)(pKeyboard))) + 9);

	oldGetDeviceState = (HRESULT(WINAPI *)(LPVOID, DWORD, LPVOID))ptrGetDeviceState;

	pDI->EnumDevices(DI8DEVCLASS_GAMECTRL, DIEnumDevicesCallback, 0, DIEDFL_ATTACHEDONLY);

	DetourRestoreAfterWith();

	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());
	DetourAttach(&(PVOID &)oldGetDeviceState, myGetDeviceState);
	DetourTransactionCommit();

	pKeyboard->Release();
	pDI->Release();
}

/*
 * Initializing the dll
 * The module is loaded as follow into the game:
 * 1. CBattleManager_OnCreate (when entering the character select menu)
 * 2. CBattleManager_OnProcess
 * 3. CBattleManager_OnRender
 * 4. CBattleManager_OnDestruct (when leaving the character select menu)
 */
extern "C" __declspec(dllexport) bool CheckVersion(const BYTE hash[16]) {
	return ::memcmp(TARGET_HASH, hash, sizeof TARGET_HASH) == 0;
}
// DPadFix and RIV initialization smushed together.
extern "C" __declspec(dllexport) bool Initialize(HMODULE hMyModule, HMODULE hParentModule) {
	s_hDllModule = hMyModule;
	HMODULE d3dx = LoadLibraryExW(L"D3DX9_33.DLL", NULL, 0);
	if (d3dx == NULL)
		return false;

	GetModuleFileName(hMyModule, s_profilePath, 1024);
	PathRemoveFileSpec(s_profilePath);
	PathAppend(s_profilePath, "ReDelay.ini");

	if (!LoadLibraryExW(L"dinput8.dll", NULL, 0)) {
		return false;
	}
	DummyDirectInput();
	DWORD old;
	::VirtualProtect((PVOID)text_Offset, text_Size, PAGE_EXECUTE_WRITECOPY, &old);
	s_origCBattleManager_OnCreate = TamperNearJmpOpr(CBattleManager_Creater, union_cast<DWORD>(CBattleManager_OnCreate));
	::VirtualProtect((PVOID)text_Offset, text_Size, old, &old);

	::VirtualProtect((PVOID)rdata_Offset, rdata_Size, PAGE_WRITECOPY, &old);
	s_origCBattleManager_OnDestruct = TamperDword(vtbl_CBattleManager + 0x00, union_cast<DWORD>(CBattleManager_OnDestruct));
	s_origCBattleManager_OnRender = TamperDword(vtbl_CBattleManager + 0x38, union_cast<DWORD>(CBattleManager_OnRender));
	s_origCBattleManager_OnProcess = TamperDword(vtbl_CBattleManager + 0x0c, union_cast<DWORD>(CBattleManager_OnProcess));
	::VirtualProtect((PVOID)rdata_Offset, rdata_Size, old, &old);

	::FlushInstructionCache(GetCurrentProcess(), NULL, 0);

	return true;
}

extern "C" int APIENTRY DllMain(HMODULE hModule, DWORD fdwReason, LPVOID lpReserved) {
	return TRUE;
}
