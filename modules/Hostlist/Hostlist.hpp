#pragma once
#include "Host.hpp"
#include "SokuAPI.hpp"
#include "Status.hpp"
#include <SokuLib.h>
#include <string>
#include <vector>
using namespace std;
using namespace Soku;

#define SHORT_WAITTIME 3000
#define LONG_WAITTIME 30000

#define PING_DELAY 3000

extern std::wstring module_path;
extern LARGE_INTEGER timer_frequency;

namespace Hostlist {
enum { WAITING, PLAYING, PAGE_COUNT };
vector<Host *> hosts[PAGE_COUNT];

CInputManagerCluster *InputManager;

bool joining = false;

Image *backgroundImg;
bool active = false;

int page_id = WAITING;
unsigned int ip_id = 0;

unsigned long long oldTime = 0;
unsigned long long newTime = 0;
unsigned long long delayTime = 0;

bool offline = false;
int refreshCount = 0;

const ImColor colorNormal = ImColor(255, 255, 255, 255);
const ImColor colorGrayedOut = ImColor(180, 180, 180, 255);
const ImColor colorError = ImColor(255, 0, 0, 255);

void Init() {
	std::wstring image_path = module_path;
	image_path.append(L"\\hostlistBG.png");

	backgroundImg = new Image();
	CreateImageFromFile(image_path, backgroundImg);

	Status::Init();
}

void OnMenuOpen() {
	active = false;
	joining = false;

	Status::OnMenuOpen();

	InputManager = SokuAPI::GetInputManager();
}

void Update() {
	LARGE_INTEGER counter;
	QueryPerformanceCounter(&counter);
	newTime = counter.QuadPart * 1000 / timer_frequency.QuadPart;

	Status::Update(newTime);

	// Get new hosts
	if (!offline) {
		if (newTime - oldTime >= delayTime) {
			try {
				string s = WebHandler::Request("http://delthas.fr:14762/games");
				JSON res = JSON::Load(s);

				for (unsigned int page = 0; page < PAGE_COUNT; ++page) {
					for (unsigned int i = 0; i < hosts[page].size(); ++i) {
						delete hosts[page][i];
					}
					hosts[page].clear();
				}

				for (int i = 0; i < res.length(); ++i) {
					Host *newHost = new Host(res[i]);
					if (newHost->playing)
						hosts[PLAYING].push_back(newHost);
					else
						hosts[WAITING].push_back(newHost);
				}

				if (active) {
					// Kinda hack-y but it works, WAITING is 0, PLAYING is 1
					// so ! switches between the two, will have to be reworked later
					if (hosts[page_id].size() == 0 && hosts[!page_id].size() != 0) {
						page_id = !page_id;
					}
					if (ip_id >= hosts[page_id].size()) {
						ip_id = hosts[page_id].size() - 1;
					}
				}

				Status::Normal("Hostlist updated.");
				delayTime = SHORT_WAITTIME;
			} catch (const char *e) {
				Status::Error(e);
				delayTime = LONG_WAITTIME;
			} catch (int e) {
				if (e == 429)
					Status::Error("Rate limit reached");
				else
					Status::Error("Error code: " + to_string(e));
				delayTime = LONG_WAITTIME;
			}

			oldTime = newTime;
			refreshCount++;
		}
	}
}

void Render() {
	if (SokuAPI::GetCMenuConnect()->Choice != 6) {
		ImGui::SetNextWindowPos(ImVec2(300, 85));
		ImGui::SetNextWindowSize(ImVec2(310, 356));
		ImGui::Begin("HostList##Hosts", 0,
			ImGuiWindowFlags_NoMouseInputs | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove
				| ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavInputs | ImGuiWindowFlags_NoNav
				| ImGuiWindowFlags_NoNavFocus);
		ImGui::PushStyleColor(ImGuiCol_FrameBg, (ImVec4)ImColor(0, 0, 0, 0));
		ImGui::PushStyleColor(ImGuiCol_Header, (ImVec4)ImColor(0, 200, 0, 200));
		ImGui::PushStyleColor(ImGuiCol_HeaderHovered, (ImVec4)ImColor(0, 200, 0, 100));
		ImGui::PushStyleColor(ImGuiCol_HeaderActive, (ImVec4)ImColor(0, 100, 0, 100));
		ImGui::PushStyleColor(ImGuiCol_Separator, (ImVec4)ImColor(255, 255, 255, 150));

		ImGui::SetCursorPos(ImVec2(5, 5));
		ImGui::Image(backgroundImg->Texture(), ImVec2(300, 350));
		ImGui::SetCursorPos(ImVec2(6, 5));

		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(4.0f, 4.0f));

		ImGui::PushItemWidth(-1);
		ImGui::ListBoxHeader("##hosts_listbox", ImVec2(0, -20));
		ImGui::SetCursorPosX(117);
		ImGui::Text("Hostlist");
		ImGui::Separator();

		ImGui::SetCursorPosX(60);
		ImGui::TextColored((page_id == WAITING) ? colorNormal : colorGrayedOut, "Waiting(%d)", hosts[WAITING].size());
		ImGui::SameLine();
		ImGui::Text(" / ");
		ImGui::SameLine();
		ImGui::TextColored((page_id == PLAYING) ? colorNormal : colorGrayedOut, "Playing(%d)", hosts[PLAYING].size());

		if (page_id == WAITING) {
			ImGui::Columns(3, "waiting");
			ImGui::SetColumnWidth(0, 100);
			ImGui::SetColumnWidth(1, 153);
			ImGui::SetColumnWidth(2, 32);
			ImGui::Separator();
			ImGui::Text("Name");
			ImGui::NextColumn();
			ImGui::Text("Message");
			ImGui::NextColumn();
			ImGui::Text("Ping");
			ImGui::NextColumn();

			if (hosts[WAITING].size() > 0) {
				ImGui::Separator(); // Necessary to avoid graphical glitches

				unsigned int start_pos = ip_id - (ip_id % 15);
				for (unsigned int i = start_pos; i < start_pos + 15; i++) {
					if (i < hosts[WAITING].size()) {
						ImGui::PushID(i);

						ImGui::Selectable(hosts[WAITING][i]->name.c_str(), i == ip_id && active, ImGuiSelectableFlags_SpanAllColumns);
						ImGui::NextColumn();

						ImGui::Text("%s", hosts[WAITING][i]->message.c_str());
						ImGui::NextColumn();

						long ping = PingMan::Ping(*hosts[WAITING][i]);
						if (ping != PingMan::PING_UNINITIALIZED)
							ImGui::TextColored(((ping >= 0) ? colorNormal : colorError), "%ld", ping);
						else
							ImGui::Text("...");
						ImGui::NextColumn();

						ImGui::PopID();
					}
				}
			}
		}
		// Pretty much a duplicate, I should find a way to streamline it
		else if (page_id == PLAYING) {
			ImGui::Columns(3, "playing");
			ImGui::SetColumnWidth(0, 125);
			ImGui::SetColumnWidth(1, 128);
			ImGui::SetColumnWidth(2, 32);
			ImGui::Separator();
			ImGui::Text("Player 1");
			ImGui::NextColumn();
			ImGui::Text("Player 2");
			ImGui::NextColumn();
			ImGui::Text("Ping");
			ImGui::NextColumn();

			if (hosts[PLAYING].size() > 0) {
				ImGui::Separator(); // Necessary to avoid graphical glitches

				unsigned int start_pos = ip_id - (ip_id % 15);
				for (unsigned int i = start_pos; i < start_pos + 15; i++) {
					if (i < hosts[PLAYING].size()) {
						ImGui::PushID(i);

						ImGui::Selectable(hosts[PLAYING][i]->name.c_str(), i == ip_id && active, ImGuiSelectableFlags_SpanAllColumns);
						ImGui::NextColumn();

						ImGui::Text("%s", hosts[PLAYING][i]->opponentName.c_str());
						ImGui::NextColumn();

						long ping = PingMan::Ping(*hosts[PLAYING][i]);
						if (ping != PingMan::PING_UNINITIALIZED)
							ImGui::TextColored(((ping >= 0) ? colorNormal : colorError), "%ld", ping);
						else
							ImGui::Text("...");
						ImGui::NextColumn();

						ImGui::PopID();
					}
				}
			}
		}

		if (hosts[page_id].size() == 0) {
			ImGui::Columns(1);
			ImGui::Separator();

			ImGui::Text("There are currently no waiting hosts.");
		}
		// Move the cursor to the end of the listbox
		// so that the column borders extend all the way down
		ImGui::SetCursorPosY(326);

		ImGui::Columns(1);
		ImGui::Separator();

		ImGui::ListBoxFooter();
		ImGui::PopItemWidth();
		ImGui::PopStyleColor(5);
		ImGui::PopStyleVar();

		ImGui::SetCursorPos(ImVec2(8, 335));
		Status::Render();

		ImGui::End();
	}
}

void HandleInput() {
	if (active) {
		// Normal menu inputs
		if (!joining) {
			if (InputManager->P1.Yaxis == 1) {
				ip_id += 1;
				if (ip_id >= hosts[page_id].size())
					ip_id = 0;
				InputManager->P1.Yaxis = 10;

				SokuAPI::SfxPlay(SFX_MOVE);
			} else if (InputManager->P1.Yaxis == -1) {
				ip_id -= 1;
				if (ip_id < 0)
					ip_id = hosts[page_id].size() - 1;
				InputManager->P1.Yaxis = -10;

				SokuAPI::SfxPlay(SFX_MOVE);
			}

			if (InputManager->P1.Xaxis == 1 || InputManager->P1.Xaxis == -1) {
				page_id = !page_id;
				ip_id = 0;
				InputManager->P1.Xaxis *= 10;

				SokuAPI::SfxPlay(SFX_MOVE);
			}

			if (InputManager->P1.A == 1 && hosts[page_id].size() != 0 && ip_id < hosts[page_id].size()) {
				SokuAPI::JoinHost(hosts[page_id][ip_id]->ip.c_str(), hosts[page_id][ip_id]->port, (page_id == PLAYING ? true : false));
				InputManager->P1.A = 10;
				joining = true;
				Status::Normal("Joining...", Status::forever);
				SokuAPI::SfxPlay(SFX_SELECT);
			}
		}

		// Block B input if currently joining.
		// This part is a bit iffy but it works I guess
		// It's necessary since otherwise if we press B while joining it'd
		// play SFX_BACK twice (first the game, then the mod), and there's
		// no other way to check if we were joining
		//(choice gets reset before we check)
		if (SokuAPI::InputBlock.Check() && InputManager->P1.B == 1 && joining) {
			Status::Normal("Joining aborted.");
			joining = false;
			InputManager->P1.B = 10;
		}
	} else {
		ip_id = 0;
	}
}
} // namespace Hostlist