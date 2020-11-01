#ifndef SWRS_H_INCLUDED
#define SWRS_H_INCLUDED

// ---------------- ここからテンプレ ----------------

#ifdef SOKU_VER_110a

#define ADDR_NEW_FUNCTION                 0x0081FBDC
#define ADDR_DELETE_FUNCTION              0x0081F6FA
#define ADDR_DEALLOCATE_FUNCTION          0x00402810
#define ADDR_ALLOCATE_FUNCTION            0x004026A0
#define TEXT_SECTION_OFFSET               0x00401000
#define TEXT_SECTION_SIZE                 0x00456000
#define RDATA_SECTION_OFFSET              0x00857000
#define RDATA_SECTION_SIZE                0x0002B000
#define DATA_SECTION_OFFSET               0x00882000
#define DATA_SECTION_SIZE                 0x00021000
#define ADDR_TEXTURE_MANAGER              0x0089F9F8
#define ADDR_D3D9_DEVICE                  0x008A0E30
#define ADDR_TEXTURE_MANAGER_LOAD_TEXTURE 0x00405030
#define ADDR_TEXTURE_MANAGER_CREATE_TEXT  0x004050A0
#define ADDR_TEXTURE_MANAGER_REMOVE       0x00405110
#define ADDR_TEXTURE_MANAGER_SET_TEXTURE  0x00405190
#define ADDR_TEXTURE_MANAGER_GET_SIZE     0x00405200
#define ADDR_BATTLE_MANAGER               0x008985E4
#define ADDR_CHECK_KEY_ONESHOT            0x0043DE30
#define ADDR_COMM_MODE                    0x00898690
#define ADDR_SUB_MODE                     0x00898688
#define ADDR_MENU_MODE                    0x00882A94
#define ADDR_VTBL_BATTLE_MANAGER          0x008588EC
#define ADDR_BATTLE_MANAGER_CREATER       0x004396C0
#define ADDR_BATTLE_MANAGER_SIZE          0x004396A2
#define ADDR_INPUT_MANAGER_READ_REPLAY    0x0042EAC0
#define ADDR_INPUT_MANAGER                0x00898718
#define ADDR_INPUT_MANAGER_CLUSTER        0x0089A248
#define ADDR_SET_BATTLE_MODE              0x0043E9A0
#define ADDR_VTBL_LOGO                    0x00857740
#define ADDR_VTBL_BATTLE                  0x008574A0
#define ADDR_SCENE_ID_NEW                 0x008a0040
#define ADDR_SCENE_ID                     0x008a0044
#define ADDR_PNETOBJECT                   0x008986A0
#define ADDR_LCHARID                      0x00899D10
#define ADDR_RCHARID                      0x00899D30
#define ADDR_SWR_FONT_CREATE              0x004116D0
#define ADDR_SWR_FONT_DESTRUCT            0x00411760
#define ADDR_SWR_FONT_SET_INDIRECT        0x00411840
#define ADDR_PLAY_SE_WAVE_BUFFER          0x0043E1E0
#define ADDR_PLAY_NET_BELL                0x00446D9A
#define ADDR_DEFAULT_FONT_NAME            0x00858764
#define ADDR_PROFILENAME_PRINT_CODE1      0x0047D857
#define ADDR_PROFILENAME_PRINT_CODE1_END  0x0047D9EE
#define ADDR_PROFILENAME_PRINT_CODE2      0x0047D9FE
#define ADDR_PROFILENAME_PRINT_CODE2_END  0x0047DB95
#define ADDR_PLAYER1_PROFILE_STR         (0x00898920 - sizeof(void *))
#define ADDR_PLAYER2_PROFILE_STR         (0x0089910C - sizeof(void *))
#define ADDR_RENDERER                     0x00896B4C
#define ADDR_RENDERER_BEGIN               0x00401000
#define ADDR_RENDERER_END                 0x00401040
#define ADDR_SELECT_SV_SIZE               0x0041E622
#define ADDR_SELECT_CL_SIZE               0x0041E6CD
#define ADDR_SELECT_SV_CREATER            0x0041E644
#define ADDR_SELECT_CL_CREATER            0x0041E6EF
#define ADDR_VTBL_SELECT_SV               0x008574DC
#define ADDR_VTBL_SELECT_CL               0x00857534
#define ADDR_GET_PACKAGED_BGM_CALLER      0x00418BE1
#define ADDR_GET_PACKAGED_SFL_CALLER      0x00418F41
#define ADDR_LOADED_STAGE_ID              0x008971CE

#else

#define ADDR_NEW_FUNCTION                 0x008116DC
#define ADDR_DELETE_FUNCTION              0x008111FA
#define ADDR_DEALLOCATE_FUNCTION          0x004175D0
#define ADDR_ALLOCATE_FUNCTION            0x00402600
#define TEXT_SECTION_OFFSET               0x00401000
#define TEXT_SECTION_SIZE                 0x00445000
#define RDATA_SECTION_OFFSET              0x00846000
#define RDATA_SECTION_SIZE                0x00029000
#define DATA_SECTION_OFFSET               0x0086F000
#define DATA_SECTION_SIZE                 0x00015000
#define ADDR_TEXTURE_MANAGER              0x0088CEE8
#define ADDR_D3D9_DEVICE                  0x0088DE10
#define ADDR_TEXTURE_MANAGER_LOAD_TEXTURE 0x00404CC0
#define ADDR_TEXTURE_MANAGER_CREATE_TEXT  0x00404D30
#define ADDR_TEXTURE_MANAGER_REMOVE       0x00404DA0
#define ADDR_TEXTURE_MANAGER_SET_TEXTURE  0x00404E20
#define ADDR_TEXTURE_MANAGER_GET_SIZE     0x00404E90
#define ADDR_BATTLE_MANAGER               0x008855C4
#define ADDR_CHECK_KEY_ONESHOT            0x0043D000
#define ADDR_COMM_MODE                    0x00885670
#define ADDR_SUB_MODE                     0x00885668
#define ADDR_MENU_MODE                    0x0086FA94
#define ADDR_VTBL_BATTLE_MANAGER          0x008478EC
#define ADDR_BATTLE_MANAGER_CREATER       0x00438B30
#define ADDR_BATTLE_MANAGER_SIZE          0x00438B12
#define ADDR_INPUT_MANAGER_READ_REPLAY    0x0042E5C0
#define ADDR_INPUT_MANAGER                0x008856F8
#define ADDR_INPUT_MANAGER_CLUSTER        0x00887228
#define ADDR_SET_BATTLE_MODE              0x0043DBC0
#define ADDR_VTBL_LOGO                    0x00846738
#define ADDR_VTBL_BATTLE                  0x00846490
#define ADDR_SCENE_ID_NEW                 0x0088D020
#define ADDR_SCENE_ID                     0x0088D024
#define ADDR_PNETOBJECT                   0x00885680
#define ADDR_LCHARID                      0x00886CF0
#define ADDR_RCHARID                      0x00886D10
#define ADDR_SWR_FONT_CREATE              0x00411170
#define ADDR_SWR_FONT_DESTRUCT            0x00411200
#define ADDR_SWR_FONT_SET_INDIRECT        0x004112E0
#define ADDR_PLAY_SE_WAVE_BUFFER          0x0043D3B0
#define ADDR_PLAY_NET_BELL                0x0044641A
#define ADDR_DEFAULT_FONT_NAME            0x0069F154
#define ADDR_PROFILENAME_PRINT_CODE1      0x0047CDC7
#define ADDR_PROFILENAME_PRINT_CODE1_END  0x0047CF5E
#define ADDR_PROFILENAME_PRINT_CODE2      0x0047CF6E
#define ADDR_PROFILENAME_PRINT_CODE2_END  0x0047D105
#define ADDR_RENDERER                     0x00883B4C
#define ADDR_RENDERER_BEGIN               0x00401000
#define ADDR_RENDERER_END                 0x00401040
#define ADDR_SELECT_SV_SIZE               0x0041DCB2
#define ADDR_SELECT_CL_SIZE               0x0041DD5D
#define ADDR_SELECT_SV_CREATER            0x0041DCD4
#define ADDR_SELECT_CL_CREATER            0x0041DD7F
#define ADDR_VTBL_SELECT_SV               0x008464CC
#define ADDR_VTBL_SELECT_CL               0x00846524
#define ADDR_GET_PACKAGED_BGM_CALLER      0x00418791
#define ADDR_GET_PACKAGED_SFL_CALLER      0x00418AF1

#endif

#ifdef SWRS_USES_HASH
extern const BYTE TARGET_HASH[16];
__declspec(selectany) const BYTE TARGET_HASH[16] =
{
	// ver1.10
	// 0x26, 0x8a, 0xfd, 0x82, 0x76, 0x90, 0x3e, 0x16, 0x71, 0x6c, 0x14, 0x29, 0xc6, 0x95, 0x9c, 0x9d
	// ver1.10a
	0xdf, 0x35, 0xd1, 0xfb, 0xc7, 0xb5, 0x83, 0x31, 0x7a, 0xda, 0xbe, 0x8c, 0xd9, 0xf5, 0x3b, 0x2e
};
#endif

#include <windows.h>

// DWORD��������
__inline
DWORD TamperDword(DWORD addr, DWORD target) {
	DWORD old = *reinterpret_cast<PDWORD>(addr);
	*reinterpret_cast<PDWORD>(addr) = target;
	return old;
}

// DWORD���Z
__inline
DWORD TamperDwordAdd(DWORD addr, DWORD delta) {
	DWORD old = *reinterpret_cast<PDWORD>(addr);
	*reinterpret_cast<PDWORD>(addr) += delta;
	return old;
}

// NEAR JMP�I�y�����h��������
__inline
DWORD TamperNearJmpOpr(DWORD addr, DWORD target) {
	DWORD old = *reinterpret_cast<PDWORD>(addr + 1) + (addr + 5);
	*reinterpret_cast<PDWORD>(addr + 1) = target - (addr + 5);
	return old;
}

// NEAR JMP��������
__inline
void TamperNearJmp(DWORD addr, DWORD target) {
	*reinterpret_cast<PBYTE>(addr + 0) = 0xE9;
	TamperNearJmpOpr(addr, target);
}

// NEAR CALL��������
__inline
void TamperNearCall(DWORD addr, DWORD target) {
	*reinterpret_cast<PBYTE>(addr + 0) = 0xE8;
	TamperNearJmpOpr(addr, target);
}

// �t�H���g�f�B�X�N���v�^
#pragma pack(push, 4)
struct SWRFONTDESC {
	char FaceName[0x100];
	BYTE R1;
	BYTE R2;
	BYTE G1;
	BYTE G2;
	BYTE B1;
	BYTE B2;
	LONG Height;
	LONG Weight;
	BYTE Italic;
	BYTE Shadow;
	BYTE UseOffset;
	DWORD BufferSize;
	DWORD OffsetX;
	DWORD OffsetY;
	DWORD CharSpaceX;
	DWORD CharSpaceY;
};

// std::string ?
struct VC9STRING {
	enum { _BUF_SIZE = 16 };

	void *alloc;
	union {
		char  buf[16];
		char* ptr;
	} body;
	size_t size;
	size_t res;

	operator char *() {
		return _BUF_SIZE <= res ? body.ptr : body.buf;
	}
	operator const char *() const {
		return _BUF_SIZE <= res ? body.ptr : body.buf;
	}
};

#ifdef __GNUC__
struct IEffectManager {
	virtual ~IEffectManager() = default;
	virtual void LoadPattern(LPCSTR fileName, int) = 0;
	virtual void ClearPattern() = 0;
	virtual void AppendRegion(int arg_0, float arg_4, float arg_8, char arg_c, char arg_10, int arg_14) = 0;
	virtual void ClearRegion() = 0;
};

// �t�@�C�����[�_�C���^�[�t�F�[�X
struct IFileReader {
	virtual ~IFileReader() = default;
	virtual bool Read(LPVOID lpBuffer, DWORD nNumberOfBytesToRead) = 0;
	virtual DWORD GetReadLength() = 0;
	virtual LONG Seek(LONG lDistanceToMove, DWORD dwMoveMethod) = 0;
	virtual DWORD GetLength() = 0;
};
#else
struct __declspec(novtable) IEffectManager {
	virtual ~IEffectManager() {}
	virtual void LoadPattern(LPCSTR fileName, int) = 0;
	virtual void ClearPattern() = 0;
	virtual void AppendRegion(int arg_0, float arg_4, float arg_8, char arg_c, char arg_10, int arg_14) = 0;
	virtual void ClearRegion() = 0;
};

// �t�@�C�����[�_�C���^�[�t�F�[�X
struct __declspec(novtable) IFileReader {
	virtual ~IFileReader() {}
	virtual bool Read(LPVOID lpBuffer, DWORD nNumberOfBytesToRead) = 0;
	virtual DWORD GetReadLength() = 0;
	virtual LONG Seek(LONG lDistanceToMove, DWORD dwMoveMethod) = 0;
	virtual DWORD GetLength() = 0;
};
#endif

// �悭�킩��Ȃ�����
struct UnknownF {
	void *Unknown[3];
	float Aaxis;
	float Baxis;
};

enum SWRSSCENE {
	SWRSSCENE_LOGO = 0,
	SWRSSCENE_OPENING = 1,
	SWRSSCENE_TITLE = 2,
	SWRSSCENE_SELECT = 3,
	SWRSSCENE_BATTLE = 5,
	SWRSSCENE_LOADING = 6,
	SWRSSCENE_SELECTSV = 8,
	SWRSSCENE_SELECTCL = 9,
	SWRSSCENE_LOADINGSV = 10,
	SWRSSCENE_LOADINGCL = 11,
	SWRSSCENE_LOADINGWATCH = 12,
	SWRSSCENE_BATTLESV = 13,
	SWRSSCENE_BATTLECL = 14,
	SWRSSCENE_BATTLEWATCH = 15,
	SWRSSCENE_SELECTSENARIO = 16,
	SWRSSCENE_ENDING = 20,
	SWRSSCENE_MAX,
};
// 0:1 = story, 1:0 = arcade, 2:1 = vscom, 3:1 = vsplayer, 5:1=Player 6:2=Watch  8:0 = practice, 0-3:2 = Replay
enum SWRSMODE {
	SWRSMODE_STORY = 0,
	SWRSMODE_ARCADE = 1,
	SWRSMODE_VSCOM = 2,
	SWRSMODE_VSPLAYER = 3,
	SWRSMODE_VSCLIENT = 4,
	SWRSMODE_VSSERVER = 5,
	SWRSMODE_VSWATCH = 6,
	SWRSMODE_PRACTICE = 8
};

enum SWRSSUBMODE {
	SWRSSUBMODE_UNKNOWN1 = 0,
	SWRSSUBMODE_UNKNOWN2 = 1,
	SWRSSUBMODE_REPLAY = 2
};

// union_cast
template<typename TDest, typename TSrc>
__forceinline 	
TDest union_cast(TSrc src) {
#ifndef __GNUC__
	static_assert(sizeof(TDest) == sizeof(TSrc), "size_mismatch");
#endif
	union {  TDest dst; TSrc src; } c;
	c.src = src;
	return c.dst;
}

// thiscall
class C {};
#define Ccall(p,f,r,T) (((C*)p)->*union_cast<r(C::*)T>(f))

// �t�H���g�I�u�W�F�N�g���\�b�h
#define SWRFont_Create(p) \
	Ccall(p,ADDR_SWR_FONT_CREATE,void,())()
#define SWRFont_Destruct(p) \
	Ccall(p,ADDR_SWR_FONT_DESTRUCT,void,())()
#define SWRFont_SetIndirect(p, pdesc) \
	Ccall(p,ADDR_SWR_FONT_SET_INDIRECT,void,(void*))(pdesc)

// �e�N�X�`���}�l�[�W�����\�b�h
#define CTextureManager_LoadTexture(p, ret, path, unk1, unk2) \
	Ccall(p,ADDR_TEXTURE_MANAGER_LOAD_TEXTURE,int*,(int*, LPCSTR, void*, void*))(ret, path, unk1, unk2)
#define CTextureManager_CreateTextTexture(p, ret, str, pdesc ,width ,height, p1, p2) \
	Ccall(p,ADDR_TEXTURE_MANAGER_CREATE_TEXT,int*,(int*, LPCSTR, void*, int, int, int*, int*))(ret, str, pdesc, width, height, p1, p2)
#define CTextureManager_Remove(p, id) \
	Ccall(p,ADDR_TEXTURE_MANAGER_REMOVE,void*,(int))(id)
#define CTextureManager_SetTexture(p, id, stage) \
	Ccall(p,ADDR_TEXTURE_MANAGER_SET_TEXTURE,void,(int, int))(id, stage)
#define CTextureManager_GetSize(p, w, h) \
	Ccall(p,ADDR_TEXTURE_MANAGER_GET_SIZE,void,(int*, int*))(w, h)
// �e�N�X�`���}�l�[�W�����\�b�h(�n���h���}�l�[�W������̌p��)
#define CTextureManager_Get(p, id) \
	((IDirect3DTexture9**)CHandleManager_Get(p, id))
#define CTextureManager_Allocate(p, id) \
	((IDirect3DTexture9**)CHandleManager_Allocate(p, id))
#define CTextureManager_Deallocate(p, id) \
	CHandleManager_Deallocate((void *)p, id)

/*
// �n���h���}�l�[�W�����\�b�h
#define CHandleManager_Get(t, p, id) \
	Ccall(p,0x00402770,t*,(int))(id)
*/
 __declspec(naked) void**

#ifdef __GNUC__
 Thunk_CHandleManager_Allocate(void *, int *) {
#else
Thunk_CHandleManager_Allocate(void *p, int *ret) {
#endif
	// thiscall�Ȃ̂ɗ�����Ǝv������edi���g���Ă����ł�����@�̊�
#ifdef __GNUC__
//TODO: Get this working with GCC __asm syntax
#else
	__asm push edi
	__asm mov edi, [esp+8]
	__asm push [esp+12]
	__asm mov eax, ADDR_ALLOCATE_FUNCTION
	__asm call eax
	__asm pop edi
	__asm retn
#endif
}

#define CHandleManager_Allocate(p, ret) \
	Thunk_CHandleManager_Allocate(p, ret)
 __declspec(naked) void

 #ifdef __GNUC__
Thunk_CHandleManager_Deallocate(void *, int) {
#else
Thunk_CHandleManager_Deallocate(void *p, int id) {
#endif
	 // thiscall(����)eax���g���Ă����ł�����@�̊�
#ifdef __GNUC__
//TODO: Get this working with GCC __asm syntax
#else
	__asm mov eax, [esp+4]
	__asm push [esp+8]
	__asm mov ecx, ADDR_DEALLOCATE_FUNCTION
	__asm call ecx
	__asm retn
#endif
}
#define CHandleManager_Deallocate(p, id) \
	Thunk_CHandleManager_Deallocate(p, id)


// �����_�����\�b�h
#define CRenderer_Begin(p)  \
	Ccall(p,0x00401000,bool,())()
#define CRenderer_End(p) \
	Ccall(p,0x00401040,void,())()

/*
// �Z���N�g�G�t�F�N�g�}�l�[�W�����\�b�h
#define CSelectEffectManager_Create_Address 0x420CE0
#define CSelectEffectManager_Free_Address   0x4221F0
#define CSelectEffectManager_Create(p) \
	Ccall(p, CSelectEffectManager_Create_Address, void, ())()
*/

// �C���v�b�g�}�l�[�W�����\�b�h
#define CInputManager_ReadReplay(p, name) \
	Ccall(p, ADDR_INPUT_MANAGER_READ_REPLAY, bool,(char *))(name)
/*

// �v���t�@�C���f�[�^���\�b�h
#define Profile_RefleshStringTexture(p, r, g, b) \
	Ccall(p, 0x00433FF0, void,(int, int, int))(r, g, b)

// �l�b�g�I�u�W�F�N�g���\�b�h
#define NetObject_CreateProfileStringTexture(p) \
	Ccall(p, 0x00453760, void,())()


// �x�N�^�I�u�W�F�N�g
#define Vector_Create \
	((void (__stdcall *)(void *, size_t, size_t, int, int))0x00811B8B)
#define Vector_Destruct \
	((void (__stdcall *)(void *, size_t, size_t, int))0x00811B28)
*/

// �V�X�e���L�[�����V���b�g
#define CheckKeyOneshot  \
	reinterpret_cast<bool(*)(int, int, int, int)>(ADDR_CHECK_KEY_ONESHOT)
/*
// �p���b�g���[�h
#define LoadPackagePalette(pflag, name, pal, bpp) \
	Ccall(pflag, 0x00408B40, void, (LPCSTR, void *, int))(name, pal, bpp)

// �f�[�^���[�h
#define LoadPackageFile(ppfile, name) \
	Ccall(ppfile, 0x0040CD90, bool, (LPCSTR))(name)

*/
// SE�Đ�
#define PlaySEWaveBuffer \
	reinterpret_cast<void (*)(int id)>(ADDR_PLAY_SE_WAVE_BUFFER)

#define PlayNetBell \
	reinterpret_cast<void (*)(int id)>(ADDR_PLAY_NET_BELL)
/*

// �L�����N�^���̎擾
#define GetCharacterAbbr \
	((LPCSTR (__cdecl *)(int id))0x0043E5E0)

*/
// �o�g�����[�h�ݒ�
#define SetBattleMode \
	reinterpret_cast<void (__cdecl *)(int comm, int sub)>(ADDR_SET_BATTLE_MODE)
/*

// �p�xcos
#define DegreeCosine \
    ((float (__cdecl *)(int deg))0x00409210)
*/

// BGM�֘A�t�@�C���ǂݍ��݊֐��Ăяo���A�h���X
#define GetPackagedBGMCaller	0x00418BE1
#define GetPackagedSFLCaller	0x00418F41

// �l�b�g�ΐ펞�v���t�@�C�����\���֐����Ăяo���Ă���A�h���X
#define ProfileNamePrintCode1	ADDR_PROFILENAME_PRINT_CODE1
#define ProfileNamePrintCode1End	ADDR_PROFILENAME_PRINT_CODE1_END
#define ProfileNamePrintCode2	ADDR_PROFILENAME_PRINT_CODE2
#define ProfileNamePrintCode2End	ADDR_PROFILENAME_PRINT_CODE2_END

// new/delete
#define SWRS_new(t) reinterpret_cast<void*(__cdecl *)(size_t)>(ADDR_NEW_FUNCTION)(t)
#define SWRS_delete(p) reinterpret_cast<void(__cdecl *)(void*)>(ADDR_DELETE_FUNCTION)(p)

// �f�t�H���g�t�H���g��
#define g_defaultFontName reinterpret_cast<const char *>(ADDR_DEFAULT_FONT_NAME)

// �e�N�X�`���}�l�[�W��
// CHandleManager<IDirect3DTexture *>
#define g_textureMgr (reinterpret_cast<void *>(ADDR_TEXTURE_MANAGER))
// Direct3D�f�o�C�X
// IDirect3DDevice9*
#define g_pd3dDev    (*reinterpret_cast<IDirect3DDevice9**>(ADDR_D3D9_DEVICE))
// �����_��
// CRenderer
#define g_renderer   reinterpret_cast<void *>(ADDR_RENDERER)
// �l�b�g���[�N�I�u�W�F�N�g
// CNetworkServer/CNetworkClient
#define g_pnetObject (*reinterpret_cast<char**>(ADDR_PNETOBJECT))
// �v���t�@�C����
// char *
#define g_pprofP1 reinterpret_cast<char*>(g_pnetObject + 0x04)
#define g_pprofP2 reinterpret_cast<char*>(g_pnetObject + 0x24)
// UDP�l�b�g���[�N�I�u�W�F�N�g
// CNetworkBase
#define g_pnetUdp    (g_pnetObject + 0x3BC)
// �s�A���
// vector<SWRClientInfo> ?
#define g_psvClients (g_pnetUdp + 0x10C)
// �T�[�o�A�h���X
// in_addr
#define g_ptoAddr    (g_pnetUdp + 0x3C)
// �o�g���}�l�[�W��
// CBattleManager *
#define g_pbattleMgr (*reinterpret_cast<void **>(ADDR_BATTLE_MANAGER))
/*
// �C���t�H�}�l�[�W��
// CInfoManager *
#define g_pinfoMgr   (*(void **)0x008855C8)
// �v���t�@�C���f�[�^
#define g_profP1    ((void *)0x00885848)
#define g_profP2    ((void *)0x00886034)
*/
// ���[�h
// int
#define g_mainMode   (*reinterpret_cast<DWORD *>(ADDR_COMM_MODE))
#define g_subMode    (*reinterpret_cast<DWORD *>(ADDR_SUB_MODE))
#define g_menuMode   (*reinterpret_cast<DWORD *>(ADDR_MENU_MODE))
// �V�[��ID
// DWORD
#define g_sceneIdNew (*reinterpret_cast<DWORD*>(ADDR_SCENE_ID_NEW))
#define g_sceneId    (*reinterpret_cast<DWORD*>(ADDR_SCENE_ID))
/*
// �R���o�[�g�f�[�^���p�t���O
// bool
#define g_useCVxData (*(bool*)0x0088D028)
// �p���b�g�I�u�W�F�N�g
// void *
#define g_paletter   (*(void **)0x00883B88)
*/
// �C���v�b�g�}�l�[�W��
// CInputManager ?
#define g_inputMgr   reinterpret_cast<void *>(ADDR_INPUT_MANAGER)
// �C���v�b�g�}�l�[�W���N���X�^
// CInputManagerCluster
#define g_inputMgrs  reinterpret_cast<void *>(ADDR_INPUT_MANAGER_CLUSTER)
// �L�����N�^ID
// int
#define g_leftCharID (*reinterpret_cast<int*>(ADDR_LCHARID))
#define g_rightCharID (*reinterpret_cast<int*>(ADDR_RCHARID))
/*
// argc/argv
#define __argc       (*(int*)0x00887B38)
#define __argv       (*(char***)0x00887B3C)

*/
// ���z�֐��e�[�u��
#define vtbl_CLogo                ADDR_VTBL_LOGO

#define vtbl_Opening              0x008467D4
#define vtbl_CLoading             0x0084665C
#define vtbl_CTitle               0x00846FA
#define vtbl_CSelect              0x00846D18
#define vtbl_CSelectScenario      0x00846DD0

#define vtbl_CBattle              ADDR_VTBL_BATTLE
/*
#define vtbl_Ending               0x0084660C
*/
#define vtbl_CSelectSV            ADDR_VTBL_SELECT_SV
/*
#define vtbl_CLoadingSV           0x008464EC
#define vtbl_CBattleSV            0x00846508
*/
#define vtbl_CSelectCL            ADDR_VTBL_SELECT_CL
/*
#define vtbl_CLoadingCL           0x00846544
#define vtbl_CBattleCL            0x00846560
#define vtbl_CLoadingWatch        0x00847178
#define vtbl_CBattleWatch         0x0084657C
*/
#define vtbl_CBattleManager       ADDR_VTBL_BATTLE_MANAGER
/*
#define vtbl_CBattleManagerArcade 0x0084799C
#define vtbl_CBattleManagerStory  0x00847934
#define vtbl_CInfoManager         0x0084A520
#define vtbl_CInfoManagerStory    0x0084A6BC

// �N���X�\�z�֐�caller
#define CLogo_Creater           0x0041DB07
#define Opening_Creater         0x0041DB41
#define CLoading_Creater        0x0041DB7B
#define CTitle_Creater          0x0041DBB5
#define CSelect_Creater         0x0041DBEF
#define CSelectScenario_Creater 0x0041DC29
#define CBattle_Creater         0x0041DC60
#define Ending_Creater          0x0041DC9A
*/
#define CSelectSV_Creater       ADDR_SELECT_SV_CREATER
/*
#define CLoadingSV_Creater      0x0041DD0E
#define CBattleSV_Creater       0x0041DD45
*/
#define CSelectCL_Creater       ADDR_SELECT_CL_CREATER
/*
#define CLoadingCL_Creater      0x0041DDB9
#define CBattleCL_Creater       0x0041DDF0
#define CLoadingWatch_Creater   0x0041DE26
#define CBattleWatch_Creater    0x0041DE59
*/
#define CBattleManager_Creater  ADDR_BATTLE_MANAGER_CREATER
/*

// �N���X�T�C�Y�I�y�����h
#define CLogo_Size              (*(DWORD*)0x0041DAE5)
#define Opening_Size            (*(DWORD*)0x0041DB1F)
#define CLoading_Size           (*(DWORD*)0x0041DB59)
#define CTitle_Size             (*(DWORD*)0x0041DB93)
#define CSelect_Size            (*(DWORD*)0x0041DBCD)
#define CSelectScenario_Size    (*(DWORD*)0x0041DC07)
#define CBattle_Size            (*(BYTE *)0x0041DC41)
#define Ending_Size             (*(DWORD*)0x0041DC78)
*/
#define CSelectSV_Size          (*reinterpret_cast<DWORD *>(ADDR_SELECT_SV_SIZE))
/*
#define CLoadingSV_Size         (*(DWORD*)0x0041DCEC)
#define CBattleSV_Size          (*(BYTE *)0x0041DD26)
*/
#define CSelectCL_Size          (*reinterpret_cast<DWORD *>(ADDR_SELECT_CL_SIZE))
/*
#define CLoadingCL_Size         (*(DWORD*)0x0041DD97)
#define CBattleCL_Size          (*(BYTE *)0x0041DDD1)
#define CLoadingWatch_Size      (*(DWORD*)0x0041DE08)
#define CBattleWatch_Size       (*(BYTE *)0x0041DE3E)
*/
#define CBattleManager_Size     (*reinterpret_cast<DWORD *>(ADDR_BATTLE_MANAGER_SIZE))

// �Z�N�V�����T�C�Y
#define text_Offset  TEXT_SECTION_OFFSET
#define text_Size    TEXT_SECTION_SIZE
#define rdata_Offset RDATA_SECTION_OFFSET
#define rdata_Size   RDATA_SECTION_SIZE
#define data_Offset  DATA_SECTION_OFFSET
#define data_Size    DATA_SECTION_SIZE

// ---------------- �����܂Ńe���v�� ----------------
#pragma pack(pop)
#endif
