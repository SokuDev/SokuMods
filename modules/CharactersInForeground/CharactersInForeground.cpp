#include "THMemPatcher.hpp"
#include <asm.h>
#include <map>
#include <set>

#define NAKED void __declspec(naked)
#define EXPORT extern "C" __declspec(dllexport)

/*	VERSION 1.1.0
#########################################################
#########################################################*/

THMemPatcher mem;

uintptr_t esiValue = 0;
uintptr_t ecxValue = 0;
uintptr_t draw_under_hud_ex_ret = 0;
uintptr_t draw_upper_hud_ex_ret = 0;

constexpr uintptr_t GAME_SetBlendMode = 0x00404B80;
constexpr uintptr_t GAME_DrawBG = 0x00470500;
constexpr uintptr_t GAME_DrawSpelcardBG = 0x0046E2E0;
constexpr uintptr_t GAME_DrawCharactersAndEfects = 0x0047A8D0;
constexpr uintptr_t GAME_DrawUpperHud = 0x0047D7A0;
constexpr uintptr_t GAME_DrawUnderHud = 0x0047DB9F;

void DRAW();
void DRAW_UPPER_HUD();
void DRAW_UPPER_HUD_EX();
void DRAW_UNDER_HUD();
void DRAW_UNDER_HUD_EX();
jmp_t<0x0047A9BD, DRAW, 20> draw;
jmp_t<GAME_DrawUnderHud, DRAW_UNDER_HUD, 6> draw_under_hud;
jmp_t<0x0047DC5E, DRAW_UNDER_HUD_EX, 10> draw_under_hud_ex;
jmp_t<GAME_DrawUpperHud, DRAW_UPPER_HUD, 6> draw_upper_hud;
jmp_t<0x0047DB95, DRAW_UPPER_HUD_EX, 10> draw_upper_hud_ex;

void InitItemList(DWORD);
void WeatherOrbFx(bool);
void DRAW_CARD_FX();
void GET_CARD_FX_LIST();
void CLEAR_CARD_FX_LIST();
jmp_t<0x0047DC23, DRAW_CARD_FX, 11> draw_card_fx;
jmp_t<0x0047F9CD, GET_CARD_FX_LIST, 6> get_card_fx_list;
jmp_t<0x0047F976, CLEAR_CARD_FX_LIST, 7> clear_card_fx_list;

/*#########################################################
#########################################################*/

static constexpr uintptr_t draw_upper_hud_ret = draw_upper_hud.ret;
NAKED DRAW_UPPER_HUD() {
	__asm
	{
		pop draw_upper_hud_ex_ret
		JUMP([draw_upper_hud_ret])
	}
}

NAKED DRAW_UPPER_HUD_EX() {
	__asm {
		// draw weather orb and effects
		MOV EDX, DWORD PTR DS : [ESI + 0x4]
		MOV EAX, DWORD PTR DS : [EDX + 0x18]
		LEA EDI, [ESI + 0x4]

		push edi
		call InitItemList
		add ESP, 4

		MOV EDX, DWORD PTR DS : [EDI]
		MOV EAX, DWORD PTR DS : [EDX + 0x18]
		PUSH 2
		MOV ECX, EDI
		CALL EAX

			/**/
		pushad
		pushfd
		push 1
		call WeatherOrbFx
		add ESP, 4
		popfd
		popad
			/**/

		MOV EDX, DWORD PTR DS : [EDI]
		MOV EAX, DWORD PTR DS : [EDX + 0x18]
		PUSH 1
		MOV ECX, EDI
		CALL EAX

			/**/
		pushad
		pushfd
		push 0
		call WeatherOrbFx
		add ESP, 4
		popfd
		popad
			/**/

		push 1
		mov ecx, 0x896B4C
		CALL(GAME_SetBlendMode)

		// draw upper hud end
		FLD DWORD PTR SS : [ESP + 0x10]
		MOV EDX, DWORD PTR DS : [EBX]
		MOV EAX, DWORD PTR DS : [EDX + 0x1C]
		SUB ESP, 8
		FSTP DWORD PTR SS : [ESP + 4]
		MOV ECX, EBX
		FLDZ
		FSTP DWORD PTR SS : [ESP]
		CALL EAX

			// draw weather orb and effects
		MOV EDX, DWORD PTR DS : [EDI]
		MOV EAX, DWORD PTR DS : [EDX + 0x18]
		PUSH 0
		MOV ECX, EDI
		CALL EAX

		JUMP([draw_upper_hud_ex_ret])
	}
}

static constexpr uintptr_t draw_under_hud_ret = draw_under_hud.ret;
NAKED DRAW_UNDER_HUD() {
	__asm {
		pop draw_under_hud_ex_ret

		push 1
		mov ecx, 0x896B4C
		CALL(GAME_SetBlendMode)

		// actual code
		FLD DWORD PTR SS : [ESP + 0x0C]
		MOV EDX, DWORD PTR DS : [ESI + 0x0CC]
		MOV EAX, DWORD PTR DS : [EDX + 0x1C]
		SUB ESP, 0x8

		JUMP([draw_under_hud_ret])
	}
}

NAKED DRAW_UNDER_HUD_EX() {
	__asm
	{
		JUMP([draw_under_hud_ex_ret])
	}
}

static constexpr uintptr_t draw_ret = draw.ret;
NAKED DRAW() {
	__asm {
		MOV esiValue, ESI
		MOV ECX, DWORD PTR DS : [0x8985E0]
		CALL(GAME_DrawBG)

		MOV ECX, DWORD PTR DS : [0x8985DC]
		PUSH -2
		CALL(GAME_DrawSpelcardBG)

		MOV ECX, DWORD PTR DS : [0x8985E8]
		MOV EAX, DWORD PTR DS : [ECX]
		MOV EDX, DWORD PTR DS : [EAX + 0x14]

		SUB ESP, 0x14
		PUSH EBX
		PUSH ESI
		PUSH EDI

		CALL(GAME_DrawUpperHud)

		MOV ecxValue, ECX
		MOV ECX, esiValue
		CALL(GAME_DrawCharactersAndEfects)
		MOV ECX, ecxValue

		CALL(GAME_DrawUnderHud)

		POP EDI
		POP ESI
		POP EBX
		ADD ESP, 0x14
		JUMP([draw_ret])
	}
}

/*#########################################################
#########################################################*/

DWORD itemList = 0;
std::set<DWORD> cardFxList;
std::map<unsigned, std::pair<float, float>> scales;

void InitItemList(DWORD ediValue) {
	itemList = *reinterpret_cast<DWORD *>(ediValue + 0x5C);
}
void InsertCardFx(DWORD val) {
	cardFxList.emplace(val);
}
void ClearCardFx() {
	cardFxList.clear();
}

bool FindCardFx(DWORD fx) {
	for (auto &ptr : cardFxList)
		if (fx == ptr)
			return true;
	return false;
}

void CardsHighlightsFx(bool enabled) {
	if (!itemList)
		return;
	unsigned ind = 0;
	DWORD nextItem = itemList;
	while (true) {
		nextItem = *reinterpret_cast<DWORD *>(nextItem);
		if (nextItem == itemList)
			break;
		DWORD fx = *reinterpret_cast<DWORD *>(nextItem + 0x8);
		if (!FindCardFx(fx)) {
			if (enabled) {
				scales[ind] = {*reinterpret_cast<float *>(fx + 0x11C), *reinterpret_cast<float *>(fx + 0x120)};
				*reinterpret_cast<float *>(fx + 0x11C) = 0;
				*reinterpret_cast<float *>(fx + 0x120) = 0;
			} else {
				auto scale = scales[ind];
				*reinterpret_cast<float *>(fx + 0x11C) = scale.first;
				*reinterpret_cast<float *>(fx + 0x120) = scale.second;
			}
		}

		++ind;
	}

	if (!enabled)
		scales.clear();
}

void WeatherOrbFx(bool enabled) {
	if (!itemList)
		return;
	DWORD nextItem = itemList;
	while (true) {
		nextItem = *reinterpret_cast<DWORD *>(nextItem);
		if (nextItem == itemList)
			break;
		DWORD fx = *reinterpret_cast<DWORD *>(nextItem + 0x8);
		if (FindCardFx(fx)) {
			*reinterpret_cast<float *>(fx + 0x11C) = !enabled; // set scale to 0 or 1
			*reinterpret_cast<float *>(fx + 0x120) = !enabled; // set scale to 0 or 1
		}
	}
}

static constexpr uintptr_t get_card_fx_list_ret = get_card_fx_list.ret;
NAKED GET_CARD_FX_LIST() {
	__asm
	{
		/**/
		pushad
		push[EDI]
		call InsertCardFx
		add ESP, 4
		popad
			/**/
		add EBP, 1
		add EDI, 4
		JUMP([get_card_fx_list_ret])
	}
}

static constexpr uintptr_t clear_card_fx_list_ret = clear_card_fx_list.ret;
NAKED CLEAR_CARD_FX_LIST() {
	__asm {
		movsx EAX, BYTE PTR DS : [EAX + 0x5E6]
		/**/
		cmp BYTE PTR [ESP + 0x2C], 2
		jne skip
		pushad
		call ClearCardFx
		popad
			/**/
		skip :
		JUMP([clear_card_fx_list_ret])
	}
}

static constexpr uintptr_t draw_card_fx_ret = draw_card_fx.ret;
NAKED DRAW_CARD_FX() {
	__asm {
		/**/
		pushad
		push edi
		call InitItemList
		add ESP, 4
		popad
		pushad
		push 1
		call CardsHighlightsFx
		add ESP, 4
		popad
			/**/

		MOV EDX, DWORD PTR DS : [EDI]
		MOV EAX, DWORD PTR DS : [EDX + 0x18]
		PUSH 1
		MOV ECX, EDI
		CALL EAX

			/**/
		pushad
		push 0
		call CardsHighlightsFx
		add ESP, 4
		popad
			/**/

		JUMP([draw_card_fx_ret])
	}
}

/*#########################################################
#########################################################*/

namespace SWRSToys {
/*SWRSToys*/
extern __declspec(selectany) const BYTE TARGET_HASH[16] = {0xdf, 0x35, 0xd1, 0xfb, 0xc7, 0xb5, 0x83, 0x31, 0x7a, 0xda, 0xbe, 0x8c, 0xd9, 0xf5, 0x3b, 0x2e};
EXPORT bool CheckVersion(const BYTE hash[16]) {
	return ::memcmp(TARGET_HASH, hash, sizeof TARGET_HASH) == 0;
}
} // namespace SWRSToys

EXPORT bool Initialize(HMODULE hMyModule, HMODULE hParentModule) {
	mem.ApplyDetourJump(draw);
	mem.ApplyDetourJump(draw_upper_hud);
	mem.ApplyDetourJump(draw_upper_hud_ex);
	mem.ApplyDetourJump(draw_under_hud);
	mem.ApplyDetourJump(draw_under_hud_ex);

	mem.Nop(0x0047A8D6, 5); // disable bg
	mem.Nop(0x0047A91B, 7); // disable spellcard bg
	mem.Nop(0x0047DBBF, 6); // disable weather orb fx
	mem.Nop(0x0047DC58, 6); // disable weather orb fx
	mem.Nop(0x0047DC3A, 25); // disable upperHud

	/*	we need to separate animation effect that highlights cards
		from other animation effects(orb, timer etc.)
		to put only the cards effect to foreground	*/
	mem.ApplyDetourJump(clear_card_fx_list);
	mem.ApplyDetourJump(get_card_fx_list);
	mem.ApplyDetourJump(draw_card_fx);

	return TRUE;
}

EXPORT void AtExit() {
	mem.Restore();
}
