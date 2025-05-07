#include <windows.h>
#include "List.h"
#include "common.h"
#include "fields.h"
#include <d3d9.h>
#include <dinput.h>
#include <shlwapi.h>

#define SWRS_USES_HASH
#include "swrs.h"

// Vertex
struct SWRVERTEX {
	float x, y, z;
	float rhw;
	D3DCOLOR color;
	float u, v;
};

#define FVF_SWRVERTEX (D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_TEX1)

// POINTのfloat版
struct Point {
	float x, y;

	Point(): x(0.f), y(0.f) {}
	Point(float _x, float _y): x(_x), y(_y) {}
};

// 自分用メンバ
struct CBattleManager_MyMember {
	bool enabled;
	int p1TexID;
	int p2TexID;
	int p1BkTexID;
	int p2BkTexID;
};

/*static void SplitString(const std::string &s, char delim, std::vector<std::string> &result) {
	std::stringstream ss(s);
	std::string item;

	result.clear();

	while (std::getline(ss, item, delim)) {
		result.push_back(item);
	}
}*/

static char *_strndup(const char *s, size_t n) {
	size_t len = strlen(s);
	if (n < len)
		len = n;

	char *r = new char[len + 1];

	for (unsigned int i = 0; i < len; ++i) {
		r[i] = s[i];
	}

	r[len] = '\0';

	return r;
}

static list::Node<const char *> *SplitString(const char *s, const char *delim) {
	list::Node<const char *> *r = NULL;
	auto len_delim = strlen(delim);

	while (*s) {
		auto p = strstr(s, delim);

		if (!p) {
			return list::cons<const char *>(_strndup(s, strlen(s)), r);
		}

		r = list::cons<const char *>(_strndup(s, p - s), r);
		s = p + len_delim;
	}

	return r;
}

static void DrawSprite(int texid, float x, float y, float cx, float cy) {
	const SWRVERTEX vertices[] = {
		{x, y, 0.0f, 1.0f, 0xffffffff, 0.0f, 0.0f},
		{x + cx, y, 0.0f, 1.0f, 0xffffffff, 1.0f, 0.0f},
		{x + cx, y + cy, 0.0f, 1.0f, 0xffffffff, 1.0f, 1.0f},
		{x, y + cy, 0.0f, 1.0f, 0xffffffff, 0.0f, 1.0f},
	};

	CTextureManager_SetTexture(g_textureMgr, texid, 0);
	g_pd3dDev->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, 2, vertices, sizeof(SWRVERTEX));
}

class TextUnit {
	Point _position;
	Point _offset;
	bool _backVisible;
	const char *_text;
	list::List<const char *> _lines;
	list::List<int> _textures;

public:
	TextUnit(): _backVisible(false), _text(NULL) {}

	void ClearTextures() {
		for (auto e = _textures.head(); e; e = e->next) {
			CTextureManager_Remove(g_textureMgr, e->data);
		}

		_textures.clear();
	}

	void SetText(const char *str, const SWRFONTDESC &font, const char *fontData) {
		_text = str;
		_lines = SplitString(_text, "\n");

		ClearTextures();
		int tex;

		for (auto e = _lines.head(); e; e = e->next) {
			CTextureManager_CreateTextTexture(g_textureMgr, &tex, e->data, (void *)fontData, 0x400, font.Height + 18, NULL, NULL);

			delete e->data;
			_textures.push(tex);
		}
	}

	void Draw(const SWRFONTDESC &font) {
		int i = 0;

		for (auto e = _textures.head(); e; e = e->next) {
			DrawSprite(e->data, _position.x + _offset.x, _position.y + _offset.y + font.Height * i++, 1024.0f, font.Height + 18.0f);
		}
		ClearTextures();
	}

	void LoadSettings(LPCSTR profilePath, LPCSTR sectionName, int defaultX, int defaultY, const SWRFONTDESC &font) {
		// 表示位置
		_position.x = (float)(int)::GetPrivateProfileInt(sectionName, "Position.X", defaultX, profilePath);
		_position.y = (float)(int)::GetPrivateProfileInt(sectionName, "Position.Y", defaultY, profilePath);
		// 文字列表示オフセット
		_offset.x = (float)(int)::GetPrivateProfileInt(sectionName, "Offset.X", 0, profilePath);
		_offset.y = (float)(int)::GetPrivateProfileInt(sectionName, "Offset.Y", 25 - font.Height, profilePath);
		// 背景属性
		_backVisible = ::GetPrivateProfileInt(sectionName, "Back.Visible", 1, profilePath) != 0;
	}
};

class ModuleState {
	TextUnit _text;
	SWRFONTDESC _font;
	char _fontData[0x1A4];

public:
	void OnCreate(void *_this) {
		SWRFont_Create(_fontData);
		SWRFont_SetIndirect(_fontData, &_font);
	}

	void OnDestruct(void *_this, int, int dyn) {
		_text.ClearTextures();
		SWRFont_Destruct(_fontData);
	}
	void OnRender(void *_this) {
		_text.Draw(_font);
	}

	void SetText(const char *str) {
		_text.SetText(str, _font, _fontData);
	}

	void LoadSettings(LPCSTR profilePath, LPCSTR section) {
		// フォント名
		::GetPrivateProfileString(section, "Font.Face", (char *)0x69F154, _font.FaceName, _countof(_font.FaceName), profilePath);
		// 色
		_font.R1 = ::GetPrivateProfileInt(section, "Font.Color1.R", 255, profilePath);
		_font.R2 = ::GetPrivateProfileInt(section, "Font.Color2.R", 255, profilePath);
		_font.G1 = ::GetPrivateProfileInt(section, "Font.Color1.G", 255, profilePath);
		_font.G2 = ::GetPrivateProfileInt(section, "Font.Color2.G", 255, profilePath);
		_font.B1 = ::GetPrivateProfileInt(section, "Font.Color1.B", 255, profilePath);
		_font.B2 = ::GetPrivateProfileInt(section, "Font.Color2.B", 255, profilePath);
		// フォント属性
		_font.Height = ::GetPrivateProfileInt(section, "Font.Height", 14, profilePath);
		_font.Weight = ::GetPrivateProfileInt(section, "Font.Bold", 0, profilePath) ? FW_BOLD : FW_NORMAL;
		_font.Italic = ::GetPrivateProfileInt(section, "Font.Italic", 0, profilePath) ? 1 : 0;
		// 文字修飾、その他
		_font.Shadow = ::GetPrivateProfileInt(section, "Font.Shadow", 1, profilePath) ? 1 : 0;
		_font.BufferSize = 1000000; // unknown buffer size
		_font.CharSpaceX = 0;

		_text.LoadSettings(profilePath, section, 0, 0, _font);
	}
};

static ModuleState self;

namespace text {
void SetText(const char *str) {
	self.SetText(str);
}

void OnCreate(void *_this) {
	self.OnCreate(_this);
}

void OnRender(void *_this) {
	self.OnRender(_this);
}

void OnDestruct(void *_this, int _, int dyn) {
	self.OnDestruct(_this, _, dyn);
}

void LoadSettings(LPCSTR profilePath, LPCSTR section) {
	self.LoadSettings(profilePath, section);
}
} // namespace text
