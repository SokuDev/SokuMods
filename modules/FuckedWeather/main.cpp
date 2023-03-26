//
// Created by PinkySmile on 18/03/2023.
//

#include <SokuLib.hpp>
#include "KeyConfigs.hpp"

const float float1 = 2;
const float float125 = 1.25;
const float float15 = 1.5;
const float float2 = 2;
const double double2 = 2;
const double float300 = 300;
float extraSpeed = 0;

void __declspec(naked) sunnyOkuuHook()
{
	__asm {
		MOV EAX, 0x7a14c2
		JNZ normalCost
		PUSH 0
		JMP EAX
	normalCost:
		PUSH 10
		JMP EAX
	}
}

const unsigned remiliaHookRet = 0x46355E;

void __declspec(naked) sunnyRemiliaHook()
{
	__asm {
		MOV EAX, [ESI + 0x52C]
		TEST EAX, EAX
		JNZ normal

		MOV EAX, [ESI + 0x34C]
		CMP EAX, 6
		JNZ normal

		MOV EAX, [float15]
		MOV [ESI + 0x568], EAX

		FLD [float125]
		FLD [ESI + 0x564]
		FMUL st(1), st(0)
		FSTP [ESI + 0x564]
	normal:
		CMP word ptr [ESI + 0x196], 0x0
		JMP remiliaHookRet
	}
}

void placeSunnyHooks()
{
	std::pair<unsigned, char> patches[] = {
		// Reimu (was 5)
		{ 0x493801, 0x00 },
		// Marisa (was 5)
		{ 0x4BF89D, 0x00 },
		// Sakuya (was 5)
		{ 0x4E931B, 0x00 },
		// Alice (was 5)
		{ 0x511CE3, 0x00 },
		// Patchouli (was 4)
		{ 0x5437C8, 0x00 },
		// Youmu (was 6)
		{ 0x56E534, 0x00 },
		// Remilia (was 100)
		{ 0x589109, 0x00 },
		// Yuyuko (was 4)
		{ 0x5A71F9, 0x00 },
		// Yukari (was 5)
		{ 0x5CDA3A, 0x00 },
		// Suika (was 5)
		{ 0x5EFE67, 0x00 },
		// Reisen (was 5)
		{ 0x61F91B, 0x00 },
		// Aya (was 5)
		{ 0x666F05, 0x00 },
		// Komachi (was 5)
		{ 0x6462F0, 0x00 },
		// Iku (was 4)
		{ 0x689761, 0x00 },
		// Tenshi (was 5)
		{ 0x6AD4CA, 0x00 },
		// Sanae (was 5)
		{ 0x73E41D, 0x00 },
		// Cirno (was 5)
		{ 0x6E50DB, 0x00 },
		// Meiling (was 5)
		{ 0x71C754, 0x00 },
		// Suwako (change regen cooldown instead of cost, was 30)
		{ 0x7694BB, 0x0C },
		{ 0x769659, 0x0C }
	};

	for (auto patch : patches)
		*(char *)patch.first = patch.second;

	// Utsuho (was 5)
	memset((void *)0x7a14bc, 0x90, 6);
	SokuLib::TamperNearJmp(0x7a14bc, sunnyOkuuHook);
	memset((void *)0x463556, 0x90, 8);
	SokuLib::TamperNearJmp(0x463556, sunnyRemiliaHook);
}

void placeDrizzleHooks()
{
	// Damage 1.25 -> 2.00
	*(const float **)0x488AE8 = &float2;
}

void placeCloudyHooks()
{
	// This part is the +200% meter gain
	// fld st(0)
	// fadd st(0),st(0)
	// faddp st(1),st(0)
	// fstp dword ptr [esi+0000054C]
	// nop
	// We full throw the case for Blue Sky but that doesn't seem to have any effect.
	// I assume the game checks for Blue Sky somewhere else to allow for cancels
	unsigned char patch1[] = { 0xD9, 0xC0, 0xDC, 0xC0, 0xDE, 0xC1, 0xD9, 0x9E, 0x4C, 0x05, 0x00, 0x00, 0x90 };
	unsigned baseAddr1 = 0x488AFD;
	// This part is the 1 cost for all
	// Patch code that renders the effect
	unsigned char patch2[] = { 0xBB, 0x01, 0x00, 0x00, 0x00, 0x90, 0x90, 0x90 };
	unsigned baseAddr2 = 0x47F793;
	// Patch code called when using the card
	// This one validates using the card
	unsigned char patch3[] = { 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x75, 0x07 };
	unsigned baseAddr3 = 0x468EB8;
	// The number of cards removed when using the card
	unsigned char patch4[] = { 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x75, 0x07 };
	unsigned baseAddr4 = 0x469D10;

	for (auto byte : patch1)
		*(unsigned char *)baseAddr1++ = byte;
	for (auto byte : patch2)
		*(unsigned char *)baseAddr2++ = byte;
	for (auto byte : patch3)
		*(unsigned char *)baseAddr3++ = byte;
	for (auto byte : patch4)
		*(unsigned char *)baseAddr4++ = byte;
}

void __declspec(naked) blueSkyHook()
{
	__asm {
		MOV EAX, [ESP + 0x04]
		MOV EAX, [EAX + 0x52C]
		CMP EAX, 0x03
		JE fuckedCancel
		CMP EAX, 0x14
		JNE normalCancel

	fuckedCancel:
		MOV AX, word ptr [ESP + 0x14]
		CMP AX, 200
		JE normalCancel

		MOV AX, 0x7FFF
		RET
	normalCancel:
		MOV EAX, dword ptr [ESI + 0x4]
		MOV ECX, dword ptr [EAX + 0x10]
		MOV AX, word ptr [ECX + 0x10]
		RET
	}
}

void placeBlueSkyHooks()
{
	memset((void *)0x48964d, 0x90, 10);
	SokuLib::TamperNearCall(0x48964d, blueSkyHook);
}

void placeHailHooks()
{
	// Damage 1.25 -> 2.00
	*(const float **)0x488B4E = &float2;
}

const unsigned springHazeMeleeHookRet = 0x47B525;

void __declspec(naked) springHazeMeleeHook()
{
	__asm {
		PUSH 0x3C
		PUSH 0xC8
		MOV ECX, ESI
		JMP springHazeMeleeHookRet
	}
}

void placeSpringHazeHooks()
{
	// Multiply by 4 the spirit damage for grazing
	// Melee
	SokuLib::TamperNearJmp(0x47B51F, springHazeMeleeHook);
	*(unsigned char *)0x47B524 = 0x90;
	// Normal bullet in SpringHaze and type 2
	*(char *)0x47B57C = 0x10;
	// Type 2 bullet in SpringHaze
	*(char *)0x47B508 = 0x28;
}

void placeHeavyFogHooks()
{
	// Regen 0.50 -> 1.00
	*(const float **)0x488b65 = &float1;
}

void placeSnowHooks()
{
	// We force the meter loss to be 500; a full card
	*(unsigned char *)0x0047AFC9 = 0xBA;
	*(unsigned *)0x0047AFCA = 500;
	memset((void *)0x47AFCE, 0x90, 17);
}

const unsigned switchNormal = 0x47C5B3;
const unsigned switchDefaultCase = 0x47C605;
const unsigned airCrushAddress = 0x47C67C;
const unsigned guardPointCrushAddress = 0x47C71E;
const unsigned wrongBlockFct = 0x47C2E0;

enum BlockResult {
	BLOCKRESULT_HIT,
	BLOCKRESULT_HIGH_RIGHTBLOCK,
	BLOCKRESULT_HIGH_WRONGBLOCKBLOCK,
	BLOCKRESULT_LOW_RIGHTBLOCK,
	BLOCKRESULT_LOW_WRONGBLOCKBLOCK,
	BLOCKRESULT_AIRBLOCK,
	BLOCKRESULT_GUARDPOINT
};

void __declspec(naked) onBlockHook()
{
	__asm {
		PUSH EAX
		MOV EAX, [EDI + 0x170]
		MOV EAX, [EAX + 0x52C]
		CMP EAX, 0x08
		POP EAX
		JNE normal

		TEST EAX, EAX //BLOCKRESULT_HIT
		JZ normal

		// Wrong blocking already results in a crush
		//BLOCKRESULT_HIGH_WRONGBLOCKBLOCK
		CMP EAX, 2
		JE normal
		//BLOCKRESULT_LOW_WRONGBLOCKBLOCK
		CMP EAX, 4
		JE normal

		//BLOCKRESULT_HIGH_RIGHTBLOCKBLOCK
		CMP EAX, 1
		JE rightBlock
		//BLOCKRESULT_LOW_RIGHTBLOCKBLOCK
		CMP EAX, 3
		JE rightBlock

		CMP EAX, 5 //BLOCKRESULT_AIRBLOCK
		JE airBlock

		CMP EAX, 6 //BLOCKRESULT_GUARDPOINT
		JE guardPoint

		// Unhandled cases
		JMP normal

	airBlock:
		// This is a copy of the case 5@47C668 before the if
		// Here we don't call the og function, we call the one called on wrongblock
		PUSH ESI
		PUSH EDI
		MOV ECX, EBX
		CALL [wrongBlockFct]
		MOV EDX, [ESI]
		MOV EAX, [EDX + 0x08]
		MOV ECX, ESI
		JMP [airCrushAddress]

	guardPoint:
		// This is a copy of the case 5@47C668 before the if
		// Here we don't call the og function, we call the one called on wrongblock
		PUSH ESI
		PUSH EDI
		MOV ECX, EBX
		CALL [wrongBlockFct]
		JMP [guardPointCrushAddress]

	rightBlock:
		// We transform the right block into a wrong block
		ADD EAX, 1

	normal:
		CMP EAX, 6
		JA defaultCase
		JMP [switchNormal]
	defaultCase:
		JMP [switchDefaultCase]
	}
}

void placeSunShowerHooks()
{
	SokuLib::TamperNearJmp(0x47C5AE, onBlockHook);
}

void placeSprinkleHooks()
{
	*(char *)0x468B73 = 0x08;
}

const unsigned hookEndLocation = 0x48A38A;

void __declspec(naked) tempestDashHook()
{
	__asm {
		CMP [ECX + 0x52C], 0x0A
		JZ apply
		CMP [ECX + 0x52C], 0x14
		JNZ noOperation
	apply:
		MOV EDX, 6
		MOV EBP, EDX
		MOV [ESP + 0x10], EDX
	noOperation:
		MOVSX EAX, byte ptr [ESI + 0x49B]
		JMP [hookEndLocation]
	}
}

void placeTempestHooks()
{
	// Speed 1.40 -> 2.00
	*(const float **)0x489107 = &float2;
	// More air options
	memset((void *)0x48A383, 0x90, 7);
	SokuLib::TamperNearJmp(0x48A383, tempestDashHook);
}

void generatePermutation(SokuLib::CharacterManager &chr)
{
	unsigned id = chr.objectBase.hp +
		chr.spiritRegenDelay +
		chr.currentSpirit +
		chr.score +
		lroundf(chr.objectBase.position.x) +
		lroundf(chr.objectBase.position.y);

	*(unsigned short *)&chr.objectBase.offset_0x1CD[1] = id % nbPermutations;
}

void generatePermutations()
{
	auto &battleMgr = SokuLib::getBattleMgr();

	generatePermutation(battleMgr.leftCharacterManager);
	generatePermutation(battleMgr.rightCharacterManager);
}

const auto normalInputProcessing = (void (*)())0x46C937;
const auto skipInputProcessing = (void (*)())0x46CA0C;

void __fastcall mixInputs(SokuLib::CharacterManager &chr)
{
	auto combination = configPermutations[*(unsigned short *)&chr.objectBase.offset_0x1CD[1]];
	auto realInputs = chr.keyManager->keymapManager->input;

	if (combination.xSwapped)
		chr.keyMap.horizontalAxis = -realInputs.horizontalAxis;
	else
		chr.keyMap.horizontalAxis = realInputs.horizontalAxis;
	if (combination.ySwapped)
		chr.keyMap.verticalAxis = -realInputs.verticalAxis;
	else
		chr.keyMap.verticalAxis = realInputs.verticalAxis;
	if (combination.axisSwapped) {
		auto tmp = chr.keyMap.horizontalAxis;

		chr.keyMap.horizontalAxis = chr.keyMap.verticalAxis;
		chr.keyMap.verticalAxis = tmp;
	}
	for (int i = 0; i < 6; i++)
		(&chr.keyMap.a)[(&combination.aPosition)[i]] = (&realInputs.a)[i];
}

void __declspec(naked) mountainVaporHook()
{
	__asm {
		PUSH EAX
		MOV EAX, [ESI + 0x52C]
		CMP EAX, 0x14
		JE mountainVapor

		CMP EAX, 0x0D
		JE typhoon

		CMP EAX, 0x0B
		JNE normalProcessing

	mountainVapor:
		POP EAX
		MOV [ESP + 0x18], EAX
		MOV [ESP + 0x14], ECX
		MOV [ESP + 0x10], EDX
		PUSH EAX
		PUSH ECX
		PUSH EDX
		MOV ECX, ESI
		CALL mixInputs
		POP EDX
		POP ECX
		POP EAX
		PUSH EAX
		MOV EAX, [ESI + 0x52C]
		CMP EAX, 0x14
		POP EAX
		JE twilight
		JMP skipInputProcessing

	twilight:
		MOV EAX, [ESI + 0xF0]
		TEST EAX, EAX
		JZ twilightEnd

		XOR EAX, EAX
		MOV dword ptr [ESI + 0x768],EAX
	twilightEnd:
		JMP skipInputProcessing

	typhoon:
		MOV EAX, [ESI + 0xF0]
		TEST EAX, EAX
		JZ normalProcessing

		XOR EAX, EAX
		MOV [EDI + 0x4C], EAX

	normalProcessing:
		POP EAX
		MOV [ESI + 0x754], EBP
		JMP normalInputProcessing
	}
}

const auto weatherFunction = (void (*)())0x4388e0;

void placeMountainVaporHooks()
{
	new SokuLib::Trampoline(0x4397C9, generatePermutations, 6);
	SokuLib::TamperNearJmp(0x46C931, mountainVaporHook);
	*(unsigned char *)0x46C936 = 0x90;
}

void __fastcall riverMistApply(SokuLib::CharacterManager &chr)
{
	chr.objectBase.position.y += extraSpeed;
}

void __fastcall riverMistHookPrepare(SokuLib::CharacterManager &chr)
{
	float riverMistTimer = *(float *)&SokuLib::getBattleMgr().leftCharacterManager.offset_0x7F8[0x10];

	extraSpeed = sin(riverMistTimer / 20) * 15 + 5;
}

void __declspec(naked) riverMistBaseHook()
{
	__asm {
		PUSH EAX
		PUSH ECX
		PUSH EDX
		MOV ECX, ESI
		CALL riverMistApply
		POP EDX
		POP ECX
		POP EAX
		CMP byte ptr [ESI + 0x572],0x0
		RET
	}
}

void __declspec(naked) riverMistBaseHook2()
{
	__asm {
		PUSH EAX
		PUSH ECX
		PUSH EDX
		MOV ECX, ESI
		CALL riverMistHookPrepare
		POP EDX
		POP ECX
		POP EAX
		MOVZX EAX, word ptr [ESI + 0x13c]
		RET
	}
}

void __declspec(naked) riverMistBaseHook3()
{
	__asm {
		FST [ESI + 0x550]
		FST [extraSpeed]
		RET
	}
}

void placeRiverMistHooks()
{
	memset((void *)0x463667, 0x90, 7);
	SokuLib::TamperNearCall(0x463667, riverMistBaseHook);
	memset((void *)0x488D3D, 0x90, 7);
	SokuLib::TamperNearCall(0x488D3D, riverMistBaseHook2);
	memset((void *)0x488A32, 0x90, 6);
	SokuLib::TamperNearCall(0x488A32, riverMistBaseHook3);
}

void placeTyphoonHooks()
{
}

void calmEffect()
{
	SokuLib::CharacterManager *mgr;

	__asm MOV mgr, ESI;

	short strength = *(short *)&mgr->offset_0x4C0[0xE];

	mgr->attackPower *= 1.f + strength / 15.f;
	mgr->magicPotionTimeLeft += mgr->magicPotionTimeLeft == 0;
	mgr->magicPotionTimeLeft++;
}

void placeCalmHooks()
{
	new SokuLib::Trampoline(0x488F52, calmEffect, 6);
}

void __declspec(naked) diamondDustBaseHook()
{
	__asm {
		ADD EAX, -200
		mov [ESI + 0x184],ax
		RET
	}
}

void placeDiamondDustHooks()
{
	*(unsigned short *)0x485344 = 200;
	memset((void *)0x485348, 0x90, 10);
	SokuLib::TamperNearCall(0x485348, diamondDustBaseHook);
}

void __declspec(naked) dustStormHitStopHook()
{
	__asm {
		// Attacker
		MOV EDX, [EDI + 0x16C]
		// Defender
		MOV ECX, [ESI + 0x16C]
		CMP [EDX + 0x52C], 0x10
		JZ active
		CMP [EDX + 0x52C], 0x14
		JNZ normal

	active:
		CMP [ECX + 0x4A8], 30
		JZ modify

		CMP word ptr [EDX + 0x4B4], 0x0
		JNZ normal

		MOV byte ptr [ESP + 0x17], 01
		MOV DX, 30
		MOV [ECX + 0x4A8], DX

	modify:
		MOV CX, [EBP + 0x2A]
		MOV DX, [EBP + 0x2C]
		ADD DX, 30
		JMP apply

	normal:
		MOV CX, [EBP + 0x2A]
		MOV DX, [EBP + 0x2C]

	apply:
		MOV [EDI + 0x196], CX
		MOV [ESI + 0x196], DX
		CMP byte ptr [ESP + 0x17], 00
		RET
	}
}

void __declspec(naked) dustStormUntechHook()
{
	__asm {
		MOV ECX, [EDI + 0x16C]
		MOV ECX, [ECX + 0x52C]
		CMP ECX, 0x10
		JZ apply
		CMP ECX, 0x14
		JNZ normal

	apply:
		MOV AX, -1

	normal:
		MOV ECX, EDI
		MOV word ptr [ESI + 0x4BA], AX
		RET
	}
}

void __declspec(naked) dustStormDamageHook()
{
	__asm {
		MOV ECX, [EDI + 0x16C]
		MOV ECX, [ECX + 0x52C]
		CMP ECX, 0x10
		JZ apply
		CMP ECX, 0x14
		JNZ normal

	apply:
		MOVSX ECX, AX
		ADD ECX, ECX
		ADD CX, AX
		ADD ECX, ECX
		RET

	normal:
		MOVSX ECX, AX
		ADD ECX, ECX
		RET
	}
}

void placeDustStormHooks()
{
	memset((void *)0x47AE24, 0x90, 22);
	SokuLib::TamperNearCall(0x47AE24, dustStormHitStopHook);
	memset((void *)0x47AECE, 0x90, 7);
	SokuLib::TamperNearCall(0x47AECE, dustStormUntechHook);
	SokuLib::TamperNearCall(0x47AEE0, dustStormDamageHook);
}

const unsigned ogFunction = 0x820230;

void __declspec(naked) scorchingSunCardGain()
{
	__asm {
		ADD AX, AX
		ADD AX, AX
		CALL ogFunction
		RET
	}
}

void __declspec(naked) scorchingSunDamage()
{
	__asm {
		ADD DI, DI
		MOVZX EAX, word ptr [ESI + 0x184]
		RET
	}
}

void placeScorchingSunHooks()
{
	*(char *)0x488FB3 = 0x05;
	SokuLib::TamperNearCall(0x489023, scorchingSunCardGain);
	memset((void *)0x489053, 0x90, 7);
	SokuLib::TamperNearCall(0x489053, scorchingSunDamage);
	*(const double **)0x489084 = &float300;
}

void __declspec(naked) monsoonLimitHook()
{
	__asm {
		MOV EDX, [EDI + 0x16C]
		MOV EDX, [EDX + 0x52C]
		CMP EDX, 0x12
		JZ skip
		CMP EDX, 0x14
		JZ skip

		ADD word ptr [ESI + 0x4BE], AX
	skip:
		RET
	}
}

void placeMonsoonHooks()
{
	memset((void *)0x47AD8C, 0x90, 7);
	SokuLib::TamperNearCall(0x47AD8C, monsoonLimitHook);
	memset((void *)0x47AB58, 0x90, 7);
	SokuLib::TamperNearCall(0x47AB58, monsoonLimitHook);
}

void placeAuroraHooks()
{
}

__declspec(naked) void twilightWeatherFct()
{
	__asm {
		PUSH 0x14
		ADD ECX, 0x130
		CALL weatherFunction
		RET
	}
}

unsigned char oldData[5];

void applyPatch(unsigned addr)
{
	DWORD old;

	VirtualProtect((void *)0x488BC2, 1, PAGE_EXECUTE_WRITECOPY, &old);
	memcpy(oldData, (void *)0x488BC2, 5);
	SokuLib::TamperNearJmp(0x488BC2, addr);
	VirtualProtect((void *)0x488BC2, 1, old, &old);
}

void removePatch()
{
	DWORD old;

	VirtualProtect((void *)0x488BC2, 1, PAGE_EXECUTE_WRITECOPY, &old);
	memcpy((void *)0x488BC2, oldData, 5);
	VirtualProtect((void *)0x488BC2, 1, old, &old);
}

unsigned EAX_saved;
unsigned EDI_saved;
unsigned ECX_saved;
unsigned ESI_saved;
unsigned EBX_saved;
unsigned EDX_saved;
const unsigned endWeatherSwitch = 0x488BC2;
const unsigned startWeatherSwitch = 0x488ACA;

void __declspec(naked) twilightWeatherSwitchHook()
{
	__asm {
		CMP EAX, 0x14
		JNZ normal

		PUSH EAX
		PUSH EBX
		PUSH ECX
		PUSH EDX
		PUSH retAddr
		CALL applyPatch
		POP EDX
		POP EDX
		POP ECX
		POP EBX
		POP EAX
		MOV EAX, 0x4895C4

	loopLabel:
		mov [EAX_saved], EAX
		mov [EDI_saved], EDI
		mov [ECX_saved], ECX
		mov [ESI_saved], ESI
		mov [EBX_saved], EBX
		mov [EDX_saved], EDX
		JMP dword ptr [EAX]
	retAddr:
		mov EAX, [EAX_saved]
		mov EDI, [EDI_saved]
		mov ECX, [ECX_saved]
		mov ESI, [ESI_saved]
		mov EBX, [EBX_saved]
		mov EDX, [EDX_saved]
		ADD EAX, 4
		CMP EAX, 0x48960C
		JL loopLabel

		PUSH EAX
		PUSH EBX
		PUSH ECX
		PUSH EDX
		CALL removePatch
		POP EDX
		POP ECX
		POP EBX
		POP EAX
		JMP endWeatherSwitch

	normal:
		ADD EAX, -0x1
		CMP EAX, 0x11
		JMP startWeatherSwitch
	}
}

const unsigned skipResetWeatherLine = 0x4889BC;
const unsigned resetWeatherLine = 0x4888D1;

void __declspec(naked) twilightResetHook()
{
	__asm {
		MOV EAX, [ESI + 0x52C]
		CMP EAX, 0x14
		JNZ normal
		JMP skipResetWeatherLine
	normal:
		JMP resetWeatherLine
	}
}

void placeTwilightHooks()
{
	*(int *)0x483F3C = *(int *)0x483F38;
	*(void (**)())0x483F38 = twilightWeatherFct;
	*(char *)0x483DC4 = 0x14;
	SokuLib::TamperNearJmp(0x488AC5, twilightWeatherSwitchHook);
	*(unsigned char *)0x488ACA = 0x90;
	SokuLib::TamperNearJmp(0x4888CB, twilightResetHook);
	*(unsigned char *)0x4888D0 = 0x90;
}

void placeClearHooks()
{
}

const auto weatherTimerReturnAddr = (void (*)())0x4824FE;
const short weatherTimes[] {
	500, // WEATHER_SUNNY
	999, // WEATHER_DRIZZLE
	999, // WEATHER_CLOUDY
	750, // WEATHER_BLUE_SKY
	999, // WEATHER_HAIL
	500, // WEATHER_SPRING_HAZE
	999, // WEATHER_HEAVY_FOG
	999, // WEATHER_SNOW
	999, // WEATHER_SUN_SHOWER
	999, // WEATHER_SPRINKLE
	999, // WEATHER_TEMPEST
	999, // WEATHER_MOUNTAIN_VAPOR
	999, // WEATHER_RIVER_MIST
	999, // WEATHER_TYPHOON
	999, // WEATHER_CALM
	999, // WEATHER_DIAMOND_DUST
	999, // WEATHER_DUST_STORM
	750, // WEATHER_SCORCHING_SUN
	999, // WEATHER_MONSOON
	999, // WEATHER_AURORA
	999, // WEATHER_TWILIGHT
};

void __declspec(naked) weatherTimerHook()
{
	__asm {
		MOV EAX, 0x8971C4
		MOV EAX, [EAX]
		MOV AX, [EAX * 0x02 + weatherTimes]
		MOV ECX, 0x008971CC
		MOV [ECX], AX
		JMP weatherTimerReturnAddr
	}
}

void placeGeneralHooks()
{
	// Increase speed of timer in clear
	*(unsigned char *)0x48242B = 0x3;
	SokuLib::TamperNearJmp(0x48247A, weatherTimerHook);
}

extern "C" __declspec(dllexport) bool CheckVersion(const BYTE hash[16]) {
	return true;
}

extern "C" __declspec(dllexport) bool Initialize(HMODULE hMyModule, HMODULE hParentModule) {
	DWORD old1;
	DWORD old2;

#ifdef _DEBUG
	FILE *_;

	AllocConsole();
	freopen_s(&_, "CONOUT$", "w", stdout);
	freopen_s(&_, "CONOUT$", "w", stderr);
#endif

	// DWORD old;
	VirtualProtect((PVOID)TEXT_SECTION_OFFSET, TEXT_SECTION_SIZE, PAGE_EXECUTE_WRITECOPY, &old2);
	VirtualProtect((PVOID)RDATA_SECTION_OFFSET, RDATA_SECTION_SIZE, PAGE_EXECUTE_WRITECOPY, &old1);
	placeSunnyHooks();
	placeDrizzleHooks();
	placeCloudyHooks();
	placeBlueSkyHooks();
	placeHailHooks();
	placeSpringHazeHooks();
	placeHeavyFogHooks();
	placeSnowHooks();
	placeSunShowerHooks();
	placeSprinkleHooks();
	placeTempestHooks();
	placeMountainVaporHooks();
	placeRiverMistHooks();
	placeTyphoonHooks();
	placeCalmHooks();
	placeDiamondDustHooks();
	placeDustStormHooks();
	placeScorchingSunHooks();
	placeMonsoonHooks();
	placeAuroraHooks();
	placeTwilightHooks();
	placeClearHooks();
	placeGeneralHooks();
	VirtualProtect((PVOID)RDATA_SECTION_OFFSET, RDATA_SECTION_SIZE, old1, &old1);
	VirtualProtect((PVOID)TEXT_SECTION_OFFSET, TEXT_SECTION_SIZE, old2, &old2);

	FlushInstructionCache(GetCurrentProcess(), nullptr, 0);
	return true;
}

extern "C" int APIENTRY DllMain(HMODULE hModule, DWORD fdwReason, LPVOID lpReserved) {
return TRUE;
}