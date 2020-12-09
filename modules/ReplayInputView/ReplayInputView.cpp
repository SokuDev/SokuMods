#include <windows.h>
#include <d3d9.h>
#include <dinput.h>
#include <shlwapi.h>

#define SWRS_USES_HASH
#include "swrs.h"

struct SWRCHARINPUT {
	int lr;
	int ud;
	int a;
	int b;
	int c;
	int d;
	int ch;
	int s;
};

struct SWRVERTEX {
	float x, y, z;
	float rhw;
	D3DCOLOR color;
	float u, v;
};

struct SWRCMDINFO {
	bool enabled;
	int prev;
	int now;

	struct {
		bool enabled;
		int id[10];
		int base;
		int len;
	} record;
};

struct MYMEMBER {
	bool m_enabled;
	int m_texID;
	int m_forwardCount;
	int m_forwardStep;
	int m_forwardIndex;
	SWRCMDINFO m_cmdp1;
	SWRCMDINFO m_cmdp2;
};

#define FVF_SWRVERTEX (D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_TEX1)

HRESULT(__stdcall *s_D3DXCreateTextureFromResource)(LPDIRECT3DDEVICE9 pDevice, HMODULE hSrcModule, LPCTSTR pSrcResource, LPDIRECT3DTEXTURE9 *ppTexture);

// バトルマネージャ
#define CBattleManager_Create(p) Ccall(p, s_origCBattleManager_OnCreate, void *, ())()
#define CBattleManager_Render(p) Ccall(p, s_origCBattleManager_OnRender, void, ())()
#define CBattleManager_Process(p) Ccall(p, s_origCBattleManager_OnProcess, int, ())()
#define CBattleManager_Destruct(p, dyn) Ccall(p, s_origCBattleManager_OnDestruct, void *, (int))(dyn)
// バトルシーン
#define CBattle_Process(p) Ccall(p, s_origCBattle_OnProcess, int, ())()
#define CBattle_Destruct(p, dyn) Ccall(p, s_origCBattle_OnDestruct, void *, (int))(dyn)

static DWORD s_origCBattleManager_OnCreate;
static DWORD s_origCBattleManager_OnDestruct;
static DWORD s_origCBattleManager_OnRender;
static DWORD s_origCBattleManager_OnProcess;
static DWORD s_origCBattleManager_Size;

static char s_profilePath[1024 + MAX_PATH];
static HMODULE s_hDllModule;

#ifndef _DEBUG
extern "C" int _fltused = 1;
#endif

int *CTextureManager_LoadTextureFromResource(void *ptextureMgr, int *ret, HMODULE hSrcModule, LPCTSTR pSrcResource) {
	int id = 0;
	LPDIRECT3DTEXTURE9 *pphandle = CTextureManager_Allocate(ptextureMgr, &id);

	*pphandle = NULL;
	if (SUCCEEDED(s_D3DXCreateTextureFromResource(g_pd3dDev, hSrcModule, pSrcResource, pphandle))) {
		*ret = id;
	} else {
		CTextureManager_Deallocate(ptextureMgr, id);
		*ret = 0;
	}
	return ret;
}

void __fastcall CBattleManager_RenderMyBack(float x, float y, int cx, int cy) {
	const SWRVERTEX vertices[] = {
		{x, y, 0.0f, 1.0f, 0xa0808080, 0.0f, 0.0f},
		{x + cx + 0, y, 0.0f, 1.0f, 0xa0808080, 1.0f, 0.0f},
		{x + cx + 5, y + cy, 0.0f, 1.0f, 0xa0202020, 1.0f, 1.0f},
		{x + 5, y + cy, 0.0f, 1.0f, 0xa0202020, 0.0f, 1.0f},
	};
	CTextureManager_SetTexture(g_textureMgr, NULL, 0);
	g_pd3dDev->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, 2, vertices, sizeof(SWRVERTEX));
}

void CBattleManager_RenderTile(float x, float y, int u, int v, int a) {
	int dif = (a << 24) | 0xFFFFFF;
	float fu = u / 256.0f;
	float fv = v / 64.0f;

	// デバッグビルドだとグダグダになるのは何なの？俺死ぬの？
	const SWRVERTEX vertices[] = {
		{x, y, 0.0f, 1.0f, dif, fu, fv},
		{x + 32.0f, y, 0.0f, 1.0f, dif, fu + 0.125f, fv},
		{x + 32.0f, y + 32.0f, 0.0f, 1.0f, dif, fu + 0.125f, fv + 0.5f},
		{x, y + 32.0f, 0.0f, 1.0f, dif, fu, fv + 0.5f},
	};
	g_pd3dDev->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, 2, vertices, sizeof(SWRVERTEX));
}

void CBattleManager_RenderInputPanel(void *This, SWRCMDINFO &cmd, float x, float y) {
	MYMEMBER &my = *(MYMEMBER *)((char *)g_pbattleMgr + s_origCBattleManager_Size);

	if (cmd.enabled) {
		// 後ろ
		CBattleManager_RenderMyBack(x, y, 24 * 6 + 24, 24 * 3 + 12);

		CTextureManager_SetTexture(g_textureMgr, my.m_texID, 0);

		// ジョイスティック
		CBattleManager_RenderTile(x + 9, y + 6, 128, 0, (cmd.now % 16 == 5 ? 255 : 48)); /* LU */
		CBattleManager_RenderTile(x + 9 + 24, y + 6, 0, 0, (cmd.now % 16 == 1 ? 255 : 48)); /* NU */
		CBattleManager_RenderTile(x + 9 + 48, y + 6, 160, 0, (cmd.now % 16 == 9 ? 255 : 48)); /* RU */
		CBattleManager_RenderTile(x + 9, y + 6 + 24, 64, 0, (cmd.now % 16 == 4 ? 255 : 48)); /* LN */
		CBattleManager_RenderTile(x + 9 + 48, y + 6 + 24, 96, 0, (cmd.now % 16 == 8 ? 255 : 48)); /* RN */
		CBattleManager_RenderTile(x + 9, y + 6 + 48, 224, 0, (cmd.now % 16 == 6 ? 255 : 48)); /* LD */
		CBattleManager_RenderTile(x + 9 + 24, y + 6 + 48, 32, 0, (cmd.now % 16 == 2 ? 255 : 48)); /* ND */
		CBattleManager_RenderTile(x + 9 + 48, y + 6 + 48, 192, 0, (cmd.now % 16 == 10 ? 255 : 48)); /* RD */
		// ボタン類
		CBattleManager_RenderTile(x + 9 + 72 + 3, y + 6 + 12, 0, 32, (cmd.now & 16 ? 255 : 48));
		CBattleManager_RenderTile(x + 9 + 72 + 3 + 27, y + 6 + 12, 32, 32, (cmd.now & 32 ? 255 : 48));
		CBattleManager_RenderTile(x + 9 + 72 + 3 + 54, y + 6 + 12, 64, 32, (cmd.now & 64 ? 255 : 48));
		CBattleManager_RenderTile(x + 9 + 72 + 6, y + 6 + 36, 96, 32, (cmd.now & 128 ? 255 : 48));
		CBattleManager_RenderTile(x + 9 + 72 + 6 + 27, y + 6 + 36, 128, 32, (cmd.now & 256 ? 255 : 48));
		CBattleManager_RenderTile(x + 9 + 72 + 6 + 54, y + 6 + 36, 160, 32, (cmd.now & 512 ? 255 : 48));
	}
}

void CBattleManager_RenderRecordPanel(void *This, SWRCMDINFO &cmd, float x, float y) {
	MYMEMBER &my = *(MYMEMBER *)((char *)This + s_origCBattleManager_Size);

	if (cmd.record.enabled) {
		// 後ろ
		CBattleManager_RenderMyBack(x, y, 24 * 10 + 6, 24 + 6);

		CTextureManager_SetTexture(g_textureMgr, my.m_texID, 0);
		for (int i = 0; i < cmd.record.len; ++i) {
			int j = (i + cmd.record.base) % _countof(cmd.record.id);
			int id = cmd.record.id[j];
			CBattleManager_RenderTile(x + 3 + i * 24, y + 3, (id % 8) * 32, (id / 8) * 32, 255);
		}
	}
}

void CBattleManager_DetermineRecord(SWRCMDINFO &cmd, int mask, int flag, int id) {
	if ((cmd.prev & mask) != flag && (cmd.now & mask) == flag) {
		int index = (cmd.record.base + cmd.record.len) % _countof(cmd.record.id);
		cmd.record.id[index] = id;
		if (cmd.record.len == _countof(cmd.record.id)) {
			cmd.record.base = (cmd.record.base + 1) % _countof(cmd.record.id);
		} else {
			cmd.record.len++;
		}
	}
}

void CBattleManager_RefleshCommandInfo(SWRCMDINFO &cmd, void *Char) {
	SWRCHARINPUT &input = *(SWRCHARINPUT *)((char *)Char + 0x754);

	cmd.prev = cmd.now;
	cmd.now = 0;
	if (input.ud < 0)
		cmd.now |= 1;
	if (input.ud > 0)
		cmd.now |= 2;
	if (input.lr < 0)
		cmd.now |= 4;
	if (input.lr > 0)
		cmd.now |= 8;
	if (input.a > 0)
		cmd.now |= 16;
	if (input.b > 0)
		cmd.now |= 32;
	if (input.c > 0)
		cmd.now |= 64;
	if (input.d > 0)
		cmd.now |= 128;
	if (input.ch > 0)
		cmd.now |= 256;
	if (input.s > 0)
		cmd.now |= 512;

	if (cmd.record.enabled) {
		CBattleManager_DetermineRecord(cmd, 15, 5, 4);
		CBattleManager_DetermineRecord(cmd, 15, 1, 0);
		CBattleManager_DetermineRecord(cmd, 15, 9, 5);
		CBattleManager_DetermineRecord(cmd, 15, 4, 2);
		CBattleManager_DetermineRecord(cmd, 15, 8, 3);
		CBattleManager_DetermineRecord(cmd, 15, 6, 7);
		CBattleManager_DetermineRecord(cmd, 15, 2, 1);
		CBattleManager_DetermineRecord(cmd, 15, 10, 6);

		CBattleManager_DetermineRecord(cmd, 16, 16, 8);
		CBattleManager_DetermineRecord(cmd, 32, 32, 9);
		CBattleManager_DetermineRecord(cmd, 64, 64, 10);
		CBattleManager_DetermineRecord(cmd, 128, 128, 11);
		CBattleManager_DetermineRecord(cmd, 256, 256, 12);
		CBattleManager_DetermineRecord(cmd, 512, 512, 13);
	}
}

void *__fastcall CBattleManager_OnCreate(void *This) {
	MYMEMBER &my = *(MYMEMBER *)((char *)This + s_origCBattleManager_Size);

	// super
	CBattleManager_Create(This);

	if (g_subMode == 2 && g_mainMode == 3) {
		// Replay
		my.m_enabled = true;
	} else {
		// それ以外
		my.m_enabled = false;
	}

	if (my.m_enabled) {
		CTextureManager_LoadTextureFromResource(g_textureMgr, &my.m_texID, s_hDllModule, MAKEINTRESOURCE(4));
		if (my.m_texID != 0) {
			my.m_forwardCount = 1;
			my.m_forwardStep = 1;
			my.m_forwardIndex = 0;

			my.m_cmdp1.enabled = ::GetPrivateProfileInt("Input", "p1.Enabled", 1, s_profilePath) != 0;
			my.m_cmdp1.prev = 0;
			my.m_cmdp1.record.base = my.m_cmdp1.record.len = 0;
			my.m_cmdp1.record.enabled = ::GetPrivateProfileInt("Record", "p1.Enabled", 0, s_profilePath) != 0;

			my.m_cmdp2.enabled = ::GetPrivateProfileInt("Input", "p2.Enabled", 1, s_profilePath) != 0;
			my.m_cmdp2.prev = 0;
			my.m_cmdp2.record.base = my.m_cmdp2.record.len = 0;
			my.m_cmdp2.record.enabled = ::GetPrivateProfileInt("Record", "p2.Enabled", 0, s_profilePath) != 0;
		} else {
			// やっぱりやめた
			my.m_enabled = false;
		}
	}
	return This;
}

int __fastcall CBattleManager_OnProcess(void *This) {
	MYMEMBER &my = *(MYMEMBER *)((char *)This + s_origCBattleManager_Size);
	int ret;

	if (my.m_enabled) {
		if (CheckKeyOneshot(DIK_F8, 0, 0, 0)) {
			bool cmdEnabled = my.m_cmdp1.enabled;
			bool recordEnabled = my.m_cmdp1.record.enabled;
			my.m_cmdp1.enabled = !recordEnabled;
			my.m_cmdp1.record.enabled = cmdEnabled;
		} else if (CheckKeyOneshot(DIK_F9, 0, 0, 0)) {
			bool cmdEnabled = my.m_cmdp2.enabled;
			bool recordEnabled = my.m_cmdp2.record.enabled;
			my.m_cmdp2.enabled = !recordEnabled;
			my.m_cmdp2.record.enabled = cmdEnabled;
		} else if (CheckKeyOneshot(DIK_F10, 0, 0, 0)) {
			if (my.m_forwardCount > 1) {
				my.m_forwardCount -= 1;
				my.m_forwardStep = 1;
			} else {
				my.m_forwardCount = 1;
				my.m_forwardStep += 1;
			}
			my.m_forwardIndex = 0;
		} else if (CheckKeyOneshot(DIK_F11, 0, 0, 0)) {
			if (my.m_forwardStep > 1) {
				my.m_forwardCount = 1;
				my.m_forwardStep -= 1;
			} else {
				my.m_forwardCount += 1;
				my.m_forwardStep = 1;
			}
			my.m_forwardIndex = 0;
		} else if (CheckKeyOneshot(DIK_F12, 0, 0, 0)) {
			my.m_forwardCount = 1;
			my.m_forwardStep = 1;
			my.m_forwardIndex = 0;
		}

		my.m_forwardIndex += my.m_forwardStep;
		if (my.m_forwardIndex >= my.m_forwardCount) {
			for (int i = my.m_forwardIndex / my.m_forwardCount; i--;) {
				ret = CBattleManager_Process(This);
				if (ret > 0 && ret < 4)
					break;

				void *p1Obj = *(void **)((char *)This + 0xC);
				void *p2Obj = *(void **)((char *)This + 0x10);
				CBattleManager_RefleshCommandInfo(my.m_cmdp1, p1Obj);
				CBattleManager_RefleshCommandInfo(my.m_cmdp2, p2Obj);
			}
			my.m_forwardIndex = 0;
		}
	} else {
		ret = CBattleManager_Process(This);
	}
	return ret;
}

void __fastcall CBattleManager_OnRender(void *This) {
	MYMEMBER &my = *(MYMEMBER *)((char *)This + s_origCBattleManager_Size);

	CBattleManager_Render(This);

	if (my.m_enabled) {
		CBattleManager_RenderInputPanel(This, my.m_cmdp1, 60, 340);
		CBattleManager_RenderInputPanel(This, my.m_cmdp2, 400, 340);
		CBattleManager_RenderRecordPanel(This, my.m_cmdp1, 0, 300);
		CBattleManager_RenderRecordPanel(This, my.m_cmdp2, 390, 300);
	}
}

void *__fastcall CBattleManager_OnDestruct(void *This, int, int dyn) {
	MYMEMBER &my = *(MYMEMBER *)((char *)This + s_origCBattleManager_Size);

	if (my.m_enabled) {
		CTextureManager_Remove(g_textureMgr, my.m_texID);

		::WritePrivateProfileString("Input", "p1.Enabled", my.m_cmdp1.enabled ? "1" : "0", s_profilePath);
		::WritePrivateProfileString("Input", "p2.Enabled", my.m_cmdp2.enabled ? "1" : "0", s_profilePath);
		::WritePrivateProfileString("Record", "p1.Enabled", my.m_cmdp1.record.enabled ? "1" : "0", s_profilePath);
		::WritePrivateProfileString("Record", "p2.Enabled", my.m_cmdp2.record.enabled ? "1" : "0", s_profilePath);
	}

	return CBattleManager_Destruct(This, dyn);
}

extern "C" __declspec(dllexport) bool CheckVersion(const BYTE hash[16]) {
	return ::memcmp(TARGET_HASH, hash, sizeof TARGET_HASH) == 0;
}

extern "C" __declspec(dllexport) bool Initialize(HMODULE hMyModule, HMODULE hParentModule) {
	s_hDllModule = hMyModule;

	HMODULE d3dx = LoadLibraryExW(L"D3DX9_33.DLL", NULL, 0);
	if (d3dx == NULL)
		return false;
	*(FARPROC *)&s_D3DXCreateTextureFromResource = GetProcAddress(d3dx, "D3DXCreateTextureFromResourceA");
	if (s_D3DXCreateTextureFromResource == NULL)
		return false;

	GetModuleFileName(hMyModule, s_profilePath, 1024);
	PathRemoveFileSpec(s_profilePath);
	PathAppend(s_profilePath, "ReplayInputView.ini");

	DWORD old;
	::VirtualProtect((PVOID)text_Offset, text_Size, PAGE_EXECUTE_WRITECOPY, &old);
	s_origCBattleManager_Size = TamperDwordAdd((DWORD)&CBattleManager_Size, sizeof(MYMEMBER));
	s_origCBattleManager_OnCreate = TamperNearJmpOpr(CBattleManager_Creater, union_cast<DWORD>(CBattleManager_OnCreate));
	::VirtualProtect((PVOID)text_Offset, text_Size, old, &old);

	::VirtualProtect((PVOID)rdata_Offset, rdata_Size, PAGE_WRITECOPY, &old);
	s_origCBattleManager_OnDestruct = TamperDword(vtbl_CBattleManager + 0x00, union_cast<DWORD>(CBattleManager_OnDestruct));
	s_origCBattleManager_OnRender = TamperDword(vtbl_CBattleManager + 0x38, union_cast<DWORD>(CBattleManager_OnRender));
	s_origCBattleManager_OnProcess = TamperDword(vtbl_CBattleManager + 0x0c, union_cast<DWORD>(CBattleManager_OnProcess));
	::VirtualProtect((PVOID)rdata_Offset, rdata_Size, old, &old);

	::FlushInstructionCache(GetCurrentProcess(), NULL, 0);

	return true;
}

extern "C" int APIENTRY DllMain(HMODULE hModule, DWORD fdwReason, LPVOID lpReserved) {
	return TRUE;
}
