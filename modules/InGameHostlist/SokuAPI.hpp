#pragma once
#include <windows.h>
//
#include "PatchMan.hpp"

#define UNKNOWN_GLOBAL 0x0089a888
#define IN_MENU *(char *)(0x089a888 + 4)

#define CMENU_OBJ (*(*((*(void ****)UNKNOWN_GLOBAL) + 1) + 2))
//#define CMENUCONNECT_CNETWORKBASEOBJ ((void**)CMENU_OBJ+4)
//#define CMENUCONNECT_CHOICE ((char*)CMENU_OBJ)[8]
//#define CMENUCONNECT_SUBCHOICE ((char*)CMENU_OBJ)[9]
//#define CMENUCONNECT_SPECTATE ((char*)CMENU_OBJ)[953]
//#define CMENUCONNECT_IPPORTSTR1 ((char*)CMENU_OBJ+1072) //This one matters
//#define CMENUCONNECT_IPPORTSTR2 ((char*)CMENU_OBJ+1096)
//#define CMENUCONNECT_PORT (((uint*)CMENU_OBJ)[266])
//#define CMENUCONNECT_CURSORPOS (((char*)CMENU_OBJ)[968])

#define MSGBOX_OBJ (*(void **)0x089a390)
//#define MSGBOX_ACTIVE (*(((unsigned char*)MSGBOX_OBJ)+20))
//#define MSGBOX_X *(((float*)MSGBOX_OBJ)+3)
//#define MSGBOX_Y *(((float*)MSGBOX_OBJ)+4)

#define CINPUTMANAGERCLUSTER_OBJ ((void *)0x0089a248)
//#define INPUT_CLICKED_XAXIS (((int*)CINPUTMANAGERCLUSTER_OBJ)[0xE])
//#define INPUT_CLICKED_YAXIS (((int*)CINPUTMANAGERCLUSTER_OBJ)[0xF])
//#define INPUT_CLICKED_A (((int*)CINPUTMANAGERCLUSTER_OBJ)[0x10])
//#define INPUT_CLICKED_B (((int*)CINPUTMANAGERCLUSTER_OBJ)[0x11])

#define PROFILE1_DECKNAME ((VC9String *)0x0899840)
#define PROFILE2_DECKNAME ((VC9String *)0x089985C)

#define SFX_PLAY_FUNPTR 0x043e1e0
#define SFX_MOVE 0x27
#define SFX_SELECT 0x28
#define SFX_BACK 0x29

struct CInputManagerCluster {
	::byte UNKNOWN[56];
	struct {
		int Xaxis;
		int Yaxis;
		int A;
		int B;
		int C;
		int D;
		int E;
		int F;
	} P1;
};

struct CDesignSprite {
	void *vftable; // =008576ac
	float UNKNOWN_1[2];
	float x;
	float y;
	::byte active;
	::byte UNKNOWN_2[3];
	int UNKNOWN_3;
};

struct CMenuConnect {
	void *vftable;
	void *CNetworkBasePtr;
	::byte Choice;
	::byte Subchoice;
	::byte UNKNOWN_0[2];
	void *CDesignBasePtr;
	::byte UNKNOWN_1[48];
	CDesignSprite *MenuItemSprites[7];
	::byte UNKNOWN_2[860];
	::byte NotSpectateFlag;
	::byte Spectate;
	::byte UNKNOWN_3[2];
	uint MenuItem_Count;
	::byte UNKNOWN_4[8];
	uint CursorPos;
	uint CursorPos2;
	::byte UNKNOWN_5[48];
	uint NumberInput_ArrowPos;
	::byte UNKNOWN_6[36];
	uint Port;
	::byte UNKNOWN_7[4];
	char IPString[20]; // Final/Used val
	char IPWString; // Unknown size
	::byte UNKNOWN_8[2051];
	uint NotInSubMenuFlag;
	::byte UNKNOWN_9[332];
	::byte InSubMenuFlag;
	::byte UNKNOWN_10[171];
	::byte UnknownJoinFlag;
	::byte UNKNOWN_12[835];
};

struct VC9String {
	enum { _BUF_SIZE = 16 };

	void *alloc;
	union {
		char buf[16];
		char *ptr;
	} body;
	size_t size;
	size_t bufsize;

	operator char *() {
		return bufsize >= _BUF_SIZE ? body.ptr : body.buf;
	}
	operator const char *() const {
		return bufsize >= _BUF_SIZE ? body.ptr : body.buf;
	}
};

namespace SokuAPI {
auto SfxPlay = (char (*)(int))SFX_PLAY_FUNPTR;

PatchMan::MultiPatch InputBlock, HideProfiles, InputWorkaround;

VC9String Profile1, Profile2;

void Init() {
	InputBlock.AddPatch(0x0448e4a, "\x30\xC0\x90\x90\x90").AddPatch(0x0448e5d, "\x3C\x01\x90\x90").AddPatch(0x0449120, "\x85\xC9\x90\x90");
	HideProfiles.AddNOPs(0x0445e36, 7);
	// This one is kinda hackish, avoids the check if address.txt and history.txt exist
	// Since soku doesn't change choices otherwise.
	InputWorkaround.AddNOPs(0x0448f52, 8).AddNOPs(0x0448f2e, 8);
}

CMenuConnect *GetCMenuConnect() {
	return (CMenuConnect *)CMENU_OBJ;
}

CDesignSprite *GetMsgBox() {
	return (CDesignSprite *)MSGBOX_OBJ;
}

CInputManagerCluster *GetInputManager() {
	return (CInputManagerCluster *)CINPUTMANAGERCLUSTER_OBJ;
}

string GetProfileName(int id) {
	if (id < 1 || id > 2) {
		return NULL;
	}
	string deckname = string(id == 1 ? *PROFILE1_DECKNAME : *PROFILE2_DECKNAME);
	int dotpos = deckname.find_last_of('.');
	return deckname.substr(0, dotpos);
}

// NOTE: This leaves the exit button
void HideNativeMenu() {
	for (int i = 0; i < 6; i++) {
		GetCMenuConnect()->MenuItemSprites[i]->active = false;
	}
}

void SetupHost(uint port, bool spectate) {
	typedef void(__thiscall * func)(void *);
	func HostFun = (func)0x0446a40;

	CMenuConnect *menu = GetCMenuConnect();
	menu->Port = port;
	menu->Spectate = spectate;
	menu->Choice = 1;
	menu->Subchoice = 2;

	HostFun(menu);
	GetMsgBox()->active = false;
}

void JoinHost(const char *ip, uint port, bool spectate = false) {
	typedef void(__thiscall * func)(void *);
	func JoinFun = (func)0x0446b20;

	CMenuConnect *menu = GetCMenuConnect();
	if (ip != NULL) {
		// Unsafe
		strcpy(menu->IPString, ip);
		menu->Port = port;
	}
	menu->Choice = 2;
	menu->Subchoice = (spectate ? 6 : 3);
	menu->UnknownJoinFlag = 1;
	menu->NotSpectateFlag = (char)!spectate;

	JoinFun(menu);
	GetMsgBox()->active = false;
}

// Resets choice/subchoice and clears any messagebox
void ClearMenu() {
	GetMsgBox()->active = false;
	GetCMenuConnect()->Choice = 0;
	GetCMenuConnect()->Subchoice = 0;
}
}; // namespace SokuAPI