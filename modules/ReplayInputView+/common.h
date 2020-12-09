#pragma once
#include "fields.h"

typedef unsigned int uint;
typedef unsigned char uchar;

#define SHOW_INT(expr) \
	sprintf_s(s_msg, 255, "%s = %d", #expr, (expr)); \
	SHOW_DEBUG_MSG()
#define SHOW_PTR(expr) \
	sprintf_s(s_msg, 255, "%s = %p", #expr, (expr)); \
	SHOW_DEBUG_MSG()
#define SHOW_FLOAT(expr) \
	sprintf_s(s_msg, 255, "%s = %0.2f", #expr, (expr)); \
	SHOW_DEBUG_MSG()

/* Macros. */
#define MSGBOX_IF_CALLED() MessageBox(NULL, "Did I get called?", "Debug", 0)
#define SHOW_DEBUG_MSG() MessageBox(NULL, s_msg, "Debug", 0)
#define SHOW_MSG(text) MessageBox(NULL, (text), "Debug", 0)
#define NOPS(memStart, memEnd) \
	for (char *mem = (char *)(memStart), *mend = (char *)(memEnd); mem != mend; ++mem) \
		*mem = (char)0x90;
#define DPRINTF(...) \
	{ \
		sprintf_s(s_msg, 255, __VA_ARGS__); \
		SHOW_DEBUG_MSG(); \
	}

#define ADDR_GETLVL_HEIGHT 0x4397F0

namespace text {
void SetText(const char *newText);
void OnCreate(void *This);
void OnRender(void *This);
void OnDestruct(void *This, int, int dyn);
void LoadSettings(LPCSTR profilePath, LPCSTR section);
}; // namespace text