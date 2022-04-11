//
// Created by Gegel85 on 05/04/2022.
//

#include <SokuLib.hpp>
#include <mutex>
#include <process.h>
#include "InputBox.hpp"
#include "Menu.hpp"

#define CURSOR_ENDX 465
#define CURSOR_STARTX 174
#define CURSOR_STARTY 228
#define CURSOR_STEP 6

bool inputBoxShown = false;
static bool changed = false;
static char lastPressed = 0;
static unsigned t = 0;
static WNDPROC Original_WndProc = nullptr;
static int cursorPos = 0;
static bool started = false;
static bool loaded = false;
static bool escPressed = false;
static bool changes = false;
static SokuLib::DrawUtils::RectangleShape whiteBox;
static SokuLib::DrawUtils::RectangleShape cursor;
static SokuLib::DrawUtils::Sprite titleSprite;
static SokuLib::DrawUtils::Sprite textSprite;
static SokuLib::DrawUtils::Sprite boxSprite;
static std::function<void (const std::string &value)> onAcceptFct;
static std::vector<char> buffer;
static BYTE current[256];
static unsigned timers[256];
static std::mutex mutex;

static void updateCursor(int newVal)
{
	int diff = newVal - cursorPos;
	int newX = cursor.getPosition().x + diff * CURSOR_STEP;

	if (newX > CURSOR_ENDX) {
		textSprite.rect.left += newX - CURSOR_ENDX;
		cursor.setPosition({CURSOR_ENDX, CURSOR_STARTY});
	} else if (newX < CURSOR_STARTX) {
		textSprite.rect.left += newX - CURSOR_STARTX;
		cursor.setPosition({CURSOR_STARTX, CURSOR_STARTY});
	} else
		cursor.setPosition({newX, CURSOR_STARTY});
	cursorPos = newVal;
}

void inputBoxRender()
{
	if (!inputBoxShown)
		return;

	boxSprite.draw();
	whiteBox.draw();
	titleSprite.draw();
	textSprite.draw();
	cursor.draw();
}

static std::string sanitizeInput()
{
	std::string result{buffer.begin(), buffer.end() - 1};

	for (size_t pos = result.find('<'); pos != std::string::npos; pos = result.find('<'))
		result[pos] = '{';
	for (size_t pos = result.find('>'); pos != std::string::npos; pos = result.find('>'))
		result[pos] = '}';
	return result;
}

void inputBoxUpdate()
{
	if (!inputBoxShown)
		return;

	for (size_t i = 0; i < sizeof(current); i++) {
		int j = GetAsyncKeyState(i);

		current[i] = j >> 8 | j & 1;
		if (current[i] & 0x80)
			timers[i]++;
		else
			timers[i] = 0;
	}
	if (escPressed) {
		inputBoxShown = (current[VK_ESCAPE] & 0x80) != 0;
		return;
	}
	if (timers[VK_ESCAPE] == 1) {
		SokuLib::playSEWaveBuffer(0x29);
		escPressed = true;
		return;
	}
	mutex.lock();
	if (timers[VK_HOME] == 1) {
		SokuLib::playSEWaveBuffer(0x27);
		updateCursor(0);
	}
	if (timers[VK_END] == 1) {
		SokuLib::playSEWaveBuffer(0x27);
		updateCursor(buffer.size() - 1);
	}
	if (timers[VK_RETURN] == 1) {
		inputBoxShown = false;
		mutex.unlock();
		try {
			onAcceptFct(buffer.data());
		} catch (...) {}
		return;
	}
	if (timers[VK_BACK] == 1 || (timers[VK_BACK] > 36 && timers[VK_BACK] % 6 == 0)) {
		if (cursorPos != 0) {
			buffer.erase(buffer.begin() + cursorPos - 1);
			updateCursor(cursorPos - 1);
			changes = true;
			SokuLib::playSEWaveBuffer(0x27);
		}
	}
	if (timers[VK_DELETE] == 1 || (timers[VK_DELETE] > 36 && timers[VK_DELETE] % 6 == 0)) {
		if (cursorPos < buffer.size() - 1) {
			buffer.erase(buffer.begin() + cursorPos);
			SokuLib::playSEWaveBuffer(0x27);
			changes = true;
			textSprite.texture.createFromText(sanitizeInput().c_str(), defaultFont12, {8 * buffer.size(), 1800});
		}
	}
	if (timers[VK_LEFT] == 1 || (timers[VK_LEFT] > 36 && timers[VK_LEFT] % 3 == 0)) {
		if (cursorPos != 0) {
			updateCursor(cursorPos - 1);
			SokuLib::playSEWaveBuffer(0x27);
		}
	}
	if (timers[VK_RIGHT] == 1 || (timers[VK_RIGHT] > 36 && timers[VK_RIGHT] % 3 == 0)) {
		if (cursorPos != buffer.size() - 1) {
			updateCursor(cursorPos + 1);
			SokuLib::playSEWaveBuffer(0x27);
		}
	}
	if (lastPressed) {
		t++;
		if (t == 1 || (t > 36 && t % 6 == 0)) {
			buffer.insert(buffer.begin() + cursorPos, lastPressed);
			updateCursor(cursorPos + 1);
			SokuLib::playSEWaveBuffer(0x27);
			changes = true;
		}
	}
	if (changes)
		textSprite.texture.createFromText(sanitizeInput().c_str(), defaultFont12, {max(292, 8 * buffer.size()), 1800});
	changes = false;
	mutex.unlock();
}

LRESULT __stdcall Hooked_WndProc(const HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (uMsg == WM_KEYDOWN && inputBoxShown) {
		BYTE keyboardState[256];

		GetKeyboardState(keyboardState);
		if(!(MapVirtualKey(wParam, MAPVK_VK_TO_CHAR) >> (sizeof(UINT) * 8 - 1) & 1)) {
			unsigned short chr = 0;
			int nb = ToAscii((UINT)wParam, lParam, keyboardState, &chr, 0);

			if (nb == 1 && chr < 0x7F && chr >= 32) {
				mutex.lock();
				if (lastPressed && t == 0) {
					buffer.insert(buffer.begin() + cursorPos, lastPressed);
					updateCursor(cursorPos + 1);
					changes = true;
					SokuLib::playSEWaveBuffer(0x27);
				}
				lastPressed = chr;
				t = 0;
				mutex.unlock();
			}
		}
	} else if (uMsg == WM_KEYUP && inputBoxShown) {
		mutex.lock();
		if (lastPressed && t == 0) {
			buffer.insert(buffer.begin() + cursorPos, lastPressed);
			updateCursor(cursorPos + 1);
			SokuLib::playSEWaveBuffer(0x27);
		}
		lastPressed = 0;
		t = 0;
		mutex.unlock();
	}
	return CallWindowProc(Original_WndProc, hWnd, uMsg, wParam, lParam);
}

void inputBoxLoadAssets()
{
	if (loaded)
		return;
	if (!Original_WndProc)
		Original_WndProc = (WNDPROC)SetWindowLongPtr(SokuLib::window, GWL_WNDPROC, (LONG_PTR)Hooked_WndProc);
	loaded = true;
	boxSprite.texture.loadFromGame("data/menu/21_Base.bmp");
	boxSprite.rect.width = boxSprite.texture.getSize().x;
	boxSprite.rect.height = boxSprite.texture.getSize().y;
	boxSprite.setSize(boxSprite.texture.getSize());
	boxSprite.setPosition({160, 192});

	whiteBox.setSize({292, 18});
	whiteBox.setPosition({174, 226});
	whiteBox.setFillColor(SokuLib::Color::White);
	whiteBox.setBorderColor(SokuLib::Color::Transparent);

	cursor.setSize({1, 14});
	cursor.setPosition({CURSOR_STARTX, CURSOR_STARTY});
	cursor.setFillColor(SokuLib::Color::Black);
	cursor.setBorderColor(SokuLib::Color::Transparent);

	textSprite.fillColors[SokuLib::DrawUtils::GradiantRect::RECT_BOTTOM_LEFT_CORNER] = SokuLib::DrawUtils::DxSokuColor{0x80, 0x80, 0xFF};
	textSprite.fillColors[SokuLib::DrawUtils::GradiantRect::RECT_BOTTOM_RIGHT_CORNER]= SokuLib::DrawUtils::DxSokuColor{0x80, 0x80, 0xFF};
	textSprite.rect.width = 292;
	textSprite.rect.height = 18;
	textSprite.setSize({291, 18});
	textSprite.setPosition({174, CURSOR_STARTY});

	titleSprite.rect.width = 292;
	titleSprite.rect.height = 24;
	titleSprite.setSize({292, 24});
	titleSprite.setPosition({174, 202});
}

void inputBoxUnloadAssets()
{
	loaded = false;
	titleSprite.texture.destroy();
	textSprite.texture.destroy();
	boxSprite.texture.destroy();
}

void openInputDialog(const char *title, const char *defaultValue)
{
	SokuLib::playSEWaveBuffer(0x28);

	memset(current, 0, sizeof(current));
	memset(timers, 0, sizeof(timers));
	lastPressed = 0;
	t = 0;
	buffer.clear();
	if (defaultValue) {
		buffer.reserve(strlen(defaultValue));
		buffer.insert(buffer.begin(), defaultValue, defaultValue + strlen(defaultValue));
	}
	buffer.push_back(0);

	titleSprite.texture.createFromText(title, defaultFont12, {292, 24});
	textSprite.texture.createFromText(sanitizeInput().c_str(), defaultFont12, {max(292, 8 * buffer.size()), 20});
	textSprite.rect.left = 0;

	cursorPos = 0;
	cursor.setPosition({CURSOR_STARTX, CURSOR_STARTY});
	updateCursor(buffer.size() - 1);

	inputBoxShown = true;
	escPressed = false;
}

void setInputBoxCallbacks(const std::function<void (const std::string &value)> &onAccept)
{
	onAcceptFct = onAccept;
}
