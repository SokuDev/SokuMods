#pragma once
#define DIRECTINPUT_VERSION 0x0800
#include <initguid.h>
#include <dinput.h>
#include <shlwapi.h>

class Joystick {
private:
	HINSTANCE instance = NULL;
	HRESULT result = NULL;
	LPDIRECTINPUT8 lpDIObject = NULL; // pointer to direct input object

public:
	LPDIRECTINPUTDEVICE8 lpDIJoypad = NULL;
	DIJOYSTATE2 joypadBuffer = {0};
	int CreateDIObject();

	static BOOL CALLBACK CreateDeviceCallback(LPCDIDEVICEINSTANCE instance, LPVOID reference);
	BOOL CreateDeviceCallback_impl(LPCDIDEVICEINSTANCE instance);

	int getDIJoypad();
	int getJoypadInputs();

	void Finalize(void);
};

extern Joystick joystick;