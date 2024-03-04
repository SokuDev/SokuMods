#include "dummy.h"

#include <Windows.h>
#include <SokuLib.hpp>
#include "SWRSToys.h"

FARPROC p_Direct3DShaderValidatorCreate9 = NULL;
FARPROC p_PSGPError = NULL;
FARPROC p_PSGPSampleTexture = NULL;
FARPROC p_D3DPERF_BeginEvent = NULL;
FARPROC p_D3DPERF_EndEvent = NULL;
FARPROC p_D3DPERF_GetStatus = NULL;
FARPROC p_D3DPERF_QueryRepeatFrame = NULL;
FARPROC p_D3DPERF_SetMarker = NULL;
FARPROC p_D3DPERF_SetOptions = NULL;
FARPROC p_D3DPERF_SetRegion = NULL;
FARPROC p_DebugSetLevel = NULL;
FARPROC p_DebugSetMute = NULL;
FARPROC p_Direct3DCreate9 = NULL;
FARPROC p_Direct3DCreate9Ex = NULL;

HMODULE orig_module = NULL;
HMODULE this_module = NULL;

int (*ogSokuMain)(int a, int b, int c, int d);

int SokuMain(int a, int b, int c, int d)
{
	Hook(this_module);
	return ogSokuMain(a, b, c, d);
}

BOOL APIENTRY DllMain(HMODULE this_module_, DWORD ul_reason_for_call, LPVOID) {
	switch (ul_reason_for_call) {
	case DLL_PROCESS_ATTACH: {
		wchar_t sys_dir[MAX_PATH];
		DWORD old;

		this_module = this_module_;
		if (FALSE == ::GetSystemDirectoryW(sys_dir, MAX_PATH)) {
			return FALSE;
		}
		wcscat(sys_dir, L"\\D3D9.DLL");

		orig_module = ::LoadLibraryW(sys_dir);
		if (orig_module == NULL) {
			return FALSE;
		}
		p_Direct3DShaderValidatorCreate9 = ::GetProcAddress(orig_module, "Direct3DShaderValidatorCreate9");
		p_PSGPError = ::GetProcAddress(orig_module, "PSGPError");
		p_PSGPSampleTexture = ::GetProcAddress(orig_module, "PSGPSampleTexture");
		p_D3DPERF_BeginEvent = ::GetProcAddress(orig_module, "D3DPERF_BeginEvent");
		p_D3DPERF_EndEvent = ::GetProcAddress(orig_module, "D3DPERF_EndEvent");
		p_D3DPERF_GetStatus = ::GetProcAddress(orig_module, "D3DPERF_GetStatus");
		p_D3DPERF_QueryRepeatFrame = ::GetProcAddress(orig_module, "D3DPERF_QueryRepeatFrame");
		p_D3DPERF_SetMarker = ::GetProcAddress(orig_module, "D3DPERF_SetMarker");
		p_D3DPERF_SetOptions = ::GetProcAddress(orig_module, "D3DPERF_SetOptions");
		p_D3DPERF_SetRegion = ::GetProcAddress(orig_module, "D3DPERF_SetRegion");
		p_DebugSetLevel = ::GetProcAddress(orig_module, "DebugSetLevel");
		p_DebugSetMute = ::GetProcAddress(orig_module, "DebugSetMute");
		p_Direct3DCreate9 = ::GetProcAddress(orig_module, "Direct3DCreate9");
		p_Direct3DCreate9Ex = ::GetProcAddress(orig_module, "Direct3DCreate9Ex");

		VirtualProtect((PVOID)TEXT_SECTION_OFFSET, TEXT_SECTION_SIZE, PAGE_EXECUTE_WRITECOPY, &old);
		ogSokuMain = SokuLib::TamperNearJmpOpr(0x821844, SokuMain);
		VirtualProtect((PVOID)TEXT_SECTION_OFFSET, TEXT_SECTION_SIZE, old, &old);
		break;
	}
	case DLL_THREAD_ATTACH:
		break;
	case DLL_THREAD_DETACH:
		break;
	case DLL_PROCESS_DETACH:
		if (orig_module != NULL) {
			::FreeLibrary(orig_module);
		}
		break;
	}
	return TRUE;
}
