//
// Created by PinkySmile on 02/09/2022.
//

#include <direct.h>
#include <directx/dinput.h>
#include "SWRSToys.h"
#include "ModConfigMenu.hpp"
#include "InputBox.hpp"

#define scrollSize 15
#define scrollSizeEx 15

ModConfigMenu::ModConfigMenu()
{
	inputBoxLoadAssets();
#ifndef _NOTEX
	this->_title.texture.loadFromResource(myModule, MAKEINTRESOURCE(4));
	this->_title.setSize(this->_title.texture.getSize());
	this->_title.rect.width = this->_title.texture.getSize().x;
	this->_title.rect.height = this->_title.texture.getSize().y;
#endif

	this->_enabled[0].texture.createFromText("Disabled", font, {300, 20});
	this->_enabled[0].setSize({200, 20});
	this->_enabled[0].rect.width = 200;
	this->_enabled[0].rect.height = 20;
	this->_enabled[0].tint = SokuLib::Color::Red;
	this->_enabled[0].setPosition({218, 96});

	this->_enabled[1].texture.createFromText("Enabled", font, {300, 20});
	this->_enabled[1].setSize({200, 20});
	this->_enabled[1].rect.width = 200;
	this->_enabled[1].rect.height = 20;
	this->_enabled[1].setPosition({218, 96});

	this->_path[0].texture.createFromText("Path", font, {300, 20});
	this->_path[0].setSize({200, 20});
	this->_path[0].rect.width = 200;
	this->_path[0].rect.height = 20;
	this->_path[0].setPosition({218, 120});

	this->_priority[0].texture.createFromText("Priority", font, {300, 20});
	this->_priority[0].setSize({200, 20});
	this->_priority[0].rect.width = 200;
	this->_priority[0].rect.height = 20;
	this->_priority[0].setPosition({218, 120});

	this->_text.emplace_back();
	this->_text.back().texture.createFromText("Add new module", font, {300, 20});
	this->_text.back().setSize({200, 20});
	this->_text.back().rect.width = 200;
	this->_text.back().rect.height = 20;
	this->_text.back().setPosition({38, 102});

	for (auto &module : modules) {
		std::string str;
		auto name = module.getName();
		auto pos = name.find_last_of('.');

		if (pos != std::string::npos)
			name = name.substr(0, pos);
		str.reserve(name.size());
		for (auto &c : name)
			str += c > 127 ? '?' : c;
		this->_text.emplace_back();
		this->_text.back().texture.createFromText(str.c_str(), font, {300, 20});
		this->_text.back().setSize({200, 20});
		this->_text.back().rect.width = 200;
		this->_text.back().rect.height = 20;
		if (module.enabled)
			this->_text.back().tint = module.isLoaded() ? SokuLib::Color::Green : SokuLib::Color{0xFF, 0x80, 0x00, 0xFF};
		else
			this->_text.back().tint = module.isLoaded() ? SokuLib::Color{0xFF, 0x80, 0x00, 0xFF} : SokuLib::Color::Red;
	}

	this->_lastError.tint = SokuLib::Color::Red;
}

ModConfigMenu::~ModConfigMenu()
{
	inputBoxUnloadAssets();
	//TODO: Apply the changes
}

void ModConfigMenu::_()
{
	*(int *)0x882a94 = 0x16;
}

void ModConfigMenu::prepareRightPanel()
{
	if (this->_selected == 0)
		return;

	std::string s;
	std::string l;
	auto &module = modules[this->_selected - 1];
	auto path = module.getPath();
	auto err = module.getLastError();

	s.reserve(path.size() + path.size() / 57);
	for (unsigned i = 0, j = 0; i < path.size(); i++) {
		s += path[i] == '\\' ? '/' : path[i] > 127 ? '?' : path[i];
		j++;
		if (j && j % 57 == 0)
			s += "<br>";
	}
	l.reserve(err.size() + err.size() / 57);
	for (unsigned i = 0, j = 0; i < err.size(); i++) {
		if (err[i] < 32 || (err[i] == ' ' && j == 0))
			continue;
		j++;
		l += err[i] == '\\' ? '/' : err[i] > 127 ? '?' : err[i];
		if (i >= 4 && err.substr(i - 4, 4) == L"<br>")
			j = 0;
		if (j && j % 57 == 0)
			l += "<br>";
	}

	this->_lastError.texture.createFromText(l.c_str(), font, {400, 200}, &this->_lastErrorSize);
	this->_lastError.setSize({356, 200});
	this->_lastError.rect.width = 356;
	this->_lastError.rect.height = 200;
	this->_lastError.setPosition({284, 96});

	if (this->_lastErrorSize.y == 0)
		this->_lastErrorSize.y = 12;

	this->_path[0].setPosition({218, 106 + this->_lastErrorSize.y});
	this->_path[1].texture.createFromText(s.c_str(), font, {400, 200}, &this->_pathSize);
	this->_path[1].setSize({356, 200});
	this->_path[1].rect.width = 356;
	this->_path[1].rect.height = 200;
	this->_path[1].setPosition({284, 106 + this->_lastErrorSize.y});

	this->_priority[0].setPosition({218, 116 + this->_lastErrorSize.y + this->_pathSize.y});
	if (module.customPriority)
		this->_priority[1].texture.createFromText((std::to_string(module.priority) + " (Custom priority)").c_str(), font, {400, 200}, nullptr);
	else if (!module.isLoaded())
		this->_priority[1].texture.createFromText("Unknown because the module is not loaded", font, {400, 200}, nullptr);
	else if (module.getPriority)
		this->_priority[1].texture.createFromText((std::to_string(module.priority) + " (Defined by the module)").c_str(), font, {400, 200}, nullptr);
	else
		this->_priority[1].texture.createFromText((std::to_string(module.priority) + " (Default value)").c_str(), font, {400, 200}, nullptr);
	this->_priority[1].setSize({356, 200});
	this->_priority[1].rect.width = 356;
	this->_priority[1].rect.height = 200;
	this->_priority[1].setPosition({284, 116 + this->_lastErrorSize.y + this->_pathSize.y});
}

void ModConfigMenu::renderRightPanel()
{
	if (this->_selected == 0)
		return;

	auto &module = modules[this->_selected - 1];

	this->_path[0].draw();
	this->_path[1].draw();

	this->_priority[0].draw();
	if (this->_expended && this->_exselected == 1)
		displaySokuCursor(
			{284, 114 + this->_lastErrorSize.y + this->_pathSize.y},
			{200, 16}
		);
	this->_priority[1].draw();

	if (module.enabled)
		this->_enabled[1].tint = module.isLoaded() ? SokuLib::Color::Green : SokuLib::Color{0xFF, 0x80, 0x00, 0xFF};
	else
		this->_enabled[0].tint = module.isLoaded() ? SokuLib::Color{0xFF, 0x80, 0x00, 0xFF} : SokuLib::Color::Red;
	if (this->_expended && this->_exselected == 0)
		displaySokuCursor(
			{218, 94},
			{80, 16}
		);
	this->_enabled[module.enabled].draw();
	this->_lastError.draw();
}

void ModConfigMenu::addNewModule()
{
	OPENFILENAMEW ofn;         // common dialog box structure
	wchar_t szFile[MAX_PATH];  // buffer for file name
	wchar_t szDir[MAX_PATH];   // buffer for dir name
	char szCWD[MAX_PATH];      // buffer for dir name
	bool result;

	// Initialize OPENFILENAME
	getcwd(szCWD, sizeof(szCWD));
	memset(&ofn, 0, sizeof(ofn));
	memset(&szFile, 0, sizeof(szFile));
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = SokuLib::window;
	ofn.lpstrFile = szFile;
	ofn.nMaxFile = sizeof(szFile) / sizeof(*szFile);
	ofn.lpstrFilter =
		L"Hisoutensoku module\0*.DLL\0"
		"Module archive\0*.ZIP\0"
		"All files\0*.*\0\0";
	ofn.nFilterIndex = 1;
	ofn.lpstrFileTitle = nullptr;
	ofn.nMaxFileTitle = 0;
	GetFullPathNameW(L"./Modules", sizeof(szDir) / sizeof(*szDir), szDir, nullptr);
	ofn.lpstrInitialDir = szDir;
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
	ofn.lpstrTitle = L"Open module";

	// Display the Open dialog box.
	result = GetOpenFileNameW(&ofn);
	chdir(szCWD);
	if (!result)
		return SokuLib::playSEWaveBuffer(0x29);

	auto ptr = wcsrchr(ofn.lpstrFile, '.');

	for (int i = 0; ofn.lpstrFile[i]; i++)
		if (ofn.lpstrFile[i] == '\\')
			ofn.lpstrFile[i] = '/';
	for (int i = 0; ptr && ptr[i]; i++)
		if (ptr[i] < 0x80)
			ptr[i] = tolower(ptr[i]);
	if (ptr && wcscmp(ptr, L".zip") == 0) {
		MessageBoxA(nullptr, "Archive files are not supported yet.", "Error", MB_ICONERROR);
		return SokuLib::playSEWaveBuffer(0x29);
	}
	modules.emplace_back(ofn.lpstrFile, gameHash, myModule);

	auto &module = modules.back();
	std::string str;
	auto name = module.getName();
	auto pos = name.find_last_of('.');

	if (pos != std::string::npos)
		name = name.substr(0, pos);
	str.reserve(name.size());
	for (auto &c : name)
		str += c > 127 ? '?' : c;
	this->_text.emplace_back();
	this->_text.back().texture.createFromText(str.c_str(), font, {300, 20});
	this->_text.back().setSize({200, 20});
	this->_text.back().rect.width = 200;
	this->_text.back().rect.height = 20;
	this->_text.back().tint = module.enabled ? SokuLib::Color::Green : SokuLib::Color::Red;
	this->_selected = this->_text.size() - 1;
	if (this->_selected >= this->_scrollStart + scrollSize)
		this->_scrollStart = this->_selected - scrollSize;
	this->prepareRightPanel();
	SokuLib::playSEWaveBuffer(0x28);
	saveSettings();
}


int ModConfigMenu::onProcess()
{
	if (inputBoxShown)
		return inputBoxUpdate(), true;
	if (SokuLib::checkKeyOneshot(DIK_ESCAPE, 0, 0, 0)) {
		SokuLib::playSEWaveBuffer(0x29);
		inputBoxShown = false;
		return false;
	}
	if (!this->_expended)
		return this->_normalProcess();
	this->_expended = this->_expendedProcess();
	return true;
}

int ModConfigMenu::onRender()
{
#ifndef _NOTEX
	this->_title.draw();
#endif
	//Display the green gradiant cursor bar
	displaySokuCursor(
		{38 + (this->_selected != 0) * 10, 101 + (this->_selected - this->_scrollStart) * 20},
		{200, 16}
	);

	auto it = this->_text.begin();

	it->setPosition({38, 102});
	it->draw();
	for (int i = 0; i <= this->_scrollStart; i++)
		it++;
	for (int i = 0; it != this->_text.end() && i < scrollSize; i++, it++) {
		it->setPosition({48, 122 + i * 20});
		it->draw();
	}
	renderRightPanel();
	inputBoxRender();
	return 0;
}

bool ModConfigMenu::_normalProcess()
{
	if (SokuLib::inputMgrs.input.b == 1) {
		SokuLib::playSEWaveBuffer(0x29);
		return false;
	}
	if (SokuLib::inputMgrs.input.a == 1) {
		SokuLib::playSEWaveBuffer(0x28);
		if (this->_selected == 0)
			this->addNewModule();
		else
			this->_expended = true;
	}
	if (SokuLib::inputMgrs.input.verticalAxis == -1 || (SokuLib::inputMgrs.input.verticalAxis <= -36 && SokuLib::inputMgrs.input.verticalAxis % 6 == 0)) {
		SokuLib::playSEWaveBuffer(0x27);
		this->_selected--;
		this->_selected += this->_text.size();
		this->_selected %= this->_text.size();
		if (this->_selected <= this->_scrollStart)
			this->_scrollStart = this->_selected ? this->_selected - 1 : 0;
		if (this->_selected >= this->_scrollStart + scrollSize)
			this->_scrollStart = this->_selected - scrollSize;
		this->prepareRightPanel();
	} else if (SokuLib::inputMgrs.input.verticalAxis == 1 || (SokuLib::inputMgrs.input.verticalAxis >= 36 && SokuLib::inputMgrs.input.verticalAxis % 6 == 0)) {
		SokuLib::playSEWaveBuffer(0x27);
		this->_selected++;
		this->_selected %= this->_text.size();
		if (this->_selected < this->_scrollStart)
			this->_scrollStart = this->_selected;
		if (this->_selected >= this->_scrollStart + scrollSize)
			this->_scrollStart = this->_selected - scrollSize;
		this->prepareRightPanel();
	}
	return true;
}

bool ModConfigMenu::_expendedProcess()
{
	if (SokuLib::inputMgrs.input.b == 1) {
		SokuLib::playSEWaveBuffer(0x29);
		return false;
	}
	if (SokuLib::inputMgrs.input.a == 1) {
		auto &module = modules[this->_selected - 1];

		SokuLib::playSEWaveBuffer(0x28);
		if (this->_exselected == 0) {
			module.enabled = !module.enabled;
			module.lastError.clear();
			if (module.enabled && !module.isInit())
				module.lastError = L"Enabling on the fly is not yet supported.<br>Restart your game to apply changes.";
			else if (!module.enabled)
				module.lastError = L"Disabling on the fly is not yet supported.<br>Restart your game to apply changes.";
			this->prepareRightPanel();
			saveSettings();
			if (module.enabled)
				this->_text[this->_selected].tint = module.isLoaded() ? SokuLib::Color::Green : SokuLib::Color{0xFF, 0x80, 0x00, 0xFF};
			else
				this->_text[this->_selected].tint = module.isLoaded() ? SokuLib::Color{0xFF, 0x80, 0x00, 0xFF} : SokuLib::Color::Red;
		} else if (this->_exselected == 1) {
			setInputBoxCallbacks([&module, this](const std::string &val){
				SokuLib::playSEWaveBuffer(0x28);
				module.lastError = L"Changing priority on the fly is not yet supported.<br>Restart your game to apply changes.";
				module.priority = std::stoi(val);
				module.customPriority = true;
				this->prepareRightPanel();
				saveSettings();
			});
			openInputDialog("Enter new priority", std::to_string(module.priority).c_str());
		}
	}
	if (SokuLib::inputMgrs.input.c == 1) {
		auto &module = modules[this->_selected - 1];

		if (this->_exselected == 1) {
			SokuLib::playSEWaveBuffer(0x28);
			module.setDefaultPriority();
			this->prepareRightPanel();
			saveSettings();
			module.lastError = L"Changing priority on the fly is not yet supported.<br>Restart your game to apply changes.";
		}
	}
	if (SokuLib::inputMgrs.input.verticalAxis == -1 || (SokuLib::inputMgrs.input.verticalAxis <= -36 && SokuLib::inputMgrs.input.verticalAxis % 6 == 0)) {
		SokuLib::playSEWaveBuffer(0x27);
		this->_exselected--;
		this->_exselected += 2;
		this->_exselected %= 2;
		if (this->_exselected <= this->_exscrollStart)
			this->_exscrollStart = this->_exselected ? this->_exselected - 1 : 0;
		if (this->_exselected >= this->_exscrollStart + scrollSizeEx)
			this->_exscrollStart = this->_exselected - scrollSizeEx;
	} else if (SokuLib::inputMgrs.input.verticalAxis == 1 || (SokuLib::inputMgrs.input.verticalAxis >= 36 && SokuLib::inputMgrs.input.verticalAxis % 6 == 0)) {
		SokuLib::playSEWaveBuffer(0x27);
		this->_exselected++;
		this->_exselected %= 2;
		if (this->_exselected < this->_exscrollStart)
			this->_exscrollStart = this->_exselected;
		if (this->_exselected >= this->_exscrollStart + scrollSizeEx)
			this->_exscrollStart = this->_exselected - scrollSizeEx;
	}
	return true;
}
