// TODO
//
// - graphical bugs:
//   - in match spellcard is drawn weird (i can't replicate!!)
//     (we're going to hack around this with <>'s)
// - verify compatibility with swrloader
// - build final version

extern "C" {

#include <windows.h>
#include <stdio.h>
#include <shlwapi.h>
#include <png.h>
#include <sys/stat.h>

#include "unzip.h"
};

#include <map>
#include <string>

//#define LOGGING
#ifdef LOGGING
static FILE *logfile;
#endif

static char deckdescfontname[1024] = "Bitstream Vera Sans Mono";
static char fontname[1024] = "MonoSpatialModSWR";
static char storyfontname[1024] = "MonoSpatialModSWR";
static char musicroomfontname[1024] = "WinterthurCondensed";
static char storyspellfontname[1024] = "WinterthurCondensed";
static char deckfontname[1024] = "Bitstream Vera Sans Mono";
static int deck_width = 5;
static int deck_height = 15;
static int deckdesc_width = 6;
static int deckdesc_height = 16;
static int musicroom_width = 5;
static char embedded_font_filename[1024] = "monospatialmodswr.ttf";
static char embedded_font_filename_2[1024] = "winterthurcondensed.ttf";
static char embedded_font_filename_3[1024] = "veramono.ttf";

#ifndef NDEBUG
static int start_stage = 0;
static int skip_spellcards = 0;
#endif

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

		__pragma(pack(push, 1))
		struct header_t {
			unsigned char bits_per_pixel;
			unsigned int width;
			unsigned int height;
			unsigned int pitch;
			unsigned int zero;
		} *header = (header_t *)file->data; __pragma(pack(pop));

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

typedef void (*orig_dealloc_t)(int addr);

static orig_dealloc_t orig_dealloc = (orig_dealloc_t)0x81f6fa;

static int WINAPI read_destruct(int a) {
	int ecx_value;
	__asm mov ecx_value, ecx;

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
	__asm mov ecx_value, ecx;

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
	__asm mov ecx_value, ecx;

	if (ecx_value == 0) {
		return 0;
	}

	file_t *file = (file_t *)(ecx_value + 4);

	return file->bytes_read;
}

static void WINAPI read_setpos(int offset, int whence) {
	int ecx_value;
	__asm mov ecx_value, ecx;

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
	__asm mov ecx_value, ecx;

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
	__asm mov esi_value, esi;

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

	int size;
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

			// for stage warp and spellcard skip, recode the story csvs
			// this is mad ghetto.
#ifndef NDEBUG
			if ((start_stage != 0 || skip_spellcards) && strstr(my_fname, "data_csv_") && strstr(my_fname, "_story.cv1")) {
				char *a = file->data;
				bool line_start = 0;
				int skipping = 0;
				int stage = 0;
				bool stage_line = 0;
				int except_the_first = 0;

				while (a < file->data_end) {
					if (*a == '\r') {
					} else if (line_start) {
						if (a[0] == ('0'+stage) && a[1] == ',' && a[2] == '\"') {
							if (stage < start_stage) {
								*a = '#';
								skipping = 255;
								stage++;
							} else {
								skipping = 0;
								stage_line = 1;
								stage++;
							}
						} else if (skipping) {
							if (*a < '0' || *a > '9') {
							} else if (except_the_first) {
								except_the_first = 0;
							} else {
								*a = '#';
								skipping--;
							}
						}

						line_start = 0;
					} else if (stage_line && *a >= '1' && *a <= '9') {
						if (!except_the_first) {
							skipping += *a - '1';
						}
						*a = '1';
						except_the_first = 1;
					} else if (*a == '\n') {
						stage_line = 0;
						line_start = 1;
					}
					a++;
				}
			}
#endif

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
static int lastFontHeight = 16;
static HFONT WINAPI myCreateFont(int nHeight, int nWidth, int nEscapement, int nOrientation,
				int fnWeight,
				DWORD fdwItalic, DWORD fdwUnderline,
				DWORD fdwStrikeOut, DWORD fdwCharSet,
				DWORD fdwOutputPrecision,
				DWORD fdwClipPrecision,
				DWORD fdwQuality,
				DWORD fdwPitchAndFamily,
				LPCTSTR lpszFace) {
	
	lastFontHeight = nHeight;
	
	return CreateFont(nHeight, nWidth, nEscapement, nOrientation,
				fnWeight,
				fdwItalic, fdwUnderline,
				fdwStrikeOut, fdwCharSet,
				fdwOutputPrecision,
				fdwClipPrecision,
				fdwQuality,
				fdwPitchAndFamily,
				lpszFace);
}

static DWORD WINAPI myGetGlyphOutline(HDC hdc, UINT uChar, UINT uFormat, LPGLYPHMETRICS lpgm,
			DWORD cbBuffer, LPVOID lpvBuffer,
			CONST MAT2 *lpmat2) {
	HFONT temp = 0;
	if (uChar > 0xff) {
		static HFONT sjisfont = 0;
		static int fontHeight = 0;
		static bool initialized = 0;
		
		if (initialized && fontHeight != lastFontHeight) {
			if (sjisfont != 0) {
				DeleteObject(sjisfont);
				sjisfont = 0;
			}
			
			initialized = 0;
		}
		
		if (!initialized) {
			fontHeight = lastFontHeight;
			sjisfont = CreateFontW(fontHeight - 2, 0, 0, 0, FW_DONTCARE, FALSE, FALSE, FALSE, 128, OUT_TT_PRECIS, CLIP_DEFAULT_PRECIS, PROOF_QUALITY, FIXED_PITCH|FF_MODERN, L"MS ゴシック");
			initialized = 1;
		}
		
		char in_buf[10];
		WCHAR out_buf[10];
		int count;
		in_buf[1] = uChar & 0xff;
		in_buf[0] = (uChar >> 8) & 0xff;
		in_buf[2] = '\0';
		out_buf[0] = '\0';
		
		count = MultiByteToWideChar(932, 0, in_buf, 2, out_buf, 10);
		if (count > 0 && count == 1) {
			uChar = out_buf[0];
			
			if (sjisfont != 0) {
				temp = (HFONT)GetCurrentObject(hdc, OBJ_FONT);
				SelectObject(hdc, sjisfont);
			}
		}
	}

	DWORD retval = GetGlyphOutlineW(hdc, uChar, uFormat, lpgm, cbBuffer, lpvBuffer, lpmat2);
	
	if (uChar == ' ') {
		// we use monospaced fonts, and the space width is sometimes
		// wrong, so we fix it with a hack.
		retval = GetGlyphOutlineW(hdc, 'l', uFormat, lpgm, cbBuffer, lpvBuffer, lpmat2);
		if (lpvBuffer) {
			memset(lpvBuffer, 0, cbBuffer);
		}
	}

	if (retval == GDI_ERROR || lpgm->gmBlackBoxX == 0 || lpgm->gmBlackBoxY == 0) {
		retval = GetGlyphOutlineW(hdc, 65, uFormat, lpgm, cbBuffer, lpvBuffer, lpmat2);
		if (lpvBuffer) {
			memset(lpvBuffer, 0, cbBuffer);
		}
	}
	
	if (temp != 0) {
		SelectObject(hdc, temp);
	}

	return retval;
}

#ifdef LINUX_HACK
static int WINAPI myToAscii(UINT uVertKey, UINT uScanCode, BYTE *lpKeyState, LPWORD lpChar, UINT uFlags) {
	BYTE tempKeyState[256];
	
	if (!lpKeyState) {
		lpKeyState = tempKeyState;
	}
	
	return ToAscii(uVertKey, uScanCode, lpKeyState, lpChar, uFlags);
}
#endif

};

// Yay Windows fonts.
static void load_embedded_font() {
	if (embedded_font_filename[0] == '\0') {
		return;
	}

	int size;
	char *data = read_whole_file(embedded_font_filename, &size);

	if (data) {
		DWORD nfonts = 0;
		AddFontMemResourceEx((void *)data, size, 0, &nfonts);
	}

	data = read_whole_file(embedded_font_filename_2, &size);

	if (data) {
		DWORD nfonts = 0;
		AddFontMemResourceEx((void *)data, size, 0, &nfonts);
	}

	data = read_whole_file(embedded_font_filename_3, &size);

	if (data) {
		DWORD nfonts = 0;
		AddFontMemResourceEx((void *)data, size, 0, &nfonts);
	}
}

#ifndef NDEBUG
static void WINAPI skip_to_stage() {
	char *esi_value;
	__asm mov esi_value, esi;

	*(esi_value+0x8e8) = start_stage;
}
#endif

// *********************************************** SPELLCARD ALIGNMENT CALC
static int WINAPI spellcard_alignment(char **eax) {
	const char *sc_name = eax[1];
	int orig_length = ((unsigned int *)eax)[5];
	int len = 0;
	int mult = 7;
	int add = 0;

	if (orig_length <= 0xf) {
		sc_name = (char *)&eax[1];
	}

	if (strstr(sc_name, "<story>")) {
		mult = 5;
		add = 3;
	}

	while (*sc_name) {
		if (*sc_name == '<') {
			++sc_name;
			
			const char *start = sc_name;
			
			while (*sc_name && *sc_name != '>') {
				++sc_name;
			}
			
			if (*start == 'a' && (sc_name - start) == 1) {
				break;
			}
			
			if (*sc_name) {
				++sc_name;
			}
			continue;
		}
		len++;
		sc_name++;
	}

	//asm volatile("movl %0, %%ecx" : : "r" (ecx_value) : "%ecx");
	//asm volatile("movl %0, %%edx" : : "r" (edx_value) : "%edx");

	return (len*mult)+add;
}

// ***************************************************** RESULTS PASSTHROUGH

static void results_passthrough(char *dest, int length, const char *format, const char *name, const char *diff, int c1, int c2, int c3, int c4) {
	char name_buf[1024];

	if (strchr(name, '<') && strlen(name) < 1023) {
		const char *s = name;
		char *d = name_buf;

		do {
			if (*s == '<') {
				while (*s && *s != '>') {
					++s;
				}
				if (*s) {
					++s;
				}
			}
			*d++ = *s;
		} while (*s++);

		name = name_buf;
	}

	snprintf(dest, length, format, name, diff, c1, c2, c3, c4);
}

// *********************************************************** DECK FONT FIX
typedef void (WINAPI *orig_deck_func_t)(int a, int b, int c);
typedef int (WINAPI *orig_deck3_func_t)(int a, int b, char *c, int d, int e, int f, int g);

static orig_deck_func_t orig_deck_func = (orig_deck_func_t)0x438500;
static orig_deck_func_t orig_deck2_func = (orig_deck_func_t)0x438450;
static const orig_deck3_func_t orig_deck3_func = (orig_deck3_func_t)0x4050a0;

static void WINAPI deck_passthrough(int a, int b, int c) {
	void *ecx_value;
	__asm mov ecx_value, ecx;

	write_byte(0x411c73, deck_width);
	write_ptr(0x438533, deckfontname);
	write_int(0x4385e1, deck_height);	// deck/game spell card text size
	write_int(0x438605, 16-deck_height);	// deck/game spell card text spacing

	__asm mov ecx, ecx_value;
	orig_deck_func(a, b, c);

	write_byte(0x411c73, 0x00);
	write_ptr(0x438533, fontname);
	write_int(0x4385e1, 0x10);	// deck/game spell card text size
	write_int(0x438605, 0x00);	// deck/game spell card text spacing
}

static void WINAPI deck2_passthrough(int a, int b, int c) {
	void *ecx_value;
	__asm mov ecx_value, ecx;

	write_byte(0x411c73, deck_width);
	write_ptr(0x438533, deckfontname);
	write_int(0x4385e1, deck_height);	// deck/game spell card text size
	write_int(0x438605, 16-deck_height);	// deck/game spell card text spacing

	__asm mov ecx, ecx_value;
	orig_deck2_func(a, b, c);

	write_byte(0x411c73, 0x00);
	write_ptr(0x438533, fontname);
	write_int(0x4385e1, 0x10);	// deck/game spell card text size
	write_int(0x438605, 0x00);	// deck/game spell card text spacing
}

static int WINAPI deck3_passthrough(int a, int b, char *c, int d, int e, int f, int g) {
	void *ecx_value;
	__asm mov ecx_value, ecx;

	write_byte(0x411c73, deckdesc_width);
	strcpy(c + 0xc, deckdescfontname);
	*(int *)(c + 0x114) = deckdesc_height;
	
	__asm mov ecx, ecx_value;
	int retval = orig_deck3_func(a, b, c, d, e, f, g);

	write_byte(0x411c73, 0x00);
	strcpy(c + 0xc, fontname);
	*(int *)(c + 0x114) = 16;

	return retval;
}

// *********************************************************** DECK POS FIX
static void __stdcall deck_desc_pos_passthrough(float a, float b) {
	void *ecx_value;
	__asm mov ecx_value, ecx;

	b -= 10.0f;

	typedef void (*func_t)(float a, float b);
	func_t func = ((func_t **)ecx_value)[0][0x1c/4];
	
	__asm mov ecx, ecx_value;
	func(a, b);
}


// ***************************************************** STORYSPELL FONT FIX
typedef void (WINAPI *orig_storyspell_func_t)(int a, int b, int c);

static orig_storyspell_func_t orig_storyspell_func = (orig_storyspell_func_t)0x437cb0;
//static orig_storyspell_func_t orig_storyspell_func = (orig_storyspell_func_t)1x437140;

static void WINAPI storyspell_passthrough(int a, int b, int c) {
	void *ecx_value;
	__asm mov ecx_value, ecx;

	if (b & 0xff) {
		write_byte(0x411c73, deck_width);
		write_ptr(0x438533, storyspellfontname);
		write_int(0x4385e1, deck_height);	// deck/game spell card text size
		write_int(0x438605, 16-deck_height);	// deck/game spell card text spacing
	}
	
	__asm mov ecx, ecx_value;
	orig_storyspell_func(a, b, c);

	if (b & 0xff) {
		write_byte(0x411c73, 0x00);
		write_ptr(0x438533, fontname);
		write_int(0x4385e1, 0x10);	// deck/game spell card text size
		write_int(0x438605, 0x00);	// deck/game spell card text spacing
	}
}

// *********************************************************** MUSICROOM FIX

typedef int (*orig_musicroom_func_t)(int a, int b, int c);

static orig_musicroom_func_t orig_musicroom_func = (orig_musicroom_func_t)0x43d760;

static int musicroom_passthrough(int a, int b, int c) {
	void *ecx_value;
	__asm mov ecx_value, ecx;

	write_byte(0x411c73, musicroom_width);
	write_ptr(0x43d7ac, storyspellfontname);

	__asm mov ecx, ecx_value;
	int d = orig_musicroom_func(a, b, c);

	write_byte(0x411c73, 0x00);
	write_ptr(0x43d7ac, fontname);

	return d;
}

// *********************************************************** SPELLCARD GET FIX

typedef int (*orig_spellcard_get_func_t)();

static orig_spellcard_get_func_t orig_spellcard_get_func = (orig_spellcard_get_func_t)0x443ef0;

static int spellcard_get_passthrough(int a, int b, int c) {
	void *ecx_value;
	__asm mov ecx_value, ecx;

	write_byte(0x411c73, musicroom_width);
	strcpy((char *)(0x89a4f8+0xc), deckfontname);

	__asm mov ecx, ecx_value;
	int d = orig_spellcard_get_func();

	write_byte(0x411c73, 0x00);
	strcpy((char *)(0x89a4f8+0xc), fontname);

	return d;
}

// **************************************************************** TEXT FIX
typedef void (WINAPI *orig_text_func_t)(void *a, void *b, void *c, unsigned int d);

static orig_text_func_t orig_text_func = (orig_text_func_t)0x40fc00;

static unsigned char* WINAPI text_passthrough(unsigned char *ebp, void *esp, void *a, void *b, void *c, unsigned int d) {
	void *ecx_value;

	ecx_value = (char *)esp + 0x68;

	static bool kana = 0;

	int dh = d & 0xff;

	if ((dh >= 0x80 && dh <= 0xa0) || dh >= 0xe0) {
		kana = 1;
	} else if (kana) {
		kana = 0;
	} else {
		if (dh == '\\') {
			unsigned char c = *(ebp+1);
			if (c == '\'' || c == '\"' || c == '\\' || c == ',') {
				ebp++;
				d = (d&~0xff) | c;
			}
		}
	}

	__asm mov ecx, ecx_value;
	orig_text_func(a, b, c, d);

	return ebp;
}

static int WINAPI script_passthrough(char *ebp, char *edx) {
	char *orig_edx = edx;
	int retval = 0;
	ebp -= 0x24;

	unsigned char ch = *edx;

	if (ch == '\\' && *(edx+1) == ',' && (ebp[','>>3]&(1<<(','&7)))) {
		do {
			*edx = *(edx+1);
			edx++;
		} while (*edx != '\0' && *edx != '\n' && *edx != '\r');
	} else {
		retval = ebp[ch>>3]&(1<<(ch&7));
	}

	__asm mov edx, orig_edx;
	return retval;
}

// **************************************************************** FONT PASSTHROUGH
static void __stdcall _font_passthrough(unsigned int *eax_value, unsigned int a, void *b, void *c) {
	// the number check is a stupid hack to get around the problems
	// with the number values on the deck screen
	if (eax_value[0x134/4] <= 1) { // && (a < '0' || a > '9') && a != '/' && a != ' ') {
		eax_value[0x134/4] = 2;
		eax_value[0x124/4] = 1;
	}
	//fprintf(logfile, "%4.4x %4.4x %8.8x %8.8x %8.8x\n", eax_value[0x134/4], eax_value[0x124/4], a, b, c);

	const int orig_font_func = 0x411e20;
	__asm {
		push c;
		push b;
		push a;
		mov eax, eax_value;
		call orig_font_func;
	}
}

static __declspec(naked) void font_passthrough() {
	__asm pop ecx;
	__asm push eax;
	__asm push ecx;
	__asm jmp _font_passthrough;
}
// **************************************************************** LINUX CRAP
#ifdef LINUX_HACK
extern "C" {
typedef void (*orig_linux_passthrough_t)(DWORD a, DWORD b, DWORD c, DWORD d, DWORD e, DWORD f);
static const orig_linux_passthrough_t orig_linux_passthrough = (orig_linux_passthrough_t)0x4012a0;

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
}
};

#endif

// **************************************************************** MAIN

static const float loading_x = 480.0;
static const double results_x = 0.0;
static const double deck_render_width = 220.0;

#ifdef NDEBUG
static const char patch_windowname[] = "Touhou Hisoutensoku ver1.10a (eng v1.1a)";
#else
static const char patch_windowname[] = "Touhou Hisoutensoku ver1.10a (eng v1.1a dev build)";
#endif

static struct {
	DWORD	addr;
	const char *string;
} text_table[] = {
	// DINPUT
	{ 0x40d66d, "Failed to create DirectInput object #0" },
	{ 0x40d6a5, "Failed to create DirectInput object #1" },
	{ 0x40d867, "Failed to initialize keyboard" },
	{ 0x40d898, "Failed to set data format" },
	{ 0x40d8ce, "Failed to set cooperation level" },
	{ 0x40d930, "Failed to queue device" },
	{ 0x40db1a, "Failed to initialize mouse" },
	{ 0x40db4e, "Failed to set data format" },
	{ 0x40db87, "Failed to set cooperation level" },
 	{ 0x40dbd8, "Failed to set properties" },
 	
 	// INTERNETS
 	{ 0x41312c, "Failed to initialize socket" },
 	{ 0x413331, "Failed to bind port" },

 	// DIRECT3D
	{ 0x414f05, "Failed to create D3D object" },
	{ 0x414f35, "Failed to get display mode" },
	{ 0x415079, "Failed to create D3D device" },

 	// DSOUND
	{ 0x418392, "Failed to get DirectSoundBuffer8" },
	{ 0x4183d1, "Failed to get DirectSound3DBuffer8" },
	{ 0x41851e, "Failed to get DirectSoundBuffer8" },
	{ 0x4185ca, "Failed to lock buffer" },
	{ 0x4186f2, "Failed to lock buffer" },
	{ 0x41884c, "Failed to create DirectSound object" },
	{ 0x41888b, "Failed to initialize DirectSound object" },
	{ 0x4188e1, "Failed to set cooperation level" },
	{ 0x4189ab, "Failed to create primary sound buffer" },
	{ 0x4189f7, "Failed to create listener" },
	{ 0x418af1, "Failed to create temporary buffer" },

 	// ERROR
	{ 0x7fb88d, "Failed to initialize" },

 	// Replay
	{ 0x442eac, "Failure to create replay folder." },
	{ 0x444d50, "Returning to select. Save replay?" },
	{ 0x444d6f, "Returning to title. Save replay?" },
	{ 0x444e75, "Replay saved." },
	{ 0x4492e4, "Replay saved." },
	{ 0x4496b0, "Replay saved." },

 	// Netplay
	{ 0x448f37, "Select from saved addresses..." },
	{ 0x448f5b, "Connect to previous address..." },
	{ 0x446842, "Input port number..." },
	{ 0x4468e2, "Connect to address..." },
	{ 0x446e8f, "Allow spectating?" },
	{ 0x446e3c, "Waiting for connection..." },
	{ 0x448872, "0 1 2 3 4 5 6 7 8 9<br>^<br>     .      .      .      : " },
	{ 0x448c92, "Spectating is not allowed." },
	{ 0x448c76, "Game has not started yet." },
	{ 0x448b73, "Connection failed." },
	{ 0x448c5a, "Connection failed." },
	{ 0x448cae, "Connection failed." },
	{ 0x448afa, "Connecting..." },
	{ 0x448ab0, "Play a match? (Select no to spectate)" },
	{ 0x44902c, " is the address?" },

	{ 0x448b6a, "Already playing." },
	{ 0x448b5a, "Already playing. Spectate?" },

 	// Replay/continue/etc
	{ 0x449319, "Continue?  %d left" },
	{ 0x449298, "Save replay?" },
	{ 0x449682, "Save replay?" },
	{ 0x44a12a, "<color 808080>? ? ? ? ? ? ? ? </color>" },
	{ 0x450bbd, "<color 808080>? ? ? ? ? ? ? ? </color>" },
	{ 0x44b654, "Delete this replay?" },

 	// Profile stuff
	{ 0x442e81, "Failure to create profile folder." },
	{ 0x44b598, "Failed to delete replay." },
	{ 0x452613, "Failed to delete profile." },
	{ 0x44b57e, "Replay deleted." },
	{ 0x4525d4, "Profile deleted." },
	{ 0x44c947, "<color 808080>? ? ? ? ? ? ? ? ? ? ? ? ? ? ? ? </color>" },
	{ 0x450256, "Save changes?" },
	{ 0x4503c9, "Save changes?" },
	{ 0x450188, "A deck must have 20 cards." },
	{ 0x452090, " creation failed." },
	{ 0x4520f8, " creation failed." },
	{ 0x452009, " created." },
	{ 0x452469, "Profile copied." },
	{ 0x4523d7, "Profile copy failed." },
	{ 0x4527e1, "Name change failed." },
	{ 0x452840, "Name change failed." },
	{ 0x45277c, "Name changed." },
	{ 0x452972, " can be overwritten?" },
	{ 0x452f6c, " can be overwritten?" },
	{ 0x452ae1, " will be copied." },
	{ 0x452c71, " will be deleted." },

  	// stupid spellcard clip fix
	{ 0x43780e, "<br><br>" },
	{ 0x437ff4, "<br><br>" },
	
	{ 0x44c90b, "%-52s %s  %03d/%03d  Time %02d.%02d" },

	{ 0x0, 0 }
};

//static const char patch_font_spacing[] = { 0x6a, 0x00, 0x90, 0x90, 0x90, 0x90 };
static const char patch_font_type[] = { 0x68, 0x01, 0x00, 0x00, 0x00 };

// ghetto.
static void ununicode(char *str) {
	short *src = (short *)str;

	do {
		*str++ = *src++;
	} while (*src);

	*str = 0;
}

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

	WCHAR th105epath[1040];
	wsprintfW(th105epath, L"%s\\th123e.zip", path);

	open_packfile(th105epath);

#ifdef LOGGING
	logfile = fopen("th123elog.txt", "w");
#endif

	// ini file stuff here.. remove later?
#ifndef NDEBUG
	bool hpcheat = 0;
	WCHAR ini_path[1024];

	memcpy(ini_path, path,2048);
	lstrcatW(ini_path, L"\\th123e.ini");

	start_stage = GetPrivateProfileIntW(L"TH123E", L"Stage", 0, ini_path);
	skip_spellcards = GetPrivateProfileIntW(L"TH123E", L"SkipSpellcards", 0, ini_path);

	// remnants of old code, convert to unicode properly sometime
	GetPrivateProfileStringW(L"TH123E", L"Font", L"MonoSpatialModSWR", (WCHAR *)fontname, 511, ini_path);
	fontname[1022] = '\0';
	fontname[1023] = '\0';
	ununicode(fontname);

	GetPrivateProfileStringW(L"TH123E", L"StoryFont", L"MonoSpatialModSWR", (WCHAR *)storyfontname, 511, ini_path);
	storyfontname[1022] = '\0';
	storyfontname[1023] = '\0';
	ununicode(storyfontname);

	GetPrivateProfileStringW(L"TH123E", L"MusicroomFont", L"WinterthurCondensed", (WCHAR *)musicroomfontname, 511, ini_path);
	musicroomfontname[1022] = '\0';
	musicroomfontname[1023] = '\0';
	ununicode(musicroomfontname);

	GetPrivateProfileStringW(L"TH123E", L"DeckFont", L"Bitstream Vera Sans Mono", (WCHAR *)deckfontname, 511, ini_path);
	deckfontname[1022] = '\0';
	deckfontname[1023] = '\0';
	ununicode(deckfontname);

	GetPrivateProfileStringW(L"TH123E", L"StorySpellFont", L"WinterthurCondensed", (WCHAR *)storyspellfontname, 511, ini_path);
	storyspellfontname[1022] = '\0';
	storyspellfontname[1023] = '\0';
	ununicode(storyspellfontname);

	GetPrivateProfileStringW(L"TH123E", L"DeckDescFont", L"Bitstream Vera Sans Mono", (WCHAR *)deckdescfontname, 511, ini_path);
	deckdescfontname[1022] = '\0';
	deckdescfontname[1023] = '\0';
	ununicode(deckdescfontname);

	deck_height = GetPrivateProfileIntW(L"TH123E", L"DeckHeight", deck_height, ini_path);
	deck_width = GetPrivateProfileIntW(L"TH123E", L"DeckWidth", deck_width, ini_path);
	deckdesc_width = GetPrivateProfileIntW(L"TH123E", L"DeckDescWidth", deckdesc_width, ini_path);
	deckdesc_height = GetPrivateProfileIntW(L"TH123E", L"DeckDescHeight", deckdesc_height, ini_path);
	musicroom_width = GetPrivateProfileIntW(L"TH123E", L"MusicroomWidth", musicroom_width, ini_path);

	GetPrivateProfileStringW(L"TH123E", L"FontFile", L"monospatialmodswr.ttf", (WCHAR *)embedded_font_filename, 511, ini_path);
	embedded_font_filename[1023] = '\0';
	embedded_font_filename[1022] = '\0';

	ununicode(embedded_font_filename);

	hpcheat = GetPrivateProfileIntW(L"TH123E", L"HPCheat", 0, ini_path);
#endif

#ifdef LINUX_HACK
	// fix linux timing
	write_int(0x419109, 14);
	write_noexec_int(0x8a0ff8, 14);
	
	// make it actually run.
	write_call(0x41e408, (void *)linux_passthrough);
	//write_call(0x401426, (void *)linux_passthrough);
	
	// fix linux rendering glitch
	
	// fix winsock freeze on canceling hosting
	
	// fix crash on quit (related to textures?)
#endif

	// make sure we lock loading stuff
	hMutex = CreateMutex(0, FALSE, 0);

	// load internal files and stuff
	load_embedded_font();

	// Patch file loader
	write_call(0x40d228, (void *)read_constructor);

	// Patch window name.
	write_ptr(0x7fb6ce, patch_windowname);
	
	// override the font creation call
	write_byte(0x411c75, 0xe8);
	write_call(0x411c76, (void *)myCreateFont);
	write_byte(0x411c7a, 0x90);
	
#ifdef LINUX_HACK
	// profile editing crash bug.
	write_byte(0x40f1d6, 0xe8);
	write_call(0x40f1d7, (void *)myToAscii);
	write_byte(0x40f1db, 0x90);
#endif

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

	// font stuff, if necessary
	if (fontname[0]) {
		// Patch font name
		// - profile name probably
		write_ptr(0x434cf6, fontname);

		// - deck/game spell card text
		write_ptr(0x438533, fontname);

		// - results screen, profile screen, music room(!), replay screen
		write_ptr(0x43d7ac, fontname);

		// - popup text
		write_ptr(0x44406f, fontname);
		
		// 0x44afc7 - what is this?
		write_ptr(0x44b987, fontname);

		// - deck screen (numbers)
		write_ptr(0x450a83, fontname);

		// - ? I DON'T KNOW!
		write_ptr(0x453bfa, fontname);

		// - story mode text
		if (storyfontname[0]) {
			write_ptr(0x462926, storyfontname); // fontname);
		} else {
			write_ptr(0x462926, fontname); // fontname);
		}

		// Patch font type
		write_code(0x411c63, patch_font_type, 5);
	}


	// font sizes/etc
	// font call became 4110c0, search for that

	write_int(0x462966, 1);		// story mode font spacing (size is ok!)
	write_int(0x4385e1, 0x10);	// deck/game spell card text size
	write_int(0x438605, 0x00);	// deck/game spell card text spacing
	write_int(0x450af3, 0x10);	// deck numbers size
	write_int(0x450b3e, 0x00);	// deck numbers spacing
	write_int(0x434d58, 0x10);	// profile name size
	write_int(0x434dd5, 0x00);	// profile name spacing (do we care?)
	write_int(0x43d83e, 0x10);	// results/profile/music/replay size
	write_int(0x43d877, 0x00);	// results/profile/music/replay spacing
	write_int(0x4440fe, 0x10);	// popup text size
	write_int(0x444137, 0x00);	// popup text spacing
	write_int(0x453c70, 0x10);	// ??? size
	write_int(0x453c36, 0x00);	// ??? spacing

	// Patch starting stage
#ifndef NDEBUG
	if (start_stage >= 1 && start_stage <= 10) {
		write_byte(0x462c5a, 0xe8);
		write_call(0x462c5b, (void *)skip_to_stage);
		write_byte(0x462c5f, 0x90);
	}
#endif

#ifndef NDEBUG
	if (hpcheat) {
		const char no_hp_ax[4] = { 0x66, 0xb8, 0x01, 0x00 };
		//const char no_hp_cx[4] = { 0x66, 0xb9, 0x01, 0x00 };
		write_code(0x46be6e, no_hp_ax, 4); // hp
		write_code(0x468fda, no_hp_ax, 4); // sc hp
	}
#endif

	// misc text
	//FILE *file = fopen("errortext.txt","w");
	for (int i = 0; text_table[i].addr; ++i) {
		//const char *str = *((const char **)text_table[i].addr);
		//fprintf(logfile, "%2d : %p %p : '%s' : '%s'\n",
		//	i,
		//	(void *)text_table[i].addr,
		//	str, str, text_table[i].string);
		//fflush(logfile);
		write_ptr(text_table[i].addr, text_table[i].string);
	}
	//fclose(file);

	// overhauling the text loader to have escape codes
	write_byte(0x40f5ff,0x54); // PUSH EBP
	write_byte(0x40f600,0x55); // PUSH ESP
	write_byte(0x40f601,0xe8); // CALL blah blah
	write_call(0x40f602, (void *)text_passthrough);
	write_byte(0x40f606,0x8b); // MOV EBP,EAX
	write_byte(0x40f607,0xe8);

	// texture resizing for text
	write_byte(0x4379de, 0x10); // card list?
	write_byte(0x4381b9, 0x10); // more cards?
	write_byte(0x43da43, 0x10); // replay list
	write_byte(0x4479c2, 0x10); // used address list
	write_byte(0x447c9e, 0x10); // ?
	write_byte(0x448860, 0x10); // ?
	write_byte(0x44de5e, 0x10); // ?
	write_byte(0x450bdc, 0x10); // deck list

	// fix deck font size/type
	write_call(0x450fa3, (void *)deck_passthrough);
	write_call(0x450fb5, (void *)deck2_passthrough);
	write_call(0x44de78, (void *)deck3_passthrough);

	// fix musicroom size/type
	write_noexec_ptr(0x859564, (void *)musicroom_passthrough);

	// fix escape codes in script loader
	write_byte(0x82112a,0x52); // push edx
	write_byte(0x82112b,0x55); // push ebp
	write_byte(0x82112c,0xe8); // call
	write_call(0x82112d, (void *)script_passthrough);
	for (int i = 0x821131; i < 0x82113d; ++i) {
		write_byte(i, 0x90);
	}
	write_byte(0x82113e, 0xc0); // test al,al

	// fix spellcard get screen
	write_call(0x4497d2, (void *)spellcard_get_passthrough);

	// fix profile entry cursor
	write_byte(0x42a230, 0xbb); // move ebx, 0e
	write_int(0x42a231, 0x0e);
	write_byte(0x42a235, 0x90); // nop

	// fix spellcard display clipping
	write_int(0x4379de, 0x200); // height 0x100 -> 0x200
	write_int(0x4381b9, 0x200);
	write_byte(0x43787e, 0x05); //shl 5
	write_byte(0x438064, 0x05);

	// fix spellcard rendering alignment
	write_byte(0x436750, 0x50); // PUSH EAX
	write_byte(0x436751, 0x51); // PUSH ECX
	write_byte(0x436752, 0x50); // PUSH EAX
	write_byte(0x436753, 0xe8); // CALL ...
	write_call(0x436754, (void *)spellcard_alignment);
	write_byte(0x436758, 0x89); // MOV EDI,EAX
	write_byte(0x436759, 0xc7); //
	write_byte(0x43675a, 0x59); // POP ECX
	write_byte(0x43675b, 0x58); // POP EAX
	
	for (unsigned int i = 0x43675c; i < 0x436767; ++i) {
		write_byte(i, 0x90);
	}

	write_byte(0x436791, 0x89);
	write_byte(0x436792, 0xfd);
	write_byte(0x436793, 0x90);
	write_byte(0x436794, 0x90);
	write_byte(0x436795, 0x90);
	write_byte(0x436796, 0x90);
	write_byte(0x436797, 0x90);
	write_byte(0x43679a, 0x90);
	write_byte(0x43679b, 0x90);

	// storyspell passthrough
	write_call(0x435bde, (void *)storyspell_passthrough);

	// results passthrough
	write_call(0x44c91d, (void *)results_passthrough);

	// results screen
	write_ptr(0x44bbd8, (const char *)&results_x);
	
	// deck screen width squishing
	write_byte(0x44e83a, 0xeb);
	write_byte(0x44ed48, 0xeb);
	
	// fucking strings
	write_byte(0x452461, 0x0f); // profile copied
	write_byte(0x4523cf, 0x15); // profile copy failed
	write_byte(0x4525d2, 0x10); // profile deleted
	
	write_byte(0x452611, 0x19); // profile delete failed
	
	// deck description position change
	static const double temp_d = -6.0;
	write_ptr(0x44edc5, (const char *)&temp_d);
	
	//   we have to overwrite this entire block of code
	//   to make room for our new position.
	static const char deck_desc_pos_code[] = {
		0x8d, 0x8e, 0x98, 0x00, 0x00, 0x00,	// LEA ECX,DWORD PTR DS:[ESI+1C4]
		0xd9, 0x44, 0x24, 0x2c,			// FLD DWORD PTR SS:[ESP+2C]
		0x83, 0xec, 0x08,			// SUB ESP, 8
		0xd9, 0x5c, 0x24, 0x04,			// FSTP DWORD PTR SS:[ESP+4]
		0xd9, 0x44, 0x24, 0x38,			// FLD DWORD PTR SS:[ESP+38]
		0xd9, 0x1c, 0x24,			// FSTP DWORD PTR SS:[ESP]
		0xe8					// CALL xxxxxxxx
	};
	
	for (unsigned int i = 0x44eea3 + sizeof(deck_desc_pos_code); i < 0x44eec6; ++i) {
		write_byte(i, 0x90);
	}
	
	write_code(0x44eea3, deck_desc_pos_code, sizeof(deck_desc_pos_code));
	write_call(0x44eea3 + sizeof(deck_desc_pos_code), (void *)deck_desc_pos_passthrough);
	
	// font passthrough
	write_call(0x4129f7, (void *)font_passthrough);
	
	return TRUE;
}
};
