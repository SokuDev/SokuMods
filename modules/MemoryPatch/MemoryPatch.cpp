#include <windows.h>
#include <shlwapi.h>
#include <tchar.h>

static BYTE TARGET_HASH[16];

__declspec(naked) __inline void *copyMemory(void *_Dst, const void *_Src, size_t _Size) {
	__asm {
		push esi
		push edi
		mov edi, [esp+12]
		mov esi, [esp+16]
		mov edx, [esp+20]
		mov eax, edi
		mov ecx, edx
		shr ecx, 2
		and edx, 3
		rep movsd
		mov ecx, edx
		rep movsb
		pop edi
		pop esi
		ret
	}
}

int __inline isxdigit(int x) {
	return (x >= '0' && x <= '9') || (x >= 'A' && x <= 'F') || (x >= 'a' && x <= 'f');
}

int __inline xdigit(TCHAR x) {
	if (x < 'A')
		return x & 0xF;
	else
		return (x & 0xF) + 9;
}

int decodeHexArray(const TCHAR *str, BYTE *out, int max) {
	int len = 0;
	for (;;) {
		while (*str == ' ')
			++str;
		if (!*str)
			break;

		if (::isxdigit((BYTE)str[0]) && ::isxdigit((BYTE)str[1]) && max > 1) {
			*out++ = (xdigit(str[0]) << 4) | xdigit(str[1]);
			len++;
			max--;
		} else
			return 0;
		str += 2;
	}
	return len;
}

bool decodeHexHash(const TCHAR *str, BYTE *out) {
	int i;
	for (i = 0; i < 16 && *str != NULL; ++i) {
		if (::isxdigit((BYTE)str[0]) && ::isxdigit((BYTE)str[1])) {
			*out++ = (xdigit(str[0]) << 4) | xdigit(str[1]);
		} else
			break;
		str += 2;
	}
	return i == 16 && *str == NULL;
}

int decodeHexValue(const TCHAR *str, const TCHAR **endPtr) {
	int value = 0;
	while (!(value & 0x80000000)) {
		if (!::isxdigit((BYTE)*str))
			break;
		value = (value << 4) | xdigit(*str++);
	}
	*endPtr = str;
	return value;
}

bool __inline LoadPatchEntry(LPCTSTR sectionName, LPCTSTR keyName, LPCTSTR profilePath) {
	bool patched = false;
	const TCHAR *endStr;
	LPVOID targetBase = (LPVOID)decodeHexValue(keyName, &endStr);
	if (*endStr == '\0') {
		TCHAR patchStr[1024];
		BYTE patch[512];
		::GetPrivateProfileString(sectionName, keyName, NULL, patchStr, _countof(patchStr), profilePath);
		int patchLen = decodeHexArray(patchStr, patch, _countof(patch));
		if (patchLen > 0) {
			DWORD old;
			if (::VirtualProtect(targetBase, patchLen, PAGE_EXECUTE_WRITECOPY, &old)) {
				copyMemory(targetBase, patch, patchLen);
				::VirtualProtect(targetBase, patchLen, old, &old);
				patched = true;
			}
		}
	}
	return patched;
}

bool __inline LoadPatchSection(LPCTSTR sectionName, LPCTSTR profilePath) {
	bool patched = false;
	if (GetPrivateProfileInt(sectionName, _T("Enabled"), 0, profilePath) != 0) {
		TCHAR keySet[1024];
		::GetPrivateProfileString(sectionName, NULL, NULL, keySet, _countof(keySet), profilePath);
		for (TCHAR *keyName = keySet; *keyName; keyName += ::_tcslen(keyName) + 1) {
			if (LoadPatchEntry(sectionName, keyName, profilePath)) {
				patched = true;
			}
		}
	}
	return patched;
}

extern "C" __declspec(dllexport) bool CheckVersion(const BYTE hash[16]) {
	::memcpy(TARGET_HASH, hash, sizeof TARGET_HASH);
	return true;
}

extern "C" __declspec(dllexport) bool Initialize(HMODULE hMyModule, HMODULE hParentModule) {
	TCHAR profilePath[1024 + MAX_PATH];
	::GetModuleFileName(hMyModule, profilePath, 1024);
	::PathRemoveFileSpec(profilePath);
	::PathAppend(profilePath, _T("MemoryPatch.ini"));

	TCHAR hashStr[64];
	BYTE hash[16];
	::GetPrivateProfileString("General", "Hash", NULL, hashStr, _countof(hashStr), profilePath);
	if (decodeHexHash(hashStr, hash) && !::memcmp(hash, TARGET_HASH, sizeof TARGET_HASH)) {
		bool patched = false;
		TCHAR sectionSet[1024];
		::GetPrivateProfileString(NULL, NULL, NULL, sectionSet, _countof(sectionSet), profilePath);
		for (TCHAR *sectionName = sectionSet; *sectionName; sectionName += ::_tcslen(sectionName) + 1) {
			if (::lstrcmpi(sectionName, "General") != 0 && LoadPatchSection(sectionName, profilePath)) {
				patched = true;
			}
		}
		if (patched) {
			FlushInstructionCache(GetCurrentProcess(), NULL, 0);
		}
	}

	return true;
}

extern "C" int APIENTRY DllMain(HMODULE hModule, DWORD fdwReason, LPVOID lpReserved) {
	return TRUE;
}
