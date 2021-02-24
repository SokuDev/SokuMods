#include <windows.h>
#include <curl/curl.h>
#include <process.h>
#include <shlwapi.h>
#include <stdbool.h>

wchar_t ini_path[MAX_PATH];

char *scene_current = (char *)0x008A0044;
char *scene_next = (char *)0x008A0040;

#define UNKNOWN_GLOBAL 0x0089A888
#define IN_MENU *(char *)(UNKNOWN_GLOBAL + 4)
#define CMENU_OBJ (*(*((*(void ****)UNKNOWN_GLOBAL) + 1) + 2))
#define CMENUCONNECT_CHOICE ((char *)CMENU_OBJ)[8]
#define CMENUCONNECT_SUBCHOICE ((char *)CMENU_OBJ)[9]
#define CMENUCONNECT_PORT (((int *)CMENU_OBJ)[266])

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

CURL *curl;

struct curl_utf8 {
	char *ptr;
	int len;
};

size_t curl_write(void *ptr, size_t size, size_t nmemb, struct curl_utf8 *s) {
	size_t new_len = s->len + size * nmemb;
	s->ptr = realloc(s->ptr, new_len + 1);
	memcpy(s->ptr + s->len, ptr, size * nmemb);
	s->ptr[new_len] = '\0';
	s->len = new_len;
	return size * nmemb;
}

void get_ip(wchar_t *url, wchar_t *buf, int size) {
	if (!curl) {
		curl_global_init(CURL_GLOBAL_ALL);
		curl = curl_easy_init();
	}
	struct curl_utf8 data = {0};
	char *url_utf8;
	url_utf8 = to_utf8(url);
	curl_easy_setopt(curl, CURLOPT_URL, url_utf8);
	curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT_MS, 5000L);
	curl_easy_setopt(curl, CURLOPT_TIMEOUT_MS, 10000L);
	curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, false);
	curl_easy_setopt(curl, CURLOPT_FAILONERROR, 1L);
	curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curl_write);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, &data);
	if (!curl_easy_perform(curl) && data.len && *data.ptr) {
		MultiByteToWideChar(CP_UTF8, 0, data.ptr, -1, buf, size);
	}
	free(url_utf8);
	free(data.ptr);
	curl_easy_cleanup(curl);
}

void loop_thread(void *unused) {
	wchar_t command[1024];
	GetPrivateProfileStringW(L"Main", L"command", L"", command, sizeof(command), ini_path);
	wchar_t message[1024];
	GetPrivateProfileStringW(L"Main", L"message", L"", message, sizeof(message), ini_path);
	wchar_t url[1024];
	GetPrivateProfileStringW(L"Main", L"url", L"http://myexternalip.com/raw", url, sizeof(url), ini_path);

	wchar_t ip[16] = {0};
	int port = 0;

	while (true) {
		Sleep(100);

		if (*ip == '\0') {
			get_ip(url, ip, sizeof(ip));
			continue;
		}
		if (*scene_current != *scene_next || *scene_current != 2 /* TITLE */ || !IN_MENU || CMENUCONNECT_CHOICE != 1 || CMENUCONNECT_SUBCHOICE != 2) {
			port = 0;
			continue;
		}
		if (CMENUCONNECT_PORT == port) {
			continue;
		}
		port = CMENUCONNECT_PORT;

		int len = _snwprintf(NULL, 0, L"%s %s:%d %s", command, ip, port, message);
		HGLOBAL buf = GlobalAlloc(GMEM_MOVEABLE | GMEM_DDESHARE, (len + 1) * sizeof(wchar_t));
		wchar_t *dst = GlobalLock(buf);
		if (*command && *message) {
			_snwprintf(dst, len + 1, L"%s %s:%d %s", command, ip, port, message);
		} else if (*command) {
			_snwprintf(dst, len + 1, L"%s %s:%d", command, ip, port);
		} else if (*message) {
			_snwprintf(dst, len + 1, L"%s:%d %s", ip, port, message);
		} else {
			_snwprintf(dst, len + 1, L"%s:%d", ip, port);
		}
		GlobalUnlock(buf);
		if (OpenClipboard(NULL)) {
			EmptyClipboard();
			SetClipboardData(CF_UNICODETEXT, buf);
			CloseClipboard();
		}
	}
}

__declspec(dllexport) bool CheckVersion(const BYTE hash[16]) {
	return true;
}

__declspec(dllexport) bool Initialize(HMODULE self, HMODULE injector) {
	if (!GetModuleFileNameW(self, ini_path, MAX_PATH)) {
		return false;
	}
	if (!PathRemoveFileSpecW(ini_path)) {
		return false;
	}
	if (!PathAppendW(ini_path, L"GetIP.ini")) {
		return false;
	}

	_beginthread(loop_thread, 0, NULL);
	return true;
}

int APIENTRY DllMain(HMODULE hModule, DWORD fdwReason, LPVOID lpReserved) {
	return true;
}
