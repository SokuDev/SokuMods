#include <windows.h>
#include <initguid.h>
#include <detours.h>
#include <dinput.h>
#include <shlwapi.h>

const BYTE TARGET_HASH[16] = {0xdf, 0x35, 0xd1, 0xfb, 0xc7, 0xb5, 0x83, 0x31, 0x7a, 0xda, 0xbe, 0x8c, 0xd9, 0xf5, 0x3b, 0x2e};

static char s_profilePath[1024 + MAX_PATH];

bool gDPadEnabled;
bool gTriggersEnabled;
int gTriggersThreshold;

HRESULT(WINAPI *oldGetDeviceState)(LPVOID IDirectInputDevice8W, DWORD cbData, LPVOID lpvData) = NULL;

HRESULT WINAPI myGetDeviceState(LPVOID IDirectInputDevice8W, DWORD cbData, LPVOID lpvData) {
	HRESULT retValue = oldGetDeviceState(IDirectInputDevice8W, cbData, lpvData);

	if (retValue != DI_OK || !(cbData == sizeof(DIJOYSTATE) || cbData == sizeof(DIJOYSTATE2))) {
		return retValue;
	}

	DIJOYSTATE *joystate = (DIJOYSTATE *)lpvData;

	if (gDPadEnabled) {
		for (int i = 0; i < sizeof(joystate->rgdwPOV) / sizeof(joystate->rgdwPOV[0]); i++) {
			if (LOWORD(joystate->rgdwPOV[i]) != 0xFFFF) {
				int angle = int(joystate->rgdwPOV[i] / 100.0f / 45.0f + 0.5f);

				// this giant switch looks ugly but its probably more efficient than calculating
				switch (angle) {
				case 0:
				case 8:
					joystate->lX = 0;
					joystate->lY = -1000;
					break;
				case 1:
					joystate->lX = 707;
					joystate->lY = -707;
					break;
				case 2:
					joystate->lX = 1000;
					joystate->lY = 0;
					break;
				case 3:
					joystate->lX = 707;
					joystate->lY = 707;
					break;
				case 4:
					joystate->lX = 0;
					joystate->lY = 1000;
					break;
				case 5:
					joystate->lX = -707;
					joystate->lY = 707;
					break;
				case 6:
					joystate->lX = -1000;
					joystate->lY = 0;
					break;
				case 7:
					joystate->lX = -707;
					joystate->lY = -707;
					break;
				default:
					break;
				}
			}
		}
	}

	if (gTriggersEnabled) {
		if (joystate->lZ < -gTriggersThreshold) {
			joystate->rgbButtons[14] = 0x80;
		}

		if (joystate->lZ > gTriggersThreshold) {
			joystate->rgbButtons[15] = 0x80;
		}
	}

	return retValue;
}

void DummyDirectInput() {
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

extern "C" {
__declspec(dllexport) bool CheckVersion(const BYTE hash[16]) {
	return ::memcmp(TARGET_HASH, hash, sizeof TARGET_HASH) == 0;
}

__declspec(dllexport) bool Initialize(HMODULE hMyModule, HMODULE hParentModule) {
	::GetModuleFileName(hMyModule, s_profilePath, 1024);
	::PathRemoveFileSpec(s_profilePath);
	::PathAppend(s_profilePath, "DPadFix.ini");

	gDPadEnabled = ::GetPrivateProfileInt("DPadToAnalog", "Enabled", 0, s_profilePath) != 0;
	gTriggersEnabled = ::GetPrivateProfileInt("TriggersToButtons", "Enabled", 0, s_profilePath) != 0;
	gTriggersThreshold = ::GetPrivateProfileInt("TriggersToButtons", "Threshold", 200, s_profilePath);

	// load DirectInput library since it won't be otherwise loaded yet
	if (!LoadLibrary("dinput8.dll")) {
		return true;
	}

	DummyDirectInput();

	return true;
}
}

// Code References:
// https://github.com/sbobovyc/WinHookInject/blob/master/VS12/WinHookInject/injectDI/dllmain.cpp
// https://github.com/thpatch/thcrap/blob/6ff9aba4b7fe179cfa1f946c1bd51043062efc0c/thcrap_tsa/src/input.cpp
