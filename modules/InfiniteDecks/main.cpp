#include <Windows.h>
#include <SokuLib.hpp>
#include <map>
#include "DrawUtils.hpp"

struct Select {};

static bool firstLoad = true;
static bool loaded = false;
static int (Select::*s_originalSelectOnProcess)();
static int (Select::*s_originalSelectCLOnProcess)();
static int (Select::*s_originalSelectSVOnProcess)();
static int (Select::*s_originalSelectOnRender)();
static int (Select::*s_originalSelectCLOnRender)();
static int (Select::*s_originalSelectSVOnRender)();
static int (Select::*s_originalInputMgrGet)();

std::array<std::map<unsigned short, DrawUtils::Sprite>, SokuLib::CHARACTER_RANDOM + 1> cardsTextures;
std::map<SokuLib::Character, std::vector<unsigned short>> characterSpellCards{
	{SokuLib::CHARACTER_ALICE, {200, 201, 202, 203, 204, 205, 206, 207, 208, 209, 210, 211}},
	{SokuLib::CHARACTER_AYA, {200, 201, 202, 203, 205, 206, 207, 208, 211, 212}},
	{SokuLib::CHARACTER_CIRNO, {200, 201, 202, 203, 204, 205, 206, 207, 208, 210, 213}},
	{SokuLib::CHARACTER_IKU, {200, 201, 202, 203, 206, 207, 208, 209, 210, 211}},
	{SokuLib::CHARACTER_KOMACHI, {200, 201, 202, 203, 204, 205, 206, 207, 211}},
	{SokuLib::CHARACTER_MARISA, {200, 202, 203, 204, 205, 206, 207, 208, 209, 211, 212, 214, 215, 219}},
	{SokuLib::CHARACTER_MEILING, {200, 201, 202, 203, 204, 205, 206, 207, 208, 209, 211}},
	{SokuLib::CHARACTER_PATCHOULI, {200, 201, 202, 203, 204, 205, 206, 207, 210, 211, 212, 213}},
	{SokuLib::CHARACTER_REIMU, {200, 201, 204, 206, 207, 208, 209, 210, 214, 219}},
	{SokuLib::CHARACTER_REMILIA, {200, 201, 202, 203, 204, 205, 206, 207, 208, 209}},
	{SokuLib::CHARACTER_SAKUYA, {200, 201, 202, 203, 204, 205, 206, 207, 208, 209, 210, 211, 212}},
	{SokuLib::CHARACTER_SANAE, {200, 201, 202, 203, 204, 205, 206, 207, 210}},
	{SokuLib::CHARACTER_SUIKA, {200, 201, 202, 203, 204, 205, 206, 207, 208, 212}},
	{SokuLib::CHARACTER_SUWAKO, {200, 201, 202, 203, 204, 205, 206, 207, 208, 209, 212}},
	{SokuLib::CHARACTER_TENSHI, {200, 201, 202, 203, 204, 205, 206, 207, 208, 209}},
	{SokuLib::CHARACTER_REISEN, {200, 201, 202, 203, 204, 205, 206, 207, 208, 209, 210, 211}},
	{SokuLib::CHARACTER_UTSUHO, {200, 201, 203, 204, 205, 206, 207, 208, 209, 210, 211, 212, 213, 214}},
	{SokuLib::CHARACTER_YOUMU, {200, 201, 202, 203, 204, 205, 206, 207, 208, 212}},
	{SokuLib::CHARACTER_YUKARI, {200, 201, 202, 203, 204, 205, 206, 207, 208, 215}},
	{SokuLib::CHARACTER_YUYUKO, {200, 201, 202, 203, 204, 205, 206, 207, 208, 209, 219}}
};
std::array<const char *, SokuLib::CHARACTER_RANDOM> names{
	"reimu",
	"marisa",
	"sakuya",
	"alice",
	"patchouli",
	"youmu",
	"remilia",
	"yuyuko",
	"yukari",
	"suika",
	"udonge",
	"aya",
	"komachi",
	"iku",
	"tenshi",
	"sanae",
	"chirno",
	"meirin",
	"utsuho",
	"suwako",
};

int __fastcall CSelectCL_OnRender(Select *This) {
	return (This->*s_originalSelectCLOnRender)();
}

int __fastcall CSelectSV_OnRender(Select *This) {
	return (This->*s_originalSelectSVOnRender)();
}

int __fastcall CSelect_OnRender(Select *This) {
	return (This->*s_originalSelectOnRender)();
}

static void loadTexture(DrawUtils::Sprite &container, const char *path, DrawUtils::Vector2<unsigned> size)
{
	int text;

	if (!SokuLib::textureMgr.loadTexture(&text, path, nullptr, nullptr) || !text) {
		MessageBoxA(SokuLib::window, ("Cannot load " + std::string(path)).c_str(), "Fatal error", MB_ICONERROR);
		abort();
	}
	printf("%s: %i\n", path, text);
	container.texture.setHandle(text, size);
}

static void loadCardAssets()
{
	char buffer[128];

	for (int i = 0; i <= 20; i++) {
		sprintf(buffer, "data/card/common/card%03i.bmp", i);
		loadTexture(cardsTextures[SokuLib::CHARACTER_RANDOM][i], buffer, {41, 65});
	}
	loadTexture(cardsTextures[SokuLib::CHARACTER_RANDOM][21], "data/battle/cardFaceDown.bmp", {41, 65});
	for (int j = 0; j < 20; j++) {
		for (int i = (j == SokuLib::CHARACTER_PATCHOULI ? 15 : 12); i; i--) {
			sprintf(buffer, "data/card/%s/card%03i.bmp", names[j], 99 + i);
			loadTexture(cardsTextures[j][99 + i], buffer, {41, 65});
		}
		for (auto &card : characterSpellCards.at(static_cast<SokuLib::Character>(j))) {
			sprintf(buffer, "data/card/%s/card%03i.bmp", names[j], card);
			loadTexture(cardsTextures[j][card], buffer, {41, 65});
		}
	}
}

static int selectProcessCommon(int v)
{
	if (v != SokuLib::SCENE_SELECT && v != SokuLib::SCENE_SELECTSV && v != SokuLib::SCENE_SELECTCL) {
		loaded = false;
		return v;
	}
	if (loaded)
		return v;
	if (firstLoad)
		loadCardAssets();
	loaded = true;
	firstLoad = false;
	return v;
}

int __fastcall CSelectCL_OnProcess(Select *This) {
	return selectProcessCommon((This->*s_originalSelectCLOnProcess)());
}

int __fastcall CSelectSV_OnProcess(Select *This) {
	return selectProcessCommon((This->*s_originalSelectSVOnProcess)());
}

int __fastcall CSelect_OnProcess(Select *This) {
	return selectProcessCommon((This->*s_originalSelectOnProcess)());
}

int __fastcall myGetInput(Select *This) {
	int ret = (This->*s_originalInputMgrGet)();
	auto keys = reinterpret_cast<SokuLib::KeymapManager *>(0x8a000c + 0x10);

	if (ret) {
		printf("%i %i %i %i %i %i %i %i\n", keys->input.horizontalAxis, keys->input.verticalAxis, keys->input.a, keys->input.b, keys->input.c, keys->input.d, keys->input.changeCard, keys->input.spellcard);
	}
	return ret;
}


extern "C" __declspec(dllexport) bool CheckVersion(const BYTE hash[16]) {
	return true;
}

extern "C" __declspec(dllexport) bool Initialize(HMODULE hMyModule, HMODULE hParentModule) {
	DWORD old;
	FILE *_;

	AllocConsole();
	freopen_s(&_, "CONOUT$", "w", stdout);
	freopen_s(&_, "CONOUT$", "w", stderr);
	puts("Hey !");
	::VirtualProtect((PVOID)RDATA_SECTION_OFFSET, RDATA_SECTION_SIZE, PAGE_EXECUTE_WRITECOPY, &old);
	s_originalSelectCLOnProcess = SokuLib::union_cast<int (Select::*)()>(SokuLib::TamperDword(
		SokuLib::vtbl_CSelectCL + SokuLib::OFFSET_ON_PROCESS,
		reinterpret_cast<DWORD>(CSelectCL_OnProcess)
	));
	s_originalSelectSVOnProcess = SokuLib::union_cast<int (Select::*)()>(SokuLib::TamperDword(
		SokuLib::vtbl_CSelectSV + SokuLib::OFFSET_ON_PROCESS,
		reinterpret_cast<DWORD>(CSelectSV_OnProcess)
	));
	s_originalSelectOnProcess = SokuLib::union_cast<int (Select::*)()>(SokuLib::TamperDword(
		SokuLib::vtbl_CSelect + SokuLib::OFFSET_ON_PROCESS,
		reinterpret_cast<DWORD>(CSelect_OnProcess)
	));
	s_originalSelectCLOnRender = SokuLib::union_cast<int (Select::*)()>(SokuLib::TamperDword(
		SokuLib::vtbl_CSelectCL + SokuLib::OFFSET_ON_RENDER,
		reinterpret_cast<DWORD>(CSelectCL_OnRender)
	));
	s_originalSelectSVOnRender = SokuLib::union_cast<int (Select::*)()>(SokuLib::TamperDword(
		SokuLib::vtbl_CSelectSV + SokuLib::OFFSET_ON_RENDER,
		reinterpret_cast<DWORD>(CSelectSV_OnRender)
	));
	s_originalSelectOnRender = SokuLib::union_cast<int (Select::*)()>(SokuLib::TamperDword(
		SokuLib::vtbl_CSelect + SokuLib::OFFSET_ON_RENDER,
		reinterpret_cast<DWORD>(CSelect_OnRender)
	));
	::VirtualProtect((PVOID)RDATA_SECTION_OFFSET, RDATA_SECTION_SIZE, old, &old);

	::VirtualProtect((PVOID)TEXT_SECTION_OFFSET, TEXT_SECTION_SIZE, PAGE_EXECUTE_WRITECOPY, &old);
	//Force deck icon to be hidden
	*(unsigned char *)0x4210e2 = 0xEB;
	s_originalInputMgrGet = SokuLib::union_cast<int (Select::*)()>(SokuLib::TamperNearJmpOpr(0x4206B3, reinterpret_cast<DWORD>(myGetInput)));
	::VirtualProtect((PVOID)TEXT_SECTION_OFFSET, TEXT_SECTION_SIZE, old, &old);

	::FlushInstructionCache(GetCurrentProcess(), nullptr, 0);
	return true;
}

extern "C" int APIENTRY DllMain(HMODULE hModule, DWORD fdwReason, LPVOID lpReserved) {
	return TRUE;
}