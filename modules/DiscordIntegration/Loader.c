#include <windows.h>
#include <VersionHelpers.h>
#include <shlwapi.h>
#include <stdbool.h>
#include <stdint.h>

static bool (*CheckVersion_og)(const BYTE hash[16]);
static bool (*Initialize_og)(HMODULE hMyModule, HMODULE hParentModule);
static int (*getPriority_og)();
static bool (*hasChainedHooks_og)();
static void (*unHook_og)();
static void **(*getConfig_og)();
static void (*freeConfig_og)(void **v);
static bool (*commitConfig_og)(void *);
static const char *(*getFailureReason_og)();

bool load(HMODULE self)
{
	if (!IsWindows7OrGreater())
		return false;

	wchar_t dll_path[MAX_PATH];
	wchar_t sdk_path[MAX_PATH];

	if (!GetModuleFileNameW(self, dll_path, sizeof(dll_path)))
		return false;
	if (!PathRemoveFileSpecW(dll_path))
		return false;
	if (!StrCpyW(sdk_path, dll_path))
		return false;
	if (!PathAppendW(sdk_path, L"discord_game_sdk.dll"))
		return false;
	if (!PathAppendW(dll_path, L"DiscordIntegration.dat"))
		return false;
	if (!LoadLibraryExW(sdk_path, NULL, 0))
		return false;

	HMODULE dll = LoadLibraryExW(dll_path, NULL, 0);

	if (!dll)
		return false;

	CheckVersion_og = (bool (*)(const BYTE hash[16]))GetProcAddress(dll, "CheckVersion");
	Initialize_og = (bool (*)(HMODULE hMyModule, HMODULE hParentModule))GetProcAddress(dll, "Initialize");
	getPriority_og = (int (*)())GetProcAddress(dll, "getPriority");
	hasChainedHooks_og = (bool (*)())GetProcAddress(dll, "hasChainedHooks");
	unHook_og = (void (*)())GetProcAddress(dll, "unHook");
	getConfig_og = (void **(*)())GetProcAddress(dll, "getConfig");
	freeConfig_og = (void (*)(void **v))GetProcAddress(dll, "freeConfig");
	commitConfig_og = (bool (*)(void *))GetProcAddress(dll, "commitConfig");
	getFailureReason_og = (const char *(*)())GetProcAddress(dll, "getFailureReason");

	if (!CheckVersion_og)
		return false;
	if (!Initialize_og)
		return false;
	if (!getPriority_og)
		return false;
	if (!hasChainedHooks_og)
		return false;
	if (!unHook_og)
		return false;
	if (!getConfig_og)
		return false;
	if (!freeConfig_og)
		return false;
	if (!commitConfig_og)
		return false;
	if (!getFailureReason_og)
		return false;
	return true;
}

bool WINAPI DllMain(HINSTANCE hInst, DWORD dwReason, LPVOID reserved) {
	if (dwReason == DLL_PROCESS_ATTACH)
		return load(hInst);
	return true;
}

__declspec(dllexport) bool CheckVersion(const BYTE hash[16])
{
	return CheckVersion_og(hash);
}

__declspec(dllexport) bool Initialize(HMODULE hMyModule, HMODULE hParentModule)
{
	return Initialize_og(hMyModule, hParentModule);
}

__declspec(dllexport) int getPriority()
{
	return getPriority_og();
}

__declspec(dllexport) bool hasChainedHooks()
{
	return hasChainedHooks_og();
}

__declspec(dllexport) void unHook()
{
	unHook_og();
}

__declspec(dllexport) void **getConfig()
{
	return getConfig_og();
}

__declspec(dllexport) void freeConfig(void **v)
{
	freeConfig_og(v);
}

__declspec(dllexport) bool commitConfig(void *s)
{
	return commitConfig_og(s);
}

__declspec(dllexport) const char *getFailureReason()
{
	return getFailureReason_og();
}

