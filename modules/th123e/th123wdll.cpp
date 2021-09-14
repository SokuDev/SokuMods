extern "C" {
// for addmemfontresourceex, it's win2k+ only
#define WINVER 0x0500

#include <windows.h>
#include <stdio.h>
#include <shlwapi.h>
#include <png.h>

#include <sys/stat.h>
//#include <unistd.h>

#include "unzip.h"
};

#include <map>
#include <string>

#ifdef LINUX_HACK
#include <d3d9.h>
#endif

// static FILE *logfile = 0;

// *************************************************************** HELPER FUNCS

// all these calls to VirtualProtect are horribly inefficient,
// but it's only done once at startup, so who cares.

// Also, should we get rid of the write bit at a later time? I can't imagine
// this harming anything leaving it open, but this being Windows who knows.

static void write_code(DWORD address, const char *data, int len) {
	DWORD old;

	VirtualProtect((void *)address, len, PAGE_EXECUTE_READWRITE, &old);

	memcpy((char *)address, data, len);
}

static void write_byte(DWORD address, unsigned char value) {
	write_code(address, (const char *)&value, 1);
}
static void write_int(DWORD address, int value) {
	write_code(address, (const char *)&value, 4);
}
static void write_ptr(DWORD address, const char *value) {
	write_code(address, (const char *)&value, 4);
}
//static void write_float(DWORD address, float value) {
//	write_code(address, (const char *)&value, 4);
//}

static void write_call(DWORD address, void *ptr) {
	DWORD value = (DWORD)ptr - (address+0x4);
	write_code(address, (const char *)&value, 4);
}

static void write_noexec_ptr(DWORD address, void *value) {
	DWORD old;

	VirtualProtect((void *)address, 4, PAGE_READWRITE, &old);

	write_code(address, (const char *)&value, 4);
}

static void write_noexec_int(DWORD address, int value) {
	DWORD old;

	VirtualProtect((void *)address, 4, PAGE_READWRITE, &old);

	write_code(address, (const char *)&value, 4);
}

// **************************************************************** ZIPFILE CODE

static bool packfile_open = 0;
static unzFile packfile;
static std::map<std::string, int> packfile_index;

static void open_packfile(const WCHAR *pack_filename) {
	packfile = unzOpen((const unsigned short *)pack_filename);

	if (!packfile) {
		return;
	}

	unzGoToFirstFile(packfile);

	char filename[1024];
	int index = 1;
	while (unzGetCurrentFileInfo(packfile, 0, filename, 1024, 0, 0, 0, 0) == UNZ_OK) {
		if (filename[0] != '\0') {
			char *c = filename;
			while (*c) {
				*c = tolower(*c);
				
				++c;
			}
			
			packfile_index[filename] = index;
		}

		index += 1;

		if (unzGoToNextFile(packfile) != UNZ_OK) {
			break;
		}
	}

	packfile_open = 1;
}

static char *read_whole_file(const char *filename, int *size) {
	if (packfile_open) {
		int index = packfile_index[filename];

		if (index > 0) {
			unzGoToFirstFile(packfile);
			while (index > 1) {
				unzGoToNextFile(packfile);
				index--;
			}

			unz_file_info info;

			unzGetCurrentFileInfo(packfile, &info, 0, 0, 0, 0, 0, 0);

			if (info.uncompressed_size > 0) {
				char *data = new char[info.uncompressed_size];

				unzOpenCurrentFile(packfile);
				unzReadCurrentFile(packfile, data, info.uncompressed_size);
				unzCloseCurrentFile(packfile);

				*size = info.uncompressed_size;

				return data;
			}
		}
	}

#ifndef NDEBUG
	char fname[1200];
	sprintf(fname, "patchdata\\%s", filename);

	struct stat fs;

	if (stat(fname, &fs) != 0) {
		return 0;
	}

	*size = fs.st_size;

	FILE *file = fopen(fname, "rb");
	if (!file) {
		return 0;
	}

	char *data = new char[*size];

	fread(data, *size, 1, file);
	fclose(file);

	return data;
#else
	return 0;
#endif
}

// **************************************************************** IMAGING CODE

// Rather than take the time to break down IaMP's image loader and replace it
// with one function of my own, I replace key portions of it.

// The result is a complete and total mess.

// I didn't bother fixing this for SWR.

static char *png_data_current = 0;
static char *png_data_end;

static void read_callback(png_structp png_ptr, png_bytep data, png_size_t length) {
	if (png_data_current + length > png_data_end) {
		png_error(png_ptr, "Read error.");
	} else {
		memcpy(data, png_data_current, length);
		png_data_current += length;
	}
}

static bool p_opened = 0;
static png_structp png_ptr = 0;
static png_infop info_ptr = 0;
static int p_width = 0;
static int p_height = 0;

static int open_image(char *png_data, int size) {
	png_data_current = png_data+8;
	png_data_end = png_data+size;

	if (png_sig_cmp((png_byte *)png_data, 0, 8)) {
		return 0;
	}

	png_ptr = png_create_read_struct(
                        PNG_LIBPNG_VER_STRING,
                        png_voidp_NULL,
                        png_error_ptr_NULL,
			png_error_ptr_NULL);
	if (!png_ptr) {
		return 0;
	}

	png_infop info_ptr = png_create_info_struct(png_ptr);
	if (!info_ptr) {
		png_destroy_read_struct(&png_ptr, 0, 0);

		return 0;
	}

	// probably going to regret this
	//if (setjmp(png_jmpbuf(png_ptr))) {
	//	png_destroy_read_struct(&png_ptr, &info_ptr, 0);
	//	break;
	//}

	png_set_sig_bytes(png_ptr, 8);
	png_set_user_limits(png_ptr, 4096, 4096);

	png_set_read_fn(png_ptr, (png_voidp *)0, read_callback);

	png_set_bgr(png_ptr);

	png_read_info(png_ptr, info_ptr);

	png_set_expand(png_ptr);

	if (png_get_bit_depth(png_ptr, info_ptr) == 16) {
		png_set_strip_16(png_ptr);
	}

	if (png_get_channels(png_ptr, info_ptr) == 3) {
		png_set_add_alpha(png_ptr, 0xff, PNG_FILLER_AFTER);
	} else if (png_get_channels(png_ptr, info_ptr) != 4) {
		png_destroy_read_struct(&png_ptr, &info_ptr, 0);

		return 0;
	}

	png_read_update_info(png_ptr, info_ptr);

	p_width = png_get_image_width(png_ptr, info_ptr);
	p_height = png_get_image_height(png_ptr, info_ptr);

	p_opened = 1;

	return 1;
}

static int load_image(char *dest, int pitch) {
	if (!p_opened) {
		return 0;
	}

	p_opened = 0;

	int width = p_width;
	int height = p_height;

	unsigned int rowbytes = width * 4;

	if (rowbytes < png_get_rowbytes(png_ptr, info_ptr)) {
		png_destroy_read_struct(&png_ptr, &info_ptr, 0);

		return 0;
	}

	png_bytep* row_ptrs = new png_bytep[height];

	if (pitch < width) {
		width = pitch;
	}

	png_bytep temp = (png_bytep)dest;
	int i;
	for (i = 0; i < height; ++i) {
		row_ptrs[i] = temp;
		temp += pitch*4;
	}

	png_read_image(png_ptr, row_ptrs);
	delete[] row_ptrs;

	png_read_end(png_ptr, 0);

	png_destroy_read_struct(&png_ptr, &info_ptr, 0);

	return 1;
}

// vtable 6c0e45

extern "C" {
static HANDLE hMutex;

struct file_t {
	char *data;
	char *data_end;
	char *data_cur;
	int bytes_read;
};

static int prepare_image(char *data, int size, file_t *file) {
	if (open_image(data, size)) {
		file->data = new char[17 + (p_width * p_height * 4)];

#ifndef __GNUC__
		__pragma(pack(push, 1))
#endif
		struct header_t {
			unsigned char bits_per_pixel;
			unsigned int width;
			unsigned int height;
			unsigned int pitch;
			unsigned int zero;
#ifndef __GNUC__
		} *header = (header_t *)file->data; __pragma(pack(pop));
#else
		} __attribute__((packed)) *header = (header_t *)file->data;
#endif

		header->bits_per_pixel = 0x20;
		header->width = p_width;
		header->height = p_height;
		header->pitch = p_width;
		header->zero = 0;

		if (load_image(file->data+17, p_width)) {
			file->data_cur = file->data;
			file->data_end = file->data + 17 + (p_width * p_height * 4);
			return 1;
		}

		delete[] file->data;

		file->data = 0;
	}

	return 0;
}

typedef void (WINAPI *orig_dealloc_t)(int addr);

static orig_dealloc_t orig_dealloc = (orig_dealloc_t)0x81f6fa;

static int WINAPI read_destruct(int a) {
	int ecx_value;

#ifdef __GNUC__
	asm volatile("movl %%ecx, %0" : "=r" (ecx_value) : : "%ecx");
#else
	__asm mov ecx_value, ecx;
#endif

	if (ecx_value) {
		file_t *file = (file_t *)(ecx_value+4);

		delete[] file->data;

		if (a & 1) {
			orig_dealloc(ecx_value);
		}
	}

	return ecx_value;
}

static int WINAPI read_read(char *dest, int size) {
	int ecx_value;

#ifdef __GNUC__
	asm volatile("movl %%ecx, %0" : "=r" (ecx_value) : : "%ecx");
#else
	__asm mov ecx_value, ecx;
#endif

	if (ecx_value == 0) {
		return 0;
	}

	file_t *file = (file_t *)(ecx_value + 4);

	file->bytes_read = 0;

	if (file->data_cur+size > file->data_end) {
		return 0;
	}

	memcpy(dest, file->data_cur, size);

	file->data_cur += size;

	file->bytes_read = size;

	return 1;
}

static int WINAPI read_getValueA() {
	int ecx_value;

#ifdef __GNUC__
	asm volatile("movl %%ecx, %0" : "=r" (ecx_value) : : "%ecx");
#else
	__asm mov ecx_value, ecx;
#endif

	if (ecx_value == 0) {
		return 0;
	}

	file_t *file = (file_t *)(ecx_value + 4);

	return file->bytes_read;
}

static void WINAPI read_setpos(int offset, int whence) {
	int ecx_value;

#ifdef __GNUC__
	asm volatile("movl %%ecx, %0" : "=r" (ecx_value) : : "%ecx");
#else
	__asm mov ecx_value, ecx;
#endif

	if (ecx_value == 0) {
		return;
	}

	file_t *file = (file_t *)(ecx_value + 4);

	switch(whence) {
	case 0:
		file->data_cur = file->data + offset;
		break;
	case 1:
		file->data_cur += offset;
		break;
	case 2:
		file->data_cur = file->data_end - offset;
		break;
	}

	if (file->data_cur < file->data) {
		file->data_cur = file->data;
	} else if (file->data_cur >= file->data_end) {
		file->data_cur = file->data_end;
	}
}

static int WINAPI read_getValueB() {
	int ecx_value;

#ifdef __GNUC__
	asm volatile("movl %%ecx, %0" : "=r" (ecx_value) : : "%ecx");
#else
	__asm mov ecx_value, ecx;
#endif

	if (ecx_value == 0) {
		return 0;
	}

	file_t *file = (file_t *)(ecx_value + 4);

	return file->data_end - file->data;
}

struct functable_t {
	// ecx == this for all
	int (WINAPI *destruct)(int a);
	int (WINAPI *read)(char *dest, int size);
	int (WINAPI *getvalueA)();
	void (WINAPI *setpos)(int offset, int whence);
	int (WINAPI *getvalueB)();
} read_table = {
	read_destruct,
	read_read,
	read_getValueA,
	read_setpos,
	read_getValueB
};

typedef int (WINAPI *read_constructor_t)(const char *filename, void *a, void *b);

static read_constructor_t orig_read_constructor = (read_constructor_t)0x41c080;

static int WINAPI read_constructor(const char *filename, void *a, void *b) {
	//int ebp_value;
	int esi_value;

#ifdef __GNUC__
	asm volatile("movl %%esi, %0" : "=r" (esi_value) : : "%esi");
	//asm volatile("movl %%ebp, %0" : "=r" (ebp_value) : : "%ecx");
#else
	__asm mov esi_value, esi;
#endif

	const char *extension;
	int len;

	len = strlen(filename);
	extension = filename + len - 4;

	char my_fname[1024];
	char *c;

	strcpy(my_fname, filename);

	if (!memcmp(extension, ".cv2", 4)) {
		strcpy(my_fname + len - 4, ".png");
	}
	
	c = my_fname;
	while (*c) {
		*c = tolower(*c);
		++c;
	}

	c = my_fname;
	while ((c = strchr(c, '/')) != 0) {
		*c = '_';
	}

	WaitForSingleObject(hMutex, INFINITE);

	int size = 0;
	char *data = read_whole_file(my_fname, &size);
	
	if (data) {
		file_t *file = (file_t *)(esi_value+4);
		bool got_it = 0;
		if (!memcmp(extension, ".cv2", 4)) {
			// image
			if (prepare_image(data, size, file)) {
				got_it = 1;
			}
			delete[] data;
		} else if (!memcmp(extension, ".cv0", 4) || !memcmp(extension, ".cv1", 4)) {
			// encrypted data
			file->data = data;
			file->data_cur = data;
			file->data_end = data + size;

			unsigned char xorval = 0x8b;
			unsigned char xoradd = 0x71;
			unsigned char xoraddadd = 0x95;
			while (file->data_cur < file->data_end) {
				*file->data_cur++ ^= xorval;

				xorval += xoradd;
				xoradd += xoraddadd;
			}

			file->data_cur = file->data;
			got_it = 1;
		} else {
			// ordinary data
			file->data = data;
			file->data_cur = data;
			file->data_end = data + size;
			got_it = 1;
		}

		if (got_it) {
			// well well!
			int *addr = (int *)esi_value;

			*addr = (int)&read_table;

			file->bytes_read = 0;

			ReleaseMutex(hMutex);

			return (int)file->data;
		}
	}

	ReleaseMutex(hMutex);

	return orig_read_constructor(filename, a, b);
}
};

// **************************************************************** MISC

// There's a problem with the font rendering code crashing in wine when
// it gets a nonexistent glyph, because the glyph size ends up being
// 0,0, and it tries to divide by the height.
// We don't want that, so we give it some other crap instead.
// This causes some graphical glitchiness, but it doesn't crash.
// Even better: Space in monospatialmod gives the wrong width, so we use
// this function to make sure it's correctly, well, monospaced.
extern "C" {
static DWORD WINAPI myGetGlyphOutline(HDC hdc, UINT uChar, UINT uFormat, LPGLYPHMETRICS lpgm,
			DWORD cbBuffer, LPVOID lpvBuffer,
			CONST MAT2 *lpmat2) {
	DWORD retval = GetGlyphOutlineA(hdc, uChar, uFormat, lpgm, cbBuffer, lpvBuffer, lpmat2);

	if (uChar == ' ') {
		// we use monospaced fonts, and the space width is sometimes
		// wrong, so we fix it with a hack.
		retval = GetGlyphOutlineA(hdc, 'l', uFormat, lpgm, cbBuffer, lpvBuffer, lpmat2);
		if (lpvBuffer) {
			memset(lpvBuffer, 0, cbBuffer);
		}
	}

	if (retval == GDI_ERROR || lpgm->gmBlackBoxX == 0 || lpgm->gmBlackBoxY == 0) {
		retval = GetGlyphOutlineA(hdc, 65, uFormat, lpgm, cbBuffer, lpvBuffer, lpmat2);
		if (lpvBuffer) {
			memset(lpvBuffer, 0, cbBuffer);
		}
	}

	//if (uChar == ' ') {
	//}

	return retval;
}
};

#ifdef LINUX_HACK
extern "C" {
typedef void (*orig_linux_passthrough_t)(DWORD a, DWORD b, DWORD c, DWORD d, DWORD e, DWORD f);
orig_linux_passthrough_t orig_linux_passthrough = (orig_linux_passthrough_t)0x4012a0;

// this is not updated
//IDirect3DDevice9 **d3d_device = (IDirect3DDevice9 **)0x8689b8;

static void WINAPI linux_passthrough(DWORD a, DWORD b, DWORD c, DWORD d, DWORD e, DWORD f) {
	int ecx_value;

	asm volatile("movl %%ecx, %0" : "=r" (ecx_value) : : "%ecx");

	unsigned int cw, orig_cw;
	__asm__ ("fnstcw %0" : "=m" (*&cw));
	orig_cw = cw;
	cw &= ~0xc00;
	__asm__ ("fldcw %0" : : "m" (*&cw));
	
	asm volatile("movl %0, %%ecx" : : "r" (ecx_value) : "%ecx");

	orig_linux_passthrough(a,b,c,d,e,f);

	__asm__ ("fldcw %0" : : "m" (*&orig_cw));
	
	// Not done yet!
	/*
	if (*d3d_device) {
		D3DVIEWPORT9 viewport;
		viewport.X = 0;
		viewport.Y = 0;
		viewport.Width = 640;
		viewport.Height = 480;
		viewport.MinZ = -2048.0;
		viewport.MaxZ = 2048.0;
	
		(*d3d_device)->SetViewport(&viewport);
	}
	 */
}
};

#endif

const BYTE TARGET_HASH[16] = {0xdf, 0x35, 0xd1, 0xfb, 0xc7, 0xb5, 0x83, 0x31, 0x7a, 0xda, 0xbe, 0x8c, 0xd9, 0xf5, 0x3b, 0x2e};

extern "C" {
__declspec(dllexport) bool CheckVersion(const BYTE hash[16]) {
	return ::memcmp(TARGET_HASH, hash, sizeof TARGET_HASH) == 0;
}

BOOL APIENTRY DllMain(HANDLE hMod, DWORD reason, LPVOID nothing) {
	return TRUE;
}

__declspec(dllexport) bool Initialize(HMODULE hMyModule, HMODULE hParentModule) {
	WCHAR path[1024];
	GetModuleFileNameW(hMyModule, path, 1024);
	PathRemoveFileSpecW(path);

	WCHAR th123wpath[1040];
	wsprintfW(th123wpath, L"%s\\th123w.zip", path);

	open_packfile(th123wpath);

	// make sure we lock loading stuff
	hMutex = CreateMutex(0, FALSE, 0);

	// Patch file loader
	write_call(0x40d228, (void *)read_constructor);

	// glyph crash bug again.
	write_byte(0x411e53, 0xe8);
	write_call(0x411e54, (void *)myGetGlyphOutline);
	write_byte(0x411e58, 0x90);
	write_byte(0x411f1a, 0xe8);
	write_call(0x411f1b, (void *)myGetGlyphOutline);
	write_byte(0x411f1f, 0x90);
	write_byte(0x4120c2, 0xe8);
	write_call(0x4120c3, (void *)myGetGlyphOutline);
	write_byte(0x4120c7, 0x90);
	write_byte(0x41213b, 0xe8);
	write_call(0x41213c, (void *)myGetGlyphOutline);
	write_byte(0x412130, 0x90);

#ifdef LINUX_HACK
	// fix linux timing
	write_int(0x419109, 14);
	write_noexec_int(0x8a0ff8, 14);
	
	// make it actually run.
	write_call(0x41e408, (void *)linux_passthrough);
	
	// fix linux rendering glitch
	
	// fix winsock freeze on canceling hosting
	
	// fix crash on quit (related to textures?)
#endif
}
};
