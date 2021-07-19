//
// Created by Gegel85 on 31/10/2020
//

#include "nlohmann/json.hpp"
#include <SokuLib.hpp>
#include <fstream>
#include <shlwapi.h>
#include <string>
#include <thread>
#include <sstream>
#include <dinput.h>

#ifndef _DEBUG
#define puts(...)
#define printf(...)
#endif

static int (SokuLib::Battle::* ogBattleOnProcess)();
static int (SokuLib::Battle::* ogBattleOnRender)();
static int (SokuLib::MenuResult::* ogResultOnProcess)();
static int (SokuLib::MenuResult::* ogResultOnRender)();
static bool stopToRepeat = false;
static SokuLib::DrawUtils::Sprite sprite;

std::map<unsigned, std::string> validCharacters{
	{ SokuLib::CHARACTER_REIMU, "reimu" },
	{ SokuLib::CHARACTER_MARISA, "marisa" },
	{ SokuLib::CHARACTER_SAKUYA, "sakuya" },
	{ SokuLib::CHARACTER_ALICE, "alice" },
	{ SokuLib::CHARACTER_PATCHOULI, "patchouli" },
	{ SokuLib::CHARACTER_YOUMU, "youmu" },
	{ SokuLib::CHARACTER_REMILIA, "remilia" },
	{ SokuLib::CHARACTER_YUYUKO, "yuyuko" },
	{ SokuLib::CHARACTER_YUKARI, "yukari" },
	{ SokuLib::CHARACTER_SUIKA, "suika" },
	{ SokuLib::CHARACTER_REISEN, "udonge" },
	{ SokuLib::CHARACTER_AYA, "aya" },
	{ SokuLib::CHARACTER_KOMACHI, "komachi" },
	{ SokuLib::CHARACTER_IKU, "iku" },
	{ SokuLib::CHARACTER_TENSHI, "tenshi" },
	{ SokuLib::CHARACTER_SANAE, "sanae" },
	{ SokuLib::CHARACTER_CIRNO, "chirno" },
	{ SokuLib::CHARACTER_MEILING, "meirin" },
	{ SokuLib::CHARACTER_UTSUHO, "utsuho" },
	{ SokuLib::CHARACTER_SUWAKO, "suwako" },
};

void loadSoku2CSV(LPWSTR path)
{
	std::ifstream stream{path};
	std::string line;

	if (stream.fail()) {
		printf("%S: %s\n", path, strerror(errno));
		return;
	}
	while (std::getline(stream, line)) {
		std::stringstream str{line};
		unsigned id;
		std::string idStr;
		std::string codeName;

		std::getline(str, idStr, ';');
		std::getline(str, codeName, ';');
		if (str.fail()) {
			printf("Skipping line %s: Stream failed\n", line.c_str());
			continue;
		}
		try {
			id = std::stoi(idStr);
		} catch (...){
			printf("Skipping line %s: Invalid id\n", line.c_str());
			continue;
		}
		validCharacters[id] = codeName;
	}
}

void loadSoku2Config()
{
	puts("Looking for Soku2 config...");

	int argc;
	wchar_t app_path[MAX_PATH];
	wchar_t setting_path[MAX_PATH];
	wchar_t **arg_list = CommandLineToArgvW(GetCommandLineW(), &argc);

	wcsncpy(app_path, arg_list[0], MAX_PATH);
	PathRemoveFileSpecW(app_path);
	if (GetEnvironmentVariableW(L"SWRSTOYS", setting_path, sizeof(setting_path)) <= 0) {
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
	printf("Config file is %S\n", setting_path);

	wchar_t moduleKeys[1024];
	wchar_t moduleValue[MAX_PATH];
	GetPrivateProfileStringW(L"Module", nullptr, nullptr, moduleKeys, sizeof(moduleKeys), setting_path);
	for (wchar_t *key = moduleKeys; *key; key += wcslen(key) + 1) {
		wchar_t module_path[MAX_PATH];

		GetPrivateProfileStringW(L"Module", key, nullptr, moduleValue, sizeof(moduleValue), setting_path);

		wchar_t *filename = wcsrchr(moduleValue, '/');

		printf("Check %S\n", moduleValue);
		if (!filename)
			filename = app_path;
		else
			filename++;
		for (int i = 0; filename[i]; i++)
			filename[i] = tolower(filename[i]);
		if (wcscmp(filename, L"soku2.dll") != 0)
			continue;

		wcscpy(module_path, app_path);
		PathAppendW(module_path, moduleValue);
		while (auto result = wcschr(module_path, '/'))
			*result = '\\';
		PathRemoveFileSpecW(module_path);
		printf("Found Soku2 module folder at %S\n", module_path);
		PathAppendW(module_path, L"\\config\\info\\characters.csv");
		loadSoku2CSV(module_path);
		return;
	}
}

void loadPack(char *path)
{

}

void loadAllPacks(LPCSTR profilePath, LPCSTR profileFolderPath, char *path)
{

}

// �ݒ胍�[�h
void LoadSettings(LPCSTR profilePath, LPCSTR profileFolderPath)
{
	char buffer[MAX_PATH];

	puts("Loading settings...");
	GetPrivateProfileStringA("TrialMode", "RefreshTime", "packs", buffer, sizeof(buffer), profilePath);
	loadAllPacks(profilePath, profileFolderPath, buffer);
}

extern "C" __declspec(dllexport) bool CheckVersion(const BYTE hash[16]) {
	return memcmp(hash, SokuLib::targetHash, 16) == 0;
}





// ToDo Launch Text function
int __fastcall myBattleOnProcess(SokuLib::Battle *This)
{
	int buffer = (This->*ogBattleOnProcess)();

	if (!stopToRepeat) {
		sprite.texture.loadFromGame("data/scene/select/character/08b_circle/circle_16.bmp");
		stopToRepeat = true;
	}

	return buffer;
}


int __fastcall myBattleOnRender(SokuLib::Battle *This)
{
	int buffer = (This->*ogBattleOnRender)();
	sprite.setPosition(SokuLib::DrawUtils::Vector2<int>{200, 300});
	sprite.setSize({128, 128});
	sprite.rect.top = sprite.rect.width = 0;
	sprite.rect.width = sprite.texture.getSize().x;
	sprite.rect.height = sprite.texture.getSize().y;
	sprite.draw();

	return buffer;
}

int __fastcall myResultOnProcess(SokuLib::MenuResult *This)
{
	auto &scene = SokuLib::currentScene;

	if (SokuLib::checkKeyOneshot(DIK_ESCAPE, 0, 0, 0)) {
		SokuLib::playSEWaveBuffer(0x29);
		return 0;
	}
	return 1;
}

int __fastcall myResultOnRender(SokuLib::MenuResult *This)
{
	return 0;
}



extern "C" __declspec(dllexport) bool Initialize(HMODULE hMyModule, HMODULE hParentModule) {
	char profilePath[1024 + MAX_PATH];
	char profileFolderPath[1024 + MAX_PATH];
	DWORD old;

#ifdef _DEBUG
	FILE *_;

	AllocConsole();
	freopen_s(&_, "CONOUT$", "w", stdout);
	freopen_s(&_, "CONOUT$", "w", stderr);
#endif

	puts("Initializing...");

	GetModuleFileName(hMyModule, profilePath, 1024);
	PathRemoveFileSpec(profilePath);
	strcpy(profileFolderPath, profilePath);
	PathAppend(profilePath, "TrialMode.ini");
	LoadSettings(profilePath, profileFolderPath);

	VirtualProtect((PVOID)RDATA_SECTION_OFFSET, RDATA_SECTION_SIZE, PAGE_EXECUTE_WRITECOPY, &old);
	ogBattleOnRender  = SokuLib::TamperDword(&SokuLib::VTable_Battle.onRender,  myBattleOnRender);
	ogBattleOnProcess = SokuLib::TamperDword(&SokuLib::VTable_Battle.onProcess, myBattleOnProcess);
	ogResultOnRender  = SokuLib::TamperDword(&SokuLib::VTable_Result.onRender,  myResultOnRender);
	ogResultOnProcess = SokuLib::TamperDword(&SokuLib::VTable_Result.onProcess, myResultOnProcess);
	VirtualProtect((PVOID)RDATA_SECTION_OFFSET, RDATA_SECTION_SIZE, old, &old);

	FlushInstructionCache(GetCurrentProcess(), nullptr, 0);
	puts("Done...");
	return true;
}

extern "C" int APIENTRY DllMain(HMODULE hModule, DWORD fdwReason, LPVOID lpReserved) {
	return TRUE;
}