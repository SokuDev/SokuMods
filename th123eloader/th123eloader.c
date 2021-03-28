#include <windows.h>
#include <shlwapi.h>
#include <stdlib.h>

// Based on the phook code included with ReplayEx.
int hook() {
	// Initialize process
	unsigned short current_dir[1024];
	unsigned short exe_path[1024];
	STARTUPINFOW si;
	PROCESS_INFORMATION pi;
	WIN32_FILE_ATTRIBUTE_DATA exe_info, dat_info;
	unsigned short dat_path[1024];

	memset(&si, 0, sizeof(si));
	memset(&pi, 0, sizeof(pi));
	si.cb = sizeof(si);

	GetModuleFileNameW(0, current_dir, 1024);
	PathRemoveFileSpecW(current_dir);

	memcpy(exe_path, current_dir, 2048);
	lstrcatW(exe_path, L"\\th123.exe");

	memcpy(dat_path, current_dir, 2048);
	lstrcatW(dat_path, L"\\th123a.dat");

	if (GetFileAttributesExW(exe_path, GetFileExInfoStandard, &exe_info) == 0) {
		MessageBoxW(0, L"Could not open th123 files.\n\nth123e.exe should be contained in the\nsame folder as th123.exe and th123a.dat.", L"th123e error", MB_OK);
		// don't print normal error
		return 1;
	}

	if (GetFileAttributesExW(dat_path, GetFileExInfoStandard, &dat_info) == 0) {
		MessageBoxW(0, L"Could not open th123a.dat.\n\nDid you install the patch and nothing else?\nBecause you still need the full game.", L"th123e error", MB_OK);
		return 1;
	}

	memcpy(dat_path, current_dir, 2048);
	lstrcatW(dat_path, L"\\th123c.dat");

	if (GetFileAttributesExW(dat_path, GetFileExInfoStandard, &dat_info) == 0) {
		MessageBoxW(0, L"Hisoutensoku is not patched!\n\nThe patch may be downloaded from\nhttp://tasofro.net/arc/index.html", L"th123e error", MB_OK);
		return 1;
	}

	if (exe_info.nFileSizeLow != 4816896 || dat_info.nFileSizeLow != 59554674) {
		MessageBoxW(0, L"This requires Hisoutensoku v1.10a, please patch to that version.\n\nThe patch may be downloaded from\nhttp://tasofro.net/arc/index.html",
			L"th123e error", MB_OK);
		return 1;
	}

	unsigned short dll_path[1024];

	memcpy(dll_path, current_dir, 2048);
	lstrcatW(dll_path, L"\\th123e.dll");

	if (GetFileAttributesExW(dll_path, GetFileExInfoStandard, &dat_info) == 0) {
		MessageBoxW(0, L"Could not find th123e.dll", L"th123e error", MB_OK);

		return 1;
	}

	if (!CreateProcessW(exe_path, GetCommandLineW(), 0, 0, TRUE, CREATE_SUSPENDED, NULL, current_dir, &si, &pi)) {
		MessageBoxW(0, L"Could not create process", L"th123e error", MB_OK);
		return 1;
	}

	// Wait for process startup, block
	// 1.02
	// void *address = (void *)0x7d7e90;
	// 1.03
	// void *address = (void *)0x7d8520;
	// 1.10
	// void *address = (void *)0x7fad70;
	// 1.10a
	void *address = (void *)0x7fb590;

	WORD lock_code = 0xFEEB;
	WORD orig_code = 0xec81;

	pi.hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pi.dwProcessId);

	WriteProcessMemory(pi.hProcess, address, (char *)&lock_code, 2, 0);

	CONTEXT ct;
	ct.ContextFlags = CONTEXT_CONTROL;
	do {
		ResumeThread(pi.hThread);
		Sleep(250);
		SuspendThread(pi.hThread);

		if (!GetThreadContext(pi.hThread, &ct)) {
			Sleep(100);
			MessageBoxW(0, L"Could not get thread context", L"th123e error", MB_OK);
			TerminateProcess(pi.hProcess, -1);
			return 1;
		}
	} while (ct.Eip != (DWORD)address);

	// Alright, hook in the new dll.
	HMODULE hKernel32 = GetModuleHandleW(L"Kernel32");
	LPTHREAD_START_ROUTINE pLoadLibrary = (LPTHREAD_START_ROUTINE)GetProcAddress(hKernel32, "LoadLibraryW");

	void *dll_addr = VirtualAllocEx(pi.hProcess, 0, 2048, MEM_COMMIT, PAGE_READWRITE);
	WriteProcessMemory(pi.hProcess, dll_addr, dll_path, 2048, 0);

	HANDLE hThread = CreateRemoteThread(pi.hProcess, 0, 0, pLoadLibrary, dll_addr, 0, NULL);
	if (!hThread) {
		MessageBoxW(0, L"Could not get remote loading thread", L"th123e error", MB_OK);
		VirtualFreeEx(pi.hProcess, dll_addr, 0, MEM_RELEASE);
		TerminateProcess(pi.hProcess, -1);
		return 1;
	}

	HMODULE hookedDLL;
	HMODULE *addr = &hookedDLL;

	WaitForSingleObject(hThread, INFINITE);
	GetExitCodeThread(hThread, (DWORD *)addr);
	CloseHandle(hThread);

	VirtualFreeEx(pi.hProcess, dll_addr, 0, MEM_RELEASE);

	if (!hookedDLL) {
		MessageBoxW(0, L"DLL loading failed", L"th123e error", MB_OK);
		TerminateProcess(pi.hProcess, -1);
		return 1;
	}

	// Continue normal process action.
	WriteProcessMemory(pi.hProcess, (void *)address, (char *)&orig_code, 2, 0);

	FlushInstructionCache(pi.hProcess, (void *)address, 2);

	ResumeThread(pi.hThread);

	return 1;
}

// int WINAMP WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
int main(int argc, char **argv) {
	// Create process and hook library.
	if (!hook()) {
		MessageBoxW(0, L"Could not hook into th123.exe\n\nDo you have GameGuard or something running?", L"th123e error", MB_OK);
	}
	return 0;
}
