#include <windows.h>
#include <d3d9.h>
#include <mmsystem.h>
#include <shlwapi.h>

#define SWRS_USES_HASH
#include "swrs.h"

// セレクト
#define CSelectSV_Create(p) Ccall(p, s_origCSelectSV_Create, void *, ())()
#define CSelectSV_Render(p) Ccall(p, s_origCSelectSV_Render, int, ())()
#define CSelectSV_Destruct(p, dyn) Ccall(p, s_origCSelectSV_Destruct, void *, (int))(dyn)
#define CSelectCL_Create(p) Ccall(p, s_origCSelectCL_Create, void *, ())()
#define CSelectCL_Render(p) Ccall(p, s_origCSelectCL_Render, int, ())()
#define CSelectCL_Destruct(p, dyn) Ccall(p, s_origCSelectCL_Destruct, void *, (int))(dyn)

static char s_profilePath[1024 + MAX_PATH];
static int s_seq = 1;
static int s_lastip = -1;

struct SWRVERTEX {
	float x, y, z;
	float rhw;
	D3DCOLOR color;
	float u, v;
};

#define FVF_SWRVERTEX (D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_TEX1)

static DWORD s_origCSelectSV_Create;
static DWORD s_origCSelectSV_Render;
static DWORD s_origCSelectSV_Destruct;
static DWORD s_origCSelectCL_Create;
static DWORD s_origCSelectCL_Render;
static DWORD s_origCSelectCL_Destruct;
static DWORD s_origCSelect_Size;

void *__fastcall CSelectCommon_OnCreate(void *This) {
	int &m_texID = *(int *)((char *)This + s_origCSelect_Size + 0x00);

	// ipちぇき
	int newip;
	switch (g_mainMode) {
	case 4: // さば
		newip = *(int *)(*(char **)g_psvClients + 4);
		break;
	case 5: // くら
		newip = *(int *)(g_ptoAddr + 4);
		break;
	}
	if (s_lastip != newip) {
		s_lastip = newip;
		s_seq = 1;
	}

	// サウンド
	char soundPath[1025];
	char keyName[16];
	wsprintf(keyName, "%d", s_seq);
	if (GetPrivateProfileString("Sound", keyName, "", soundPath, _countof(soundPath), s_profilePath)) {
		PlaySound(soundPath, NULL, SND_FILENAME | SND_NOWAIT);
	}

	// 新しくテクスチャ作る
	char font[0x1A4]; // 多くとも 0x1A4
	SWRFont_Create(font);

	SWRFONTDESC sfdesc;
	strcpy(sfdesc.FaceName, "Impact");
	sfdesc.R1 = 0xEE;
	sfdesc.G1 = 0xEE;
	sfdesc.B1 = 0xEE;
	sfdesc.R2 = 0xEE;
	sfdesc.G2 = 0xEE;
	sfdesc.B2 = 0xEE;
	sfdesc.Height = 28;
	sfdesc.Weight = 300;
	sfdesc.Italic = 0;
	sfdesc.Shadow = 1;
	sfdesc.UseOffset = 0;
	sfdesc.BufferSize = 100000;
	sfdesc.OffsetX = 0;
	sfdesc.OffsetY = 0;
	sfdesc.CharSpaceX = 0;
	sfdesc.CharSpaceY = 2;
	SWRFont_SetIndirect(font, &sfdesc);

	char dispString[64];
	if(s_seq == 1) {
		wsprintf(dispString, "1st duel");
	} else if(s_seq == 2) {
		wsprintf(dispString, "2nd duel");
	} else if(s_seq == 3) {
		wsprintf(dispString, "3rd duel");
	} else {
		wsprintf(dispString, "%dth duel", s_seq);
	}
	s_seq++;
	CTextureManager_CreateTextTexture(g_textureMgr, &m_texID, dispString, font, 200, 50, NULL, NULL);

	SWRFont_Destruct(font);

	return This;
}

void *__fastcall CSelectSV_OnCreate(void *This) {
	// super
	CSelectSV_Create(This);
	return CSelectCommon_OnCreate(This);
}

void *__fastcall CSelectCL_OnCreate(void *This) {
	// super
	CSelectCL_Create(This);
	return CSelectCommon_OnCreate(This);
}

void __fastcall CSelectCommon_OnRender(void *This) {
	int &m_texID = *(int *)((char *)This + s_origCSelect_Size + 0x00);

	if (CRenderer_Begin(g_renderer)) {
		// CSpriteに任せてもいいんだけど、まあいいでしょ．
		static const SWRVERTEX vertices[] = {
			{0.0f, 0.0f, 0.0f, 1.0f, 0xffffffff, 0.0f, 0.0f},
			{200.0f, 0.0f, 0.0f, 1.0f, 0xffffffff, 1.0f, 0.0f},
			{200.0f, 50.0f, 0.0f, 1.0f, 0xffffffff, 1.0f, 1.0f},
			{0.0f, 50.0f, 0.0f, 1.0f, 0xffffffff, 0.0f, 1.0f},
		};
		static const SWRVERTEX vertices2[] = {
			{0.0f, 0.0f, 0.0f, 1.0f, 0x40000000, 0.0f, 0.0f},
			{100.0f, 0.0f, 0.0f, 1.0f, 0x40000000, 1.0f, 0.0f},
			{100.0f, 32.0f, 0.0f, 1.0f, 0x40000000, 1.0f, 1.0f},
			{0.0f, 32.0f, 0.0f, 1.0f, 0x40000000, 0.0f, 1.0f},
		};

		CTextureManager_SetTexture(g_textureMgr, NULL, 0);
		g_pd3dDev->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, 2, vertices2, sizeof(SWRVERTEX));

		CTextureManager_SetTexture(g_textureMgr, m_texID, 0);
		g_pd3dDev->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, 2, vertices, sizeof(SWRVERTEX));

		CRenderer_End(g_renderer);
	}
}

int __fastcall CSelectSV_OnRender(void *This) {
	// super
	int ret = CSelectSV_Render(This);
	CSelectCommon_OnRender(This);
	return ret;
}

int __fastcall CSelectCL_OnRender(void *This) {
	// super
	int ret = CSelectCL_Render(This);
	CSelectCommon_OnRender(This);
	return ret;
}

void __fastcall CSelectCommon_OnDestruct(void *This) {
	int &m_texID = *(int *)((char *)This + s_origCSelect_Size + 0x00);
	CTextureManager_Remove(g_textureMgr, m_texID);
}

void *__fastcall CSelectSV_OnDestruct(void *This, int, int dyn) {
	CSelectCommon_OnDestruct(This);
	return CSelectSV_Destruct(This, dyn);
}

void *__fastcall CSelectCL_OnDestruct(void *This, int, int dyn) {
	CSelectCommon_OnDestruct(This);
	return CSelectCL_Destruct(This, dyn);
}

extern "C" __declspec(dllexport) bool CheckVersion(const BYTE hash[16]) {
	return ::memcmp(TARGET_HASH, hash, sizeof TARGET_HASH) == 0;
}

extern "C" __declspec(dllexport) bool Initialize(HMODULE hMyModule, HMODULE hParentModule) {
	GetModuleFileName(hMyModule, s_profilePath, 1024);
	PathRemoveFileSpec(s_profilePath);
	PathAppend(s_profilePath, "NetBattleCounter.ini");

	// 書き換える
	DWORD old;
	::VirtualProtect((PVOID)text_Offset, text_Size, PAGE_EXECUTE_WRITECOPY, &old);
	s_origCSelect_Size = max(CSelectSV_Size, CSelectCL_Size);
	CSelectSV_Size = s_origCSelect_Size + 4;
	CSelectCL_Size = s_origCSelect_Size + 4;

	s_origCSelectSV_Create = TamperNearJmpOpr(CSelectSV_Creater, union_cast<DWORD>(CSelectSV_OnCreate));
	s_origCSelectCL_Create = TamperNearJmpOpr(CSelectCL_Creater, union_cast<DWORD>(CSelectCL_OnCreate));
	::VirtualProtect((PVOID)text_Offset, text_Size, old, &old);

	::VirtualProtect((PVOID)rdata_Offset, rdata_Size, PAGE_EXECUTE_WRITECOPY, &old);
	s_origCSelectSV_Destruct = TamperDword(vtbl_CSelectSV + 0x00, union_cast<DWORD>(CSelectSV_OnDestruct));
	s_origCSelectSV_Render = TamperDword(vtbl_CSelectSV + 0x08, union_cast<DWORD>(CSelectSV_OnRender));
	s_origCSelectCL_Destruct = TamperDword(vtbl_CSelectCL + 0x00, union_cast<DWORD>(CSelectCL_OnDestruct));
	s_origCSelectCL_Render = TamperDword(vtbl_CSelectCL + 0x08, union_cast<DWORD>(CSelectCL_OnRender));
	::VirtualProtect((PVOID)rdata_Offset, rdata_Size, old, &old);

	::FlushInstructionCache(GetCurrentProcess(), NULL, 0);
	return TRUE;
}

extern "C" int APIENTRY DllMain(HMODULE hModule, DWORD fdwReason, LPVOID lpReserved) {
	return TRUE;
}
