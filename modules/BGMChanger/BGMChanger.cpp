#include <windows.h>
#include <shlwapi.h>
#include <tchar.h>
#include <vector>

#define SWRS_USES_HASH
#include "swrs.h"

class CFileReader: public IFileReader {
public:
	CFileReader(LPCSTR filename) {
		read_length = 0;
		file = ::CreateFileA(filename, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		if (file == INVALID_HANDLE_VALUE) {
			file = NULL;
		}
	}
	virtual ~CFileReader() {
		if (file != NULL) {
			::CloseHandle(file);
		}
	}

public:
	virtual bool Read(LPVOID buffer, DWORD read_size) {
		return (::ReadFile(file, buffer, read_size, &read_length, NULL) != 0);
	}
	virtual DWORD GetReadLength(void) {
		return read_length;
	}
	virtual LONG Seek(LONG distance, DWORD move_method) {
		return ::SetFilePointer(file, distance, NULL, move_method);
	}
	virtual DWORD GetLength(void) {
		return ::GetFileSize(file, NULL);
	}
	static void *operator new(size_t _Size) {
		return SWRS_new(_Size);
	}
	static void operator delete(void *_Buf) {
		SWRS_delete(_Buf);
	}

public:
	HANDLE file;
	DWORD read_length;
};

struct CFakeSFLReader: IFileReader {
private:
	struct FakeSFL {
		DWORD cue;
		DWORD adtl;
		DWORD _padding[2];
		DWORD intro_samples;
		DWORD loop_samples;
	};

public:
	CFakeSFLReader(DWORD intro_samples, DWORD loop_samples): intro_samples(intro_samples), loop_samples(loop_samples) {}
	virtual ~CFakeSFLReader() {}

public:
	virtual bool Read(LPVOID buffer, DWORD read_size) {
		FakeSFL *const fake = reinterpret_cast<FakeSFL *>(buffer);
		fake->cue = MAKEFOURCC('c', 'u', 'e', ' ');
		fake->adtl = MAKEFOURCC('a', 'd', 't', 'l');
		fake->_padding[0] = fake->_padding[1] = 0;
		fake->intro_samples = intro_samples;
		fake->loop_samples = loop_samples;
		return true;
	}
	virtual DWORD GetReadLength(void) {
		return sizeof(FakeSFL);
	}
	virtual LONG Seek(LONG /*distance*/, DWORD /*move_method*/) {
		return 0;
	}
	virtual DWORD GetLength(void) {
		return sizeof(FakeSFL);
	}
	static void *operator new(size_t _Size) {
		return SWRS_new(_Size);
	}
	static void operator delete(void *_Buf) {
		SWRS_delete(_Buf);
	}

private:
	DWORD intro_samples;
	DWORD loop_samples;
};

static struct {
	LPCSTR name;
	DWORD intro_samples;
	DWORD loop_samples;
	CHAR custom_path[1024];
} s_entries[] = {
	{"sr", 0, 0, ""},
	{"sr2", 0, 0, ""},
	{"op", 0, 0, ""},
	{"op2", 0, 0, ""},
	{"select", 0, 0, ""},
	{"room", 0, 0, ""},
	{"st00", 0, 0, ""},
	{"st01", 0, 0, ""},
	{"st02", 0, 0, ""},
	{"st03", 0, 0, ""},
	{"st04", 0, 0, ""},
	{"st05", 0, 0, ""},
	{"st06", 0, 0, ""},
	{"st07", 0, 0, ""},
	{"st08", 0, 0, ""},
	{"st09", 0, 0, ""},
	{"st10", 0, 0, ""},
	{"st11", 0, 0, ""},
	{"st12", 0, 0, ""},
	{"st13", 0, 0, ""},
	{"st14", 0, 0, ""},
	{"st15", 0, 0, ""},
	{"st16", 0, 0, ""},
	{"st17", 0, 0, ""},
	{"st18", 0, 0, ""},
	{"st19", 0, 0, ""},
	{"st20", 0, 0, ""},
	{"st21", 0, 0, ""},
	{"st22", 0, 0, ""},
	{"st30", 0, 0, ""},
	{"st31", 0, 0, ""},
	{"st32", 0, 0, ""},
	{"st33", 0, 0, ""},
	{"st34", 0, 0, ""},
	{"st35", 0, 0, ""},
	{"st36", 0, 0, ""},
	{"st40", 0, 0, ""},
	{"st41", 0, 0, ""},
	{"st42", 0, 0, ""},
	{"st43", 0, 0, ""},
	{"st99", 0, 0, ""},
	{"ta00", 0, 0, ""},
	{"ta01", 0, 0, ""},
	{"ta02", 0, 0, ""},
	{"ta03", 0, 0, ""},
	{"ta04", 0, 0, ""},
	{"ta05", 0, 0, ""},
	{"ta06", 0, 0, ""},
	{"ta07", 0, 0, ""},
	{"ta08", 0, 0, ""},
	{"ta20", 0, 0, ""},
	{"ta21", 0, 0, ""},
	{"ta22", 0, 0, ""},
};

static bool(__fastcall *GetPackagedBGM)(IFileReader **pp, int unknown, LPCSTR entory_name);
static bool(__fastcall *GetPackagedSFL)(IFileReader **pp, int unknown, LPCSTR entory_name);

int GetEntryIndex(LPCSTR entory_name) {
	for (int i = 0; i < _countof(s_entries); ++i) {
		if (s_entries[i].custom_path[0] != '\0' && ::StrStrA(entory_name, s_entries[i].name)) {
			return i;
		}
	}
	return -1;
}

bool __fastcall OnGetPackagedBGM(IFileReader **pp, int /*unknown*/, LPCSTR entory_name) {
	const int nEntryIndex = GetEntryIndex(entory_name);
	if (nEntryIndex < 0) {
		return GetPackagedBGM(pp, 0, entory_name);
	}

	if (*pp != NULL) {
		delete *pp;
		*pp = NULL;
	}
	*pp = new CFileReader(s_entries[nEntryIndex].custom_path);
	return true;
}

bool __fastcall OnGetPackagedSFL(IFileReader **pp, int /*unknown*/, LPCSTR entory_name) {
	const int nEntryIndex = GetEntryIndex(entory_name);
	if (nEntryIndex < 0) {
		return GetPackagedSFL(pp, 0, entory_name);
	}

	if (*pp != NULL) {
		delete *pp;
		*pp = NULL;
	}
	*pp = new CFakeSFLReader(s_entries[nEntryIndex].intro_samples, s_entries[nEntryIndex].loop_samples);
	return true;
}

extern "C" __declspec(dllexport) bool CheckVersion(const BYTE hash[16]) {
	return (::memcmp(TARGET_HASH, hash, sizeof TARGET_HASH) == 0);
}

extern "C" __declspec(dllexport) bool Initialize(HMODULE hMyModule, HMODULE /*parent_module*/) {
	TCHAR profilePath[1024 + MAX_PATH];
	::GetModuleFileName(hMyModule, profilePath, 1024);
	::PathRemoveFileSpec(profilePath);
	::PathAppend(profilePath, _T("BGMChanger.ini"));

	for (unsigned int i = 0; i < _countof(s_entries); i++) {
		::GetPrivateProfileStringA(s_entries[i].name, "Path", "", s_entries[i].custom_path, _countof(s_entries->custom_path), profilePath);
		s_entries[i].intro_samples = ::GetPrivateProfileIntA(s_entries[i].name, "Intro", 0, profilePath);
		s_entries[i].loop_samples = ::GetPrivateProfileIntA(s_entries[i].name, "Loop", 0, profilePath);
	}

	DWORD old_protect;
	::VirtualProtect(reinterpret_cast<LPVOID>(text_Offset), text_Size, PAGE_EXECUTE_WRITECOPY, &old_protect);
	*reinterpret_cast<DWORD *>(&GetPackagedBGM) = TamperNearJmpOpr(GetPackagedBGMCaller, reinterpret_cast<DWORD>(OnGetPackagedBGM));
	*reinterpret_cast<DWORD *>(&GetPackagedSFL) = TamperNearJmpOpr(GetPackagedSFLCaller, reinterpret_cast<DWORD>(OnGetPackagedSFL));
	::VirtualProtect(reinterpret_cast<LPVOID>(text_Offset), text_Size, old_protect, &old_protect);

	return TRUE;
}

extern "C" int APIENTRY DllMain(HMODULE /*hModule*/, DWORD /*fdwReason*/, LPVOID /*lpReserved*/) {
	return TRUE;
}
