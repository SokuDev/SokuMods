//
// Created by PinkySmile on 18/02/2021.
//

#include <windows.h>

extern "C" __declspec(dllexport) bool CheckVersion(const BYTE hash[16])
{
	return true;
}

extern "C" __declspec(dllexport) bool Initialize(HMODULE hMyModule, HMODULE hParentModule)
{
	DWORD old;

	::VirtualProtect((PVOID)0x00724316, 4, PAGE_EXECUTE_WRITECOPY, &old);
	*(unsigned char*)(0x00724316) = 0x66;
	*(unsigned char*)(0x00724317) = 0xB9;
	*(unsigned char*)(0x00724318) = 0x0F;
	*(unsigned char*)(0x00724319) = 0x00;
	::VirtualProtect((PVOID)0x00724316, 4, old, &old);
	return true;
}

extern "C" int APIENTRY DllMain(HMODULE hModule, DWORD fdwReason, LPVOID lpReserved)
{
	return TRUE;
}