#include "joystick.h"

#include "stdafx.h"

int Joystick::CreateDIObject() {
	Joystick::result = DirectInput8Create(GetModuleHandle(NULL), DIRECTINPUT_VERSION, IID_IDirectInput8, (void **)&lpDIObject, NULL);
	if (FAILED(result)) {
		return 1;
	}
	return 0;
}

// Enumeration callback
BOOL CALLBACK Joystick::CreateDeviceCallback(LPCDIDEVICEINSTANCE instance, LPVOID reference) {
	return ((Joystick *)reference)->CreateDeviceCallback_impl(instance);
}

BOOL Joystick::CreateDeviceCallback_impl(LPCDIDEVICEINSTANCE instance) {
	HRESULT result;
	result = lpDIObject->CreateDevice(instance->guidInstance, &lpDIJoypad, NULL);

	if (FAILED(result)) {
		return DIENUM_CONTINUE;
	}

	return DIENUM_STOP;
}

BOOL CALLBACK SetGameControllerAxesRanges(LPCDIDEVICEOBJECTINSTANCE devObjInst, LPVOID pvRef) {
	LPDIRECTINPUTDEVICE8 gameController = (LPDIRECTINPUTDEVICE8)pvRef;
	gameController->Unacquire();

	DIPROPRANGE gameControllerRange;

	gameControllerRange.lMin = -1000;
	gameControllerRange.lMax = 1000;

	// set the size of the structure
	gameControllerRange.diph.dwSize = sizeof(DIPROPRANGE);
	gameControllerRange.diph.dwHeaderSize = sizeof(DIPROPHEADER);

	// set the object that we want to change
	gameControllerRange.diph.dwHow = DIPH_BYID;
	gameControllerRange.diph.dwObj = devObjInst->dwType;

	// now set the range for the axis
	if (FAILED(gameController->SetProperty(DIPROP_RANGE, &gameControllerRange.diph))) {
		return DIENUM_STOP;
	}

	return DIENUM_CONTINUE;
}

int Joystick::getDIJoypad() {
	result = lpDIObject->EnumDevices(DI8DEVCLASS_GAMECTRL, CreateDeviceCallback, (LPVOID)this, DIEDFL_ATTACHEDONLY);

	if (FAILED(result)) {
		// Possible errors: DIERR_INVALIDPARAM, DIERR_NOTINITIALIZED
		Finalize();
		return 5;
	}

	if (lpDIJoypad == NULL) {
		std::cout << "No joystick." << std::endl;
		return 10;
	}

	result = lpDIJoypad->SetDataFormat(&c_dfDIJoystick2);

	if (FAILED(result)) {
		// Possible errors: DIERR_ACQUIRED, DIERR_INVALIDPARAM, DIERR_NOTINITIALIZED
		Finalize();
		return 6;
	}

	if (lpDIJoypad) {
		lpDIJoypad->EnumObjects(SetGameControllerAxesRanges, lpDIJoypad, DIDFT_AXIS);
		lpDIJoypad->Acquire();
	} else {
		return 11;
	}

	std::cout << "Joystick acquired!" << std::endl;

	return 0;
}

int Joystick::getJoypadInputs() {
	result = lpDIJoypad->GetDeviceState(sizeof(joypadBuffer), (LPVOID)&joypadBuffer);

	if (FAILED(result)) {
		// Possible errors: DIERR_INPUTLOST, DIERR_INVALIDPARAM, DIERR_NOTACQUIRED, DIERR_NOTINITIALIZED, E_PENDING
		return 8;
	}
}

void Joystick::Finalize(void) {
	if (lpDIObject) {
		if (lpDIJoypad) {
			lpDIJoypad->Unacquire();
			lpDIJoypad->Release();
			lpDIJoypad = NULL;
		}

		lpDIObject->Release();
		lpDIObject = NULL;
	}
}