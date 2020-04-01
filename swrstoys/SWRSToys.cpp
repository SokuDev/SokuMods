#include <windows.h>
#include <shlwapi.h>

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
	wchar_t setting_path[MAX_PATH];
	GetModuleFileNameW(this_module, app_path, MAX_PATH);
	PathRemoveFileSpecW(app_path);
	wcscpy(setting_path, app_path);
	PathAppendW(setting_path, L"\\SWRSToys.ini");

	wchar_t moduleKeys[1024];
	wchar_t moduleValue[MAX_PATH];
	GetPrivateProfileStringW(L"Module", NULL, NULL, moduleKeys, sizeof(moduleKeys), setting_path);
	for (wchar_t *key = moduleKeys; *key; key += wcslen(key) + 1) {
		GetPrivateProfileStringW(L"Module", key, NULL, moduleValue, sizeof(moduleValue), setting_path);
		wchar_t module_path[MAX_PATH];
		wcscpy(module_path, app_path);
		PathAppendW(module_path, L"\\");
		PathAppendW(module_path, moduleValue);

		HMODULE module = LoadLibraryW(module_path);
		if (module != NULL) {
			bool (*const CheckVersion)(const BYTE[16]) = reinterpret_cast<bool (*)(const BYTE[16])>(GetProcAddress(module, "CheckVersion"));
			bool (*const Initialize)(HMODULE, HMODULE) = reinterpret_cast<bool (*)(HMODULE, HMODULE)>(GetProcAddress(module, "Initialize"));
			if (CheckVersion == NULL || Initialize == NULL || !CheckVersion(hash) || !Initialize(module, this_module)) {
				FreeLibrary(module);
			}
		}
	}

	return true;
}
