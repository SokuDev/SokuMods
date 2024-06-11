#include "dummy.h"

#include <windows.h>
#include <SokuLib.hpp>
#include <direct.h>
#include <crtdbg.h>
#include <minidumpapiset.h>
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

typedef int (*PIFV)(void);
int (*ogInittermE)(PIFV* start, PIFV* end);
int (*ogSokuMain)(int a, int b, int c, int d);

LONG WINAPI UnhandledExFilter(PEXCEPTION_POINTERS ExPtr)
{
	if (!ExPtr) {
		puts("No ExPtr....");
		return EXCEPTION_CONTINUE_SEARCH;
	}
	puts("Caught fatal exception! Generating dump...");

	wchar_t buf[8192];
	wchar_t buf2[MAX_PATH];
	time_t timer;
	char timebuffer[31];
	struct tm* tm_info;

	time(&timer);
	tm_info = localtime(&timer);
	strftime(timebuffer, sizeof(timebuffer), "%Y-%m-%d-%H-%M-%S", tm_info);
	mkdir("crashes");
	wsprintfW(buf2, L"crashes/crash_%S.dmp", timebuffer);
	wsprintfW(buf, L"Game crashed!\nReceived exception %X at address %x.\n", ExPtr->ExceptionRecord->ExceptionCode, ExPtr->ExceptionRecord->ExceptionAddress);

	HANDLE hFile = CreateFileW(buf2, GENERIC_READ | GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);

	if (hFile != INVALID_HANDLE_VALUE) {
		MINIDUMP_EXCEPTION_INFORMATION md;
		md.ThreadId = GetCurrentThreadId();
		md.ExceptionPointers = ExPtr;
		md.ClientPointers = FALSE;
		BOOL win = MiniDumpWriteDump(GetCurrentProcess(), GetCurrentProcessId(), hFile, MiniDumpNormal, &md, nullptr, nullptr);

		if (!win) {
			wcscat(buf, L"MiniDumpWriteDump failed.\n");
			wcscat(buf, getLastError().c_str());
			wcscat(buf, L": ");
			wcscat(buf, buf2);
		} else {
			wcscat(buf, L"Minidump created ");
			wcscat(buf, buf2);
			wcscat(buf, L"\n\nPlease include this file, the list of mods enabled and a brief explanation of what you were doing when this happened in your bug report.");
		}
		CloseHandle(hFile);
	} else {
		wcscat(buf, L"CreateFileW(");
		wcscat(buf, buf2);
		wcscat(buf, L") failed: ");
		wcscat(buf, getLastError().c_str());
	}
	printf("%S\n", buf);
	MessageBoxW(nullptr, buf, L"Error", MB_ICONERROR);
	return EXCEPTION_CONTINUE_SEARCH;
}

int SokuMain(int a, int b, int c, int d)
{
	return ogSokuMain(a, b, c, d);
}

int myInittermE(PIFV* start, PIFV* end)
{
	int ret = ogInittermE(start, end);
	if (ret == 0) {
		// if initialize successfully
		SetUnhandledExceptionFilter(UnhandledExFilter);
		Hook(this_module);
	}
	return ret;
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
		ogInittermE = SokuLib::TamperNearJmpOpr(0x8240b3, myInittermE);
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
