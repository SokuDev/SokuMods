#include <windows.h>
#include <cstdio>
#include <shlwapi.h>
#include <crtdbg.h>
#include <nlohmann/json.hpp>
#include <locale>
#include <codecvt>
#include <fstream>
#include <filesystem>
#include <direct.h>
#include <minidumpapiset.h>
#include "Module.hpp"
#include "ModConfigMenu.hpp"
#include "SWRSToys.h"

#define CRenderer_Unknown1 ((void (__thiscall *)(int, int))0x404AF0)
#define REAL_VERSION_STR "alpha 0.4.1"
#ifdef _NOTEX
#define MOD_REAL_VERSION_STR REAL_VERSION_STR " no texture"
#else
#define MOD_REAL_VERSION_STR REAL_VERSION_STR
#endif
#ifdef _DEBUG
#define VERSION_STR "version " MOD_REAL_VERSION_STR " DEBUG"
#else
#define VERSION_STR "version " MOD_REAL_VERSION_STR
#endif
#define hasEnglishPatch (*(int *)0x411c64 == 1)

static int (SokuLib::Title::*ogTitleOnProcess)();
static int (SokuLib::Title::*ogTitleOnRender)();

static bool loaded = false;
static SokuLib::DrawUtils::Sprite help;
static SokuLib::DrawUtils::Sprite version;

wchar_t app_path[MAX_PATH];
BYTE gameHash[16];
std::vector<Module> modules;
Module *head = nullptr;
SokuLib::SWRFont font;
HMODULE myModule;

std::wstring getLastError(int err = GetLastError())
{
	wchar_t *s = nullptr;
	std::wstring str;

	FormatMessageW(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		nullptr, err, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPWSTR)&s, 0, nullptr
	);
	str = s;
	LocalFree(s);
	return str;
}

LONG WINAPI UnhandledExFilter(PEXCEPTION_POINTERS ExPtr)
{
	if (!ExPtr) {
		puts("No ExPtr....");
		return 0;
	}
	puts("Caught fatal exception! Generating dump...");

	wchar_t buf[2048];
	wchar_t buf2[MAX_PATH];
	time_t timer;
	char timebuffer[31];
	struct tm* tm_info;

	time(&timer);
	tm_info = localtime(&timer);
	strftime(timebuffer, sizeof(timebuffer), "%Y-%m-%d-%H-%M-%S", tm_info);
	mkdir("crashes");
	wsprintfW(buf2, L"crashes/crash_%S.dmp", timebuffer);
	wsprintfW(buf, L"Game crashed!\nReceived fatal exception %X at address %x.\n", ExPtr->ExceptionRecord->ExceptionCode, ExPtr->ExceptionRecord->ExceptionAddress);

	HANDLE hFile = CreateFileW(buf2, GENERIC_READ | GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);

	if (hFile != INVALID_HANDLE_VALUE) {
		MINIDUMP_EXCEPTION_INFORMATION md;
		md.ThreadId = GetCurrentThreadId();
		md.ExceptionPointers = ExPtr;
		md.ClientPointers = FALSE;
		BOOL win = MiniDumpWriteDump(GetCurrentProcess(), GetCurrentProcessId(), hFile, MiniDumpNormal, &md, nullptr, nullptr);

		if (!win) {
			wcscat(buf, L"MiniDumpWriteDump failed.\n");
			wcscat(buf, getLastError().c_str());
			wcscat(buf, L": ");
		} else
			wcscat(buf, L"Minidump created ");
		wcscat(buf, buf2);
		CloseHandle(hFile);
	} else {
		wcscat(buf, L"CreateFileW(");
		wcscat(buf, buf2);
		wcscat(buf, L") failed: ");
		wcscat(buf, getLastError().c_str());
	}
	printf("%S\n", buf);
	MessageBoxW(nullptr, buf, L"Fatal Error", MB_ICONERROR);
	return EXCEPTION_CONTINUE_SEARCH;
}

void loadAssets()
{
	if (loaded)
		return;
	loaded = true;
	SokuLib::FontDescription desc;

	puts("Placed exception handler!");
	desc.r1 = 255;
	desc.r2 = 255;
	desc.g1 = 255;
	desc.g2 = 255;
	desc.b1 = 255;
	desc.b2 = 255;
	desc.height = 12 + hasEnglishPatch * 2;
	desc.weight = FW_NORMAL;
	desc.italic = 0;
	desc.shadow = 1;
	desc.bufferSize = 1000000;
	desc.charSpaceX = 0;
	desc.charSpaceY = 0;
	desc.offsetX = 0;
	desc.offsetY = 0;
	desc.useOffset = 0;
	strcpy(desc.faceName, "MonoSpatialModSWR");
	desc.weight = FW_REGULAR;
	font.create();
	font.setIndirect(desc);

	SokuLib::Vector2i size;

	version.texture.createFromText(("SokuModLoader " + std::string(VERSION_STR)).c_str(), font, {300, 150}, &size);
	version.setSize({300, 150});
	version.rect.width = 300;
	version.rect.height = 150;
	version.setPosition({635 - size.x, 5});

	int height = size.y;

	help.texture.createFromText("Press D or Pad 5 to access mod settings", font, {300, 150}, &size);
	help.setSize({300, 150});
	help.rect.width = 300;
	help.rect.height = 150;
	help.setPosition({635 - size.x, 5 + height});
}

void displaySokuCursor(SokuLib::Vector2i pos, SokuLib::Vector2u size)
{
	SokuLib::Sprite (&CursorSprites)[3] = *(SokuLib::Sprite (*)[3])0x89A6C0;

	//0x443a50 -> Vanilla display cursor
	//We have this here because the vanilla cursor display function has a fixed size
	CursorSprites[0].scale.x = size.x * 0.00195313f;
	CursorSprites[0].scale.y = size.y / 16.f;
	pos.x -= 7;
	CursorSprites[0].render(pos.x, pos.y);
	CRenderer_Unknown1(0x896B4C, 2);
	CursorSprites[1].rotation = *(float *)0x89A450 * 4.00000000f;
	CursorSprites[1].render(pos.x, pos.y + 8.00000000f);
	CursorSprites[2].rotation = -*(float *)0x89A450 * 4.00000000f;
	CursorSprites[2].render(pos.x - 14.00000000f, pos.y - 1.00000000f);
	CRenderer_Unknown1(0x896B4C, 1);
}

class CryptHash {
public:
	CryptHash(): prov(NULL), hash(NULL) {}
	~CryptHash() {
		if (hash != NULL)
			::CryptDestroyHash(hash);
		if (prov != NULL)
			::CryptReleaseContext(prov, 0);
	}
	bool Initialize() {
		if (FALSE == ::CryptAcquireContextW(&prov, nullptr, nullptr, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT))
			return false;
		if (FALSE == ::CryptCreateHash(prov, CALG_MD5, 0, 0, &hash))
			return false;
		return true;
	}
	bool AddData(const BYTE *buffer, unsigned int buffer_size) {
		return ::CryptHashData(hash, buffer, buffer_size, 0);
	}
	bool GetValue(BYTE *buffer, DWORD buffer_size) const {
		DWORD ret_size = buffer_size;
		if (FALSE == ::CryptGetHashParam(hash, HP_HASHVAL, nullptr, &ret_size, 0) || ret_size != buffer_size)
			return false;
		if (FALSE == ::CryptGetHashParam(hash, HP_HASHVAL, buffer, &ret_size, 0))
			return false;
		return true;
	}

protected:
	HCRYPTPROV prov;
	HCRYPTHASH hash;
};

// convert UTF-8 string to wstring
std::wstring utf8_to_wstring(const std::string& str)
{
	std::wstring_convert<std::codecvt_utf8<wchar_t>> myconv;

	return myconv.from_bytes(str);
}

// convert wstring to UTF-8 string
std::string wstring_to_utf8(const std::wstring &str)
{
	std::wstring_convert<std::codecvt_utf8<wchar_t>> myconv;

	return myconv.to_bytes(str);
}

void generateFakeIni()
{
	std::ofstream stream{std::wstring(app_path) + L"\\SWRSToys.ini"};

	stream << "[Module]" << std::endl;
	for (size_t i = 0; i < modules.size(); i++) {
		if (!modules[i].enabled)
			stream << ";";

		auto str = std::filesystem::path(modules[i].getPath()).string();

		for (char & j : str)
			if (j == '\\')
				j = '/';
		stream << "Mod" << i << "=" << str << std::endl;
	}
	stream.close();
}

void reloadModules()
{
	Module *last = nullptr;
	std::ifstream ifs{std::wstring(app_path) + L"/currentModule.txt"};
	std::string line;

	std::getline(ifs, line);
	ifs.close();
	for (auto &module : modules) {
		if (!module.enabled)
			continue;

		if (line == wstring_to_utf8(module.getPath())) {
			MessageBoxW(nullptr, (module.getName() + L" has been disabled because the game crashed while loading it last time.\nYou can enable it again in the mod config menu.").c_str(), L"Module disabled", MB_ICONWARNING);
			module.enabled = false;
			saveSettings();
			continue;
		}

		std::ofstream of{std::wstring(app_path) + L"/currentModule.txt"};

		of << wstring_to_utf8(module.getPath()) << std::endl;
		of.close();
		if (!module.load())
			MessageBoxW(nullptr, module.getLastError().c_str(), L"Module prepare error", MB_ICONWARNING);
	}
	std::sort(modules.begin(), modules.end());
	generateFakeIni();
	for (auto &module : modules) {
		if (!module.enabled)
			break;
		if (!module.getLastError().empty())
			continue;

		std::ofstream of{std::wstring(app_path) + L"/currentModule.txt"};

		of << wstring_to_utf8(module.getPath()) << std::endl;
		of.close();
		if (!module.init()) {
			MessageBoxW(nullptr, module.getLastError().c_str(), L"Module init error", MB_ICONERROR);
			continue;
		}

		module.next = nullptr;
		module.prev = last;
		if (last)
			last->next = &module;
		else
			head = &module;
		last = &module;
	}
	if (_wunlink((std::wstring(app_path) + L"/currentModule.txt").c_str()) < 0 && errno != ENOENT)
		MessageBoxA(nullptr, ("Could not delete currentModule.txt: " + std::string(strerror(errno))).c_str(), "Error", MB_ICONERROR);
}

bool getSWRSHash(BYTE retVal[16]) {
	CryptHash hash;
	if (!hash.Initialize()) {
		return false;
	}

	wchar_t swrs_path[MAX_PATH];
	if (FALSE == ::GetModuleFileNameW(nullptr, swrs_path, MAX_PATH)) {
		return false;
	}

	HANDLE file = ::CreateFileW(swrs_path, GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, 0, nullptr);
	if (!file) {
		return false;
	}

	while (true) {
		BYTE buff[1024];
		DWORD readSize;
		if (FALSE == ::ReadFile(file, buff, sizeof(buff), &readSize, nullptr)) {
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

void saveSettings()
{
	nlohmann::json json;
	nlohmann::json moduleObj;

	for (auto &module : modules) {
		nlohmann::json moduleJson;

		moduleJson["enabled"] = module.enabled;
		if (module.customPriority)
			moduleJson["priority"] = module.priority;
		moduleObj[wstring_to_utf8(module.getPath())] = moduleJson;
	}
	json["modules"] = moduleObj;

	auto str = json.dump(4);
	std::ofstream stream("ModLoaderSettings.json");

	stream << str;
}

void loadImportList(byte hash[16], HMODULE this_module)
{
	auto listPath = std::wstring(app_path) + L"/modImportList.txt";
	std::ifstream stream(listPath);

	if (stream.fail())
		return;
	if (MessageBoxA(nullptr, "A mod import list has been found. Do you want to load the mods from list? All the mods specified in the list will be imported and enabled.", "Import mod list?", MB_ICONINFORMATION | MB_YESNO) == IDNO)
		return (void)_wunlink(listPath.c_str());

	std::string line;

	while (std::getline(stream, line)) {
		std::filesystem::path path = utf8_to_wstring(line);
		auto moduleIt = std::find_if(modules.begin(), modules.end(), [&path](const Module &module) {
			return module.getPath() == path;
		});

		if (path.is_relative())
			path = std::filesystem::path(app_path) / path;
		if (moduleIt == modules.end()) {
			DWORD attrs = GetFileAttributesW(path.c_str());

			if (attrs == INVALID_FILE_ATTRIBUTES || (attrs & FILE_ATTRIBUTE_DIRECTORY))
				continue;
			modules.emplace_back(path.c_str(), hash, this_module);
			moduleIt = std::prev(modules.end());
		}
		moduleIt->enabled = true;
	}
	stream.close();
	_wunlink(listPath.c_str());
}

void loadSettings(byte hash[16], HMODULE this_module)
{
	auto jsonPath = std::wstring(app_path) + L"/ModLoaderSettings.json";
	std::ifstream stream(jsonPath);
	nlohmann::json json;

	printf("Loading settings json from %S\n", jsonPath.c_str());
	if (!stream.is_open()) {
		MessageBoxA(nullptr, ("ModLoaderSettings.json could not be loaded: " + std::string(strerror(errno))).c_str(), "Settings loading error", MB_ICONERROR);
		return;
	}

	stream >> json;
	for (auto &module : json["modules"].items()) {
		auto val = module.value();
		std::filesystem::path path = utf8_to_wstring(module.key());
		auto enabled = val.contains("enabled") && val["enabled"].is_boolean() && val["enabled"];
		auto moduleIt = std::find_if(modules.begin(), modules.end(), [&path](const Module &module) {
			return module.getPath() == path;
		});

		if (path.is_relative())
			path = std::filesystem::path(app_path) / path;
		if (moduleIt == modules.end()) {
			DWORD attrs = GetFileAttributesW(path.c_str());

			if (attrs == INVALID_FILE_ATTRIBUTES || (attrs & FILE_ATTRIBUTE_DIRECTORY))
				continue;
			modules.emplace_back(path.c_str(), hash, this_module);
			moduleIt = std::prev(modules.end());
		}
		moduleIt->enabled = enabled;
		if (val.contains("priority") && val["priority"].is_number_integer()) {
			moduleIt->customPriority = true;
			moduleIt->priority = val["priority"];
		}
	}
	loadImportList(hash, this_module);
}

void convertIni(byte hash[16], HMODULE this_module)
{
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
		if (arg_list)
			LocalFree(arg_list);
	}

	wchar_t moduleKeys[1024];
	wchar_t moduleValue[MAX_PATH];

	GetPrivateProfileStringW(L"Module", nullptr, nullptr, moduleKeys, sizeof(moduleKeys), setting_path);
	for (wchar_t *key = moduleKeys; *key; key += wcslen(key) + 1) {
		GetPrivateProfileStringW(L"Module", key, nullptr, moduleValue, sizeof(moduleValue), setting_path);
		while (auto ptr = wcschr(moduleValue, L'/'))
			*ptr = L'\\';

		auto module = std::find_if(modules.begin(), modules.end(), [&](const Module &m) {
			try {
				return std::filesystem::equivalent(m.getPath(), moduleValue);
			} catch(std::filesystem::filesystem_error const& ex) {
				return false;
			}
		});

		if (module == modules.end()) {
			auto path = std::filesystem::path(moduleValue);

			if (path.is_relative())
				path = app_path / path;

			DWORD attrs = GetFileAttributesW(path.c_str());

			if (attrs == INVALID_FILE_ATTRIBUTES || (attrs & FILE_ATTRIBUTE_DIRECTORY)) {
				MessageBoxW(nullptr, (L"Module " + std::wstring(moduleValue) + L" specified in old INI is not a valid file\nIt will be ignored").c_str(), L"Module loading error", MB_ICONWARNING);
				continue;
			}
			modules.emplace_back(Module(moduleValue, hash, this_module));
			module = std::prev(modules.end());
		}
		module->enabled = true;
	}
	saveSettings();
}

bool exists(const wchar_t *path)
{
	DWORD attrs = GetFileAttributesW(path);

	return (attrs != INVALID_FILE_ATTRIBUTES && !(attrs & FILE_ATTRIBUTE_DIRECTORY));
}

void importModulesFromFolder(byte hash[16], HMODULE this_module, const std::wstring &path, unsigned maxRecursion = 2)
{
	if (maxRecursion == 0)
		return;

	WIN32_FIND_DATAW findData;
	auto ret = FindFirstFileW((path + L"\\*").c_str(), &findData);

	printf("Scanning folder %S\n", path.c_str());
	if (ret == INVALID_HANDLE_VALUE)
		return;
	do {
		if (wcscmp(findData.cFileName, L".") == 0 || wcscmp(findData.cFileName, L"..") == 0)
			continue;

		std::filesystem::path module_path = path + L"/" + findData.cFileName;
		DWORD attrs = GetFileAttributesW(module_path.c_str());

		if (attrs != INVALID_FILE_ATTRIBUTES) {
			if (attrs & FILE_ATTRIBUTE_DIRECTORY)
				importModulesFromFolder(hash, this_module, module_path, maxRecursion - 1);
			else if (module_path.extension() == L".dll")
				modules.emplace_back(module_path.c_str(), hash, this_module);
		}
	} while (FindNextFileW(ret, &findData));
	FindClose(&findData);
}

void loadModuleList(byte hash[16], HMODULE this_module)
{
	importModulesFromFolder(hash, this_module, std::wstring(app_path) + L"\\Modules");
	printf("%zi modules found\n", modules.size());
}

int __fastcall myTitleOnProcess(SokuLib::Title *This)
{
	int ret = (This->*ogTitleOnProcess)();

	loadAssets();
	if (SokuLib::inputMgrs.input.spellcard == 1 && SokuLib::menuManager.empty()) {
		SokuLib::playSEWaveBuffer(0x3D);
		SokuLib::activateMenu(new ModConfigMenu());
	}
	return ret;
}

int __fastcall myTitleOnRender(SokuLib::Title *This)
{
	int ret = (This->*ogTitleOnRender)();

	version.draw();
	help.draw();
	return ret;
}

void trapDebugger(bool b)
{
	if (!b)
		return;
	DebugBreak();
}

bool Hook(HMODULE this_module)
{
#ifdef _DEBUG
	FILE *_;

	AllocConsole();
	freopen_s(&_, "CONOUT$", "w", stdout);
	freopen_s(&_, "CONOUT$", "w", stderr);
#endif

	GetModuleFileNameW(this_module, app_path, MAX_PATH);
	PathRemoveFileSpecW(app_path);
	myModule = this_module;
	puts("SokuModLoader " VERSION_STR);
	try {
		DWORD old;

		if (!getSWRSHash(gameHash))
			return false;
		puts("Discovering modules in Modules folder");
		loadModuleList(gameHash, this_module);
		if (!exists((std::wstring(app_path) + L"/ModLoaderSettings.json").c_str())) {
			puts("No settings file found, creating one from SWRSToys.ini");
			convertIni(gameHash, this_module);
		}
		puts("Loading settings file");
		loadSettings(gameHash, this_module);
		puts("Placing hooks");
		SetUnhandledExceptionFilter(UnhandledExFilter);
		VirtualProtect((PVOID)TEXT_SECTION_OFFSET, TEXT_SECTION_SIZE, PAGE_EXECUTE_WRITECOPY, &old);
		*(char *)0x81FF00 = 0x90;
		*(char *)0x81FF01 = 0x50;
		*(char *)0x81FF04 = 0x90;
		SokuLib::TamperNearCall(0x81FF05, trapDebugger);
		VirtualProtect((PVOID)TEXT_SECTION_OFFSET, TEXT_SECTION_SIZE, old, &old);
		VirtualProtect((PVOID)RDATA_SECTION_OFFSET, RDATA_SECTION_SIZE, PAGE_EXECUTE_WRITECOPY, &old);
		ogTitleOnProcess = SokuLib::TamperDword(&SokuLib::VTable_Title.onProcess, myTitleOnProcess);
		ogTitleOnRender  = SokuLib::TamperDword(&SokuLib::VTable_Title.onRender,  myTitleOnRender);
		VirtualProtect((PVOID)RDATA_SECTION_OFFSET, RDATA_SECTION_SIZE, old, &old);
		reloadModules();
	} catch (std::exception &e) {
		MessageBoxA(nullptr, e.what(), "Exception", MB_ICONERROR);
		return false;
	}
	return true;
}
