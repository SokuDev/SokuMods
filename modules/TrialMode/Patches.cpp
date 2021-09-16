//
// Created by PinkySmile on 16/09/2021.
//

#include <SokuLib.hpp>
#include "Patches.hpp"

static char bufferFS[11];
static bool hasFSPatch = false;
static bool hasCHOPatch = false;

void applyFileSystemPatch()
{
	DWORD old;

	//[FilesystemFallback]
	//0040D237=0C
	if (hasFSPatch)
		return;
	hasFSPatch = true;
	puts("Apply filesystem patch");
	bufferFS[0] = *(unsigned char *)0x40D1FB;
	bufferFS[1] = *(unsigned char *)0x40D245;
	bufferFS[2] = *(unsigned char *)0x40D27A;
	bufferFS[3] = *(unsigned char *)0x40D27B;
	bufferFS[4] = *(unsigned char *)0x40D27C;
	bufferFS[5] = *(unsigned char *)0x40D27D;
	bufferFS[6] = *(unsigned char *)0x40D27E;
	bufferFS[7] = *(unsigned char *)0x40D27F;
	bufferFS[8] = *(unsigned char *)0x40D280;
	bufferFS[9] = *(unsigned char *)0x40D281;
	bufferFS[10]= *(unsigned char *)0x40D282;

	// This applies the filesystem first patch from Slen
	::VirtualProtect((PVOID)TEXT_SECTION_OFFSET, TEXT_SECTION_SIZE, PAGE_EXECUTE_READWRITE, &old);
	*(unsigned char *)0x40D1FB = 0xEB;
	*(unsigned char *)0x40D245 = 0x1C;
	*(unsigned char *)0x40D27A = 0x74;
	*(unsigned char *)0x40D27B = 0x91;
	for (int i = 0; i < 7; i++)
		((unsigned char *)0x40D27C)[i] = 0x90;
	::VirtualProtect((PVOID)TEXT_SECTION_OFFSET, TEXT_SECTION_SIZE, old, &old);
	::FlushInstructionCache(GetCurrentProcess(), nullptr, 0);
}

void removeFileSystemPatch()
{
	DWORD old;

	if (!hasFSPatch)
		return;
	hasFSPatch = false;
	puts("Remove filesystem patch");
	::VirtualProtect((PVOID)TEXT_SECTION_OFFSET, TEXT_SECTION_SIZE, PAGE_EXECUTE_READWRITE, &old);
	// We remove the filesystem first patch if it was there
	*(unsigned char *)0x40D1FB = bufferFS[0];
	*(unsigned char *)0x40D245 = bufferFS[1];
	for (int i = 2; i < 11; i++)
		((unsigned char *)0x40D27A)[i - 2] = bufferFS[i];
	::VirtualProtect((PVOID)TEXT_SECTION_OFFSET, TEXT_SECTION_SIZE, old, &old);
	::FlushInstructionCache(GetCurrentProcess(), nullptr, 0);
}

void applyCounterHitOnlyPatch()
{
	DWORD old;

	if (hasCHOPatch)
		return;
	hasCHOPatch = true;
	puts("Apply counter hit only patch");
	::VirtualProtect((PVOID)TEXT_SECTION_OFFSET, TEXT_SECTION_SIZE, PAGE_EXECUTE_READWRITE, &old);
	// This removes the check for counter hits so moves will always counter hit if they can
	*(unsigned short *)0x47abb7 = 0x9090;
	*(unsigned short *)0x47abc3 = 0x9090;
	::VirtualProtect((PVOID)TEXT_SECTION_OFFSET, TEXT_SECTION_SIZE, old, &old);
	::FlushInstructionCache(GetCurrentProcess(), nullptr, 0);
}

void removeCounterHitOnlyPatch()
{
	DWORD old;

	if (!hasCHOPatch)
		return;
	hasCHOPatch = false;
	puts("Remove counter hit only patch");
	::VirtualProtect((PVOID)TEXT_SECTION_OFFSET, TEXT_SECTION_SIZE, PAGE_EXECUTE_READWRITE, &old);
	// We remove the always counter hit patch
	*(unsigned short *)0x47abb7 = 0x1e75; //           JNZ        LAB_0047abd7
	*(unsigned short *)0x47abc3 = 0x1275; //           JNZ        LAB_0047abd7
	::VirtualProtect((PVOID)TEXT_SECTION_OFFSET, TEXT_SECTION_SIZE, old, &old);
	::FlushInstructionCache(GetCurrentProcess(), nullptr, 0);
}
