#include <windows.h>
#include <curl/curl.h>
#include <process.h>
#include <shlwapi.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

const wchar_t *dll_name = L"autopunch.dat";
const wchar_t *dll_tmp_name = L"autopunch.dat.tmp";
const wchar_t *fetch_base_url = L"http://autopunch-update.delthas.fr/update?version=";

HANDLE self;

wchar_t dll_path[MAX_PATH];
wchar_t dll_tmp_path[MAX_PATH];
bool curl_init;

void display_dialog(void *data) {
	MessageBoxW(NULL, data, L"autopunch", MB_ICONINFORMATION | MB_OK);
}

char *to_utf8(const wchar_t *str) {
	int length = WideCharToMultiByte(CP_UTF8, 0, str, -1, 0, 0, NULL, NULL);
	if (!length) {
		return NULL;
	}
	char *str_utf8 = malloc(length);
	int r = WideCharToMultiByte(CP_UTF8, 0, str, -1, str_utf8, length, NULL, NULL);
	if (!r) {
		free(str_utf8);
		return NULL;
	}
	return str_utf8;
}

bool init_file_paths() {
	if (self == NULL) {
		return false;
	}
	if (!GetModuleFileNameW(self, dll_path, MAX_PATH)) {
		return false;
	}
	if (!PathRemoveFileSpecW(dll_path)) {
		return false;
	}
	if (!StrCpyW(dll_tmp_path, dll_path)) {
		return false;
	}
	if (!PathAppendW(dll_path, dll_name)) {
		return false;
	}
	if (!PathAppendW(dll_tmp_path, dll_tmp_name)) {
		return false;
	}
	return true;
}

wchar_t *file_version(wchar_t *filename) {
	DWORD ignored;
	int size = GetFileVersionInfoSizeW(filename, &ignored);
	if (!size) {
		goto error0;
	}
	char *version_data = malloc(size);
	if (!GetFileVersionInfoW(filename, 0, size, version_data)) {
		goto error1;
	}
	VS_FIXEDFILEINFO *fileinfo;
	UINT block_size;
	if (!VerQueryValueW(version_data, L"\\", (void **)(&fileinfo), &block_size) || block_size <= 0) {
		goto error1;
	}
	wchar_t *version = malloc(128);
	if (fileinfo->dwFileVersionMS == 0 && fileinfo->dwFileVersionLS == 42) {
		version[0] = '\0';
	} else {
		if (!wsprintfW(version, L"%u.%u.%u.%u", HIWORD(fileinfo->dwFileVersionMS), LOWORD(fileinfo->dwFileVersionMS), HIWORD(fileinfo->dwFileVersionLS),
					LOWORD(fileinfo->dwFileVersionLS))) {
			goto error2;
		}
	}
	free(version_data);
	return version;
error2:
	free(version);
error1:
	free(version_data);
error0:
	return NULL;
}

size_t write_mod(void *ptr, size_t size, size_t n, FILE **out) {
	if (size <= 0) {
		return 0;
	}
	if (*out == NULL) {
		*out = _wfopen(dll_tmp_path, L"wb");
		if (*out == NULL) {
			return 0;
		}
	}
	return fwrite(ptr, size, n, *out);
}

bool fetch_mod(wchar_t *version) {
	if (!curl_init) {
		curl_init = true;
		curl_global_init(CURL_GLOBAL_ALL);
	}
	CURL *curl = curl_easy_init();
	if (!curl) {
		goto error0;
	}
	char *fetch_url_utf8;
	if (version) {
		wchar_t fetch_url[512];
		if (!wsprintfW(fetch_url, L"%s%s", fetch_base_url, version)) {
			goto error1;
		}
		fetch_url_utf8 = to_utf8(fetch_url);
	} else {
		fetch_url_utf8 = to_utf8(fetch_base_url);
	}
	if (fetch_url_utf8 == NULL) {
		goto error1;
	}
	FILE *out = NULL;
	curl_easy_setopt(curl, CURLOPT_URL, fetch_url_utf8);
	curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT_MS, 2000L);
	curl_easy_setopt(curl, CURLOPT_TIMEOUT_MS, 20000L);
	curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, false);
	curl_easy_setopt(curl, CURLOPT_FAILONERROR, 1L);
	curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_mod);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, &out);
	if (curl_easy_perform(curl) || out == NULL) {
		if (version == NULL) {
			_beginthread(display_dialog, 0, L"Autopunch update failed, disabling autopunch.");
		} else if (out != NULL) {
			_beginthread(display_dialog, 0, L"Autopunch update failed, starting with old version.");
		}
		goto error2;
	}
	fclose(out);
	free(fetch_url_utf8);
	curl_easy_cleanup(curl);
	return true;
error2:
	free(fetch_url_utf8);
	if (out) {
		fclose(out);
	}
error1:
	curl_easy_cleanup(curl);
error0:
	return false;
}

void load_thread(void *data) {
	wchar_t *version = data;
	int r;
	if (version != NULL && version[0] == '\0') {
		r = false;
	} else {
		r = fetch_mod(version);
	}
	free(version);
	if (!r) {
		if (version == NULL) {
			return;
		}
	} else {
		bool s = true;
		if (!ReplaceFileW(dll_path, dll_tmp_path, NULL, 0, NULL, NULL)) {
			if (!MoveFileW(dll_tmp_path, dll_path)) {
				if (version == NULL) {
					_beginthread(display_dialog, 0, L"Autopunch update failed (replacing with a new version), disabling autopunch.");
					return;
				}
				_beginthread(display_dialog, 0, L"Autopunch update failed (replacing with a new version), starting with old version.");
				s = false;
			}
		}
		if (s) {
			_beginthread(display_dialog, 0, L"Autopunch update complete, please restart your game now.");
		}
	}
	// LoadLibraryW is detoured by SokuEngine and causes the mod to be unloaded: use LoadLibraryExW instead
	if (!LoadLibraryExW(dll_path, NULL, 0)) {
		_beginthread(display_dialog, 0, L"Injecting autopunch failed.");
	}
}

bool load() {
	if (!init_file_paths()) {
		return false;
	}
	wchar_t *version = file_version(dll_path);
	_beginthread(load_thread, 0, version);
	return true;
}

void unload() {
	if (curl_init) {
		curl_init = false;
		// TODO: introduce a lock
		curl_global_cleanup();
	}
}

__declspec(dllexport) bool CheckVersion(const BYTE hash[16]) {
	return true;
}

__declspec(dllexport) bool Initialize(HMODULE hSelf, HMODULE hInjector) {
	if (!load()) {
		return false;
	}
	return true;
}

bool WINAPI DllMain(HINSTANCE hInst, DWORD dwReason, LPVOID reserved) {
	if (dwReason == DLL_PROCESS_ATTACH) {
		self = hInst;
	}
	return true;
}
