#pragma once
#include <SokuLib.h>
#include <fstream>
#include <iostream>
#include <json.hpp>
#include <sstream>
using namespace std;
using namespace json;

extern std::wstring module_path;

namespace HostingOptions {
bool enabled = false;
int port;
bool spectate = true;
bool publicHost = true;
char message[256] = {0};

void SaveConfig() {
	std::wstring config_path = module_path;
	config_path.append(L"\\InGameHostlist.ini");

	WritePrivateProfileStringW(L"InGameHostlist", L"Port", &std::to_wstring(port)[0], &config_path[0]);
	WritePrivateProfileStringW(L"InGameHostlist", L"Spectatable", spectate ? L"1" : L"0", &config_path[0]);
	WritePrivateProfileStringW(L"InGameHostlist", L"Hostlist", publicHost ? L"1" : L"0", &config_path[0]);
	WritePrivateProfileStringA("InGameHostlist", "Message", message, &std::string(config_path.begin(), config_path.end())[0]);
}

void LoadConfig() {
	std::wstring config_path = module_path;
	config_path.append(L"\\InGameHostlist.ini");

	port = GetPrivateProfileIntW(L"InGameHostlist", L"Port", 10800, &config_path[0]);
	spectate = GetPrivateProfileIntW(L"InGameHostlist", L"Spectatable", 1, &config_path[0]) != 0;
	publicHost = GetPrivateProfileIntW(L"InGameHostlist", L"Hostlist", 1, &config_path[0]) != 0;
	GetPrivateProfileStringA("InGameHostlist", "Message", "", message, sizeof(message), &std::string(config_path.begin(), config_path.end())[0]);
}

void Render() {
	if (enabled) {
		ImGui::SetNextWindowSize(ImVec2(150, 190));
		ImGui::SetNextWindowPos(ImVec2(231, 158));
		ImGui::Begin("Hosting Options##HostingOptions", &enabled, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings);
		ImGui::SetCursorPosY(30);
		ImGui::AlignTextToFramePadding();
		ImGui::Text("Port");
		ImGui::SameLine();
		ImGui::InputInt("##port", &port, 0, 0);
		ImGui::AlignTextToFramePadding();
		ImGui::Text("Spectate");
		ImGui::SameLine();
		ImGui::Checkbox("##spec", &spectate);
		ImGui::AlignTextToFramePadding();
		ImGui::Text("Public");
		ImGui::SameLine();
		ImGui::Checkbox("##public", &publicHost);
		ImGui::Text("Host message:");
		ImGui::PushItemWidth(-1);
		ImGui::InputText("##msg", message, 255);
		if (ImGui::Button("Save")) {
			SaveConfig();
			enabled = false;
		}
		ImGui::PopItemWidth();
		ImGui::End();
	}
}
}; // namespace HostingOptions
