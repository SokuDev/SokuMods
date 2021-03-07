#include <Windows.h>
#include "THMemPatcher2.hpp"
#include <asm.h>

/*	VERSION 1.1.0
#########################################################
#########################################################*/

#define NAKED __declspec(naked) void
#define EXPORT extern "C" __declspec(dllexport)

const BYTE TARGET_HASH[16] = {0xdf, 0x35, 0xd1, 0xfb, 0xc7, 0xb5, 0x83, 0x31, 0x7a, 0xda, 0xbe, 0x8c, 0xd9, 0xf5, 0x3b, 0x2e};

enum class Response : uint32_t { SUCCESS_SERVER = 10, SUCCESS_CLIENT = 11, EXIT = 2 };

enum class ProfileMenuItem : uint32_t { P1_PROFILE_SET, P2_PROFILE_SET, P1_DECK_BUILDER, P2_DECK_BUILDER, P1_KEY_CONFIG, P2_KEY_CONFIG, GO_TO_TITLE, EXIT };

#define KEY_C_KEYBOARD *reinterpret_cast<int32_t *>(0x008986A8 + 0x48)
#define KEY_C_GAMEPAD *reinterpret_cast<int32_t *>(0x0089A248 + 0x48)
#define IS_BUTTON_CLICKED() (KEY_C_KEYBOARD == 1 || KEY_C_GAMEPAD == 1)

#define GAME_SERVER_ADDR reinterpret_cast<LPVOID>(0x008574DC + 0x4)
#define GAME_SERVER_SCENE reinterpret_cast<Response (*)(void *This)>(0x004284E0)

#define GAME_CLIENT_ADDR reinterpret_cast<LPVOID>(0x00857534 + 0x4)
#define GAME_CLIENT_SCENE reinterpret_cast<Response (*)(void *This)>(0x004286F0)

static constexpr auto GAME_PLAY_SE_WAVE_BUFFER = 0x0043E1E0;
static constexpr auto GAME_NEW = 0x0081FBDC;
static constexpr auto GAME_CREATE_PROFILE_MENU = 0x0044D180;
static constexpr auto GAME_ADD_MENU = 0x0043E130;

void PROFILE_MENU();
void PROFILE_MENU_EXIT();
jmp_t<0x0044CC48, PROFILE_MENU, 6> profile_menu;
jmp_t<0x0044CE58, PROFILE_MENU_EXIT, 5> profile_menu_exit;
static constexpr auto profile_menu_ret = profile_menu.ret;

patch_t<0x004559D2, 0xB8, 0x00, 0x00> disable_keys_for_server;
patch_t<0x00454CC6, 0x90, 0x90, 0x90> disable_keys_for_client;
patch_t<0x0040A9CC, 0x00> disable_esc;

THMemPatcher2 patches;
bool inited = false;

/*###################################################################################################################*/
/*#################################################     src     #####################################################*/
/*###################################################################################################################*/

void disable_keys_in_character_select() {
	inited = true;
	patches.ApplyPatches();
}

void enable_keys_in_character_select() {
	patches.RestorePatches();
	inited = false;
}

NAKED CREATE_PROFILE_MENU() {
	__asm {
		PUSH 0x28
		CALL(GAME_PLAY_SE_WAVE_BUFFER)
		PUSH 0x1C4
		CALL(GAME_NEW)
		ADD ESP, 0x8
		CMP EAX, EDI
		JE loc1
		MOV ECX, EAX
		CALL(GAME_CREATE_PROFILE_MENU)
		JMP loc2
	loc1 :
		XOR EAX, EAX
	loc2 :
		PUSH EAX
		CALL(GAME_ADD_MENU)
		ADD ESP, 4
		RETN
	}
}

Response __fastcall SERVER_SCENE(void *This) {
	auto res = GAME_SERVER_SCENE(This);

	if (res == Response::SUCCESS_SERVER || res == Response::EXIT) {
		enable_keys_in_character_select();
	} else if (!inited && IS_BUTTON_CLICKED()) {
		disable_keys_in_character_select();
		CREATE_PROFILE_MENU();
	}

	return res;
}

Response __fastcall CLIENT_SCENE(void *This) {
	auto res = GAME_CLIENT_SCENE(This);

	if (res == Response::SUCCESS_CLIENT || res == Response::EXIT) {
		enable_keys_in_character_select();
	} else if (!inited && IS_BUTTON_CLICKED()) {
		disable_keys_in_character_select();
		CREATE_PROFILE_MENU();
	}

	return res;
}

NAKED PROFILE_MENU() {
	__asm {
		CMP EAX, 2 /* ProfileMenuItem::P1_DECK_BUILDER */
		JE default_action
		CMP EAX, 3 /* ProfileMenuItem::P2_DECK_BUILDER */
		JE default_action
		CMP EAX, 7 /* ProfileMenuItem::EXIT */
		JE default_action

	no_action:
		MOV AL, 1
		MOV ECX, DWORD PTR SS : [ESP + 0x18]
		MOV DWORD PTR FS : [0], ECX
		POP ECX
		POP EDI
		POP ESI
		POP EBP
		POP EBX
		ADD ESP, 0x10
		RETN
		
	default_action:
		JUMP([profile_menu_ret])
	}
}

NAKED PROFILE_MENU_EXIT() {
	__asm
	{
		PUSH EAX
		PUSH ECX
		CALL [enable_keys_in_character_select]
		POP ECX
		POP EAX
		POP EBX
		ADD ESP, 0x10
		RETN
	}
}

EXPORT bool CheckVersion(const BYTE hash[16]) {
	return memcmp(TARGET_HASH, hash, sizeof TARGET_HASH) == 0;
}

EXPORT bool Initialize(HMODULE hMyModule, HMODULE hParentModule) {
	patches.VTBLHook(GAME_SERVER_ADDR, SERVER_SCENE, true);
	patches.VTBLHook(GAME_CLIENT_ADDR, CLIENT_SCENE, true);
	patches.ApplyPatches();
	patches.AddDetourJump(profile_menu);
	patches.AddDetourJump(profile_menu_exit);
	patches.AddPatch(disable_keys_for_server);
	patches.AddPatch(disable_keys_for_client);
	patches.AddPatch(disable_esc);

	return true;
}

EXPORT void AtExit() {
	patches.ClearPatches();
}
