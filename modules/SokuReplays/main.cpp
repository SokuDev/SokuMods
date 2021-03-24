#include <windows.h>
#include <curl/curl.h>
#include <inttypes.h>
#include <process.h>
#include <psapi.h>
#include <random>
#include <shlwapi.h>

char *to_utf8(const wchar_t *str) {
	int length = WideCharToMultiByte(CP_UTF8, 0, str, -1, 0, 0, NULL, NULL);
	if (!length) {
		return NULL;
	}
	char *str_utf8 = (char *)malloc(length);
	int r = WideCharToMultiByte(CP_UTF8, 0, str, -1, str_utf8, length, NULL, NULL);
	if (!r) {
		free(str_utf8);
		return NULL;
	}
	return str_utf8;
}

void loop_thread(void *unused) {
	HKEY key;
	unsigned long long uid;
	if (!RegCreateKeyExW(HKEY_CURRENT_USER, L"Software\\Touhou\\Soku", 0, NULL, 0, KEY_READ | KEY_WRITE | KEY_WOW64_64KEY, NULL, &key, NULL)) {
		DWORD size = sizeof(uid);
		if (RegQueryValueExW(key, L"SokuReplaysID", NULL, NULL, (byte *)&uid, &size)) {
			std::random_device rd;
			std::mt19937_64 gen(rd());
			std::uniform_int_distribution<unsigned long long> dis;
			uid = dis(gen) >> 1; // let's only keep 63-bits, positive values to avoid any sign quirk...
			RegSetValueExW(key, L"SokuReplaysID", 0, REG_QWORD, (BYTE*)&uid, size);
		}
		RegCloseKey(key);
	}

	// give time for discordintegration to initialize itself
	Sleep(4000);

	unsigned long long discord_id = 0;

	HMODULE modules[1024];
	DWORD needed;
	if (EnumProcessModules(GetCurrentProcess(), modules, sizeof(modules), &needed)) {
		for (int i = 0; i < (needed / sizeof(HMODULE)); i++) {
			wchar_t module_name[MAX_PATH];
			if (GetModuleFileNameExW(GetCurrentProcess(), modules[i], module_name, MAX_PATH) && StrStrIW(module_name, L"DiscordIntegration")) {
				unsigned long long (*get_discord_id)() = (unsigned long long (*)())GetProcAddress(modules[i], "DiscordId");
				if (get_discord_id) {
					discord_id = get_discord_id();
					break;
				}
			}
		}
	}

	curl_global_init(CURL_GLOBAL_ALL);
	CURL *curl = curl_easy_init();

	wchar_t buf[1024];
	const wchar_t *url_template = L"http://sokureplaysback.delthas.fr/mod/ip?uid=%" PRIu64 "&discord_id=%" PRIu64;
	_snwprintf(buf, sizeof(buf), url_template, uid, discord_id);
	char *url_utf8;
	url_utf8 = to_utf8(buf);

	curl_easy_setopt(curl, CURLOPT_URL, url_utf8);
	curl_easy_setopt(curl, CURLOPT_USERAGENT, "swrstoys-sokureplays");
	curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT_MS, 5000L);
	curl_easy_setopt(curl, CURLOPT_TIMEOUT_MS, 10000L);
	curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, false);
	curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
	curl_easy_setopt(curl, CURLOPT_IPRESOLVE, CURL_IPRESOLVE_V4);
	curl_easy_perform(curl);
	free(url_utf8);
	curl_easy_cleanup(curl);
	curl_global_cleanup();
}

extern "C" __declspec(dllexport) bool CheckVersion(const BYTE hash[16]) {
	return true;
}

extern "C" __declspec(dllexport) bool Initialize(HMODULE self, HMODULE injector) {
	_beginthread(loop_thread, 0, NULL);
	return true;
}

extern "C" int APIENTRY DllMain(HMODULE hModule, DWORD fdwReason, LPVOID lpReserved) {
	return true;
}
