#pragma once
#include "SokuAPI.hpp"
#include <SokuLib.h>
#include <string>
#include <vector>
using namespace std;

extern std::wstring module_path;

namespace Menu {
enum class Event { AlreadyPlaying, ConnectionFailed };

ImFont *menuFont;
vector<string> menuText;
// Uses std::function, which are a bit innefficient, but as long
// as it doesn't impact performance too much it's fine for now
vector<function<void(void)>> menuCallbacks;
vector<function<void(void)>> eventCallbacks(2);

CMenuConnect *menu;

void Init() {
	std::wstring font_path = module_path;
	font_path.append(L"\\romanan.ttf");
	char font_path_ansi[MAX_PATH];
	wcstombs(font_path_ansi, &font_path[0], MAX_PATH);
	menuFont = ImGui::AddFontFromFile(font_path_ansi, 20.0f);

	SokuAPI::HideProfiles.Toggle(true);
}

void OnMenuOpen() {
	SokuAPI::HideNativeMenu();

	menu = SokuAPI::GetCMenuConnect();
}

void Render() {
	ImGui::SetNextWindowSize(ImVec2(640, 480));
	ImGui::SetNextWindowPos(ImVec2(0, 0));
	ImGui::Begin("##menu", 0,
		ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoBringToFrontOnFocus
			| ImGuiWindowFlags_NoNavInputs | ImGuiWindowFlags_NoSavedSettings);
	ImGui::PushFont(menuFont);
	for (unsigned int i = 0; i < menuText.size(); ++i) {
		ImGui::SetCursorPos(ImVec2(64 + 2, 125 + 32 * i + 2));
		ImGui::TextColored((ImVec4)ImColor(0, 0, 0, 80), menuText[i].c_str());
		ImGui::SetCursorPos(ImVec2(64, 125 + 32 * i));
		ImGui::Text(menuText[i].c_str());
	}
	ImGui::PopFont();
	ImGui::End();
}

bool AddItem(string text) {
	if (menuText.size() >= 6)
		return false;

	menuText.push_back(text);
	return true;
}

bool AddItem(string text, function<void(void)> func) {
	if (!AddItem(text))
		return false;

	menuCallbacks.push_back(func);
	return true;
}

void AddEventHandler(Event e, function<void(void)> func) {
	eventCallbacks[int(e)] = func;
}

void HandleInput() {
	if (menu->Choice > 0) {
		if (menu->Choice < menuCallbacks.size() + 1 && menu->Subchoice == 0) {
			menuCallbacks[menu->Choice - 1]();
		} else if (menu->Subchoice == 5) {
			eventCallbacks[int(Event::AlreadyPlaying)]();
		} else if (menu->Subchoice == 10) {
			eventCallbacks[int(Event::ConnectionFailed)]();
		}
	}
}
}; // namespace Menu