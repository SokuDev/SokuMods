#include <windows.h>
#include <cstdio>
#include <shlwapi.h>

#define WARN(fmt, ...) \
	{ \
		size_t needed = _snwprintf(NULL, 0, fmt, ##__VA_ARGS__); \
		wchar_t *buf = (wchar_t *)malloc((needed + 1) * 2); \
		_snwprintf(buf, (needed + 1), fmt, ##__VA_ARGS__); \
		MessageBoxW(NULL, buf, L"SWRSToys", MB_ICONWARNING | MB_OK); \
		free(buf); \
	}

class CryptHash {
public:
	CryptHash(): prov(NULL), hash(NULL) {}
	~CryptHash() {
		if (hash != NULL) {
			const BOOL result = ::CryptDestroyHash(hash);
		}
		if (prov != NULL) {
			const BOOL result = ::CryptReleaseContext(prov, 0);
		}
	}
	bool Initialize(void) {
		if (FALSE == ::CryptAcquireContextW(&prov, NULL, NULL, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT)) {
			return false;
		}
		if (FALSE == ::CryptCreateHash(prov, CALG_MD5, 0, 0, &hash)) {
			return false;
		}
		return true;
	}
	bool AddData(const BYTE *buffer, unsigned int buffer_size) {
		if (FALSE == ::CryptHashData(hash, buffer, buffer_size, 0)) {
			return false;
		}
		return true;
	}
	bool GetValue(BYTE *buffer, DWORD buffer_size) const {
		DWORD ret_size = buffer_size;
		if (FALSE == ::CryptGetHashParam(hash, HP_HASHVAL, NULL, &ret_size, 0) || ret_size != buffer_size) {
			return false;
		}
		if (FALSE == ::CryptGetHashParam(hash, HP_HASHVAL, buffer, &ret_size, 0)) {
			return false;
		}
		return true;
	}

protected:
	HCRYPTPROV prov;
	HCRYPTHASH hash;
};

bool getSWRSHash(BYTE retVal[16]) {
	CryptHash hash;
	if (!hash.Initialize()) {
		return false;
	}

	wchar_t swrs_path[MAX_PATH];
	if (FALSE == ::GetModuleFileNameW(NULL, swrs_path, MAX_PATH)) {
		return false;
	}

	HANDLE file = ::CreateFileW(swrs_path, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
	if (!file) {
		return false;
	}

	while (true) {
		BYTE buff[1024];
		DWORD readSize;
		if (FALSE == ::ReadFile(file, buff, sizeof(buff), &readSize, NULL)) {
			::CloseHandle(file);
			return false;
		}
		if (readSize == 0) {
			break;
		}

		if (!hash.AddData(buff, readSize)) {
			::CloseHandle(file);
			return false;
		}
	}
	::CloseHandle(file);

	if (!hash.GetValue(retVal, 16)) {
		return false;
	}
	return true;
}

bool Hook(HMODULE this_module) {
	BYTE hash[16];
	if (!getSWRSHash(hash)) {
		return false;
	}

	wchar_t app_path[MAX_PATH];
	GetModuleFileNameW(this_module, app_path, MAX_PATH);
	PathRemoveFileSpecW(app_path);

	wchar_t setting_path[MAX_PATH];
	if (GetEnvironmentVariableW(L"SWRSTOYS", setting_path, sizeof(setting_path)) <= 0) {
		int argc;
		wchar_t **arg_list = CommandLineToArgvW(GetCommandLineW(), &argc);
		if (arg_list && argc > 1 && StrStrIW(arg_list[1], L"ini")) {
			wcscpy(setting_path, arg_list[1]);
			LocalFree(arg_list);
		} else {
			wcscpy(setting_path, app_path);
			PathAppendW(setting_path, L"\\SWRSToys.ini");
		}
		if (arg_list) {
			LocalFree(arg_list);
		}
	}

	wchar_t moduleKeys[1024];
	wchar_t moduleValue[MAX_PATH];
	GetPrivateProfileStringW(L"Module", NULL, NULL, moduleKeys, sizeof(moduleKeys), setting_path);
	for (wchar_t *key = moduleKeys; *key; key += wcslen(key) + 1) {
		GetPrivateProfileStringW(L"Module", key, NULL, moduleValue, sizeof(moduleValue), setting_path);
		wchar_t module_path[MAX_PATH];
		wcscpy(module_path, app_path);
		PathAppendW(module_path, L"\\");
		PathAppendW(module_path, moduleValue);

		// don't show errors for sokuengine, which "fails" even when it's loaded successfully
		bool warn = !StrStrIW(moduleValue, L"SokuEngine");

		DWORD attrs = GetFileAttributesW(module_path);
		if (attrs == INVALID_FILE_ATTRIBUTES || attrs & FILE_ATTRIBUTE_DIRECTORY) {
			if (warn) {
				WARN(L"Failed loading %s: file does not exist", moduleValue)
			}
			continue;
		}

		HMODULE module = LoadLibraryW(module_path);
		if (module == NULL) {
			if (warn) {
				WARN(L"Failed loading %s: loading failed: %x", moduleValue, GetLastError())
			}
			continue;
		}
		bool (*const CheckVersion)(const BYTE[16]) = reinterpret_cast<bool (*)(const BYTE[16])>(GetProcAddress(module, "CheckVersion"));
		if (CheckVersion == NULL) {
			if (warn) {
				WARN(L"Failed loading %s: CheckVersion not found", moduleValue)
			}
			FreeLibrary(module);
			continue;
		}
		bool (*const Initialize)(HMODULE, HMODULE) = reinterpret_cast<bool (*)(HMODULE, HMODULE)>(GetProcAddress(module, "Initialize"));
		if (Initialize == NULL) {
			if (warn) {
				WARN(L"Failed loading %s: Initialize not found", moduleValue)
			}
			FreeLibrary(module);
			continue;
		}
		if (!CheckVersion(hash)) {
			if (warn) {
				WARN(L"Failed loading %s: CheckVersion failed", moduleValue)
			}
			FreeLibrary(module);
			continue;
		}
		if (!Initialize(module, this_module)) {
			if (warn) {
				WARN(L"Failed loading %s: Initialize failed", moduleValue)
			}
			FreeLibrary(module);
			continue;
		}
	}

	return true;
}
