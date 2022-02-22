#include <windows.h>

namespace {
	const BYTE TARGET_HASH[16] = { 0xdf, 0x35, 0xd1, 0xfb, 0xc7, 0xb5, 0x83, 0x31, 0x7a, 0xda, 0xbe, 0x8c, 0xd9, 0xf5, 0x3b, 0x2e };
	bool _initialized = false;
}

extern "C" __declspec(dllexport) bool CheckVersion(const BYTE hash[16]) {
	return ::memcmp(TARGET_HASH, hash, sizeof TARGET_HASH) == 0;
}

static inline DWORD TamperNearJmp(DWORD addr, DWORD target) {
	*reinterpret_cast<PBYTE>(addr + 0) = 0xE9;
	DWORD old = *reinterpret_cast<PDWORD>(addr + 1) + (addr + 5);
	*reinterpret_cast<PDWORD>(addr + 1) = target - (addr + 5);
	return old;
}

template <int N>
static inline void TamperData(DWORD addr, const unsigned char (&data)[N]) {
	for (int i = 0; i < N; ++i) {
		*(unsigned char *)(addr + i) = data[i];
	}
}

static void __declspec(naked) setTextureParams() {
	__asm {
		cmp dl, 0x10;			// if 16bit
		jz jmp_409014;			// goto original 16bit setup
		cmp dl, 0x8;			// if not 8bit
		jnz jmp_4090fa;			// goto original failure setup
		push eax;
		mov eax, ds:[0x896b88];	// get palette bitPerPixel
		cmp eax, 0x10;			// if <= 16bits
		pop eax;
		jbe jmp_409014			// goto original 16bit setup

		// --- BEGIN copying code ---
		test esi, esi;
		mov ebx, ecx;
		mov edi, eax;
		je lbl_false;
		cmp ecx, eax;
		jae lbl_true;
		mov ebx, eax;
		jmp lbl_false;
	lbl_true:
		mov edi, ecx;
	lbl_false:
		push 0x8a0e14;
		mov eax, ds:[0x85711c];
		call eax;
		mov eax, [esp+0x18];
		mov ecx, [esp+0x10];
		push eax;
		push 1;
		// --- END copying code ---

		push 0x15;				// texture format ARGB
		jmp jmp_40903e;			// goto remaining original code

	// direct jumps
	jmp_409014:
		push 0x409014;
		ret;
	jmp_4090fa:
		push 0x4090fa;
		ret;
	jmp_40903e:
		push 0x40903e;
		ret;
	}
}

static void __declspec(naked) copyImageToBuffer_normal() {
	// eax = bitmapdata;
	// ebx = j = 0;
	// ecx = line length;
	// esi = output buffer
	__asm {
		mov edx, ds:[0x896b88];	// get palette bitPerPixel
		cmp edx, 0x10;			// if not 16bit
		jg lbl_32bits			// goto replacement code
		shr ecx, 1;
		cmp [eax+0x0c], ebx;	// copied replaced hook
		jmp jmp_41b1e9;			// go back to original code

	lbl_32bits:
		mov edi, esi;			// edi = output buffer
		mov esi, [eax+0x1c];	// esi = index data
		cmp [eax+0x0c], ebx;	// if height <= zero
		jle jmp_41b320;			// goto original failure code
		mov edx, [eax+0x08];	// edx = width
		mov [esp+0x24], ecx		// esp+24 = line length
		mov ebp, [eax+0x18];	// ebp = palete data

	lbl_loop_lines:
		xor ecx, ecx;			// ecx = i = 0;
		test edx, edx;			// if width <= zero
		jle lbl_loop_end;		// got loop_end

	lbl_loop_pixels:
		movzx dl, [ecx+esi];	// dl = index value (with zeros)
		mov edx, [ebp+edx*4];	// edx = color value
		mov [edi+ecx*4], edx;	// buffer[ecx] = color
		mov edx, [eax+0x08];	// edx = width
		add ecx, 1;				// ++ecx
		cmp ecx, edx;
		jl lbl_loop_pixels;		// while ecx < width

		add edi, [esp+0x24];	// edi += line length
		add esi, [eax+0x10];	// esi += padded width
		add ebx, 1;				// ++ebx
		cmp ebx, [eax+0x0c];
		jl lbl_loop_lines;		// while ebx < height

	lbl_loop_end:
		jmp jmp_41b239;			// goto original code end

	// direct jumps
	jmp_41b1e9:
		push 0x41b1e9;
		ret;
	jmp_41b239:
		push 0x41b239;
		ret;
	jmp_41b320:
		push 0x41b320;
		ret;
	}
}

// TODO make copy for alternative image storage

static void doHook() {
	DWORD dwOldProtect;
	VirtualProtect((LPVOID)0x409006, 0x020000, PAGE_EXECUTE_WRITECOPY, &dwOldProtect);
	TamperNearJmp(0x409006, (DWORD)&setTextureParams);
	TamperNearJmp(0x41b1e4, (DWORD)&copyImageToBuffer_normal);
	VirtualProtect((LPVOID)0x409006, 0x020000, dwOldProtect, &dwOldProtect);
}

static void undoHook() {
	if (!_initialized) return;
	DWORD dwOldProtect;
	VirtualProtect((LPVOID)0x409006, 0x020000, PAGE_EXECUTE_WRITECOPY, &dwOldProtect);
	TamperData(0x409006, {0x80, 0xFA, 0x08, 0x74, 0x09});
	TamperData(0x41b1e4, {0xD1, 0xE9, 0x39, 0x58, 0x0C});
	VirtualProtect((LPVOID)0x409006, 0x020000, dwOldProtect, &dwOldProtect);
}

extern "C" __declspec(dllexport) bool Initialize(HMODULE hMyModule, HMODULE hParentModule) {
	if (!_initialized) _initialized = true;
	else return false;

	doHook();

	return true;
}

BOOL WINAPI DllMain(HMODULE hModule, DWORD fdwReason, LPVOID lpReserved) {
	if (fdwReason == DLL_PROCESS_DETACH) {
		undoHook();
	}

	return TRUE;
}
