#include <windows.h>
#include "SokuRoll.inl"
#include <MemoryModule.h>
#include <process.h>
#include <shlwapi.h>

const BYTE TARGET_HASH[16] = {0xdf, 0x35, 0xd1, 0xfb, 0xc7, 0xb5, 0x83, 0x31, 0x7a, 0xda, 0xbe, 0x8c, 0xd9, 0xf5, 0x3b, 0x2e};

static char s_profilePath[1024 + MAX_PATH];

#pragma pack(push, 1)
struct SokuRollParams {
	int change_title = 0x01;
	int maximum_rollback = 0x04;
	int initial_delay = 0x04;
	int increase_delay = 0x4E;
	int decrease_delay = 0x4A;
	int debug = 0x00;
} SokuRollParams;
#pragma pack(pop)

unsigned __stdcall loadDelay(void *) {
	// wait for the game window to be loaded so that it is injectable by sokuroll:
	// 1 second is largely enough for even slower computers to init the game.
	// it'd probably be cleaner to do with a hook on a specific function,
	// but wasting 1 second here is fine and mimicks the current manual behaviour anyways.
	Sleep(1000);

	HMEMORYMODULE handle = MemoryLoadLibrary(&sokuroll_data, sokuroll_size);
	if (handle == NULL) {
		return 0;
	}
	FARPROC roll_init = MemoryGetProcAddress(handle, "init");
	_beginthreadex(NULL, NULL, reinterpret_cast<_beginthreadex_proc_type>(roll_init), &SokuRollParams, NULL, NULL);
	return 0;
}

extern "C" {
__declspec(dllexport) bool CheckVersion(const BYTE hash[16]) {
	return ::memcmp(TARGET_HASH, hash, sizeof TARGET_HASH) == 0;
}

__declspec(dllexport) bool Initialize(HMODULE hMyModule, HMODULE hParentModule) {
	::GetModuleFileName(hMyModule, s_profilePath, 1024);
	::PathRemoveFileSpec(s_profilePath);
	::PathAppend(s_profilePath, "SWRSokuRoll.ini");

	SokuRollParams.change_title = ::GetPrivateProfileInt("Parameters", "ChangeTitle", 0, s_profilePath);
	SokuRollParams.initial_delay = ::GetPrivateProfileInt("Parameters", "InitialDelay", 4, s_profilePath);
	SokuRollParams.maximum_rollback = ::GetPrivateProfileInt("Parameters", "MaximumRollback", 4, s_profilePath);
	SokuRollParams.debug = ::GetPrivateProfileInt("Parameters", "Debug", 0, s_profilePath);
	SokuRollParams.increase_delay = ::GetPrivateProfileInt("Keyboard", "IncreaseDelay", 0x4E, s_profilePath);
	SokuRollParams.decrease_delay = ::GetPrivateProfileInt("Keyboard", "DecreaseDelay", 0x4A, s_profilePath);

	_beginthreadex(NULL, NULL, reinterpret_cast<_beginthreadex_proc_type>(loadDelay), NULL, NULL, NULL);

	return true;
}
}
