//
// Created by PC_volt on 10/07/2023.
//

#include <SokuLib.hpp>

static bool mCheckFKey(byte arg1, bool arg2, bool arg3, bool arg4)

int timeHeld = 0;
bool isEscapeHeld()
{
	constexpr int ESC_KEYCODE = 0x1B;

	if (GetAsyncKeyState(ESC_KEYCODE) & 0x8000)
	{
		if (timeHeld < 120)
		{
			++timeHeld;
			return false;
		}
		else
		{
			timeHeld = 0;
			return true;
		}
	}

	timeHeld = 0;

	return false;
}

//__stdcall or __cdecl?
bool mCheckFKey_VSNetwork(byte arg1, bool arg2, bool arg3, bool arg4)
{
	return isEscapeHeld();
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

	VirtualProtect((PVOID)TEXT_SECTION_OFFSET, TEXT_SECTION_SIZE, PAGE_EXECUTE_WRITECOPY, &old);
	original_mCheckFKey = SokuLib::TamperDword(0x482591, mCheckFKey_VSNetwork);
	VirtualProtect((PVOID)TEXT_SECTION_OFFSET, TEXT_SECTION_SIZE, old, &old);

	FlushInstructionCache(GetCurrentProcess(), nullptr, 0);
	return true;
}

extern "C" int APIENTRY DllMain(HMODULE hModule, DWORD fdwReason, LPVOID lpReserved)
{
	return TRUE;
}