#include <windows.h>
#include <initguid.h>
#include <detours.h>
#include <dinput.h>
#include <shlwapi.h>
#include <SokuLib.hpp>
#include <tchar.h>
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <intrin.h>
#include <map>
#include <queue>
#include <algorithm>
#include <functional>
#include "../NetInfo/PatchMan.hpp"
#include <cstring>
#include <mutex>

#ifndef vtbl_CBattleManagerStory
#define vtbl_CBattleManagerStory 0x858934
#endif

#ifndef vtbl_CBattleManagerArcade
#define vtbl_CBattleManagerArcade 0x85899c
#endif 

SokuLib::BattleManager_VTABLE &VTable_BattleManagerStory = *reinterpret_cast<SokuLib::BattleManager_VTABLE *>(vtbl_CBattleManagerStory);
SokuLib::BattleManager_VTABLE &VTable_BattleManagerArcade = *reinterpret_cast<SokuLib::BattleManager_VTABLE *>(vtbl_CBattleManagerArcade);

static char s_profilePath[1024 + MAX_PATH];

using namespace std;

#define printf(s, ...) \
	do { \
		if (gDebug) { \
			_tprintf(_T(s), ##__VA_ARGS__); \
		} \
	} while (0)


#define fflush(...) \
	do { \
		if (gDebug) { \
			fflush(__VA_ARGS__); \
		} \
	} while (0)

HRESULT(WINAPI *oldGetDeviceState)(LPVOID IDirectInputDevice8W, DWORD cbData, LPVOID lpvData) = NULL;

typedef void (__fastcall  *ChordInputDetect)(unsigned int param_1, unsigned int param_2, int param_1_00, char param_2_00, char param_3);
ChordInputDetect cidfn = reinterpret_cast<ChordInputDetect>((uintptr_t)(0x0046CC90));

static SokuLib::BattleManager *battleMgr;

static int gTriggersThreshold = 500;
static int gTriggersEnabled = 0;
static size_t inputCounter = 0;
static bool gVirtualInput = true;
static bool gDebug = false;
static bool netEnable = false;

const size_t MAX_RGBBUTTON_INDEX = sizeof(DIJOYSTATE::rgbButtons);
const size_t RGBBUTTON_SELECT_MAP = MAX_RGBBUTTON_INDEX - 1;

// how many frames we have to wait after pressing 3 before a 236 will register instead of a 623
#define DISTINCTION_FRAMES_236_623 11

struct KeyboardInputState {
	bool enabled = false;
	uint8_t iMacro = DIK_LSHIFT;
	bool macroKeyDown;
	bool macroKeyRelease;
	bool macroKey2Down;
	bool macroKey2Release;

	// only used in virtual input mode.
	typedef std::function<void(char *)> InputCB;
	std::queue<InputCB> queuedInput;

	// which way the character using this input state is facing
	// (read from SokuLib::CharacterManager)
	bool facingRight = false;

	// keyboard mapping (DirectX keycodes)
	// (read from SokuLib::KeyManager)
	uint8_t iUp;
	uint8_t iDown;
	uint8_t iLeft;
	uint8_t iRight;
	uint8_t iA;
	uint8_t iB;
	uint8_t iC;
	uint8_t iD;

	int newFrame = 0;

	bool virtualMacroBPrev = false;
	bool virtualMacroCPrev = false;
	int dxprevs[DISTINCTION_FRAMES_236_623];
};

static KeyboardInputState keyboardstate[2];

// for gamepads
struct VirtualInputState {
	//typedef void (*InputCB)(DIJOYSTATE *);
	typedef std::function<void(DIJOYSTATE *)> InputCB;
	std::queue<InputCB> queuedInput;

	// if this counter exceeds 256, then the game isn't using this input, so delete it.
	int newFrame = 0;

	// which way the character using this input state is facing
	// (read from SokuLib::CharacterManager)
	bool facingRight = false;

	// rgb button index mapping for gamepad buttons A/B/C/D.
	// (read from SokuLib::KeyManager)
	size_t iA = -1;
	size_t iB = -1;
	size_t iC = -1;
	size_t iD = -1;

	bool virtualMacroBPrev = false;
	bool virtualMacroCPrev = false;
	int dxprevs[DISTINCTION_FRAMES_236_623];
};

typedef int VirtualInputMapKey;

static std::map<VirtualInputMapKey, VirtualInputState> virtualInputStates;

// This is only used in non-virtual-input mode
// one of these is stored per character.
struct ExtraInput {
	bool holdA;
	bool holdB;
	bool holdC;
	bool holdD;
	bool ignoreHoldA;
	bool ignoreHoldB;
	bool ignoreHoldC;
	bool ignoreHoldD;
};

// this is only used in non-VirtualInput mode.
// one of these is used per character

enum class InputAssociation {
	UNKNOWN,
	COMPUTER,
	KEYBOARD0,
	KEYBOARD1,
	JOY0,
	JOY1
};

static InputAssociation getInputAssociationForCharacterManager(SokuLib::CharacterManager &characterManager) {
	if (!characterManager.keyManager || !characterManager.keyManager->keymapManager) {
		return InputAssociation::COMPUTER;
	} else if (characterManager.keyManager->keymapManager->readInKeys) {
		return InputAssociation::UNKNOWN;
	} else switch (characterManager.keyManager->keymapManager->isPlayer) {
		case -1:
			if (characterManager.isRightPlayer)
				return InputAssociation::KEYBOARD1;
			else
				return InputAssociation::KEYBOARD0;
		case 0:
			return InputAssociation::JOY0;
		case 1:
			return InputAssociation::JOY1;
		default:
			return InputAssociation::UNKNOWN;
	}
}

static ExtraInput extraInput[2];
static ExtraInput extraInputGamepads[2];
static ExtraInput extraInputVirtualKeyboards[2];

static int sign(int x) {
	if (x == 0)
		return 0;
	if (x > 0) return 1;
	return -1;
}

// This function directly (!) triggers chord-input recognition, which normally
// is only triggered when the correct key sequence (e.g. 236B) is input.
// 
// This is called redundantly dozens of times in the battle update loop,
// because we weren't exactly sure when exactly was the right moment,
// so we opted for idempotent spamming.

__declspec(noinline) static void altInputPlayer(int slot) {

	// character manager and extra input for this slot.
	SokuLib::CharacterManager &characterManager = (slot == 0) ? battleMgr->leftCharacterManager : battleMgr->rightCharacterManager;

	if  (!characterManager.keyManager || !characterManager.keyManager->keymapManager)
		return;

	int facing = sign(characterManager.objectBase.direction);
	if (facing == 0)
		facing = 1;

	int hdir = sign(characterManager.keyMap.horizontalAxis);
	int vdir = sign(characterManager.keyMap.verticalAxis);

	// limit combo direction to non-diagonals
	if (vdir > 0 && hdir != 0)
		vdir = 0;
	if (vdir < 0 && hdir != 0)
		hdir = 0;

	ExtraInput &exinput = extraInput[characterManager.isRightPlayer ? 1 : 0];

	const bool select = characterManager.keyManager->keymapManager->select;

	// all this to distinguish hold from press...
	bool a = characterManager.keyMap.a;
	bool b = characterManager.keyMap.b;
	bool c = characterManager.keyMap.c;
	bool d = characterManager.keyMap.d;

	exinput.holdA = a;
	exinput.holdB = b;
	exinput.holdC = c;
	exinput.holdD = d;

	// a/b/c/d should be true only if pressed this frame.
	if (exinput.ignoreHoldA)
		a = false;
	if (exinput.ignoreHoldB)
		b = false;
	if (exinput.ignoreHoldC)
		c = false;
	if (exinput.ignoreHoldD)
		d = false;

	if (select) {
		// prevent jumping (needed for up-combo)
		if (characterManager.keyMap.verticalAxis < 0 && !d) {
			characterManager.keyMap.verticalAxis = 0;
		}
		if (hdir == 0 && vdir == 0) {
			if (a)
				characterManager.keyCombination._22a = true;
			if (b)
				characterManager.keyCombination._22b = true;
			if (c)
				characterManager.keyCombination._22c = true;
			if (d)
				characterManager.keyCombination._22d = true;
		} else if (vdir == 1) {
			if (a)
				characterManager.keyCombination._421a = true;
			if (b)
				characterManager.keyCombination._421b = true;
			if (c)
				characterManager.keyCombination._421c = true;
			if (d)
				characterManager.keyCombination._421d = true;
		} else if (vdir == -1) {
			if (a)
				characterManager.keyCombination._623a = true;
			if (b)
				characterManager.keyCombination._623b = true;
			if (c)
				characterManager.keyCombination._623c = true;
			if (d)
				characterManager.keyCombination._623d = true;
		} else if (hdir == 1 * facing) {
			if (a)
				characterManager.keyCombination._236a = true;
			if (b)
				characterManager.keyCombination._236b = true;
			if (c)
				characterManager.keyCombination._236c = true;
			if (d)
				characterManager.keyCombination._236d = true;
		} else if (hdir == -1 * facing) {
			if (a)
				characterManager.keyCombination._214a = true;
			if (b)
				characterManager.keyCombination._214b = true;
			if (c)
				characterManager.keyCombination._214c = true;
			if (d)
				characterManager.keyCombination._214d = true;
		}
	}
}

// returns true if macro input should be performable at this time. (i.e. when in battle.)
// Note that this only physical input devices; received chord inputs from replays or network will
// still function.
static inline bool macroInputEnabled() {
	// (thanks PinkySmile!)
	// 
	// Disable outside of VSPlayer and VSCom
	switch(SokuLib::sceneId) {
	case SokuLib::SCENE_BATTLE:
		return true;
	case SokuLib::SCENE_BATTLECL:
	case SokuLib::SCENE_BATTLESV:
		return netEnable;
	default:
		return false;
	}

	// Also disable in replays
	if (SokuLib::subMode == SokuLib::BATTLE_SUBMODE_REPLAY)
		return false;

	return true;
}

__declspec(noinline) static void altInput()
{
	 if (battleMgr && !gVirtualInput) {
		 altInputPlayer(0);
		 altInputPlayer(1);
	 }
}

void __fastcall MyChordInputDetect(unsigned int param_1,unsigned int param_2,int param_1_00,char param_2_00,char param_3)
{
	altInput();

	cidfn(param_1, param_2, param_1_00, param_2_00, param_3);
}

// communicates some game state to the virtual input methods.
// This isn't thread-safe, but it doesn't need to be; data errors in the
// direct input thread should be handed gracefully. And these values are all individually atomic anyway.
// 
// figures out which controllers are facing left vs which are facing right.
// also figures out the button mapping for each controller, reading from SokuLib::KeyManager
static void setVirtualInputStateGameData(int slot) {
	if (battleMgr) {
		 SokuLib::CharacterManager &characterManager = (slot == 0) ? battleMgr->leftCharacterManager : battleMgr->rightCharacterManager;
		 bool facingRight = characterManager.objectBase.direction == SokuLib::Direction::RIGHT;
		 switch (InputAssociation ia = getInputAssociationForCharacterManager(characterManager)) {
		 case InputAssociation::KEYBOARD0:
		 case InputAssociation::KEYBOARD1:
			keyboardstate[slot].facingRight = facingRight;
			keyboardstate[slot].iUp = characterManager.keyManager->keymapManager->bindingUp;
			keyboardstate[slot].iDown = characterManager.keyManager->keymapManager->bindingDown;
			keyboardstate[slot].iLeft = characterManager.keyManager->keymapManager->bindingLeft;
			keyboardstate[slot].iRight = characterManager.keyManager->keymapManager->bindingRight;
			keyboardstate[slot].iA = characterManager.keyManager->keymapManager->bindingA;
			keyboardstate[slot].iB = characterManager.keyManager->keymapManager->bindingB;
			keyboardstate[slot].iC = characterManager.keyManager->keymapManager->bindingC;
			keyboardstate[slot].iD = characterManager.keyManager->keymapManager->bindingD;
			break;
		 case InputAssociation::JOY0:
		 case InputAssociation::JOY1: {
			int i = (ia == InputAssociation::JOY0) ? 0 : 1;
			virtualInputStates[i].facingRight = facingRight;
			virtualInputStates[i].iA = characterManager.keyManager->keymapManager->bindingA;
			virtualInputStates[i].iB = characterManager.keyManager->keymapManager->bindingB;
			virtualInputStates[i].iC = characterManager.keyManager->keymapManager->bindingC;
			virtualInputStates[i].iD = characterManager.keyManager->keymapManager->bindingD;
			break;
		 }
		 default:
			return;
		 }
	}
}

static int (SokuLib::BattleManager::*og_BattleManagerOnProcess)() = nullptr;
static int (SokuLib::BattleManager::*og_BattleManagerStoryOnProcess)() = nullptr;
static int (SokuLib::BattleManager::*og_BattleManagerArcadeOnProcess)() = nullptr;

static void setMacroBindings(SokuLib::CharacterManager &cm) {
	switch (getInputAssociationForCharacterManager(cm)) {
	case InputAssociation::KEYBOARD0:
		 cm.keyManager->keymapManager->bindingSelect = keyboardstate[0].iMacro;
		 break;
	case InputAssociation::KEYBOARD1:
		 cm.keyManager->keymapManager->bindingSelect = keyboardstate[1].iMacro;
		 break;
	case InputAssociation::JOY0:
	case InputAssociation::JOY1:
		 cm.keyManager->keymapManager->bindingSelect = RGBBUTTON_SELECT_MAP;
		 break;
	default:
		 break;
	}
}

static void interceptBattleOnProcess() {
	inputCounter = 0;

	setMacroBindings(battleMgr->leftCharacterManager);
	setMacroBindings(battleMgr->rightCharacterManager);

	for (size_t i = 0; i <= 1; ++i) {
		 extraInput[i].ignoreHoldA = extraInput[i].holdA;
		 extraInput[i].ignoreHoldB = extraInput[i].holdB;
		 extraInput[i].ignoreHoldC = extraInput[i].holdC;
		 extraInput[i].ignoreHoldD = extraInput[i].holdD;
	}

	if (gVirtualInput) {
		 setVirtualInputStateGameData(0);
		 setVirtualInputStateGameData(1);

		 // set newFrame flag to allow next virtual input sequence iterand to be performed.
		 // remove any devices that haven't been seen in >= 200 frames.
		 keyboardstate[0].newFrame = 1;
		 keyboardstate[1].newFrame = 1;
		 for (auto it = virtualInputStates.begin(); it != virtualInputStates.end();) {
			if (it->second.newFrame++ >= 200) {
				it = virtualInputStates.erase(it);
			} else {
				++it;
			}
		 }
	}
}

static int __fastcall BattleOnProcess(SokuLib::BattleManager *This)
{
	battleMgr = This;
	interceptBattleOnProcess();
	int ret = (This->*og_BattleManagerOnProcess)();
	battleMgr = nullptr;
	return ret;
}

static int __fastcall BattleOnProcessStory(SokuLib::BattleManager *This) {
	battleMgr = This;
	interceptBattleOnProcess();
	int ret = (This->*og_BattleManagerStoryOnProcess)();
	battleMgr = nullptr;
	return ret;
}

static int __fastcall BattleOnProcessArcade(SokuLib::BattleManager *This) {
	battleMgr = This;
	interceptBattleOnProcess();
	int ret = (This->*og_BattleManagerArcadeOnProcess)();
	battleMgr = nullptr;
	return ret;
}

#define AXIS_ORTHO 1000
#define AXIS_DIAG 707
#define BUTTON_PRESSED 0x80

//replay_assemble_outs 0042d960
//replay_write_x 0042d770
//replay_write 0042cf90

struct KeyboardVirtualArgs {
	char *keystate;
	KeyboardInputState &is;
	ExtraInput &exin;

	typedef KeyboardInputState::InputCB InputCB;
	typedef char* InputCBArg;

	// returns argument for queued input, and zeroes out relevant muddling input.
	char *prepareQueuedInput() const {
		keystate[is.iUp]    &= 0x7F;
		keystate[is.iDown]  &= 0x7F;
		keystate[is.iLeft]  &= 0x7F;
		keystate[is.iRight] &= 0x7F;
		keystate[is.iA]     &= 0x7F;
		keystate[is.iB]     &= 0x7F;
		keystate[is.iC]     &= 0x7F;
		keystate[is.iD]     &= 0x7F;

		return keystate;
	}

	KeyboardInputState::InputCB setDirCB(int dx, int dy) const {
		dx = sign(dx);
		dy = sign(dy);
		const int iUp = is.iUp;
		const int iDown = is.iDown;
		const int iLeft = is.iLeft;
		const int iRight = is.iRight;
		return [dx, dy, iUp, iDown, iLeft, iRight](char* keystate) {
			if (dx < 0) keystate[iLeft]  |= 0x80;
			if (dx > 0) keystate[iRight] |= 0x80;
			if (dy < 0) keystate[iUp]    |= 0x80;
			if (dy > 0) keystate[iDown]  |= 0x80;
		};
	}

	KeyboardInputState::InputCB setButtonCB(uint8_t i) const {
		return [i](char* keystate) { keystate[i] |= BUTTON_PRESSED; };
	}

	std::pair<int, int> getDirectionalInput() const {
		return {!!keystate[is.iRight] - !!keystate[is.iLeft], !!keystate[is.iDown] - !!keystate[is.iUp]};
	}

	void preventJumping() const {
		if (!keystate[is.iD]) {
			 keystate[is.iUp] &= 0x7f;
		}
	}

	bool getMacroInput() const {
		if (is.enabled && keystate[is.iMacro]) return true;

		return false;
	}

	bool getMacroBInput(bool held=false) const {
		if (exin.ignoreHoldB && !held) return false;
		return !!keystate[is.iB];
	}

	bool getMacroCInput(bool held=false) const {
		if (exin.ignoreHoldC && !held) return false;
		return !!keystate[is.iC];
	}

	void calcHold() const {
		exin.holdA = !!keystate[is.iA];
		exin.holdB = !!keystate[is.iB];
		exin.holdC = !!keystate[is.iC];
		exin.holdD = !!keystate[is.iD];
	}
};

struct GamepadVirtualArgs {
	DIJOYSTATE *joystate;
	size_t slot;
	VirtualInputState &is;
	ExtraInput &exin;

	static const int ySensitivity = 500;
	static const int xSensitivity = 500;

	typedef VirtualInputState::InputCB InputCB;
	typedef DIJOYSTATE* InputCBArg;

	// returns argument for queued input, and zeroes out relevant muddling input.
	DIJOYSTATE* prepareQueuedInput() const {
		if (is.iA < MAX_RGBBUTTON_INDEX) joystate->rgbButtons[is.iA] &= 0x7F;
		if (is.iB < MAX_RGBBUTTON_INDEX) joystate->rgbButtons[is.iB] &= 0x7F;
		if (is.iC < MAX_RGBBUTTON_INDEX) joystate->rgbButtons[is.iC] &= 0x7F;
		if (is.iD < MAX_RGBBUTTON_INDEX) joystate->rgbButtons[is.iD] &= 0x7F;

		joystate->lX = 0;
		joystate->lY = 0;

		return joystate;
	}

	VirtualInputState::InputCB setDirCB(int dx, int dy) const {
		if (dx > 0 && dy > 0)
			 return [](DIJOYSTATE *joystate) { joystate->lX = joystate->lY = AXIS_DIAG; };
		if (dx < 0 && dy > 0)
			 return [](DIJOYSTATE *joystate) {
				 joystate->lX = -AXIS_DIAG;
				 joystate->lY = AXIS_DIAG;
			 };
		if (dx > 0)
			 return [](DIJOYSTATE *joystate) { joystate->lX = AXIS_ORTHO; };
		if (dx < 0)
			 return [](DIJOYSTATE *joystate) { joystate->lX = -AXIS_ORTHO; };
		if (dy > 0)
			 return [](DIJOYSTATE *joystate) { joystate->lY = AXIS_ORTHO; };
		if (dy < 0)
			 return [](DIJOYSTATE *joystate) { joystate->lY = -AXIS_ORTHO; };

		return [](DIJOYSTATE *joystate) {};
	}

	VirtualInputState::InputCB setButtonCB(size_t i) const {
		if (i < MAX_RGBBUTTON_INDEX)
			 return [i](DIJOYSTATE *joystate) { joystate->rgbButtons[i] |= BUTTON_PRESSED; };

		// (paranoia)
		return [](DIJOYSTATE *joystate) {};
	}

	std::pair<int, int> getDirectionalInput() const {
		return {
			(joystate->lX > xSensitivity) - (joystate->lX <= -xSensitivity),
			(joystate->lY > ySensitivity) - (joystate->lY <= -ySensitivity)
		};
	}

	void preventJumping() const {
		if (is.iD < MAX_RGBBUTTON_INDEX && !joystate->rgbButtons[is.iD]) {
			if (joystate->lY < 0) {
				joystate->lY = 0;
			}
		}
	}

	bool getMacroInput() const {
		return joystate->lZ > gTriggersThreshold || joystate->lZ < -gTriggersThreshold;
	}

	bool getMacroBInput(bool held=false) const {
		if (exin.ignoreHoldB && !held) return false;
		return is.iB < MAX_RGBBUTTON_INDEX ? !!joystate->rgbButtons[is.iB] : false;
	}

	bool getMacroCInput(bool held=false) const {
		if (exin.ignoreHoldC && !held) return false;
		return is.iB < MAX_RGBBUTTON_INDEX ? !!joystate->rgbButtons[is.iB] : false;
	}

	void calcHold() const {
		exin.holdA = is.iA < MAX_RGBBUTTON_INDEX && !!joystate->rgbButtons[is.iA];
		exin.holdB = is.iA < MAX_RGBBUTTON_INDEX && !!joystate->rgbButtons[is.iB];
		exin.holdC = is.iA < MAX_RGBBUTTON_INDEX && !!joystate->rgbButtons[is.iC];
		exin.holdD = is.iA < MAX_RGBBUTTON_INDEX && !!joystate->rgbButtons[is.iD];
	}
};

template<typename C>
static void virtualChordInput(const C& args) {
	if (!macroInputEnabled())
		return;

	const size_t iA = args.is.iA;
	const size_t iB = args.is.iB;
	const size_t iC = args.is.iC;
	const size_t iD = args.is.iD;

	const bool facingRight = args.is.facingRight;
	const int facing = facingRight ? 1 : -1;

	args.calcHold();

	// if there is no queued input yet, check for macro input.
	if (args.is.queuedInput.empty() && args.is.newFrame) {
		
		// macros can either be A or B, depending on buttons.
		bool macroB = args.getMacroBInput();
		bool macroC = args.getMacroCInput();
		std::pair<int, int> dxy = args.getDirectionalInput();
		int dx = dxy.first;
		int dy = dxy.second;

		auto multiInputCB = [](C::InputCB a, C::InputCB b) -> C::InputCB {
			return [a, b](C::InputCBArg arg) {
				a(arg);
				b(arg);
			};
		};

		// returns a value in the range 0-11 inclusive
		auto get623WaitTime = [&args](int facing) -> int {
			#if 0
			for (size_t i = DISTINCTION_FRAMES_236_623; i-- > 0;) {
				if (args.is.dxprevs[i] == facing)
					return i + 1;
			}
			#else
			for (size_t i = DISTINCTION_FRAMES_236_623; i-- > 1;) {
				if (args.is.dxprevs[i] == facing && args.is.dxprevs[i-1] != facing)
					return i + 1; // FIXME: do we really need to do i+1? will i suffice?
			}
			#endif
			return 0;
		};

		if ((macroB || macroC) && args.getMacroInput()) {
			// okay girls and boys, we're doing the chord input so let's figure out which one it is exactly and then
			// queue up the input.

			int macroButton = macroB ? iB : iC;

			if (macroC && !args.is.virtualMacroCPrev)
				macroButton = iC;

			bool macroBCPressed = (macroB && !args.is.virtualMacroBPrev) || (macroC && !args.is.virtualMacroCPrev);

			args.is.virtualMacroBPrev = macroB;
			args.is.virtualMacroCPrev = macroC;

			if (macroBCPressed) {
				if (dx == 0 && dy == 0) {
					// 22
					args.is.queuedInput.emplace(args.setDirCB(0, 1));
					args.is.queuedInput.emplace(args.setDirCB(0, 0));
					args.is.queuedInput.emplace(args.setDirCB(0, 1));
					args.is.queuedInput.emplace(args.setButtonCB(macroButton));
				} else if (dy == -1) {
					// 623
					args.is.queuedInput.emplace(args.setDirCB(facing, 0));
					args.is.queuedInput.emplace(args.setDirCB(0, 1));
					args.is.queuedInput.emplace(args.setDirCB(facing, 1));
					args.is.queuedInput.emplace(args.setButtonCB(macroButton));
				} else if (dx == facing) {
					// 236

					// the reason we need to wait is in order to prevent a dash (66) input
					// TODO: this can be reduced in some circumstances perhaps
					const size_t wait_time = get623WaitTime(facing);
					for (size_t i = 0; i < wait_time; ++i) {
						args.is.queuedInput.emplace(args.setDirCB(0, 0));
					}

					args.is.queuedInput.emplace(args.setButtonCB(args.is.iD)); // this cancels 623B/C buffering, ensuring we do 236B/C instead.
					args.is.queuedInput.emplace(args.setDirCB(0, 1));
					args.is.queuedInput.emplace(args.setDirCB(facing, 1));
					args.is.queuedInput.emplace(multiInputCB(args.setDirCB(facing, 0), args.setButtonCB(macroButton)));
				} else if (dx == -facing) {
					// 214
					// the reason we need to wait is in order to prevent a backward dash (44) input
					// TODO: can this be reduced in some circumstances?
					const size_t wait_time = get623WaitTime(-facing);
					for (size_t i = 0; i < wait_time; ++i) {
						args.is.queuedInput.emplace(args.setDirCB(0, 0));
					}

					args.is.queuedInput.emplace(args.setButtonCB(args.is.iD)); // this cancels 421B/C buffering, ensuring we do 214B/C instead.
					args.is.queuedInput.emplace(args.setDirCB(0, 1));
					args.is.queuedInput.emplace(args.setDirCB(-facing, 1));
					args.is.queuedInput.emplace(multiInputCB(args.setDirCB(-facing, 0), args.setButtonCB(macroButton)));
				} else if (dy == 1) {
					// 421
					args.is.queuedInput.emplace(args.setDirCB(-facing, 0));
					args.is.queuedInput.emplace(args.setDirCB(0, 1));
					args.is.queuedInput.emplace(args.setDirCB(-facing, 1));
					args.is.queuedInput.emplace(args.setButtonCB(macroButton));
				}
			}
		} else {
			args.is.virtualMacroBPrev = false;
			args.is.virtualMacroCPrev = false;
		}

		// record dx for the dxprevs buffer
		for (size_t i = 0; i < DISTINCTION_FRAMES_236_623 - 1; ++i) {
			args.is.dxprevs[i] = args.is.dxprevs[i + 1];
		}
		args.is.dxprevs[DISTINCTION_FRAMES_236_623 - 1] = dx;

		// prevent jumping if macro button is held unless the 'fly' button is also held.
		if (args.getMacroInput()) args.preventJumping();
	}

	// if there is queued input, perform that.
	// (That includes input that was queued this frame, just now, above.
	if (!args.is.queuedInput.empty()) {

		// zero out muddling input (e.g. joystick and face buttons.), and
		// perform queued input
		args.is.queuedInput.front()(args.prepareQueuedInput());

		// if no frame was skipped, pop this input and move on to the next.
		if (args.is.newFrame) {
			args.is.queuedInput.pop();
		}
	}

	args.exin.ignoreHoldA = args.exin.holdA;
	args.exin.ignoreHoldB = args.exin.holdB;
	args.exin.ignoreHoldC = args.exin.holdC;
	args.exin.ignoreHoldD = args.exin.holdD;

	// mark as processed so we don't skip a queued input if a frame is skipped.
	args.is.newFrame = 0;
}

static void interceptDeviceStateKeyboard(char *keystate) {
	for (size_t i = 0; i <= 1; ++i) {
		KeyboardInputState &kis = keyboardstate[i];
		if (kis.enabled && macroInputEnabled()) {
			if (gVirtualInput) {
				virtualChordInput<KeyboardVirtualArgs>({keystate, kis, extraInputVirtualKeyboards[i]});
			}
		}
	}
}

static void interceptDeviceStateGamepad(DIJOYSTATE* joystate) {

	size_t slot = inputCounter++;

	printf("Slot %d: %d %d\n", slot, joystate->rgbButtons[0], joystate->lZ);

	if (gVirtualInput && gTriggersEnabled && slot < 2) {
		virtualChordInput<GamepadVirtualArgs>({joystate, slot, virtualInputStates[slot], extraInputGamepads[slot]});
	}

	else if (!gVirtualInput) {
		if (gTriggersEnabled) {
			 joystate->rgbButtons[RGBBUTTON_SELECT_MAP] = 0x0;

			 if (joystate->lZ < -gTriggersThreshold || joystate->lZ > gTriggersThreshold) {
				joystate->rgbButtons[RGBBUTTON_SELECT_MAP] |= 0x80;
			 }
		}
	}
}

static HRESULT WINAPI myGetDeviceState(LPVOID IDirectInputDevice8W, DWORD cbData, LPVOID lpvData) {

	HRESULT retValue = oldGetDeviceState(IDirectInputDevice8W, cbData, lpvData);

	if (retValue != DI_OK || cbData == sizeof(DIJOYSTATE2)) {
		return retValue;
	}

	// reset state in menus
	switch (SokuLib::sceneId) {
	case SokuLib::SCENE_BATTLE:
	case SokuLib::SCENE_BATTLECL:
	case SokuLib::SCENE_BATTLESV:
	case SokuLib::SCENE_BATTLEWATCH:
		break;
	default:
		virtualInputStates.clear();
		memset(&extraInput, 0, 2 * sizeof(extraInput));
		break;
	}

	if (cbData == sizeof(DIJOYSTATE)) {
		interceptDeviceStateGamepad((DIJOYSTATE *)lpvData);
	} else if (cbData == 0x100) {
		interceptDeviceStateKeyboard((char *)lpvData);
	}

	return retValue;
}

static void DummyDirectInput() {
	LPDIRECTINPUT8 pDI;
	DirectInput8Create(GetModuleHandle(NULL), DIRECTINPUT_VERSION, IID_IDirectInput8, (void **)&pDI, NULL);

	LPDIRECTINPUTDEVICE8 pKeyboard;
	pDI->CreateDevice(GUID_SysKeyboard, &pKeyboard, NULL);

	LPVOID ptrGetDeviceState = *(((LPVOID *)(*(LPVOID *)(pKeyboard))) + 9);

	oldGetDeviceState = (HRESULT(WINAPI *)(LPVOID, DWORD, LPVOID))ptrGetDeviceState;

	DetourRestoreAfterWith();

	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());
	DetourAttach(&(PVOID &)oldGetDeviceState, myGetDeviceState);
	DetourTransactionCommit();

	pKeyboard->Release();
	pDI->Release();
}

static void ChordInputDetectDetour()
{
    DetourTransactionBegin();
    DetourUpdateThread(GetCurrentThread());
    DetourAttach(&(PVOID&)cidfn, MyChordInputDetect);
    DetourTransactionCommit();
}

typedef int(__stdcall *recvfromFn)(SOCKET s, char *buf, int len, int flags, sockaddr *from, int *fromlen);
static recvfromFn Original_recvfrom = NULL;

typedef int(__stdcall *sendtoFn)(SOCKET s, char *buf, int len, int flags, sockaddr *to, int tolen);
static sendtoFn Original_sendto = NULL;

#define NETMON 0

static const char *MARKER = "\x07" "CHORDMACRO" "\x05\x07\x05";

static int __stdcall Hooksendto(SOCKET s, char *buf, int len, int flags, sockaddr *to, int tolen) {

	SokuLib::Packet& packet = *reinterpret_cast<SokuLib::Packet *>(buf);

	int n;

	bool game = (packet.type == SokuLib::HOST_GAME || packet.type == SokuLib::CLIENT_GAME);

	if (game && (packet.game.event.type == SokuLib::GAME_LOADED || packet.game.event.type == SokuLib::GAME_LOADED_ACK)) {
		if (len > 0) {
			 char *buff = new char[len + strlen(MARKER)];
			 memcpy(buff, buf, len);
			 memcpy(buff + len, MARKER, strlen(MARKER));

			 printf("SEND: APPROVE CHORDMACRO FOR PEER\n", len + strlen(MARKER));
			 fflush(stdout);

			 n = Original_sendto(s, buff, len + strlen(MARKER), flags, to, tolen);
			 delete[] buff;
			 return n;
		}
	}

	#if NETMON
	std::cout << "> ";
	SokuLib::displayPacketContent(std::cout, packet);
	std::cout << "\n";
	#endif

	n = Original_sendto(s, buf, len, flags, to, tolen);
	if (n <= 0) {
		return n;
	}
	
	return n;
}

static const void* memmem(const void* haystack, size_t h, const void* needle, size_t n) {
	if (n > h) {
		return NULL;
	}

	for (size_t i = 0; i <= h - n; ++i) {
		if (memcmp((const char*)haystack + i, needle, n) == 0) {
			 return (const char*)haystack + i;
		}
	}

	return NULL;
}

static int __stdcall Hookrecvfrom(SOCKET s, char *buf, int len, int flags, sockaddr *from, int *fromlen) {
	int n = Original_recvfrom(s, buf, len, flags, from, fromlen);

	SokuLib::Packet &packet = *reinterpret_cast<SokuLib::Packet *>(buf);

	if ((packet.type == SokuLib::HOST_GAME || packet.type == SokuLib::CLIENT_GAME)
		&& (packet.game.event.type == SokuLib::GAME_LOADED || packet.game.event.type == SokuLib::GAME_LOADED_ACK)) {

		if (memmem(buf, n, MARKER, strlen(MARKER))) {
			 printf("NETPLAY: PEER APPROVES CHORDMACRO\n");
			 fflush(stdout);
			 netEnable = true;
		} else {
			 printf("NETPLAY: PEER DENIES CHORDMACRO\n");
			 fflush(stdout);
			 netEnable = false;
		}
	}

	#if NETMON
	std::cout << "< ";
	SokuLib::displayPacketContent(std::cout, packet);
	std::cout << "\n";
	#endif

	return n;
}

// FUN_0040a1a0(KeymapManager*) // read inputs from devices
typedef void(__fastcall *ReadInputs)(SokuLib::CharacterManager *cm);
ReadInputs readInputs = reinterpret_cast<ReadInputs>((uintptr_t)(0x0046C8E0));

typedef void(__fastcall *ReadInputsFromDevice)(SokuLib::KeymapManager *km);
ReadInputsFromDevice readInputsFromDevice = reinterpret_cast<ReadInputsFromDevice>((uintptr_t)(0x0040a1a0));

static SokuLib::CharacterManager *localcm = nullptr;

static void __fastcall readInputsHook(SokuLib::CharacterManager *cm) {
	localcm = cm;
	readInputs(cm);
	localcm = nullptr;
};

static void __fastcall readInputsFromDeviceHook(SokuLib::KeymapManager *km) {

	if (!localcm && netEnable) {
		// netplay input select map
		if (km->isPlayer == -1) {
			 if (keyboardstate[0].enabled) {
				km->bindingSelect = keyboardstate[0].iMacro;
			 }
		} else if (km->isPlayer == 0 || km->isPlayer == 1) {
			 if (gTriggersEnabled) {
				km->bindingSelect = RGBBUTTON_SELECT_MAP;
			 }
		}
	}

	readInputsFromDevice(km);
};

extern "C" {
	__declspec(dllexport) bool CheckVersion(const BYTE hash[16]) {
		return true;
	}

	__declspec(dllexport) bool Initialize(HMODULE hMyModule, HMODULE hParentModule) {
		::GetModuleFileName(hMyModule, s_profilePath, 1024);
		::PathRemoveFileSpec(s_profilePath);
		::PathAppend(s_profilePath, "ChordMacro.ini");

		gVirtualInput = ::GetPrivateProfileInt("ChordMacro", "VirtualInput", 1, s_profilePath) != 0;
		gDebug = ::GetPrivateProfileInt("ChordMacro", "Debug", 0, s_profilePath) != 0;
		gTriggersEnabled = ::GetPrivateProfileInt("GamepadInput", "TriggerEnabled", 0, s_profilePath) != 0;
		gTriggersThreshold = ::GetPrivateProfileInt("GamepadInput", "TriggerThreshold", 200, s_profilePath);

		keyboardstate[0].enabled = ::GetPrivateProfileInt("KeyboardInput", "MacroKey", -1, s_profilePath) >= 0;
		keyboardstate[0].iMacro = ::GetPrivateProfileInt("KeyboardInput", "MacroKey", DIK_LSHIFT, s_profilePath);
		keyboardstate[1].enabled = ::GetPrivateProfileInt("KeyboardInput", "MacroKey", -1, s_profilePath) >= 0;
		keyboardstate[1].iMacro = ::GetPrivateProfileInt("KeyboardInputP2", "MacroKey", DIK_RSHIFT, s_profilePath);

		// load DirectInput library since it won't be otherwise loaded yet
    
		if (!LoadLibraryExW(L"dinput8.dll", NULL, 0)) {
			return false;
		}

		DWORD old;
		VirtualProtect((PVOID)RDATA_SECTION_OFFSET, RDATA_SECTION_SIZE, PAGE_EXECUTE_WRITECOPY, &old);
		og_BattleManagerOnProcess = SokuLib::TamperDword(&SokuLib::VTable_BattleManager.onProcess, BattleOnProcess);
		og_BattleManagerStoryOnProcess = SokuLib::TamperDword(&VTable_BattleManagerStory.onProcess, BattleOnProcessStory);
		og_BattleManagerArcadeOnProcess = SokuLib::TamperDword(&VTable_BattleManagerArcade.onProcess, BattleOnProcessArcade);
		VirtualProtect((PVOID)RDATA_SECTION_OFFSET, RDATA_SECTION_SIZE, old, &old);

		// enable sending SELECT over netplay.
		char *patchaddr3 = (char *)0x004559d5;
		char *patchaddr4 = (char *)0x00454cc0;
		char *patchaddr5 = (char *)0x00454cd5;

		VirtualProtect(patchaddr3, 5, PAGE_EXECUTE_WRITECOPY, &old);
		patchaddr3[3] = 8|3;
		VirtualProtect(patchaddr3, 5, old, &old);
		
		VirtualProtect(patchaddr4, 6, PAGE_EXECUTE_WRITECOPY, &old);
		patchaddr4[3] = 8|3;
		patchaddr5[4] = 8|3;
		VirtualProtect(patchaddr4, 6, old, &old);
   
		Original_recvfrom = (recvfromFn)PatchMan::HookNear(0x41DAE5, (DWORD)Hookrecvfrom);
		Original_sendto = (sendtoFn)PatchMan::HookNear(0x4171CD, (DWORD)Hooksendto);
		{
			DetourTransactionBegin();
			DetourUpdateThread(GetCurrentThread());
			DetourAttach(&(PVOID &)readInputs, readInputsHook);
			DetourAttach(&(PVOID &)readInputsFromDevice, readInputsFromDeviceHook);
			DetourTransactionCommit();
		}
		ChordInputDetectDetour();
		DummyDirectInput();


		return true;
	}
}
