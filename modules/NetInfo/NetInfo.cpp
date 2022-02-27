#pragma warning(disable : 4996)

#define WIN32_LEAN_AND_MEAN

#include <Windows.h>
#include <WinSock2.h>
#include <stdio.h>

#include <SokuLib.hpp>
#include <implot.h>
#include <imgui.h>
#include <imgui_impl_dx9.h>
#include <imgui_impl_win32.h>
#include <d3d9.h>
#include <thread>

#include "PatchMan.hpp"

LARGE_INTEGER Frequency;

const auto ResetPatchAddr = (DWORD)0x4151a6;
const auto ResetCallAddr = (DWORD)0x4151ac;
const auto EndSceneCallAddr = (DWORD)0x40104c;
const auto D3D9DevicePtr = (IDirect3DDevice9**)0x8a0e30;
const auto D3DPresentParams = (D3DPRESENT_PARAMETERS*)0x8a0f68;

typedef int (__stdcall *recvfromFn)(SOCKET s, char *buf, int len, int flags, sockaddr *from, int * fromlen);
recvfromFn Original_recvfrom = NULL;

typedef int (__stdcall *sendtoFn)(SOCKET s, char *buf, int len, int flags, sockaddr *to, int tolen);
sendtoFn Original_sendto = NULL;

typedef LRESULT(CALLBACK* WNDPROC)(HWND, UINT, WPARAM, LPARAM);
WNDPROC Original_WndProc = NULL;

typedef bool(__thiscall* SokuSetupFn)(void**, HWND*);
SokuSetupFn Original_SokuSetup = NULL;

typedef HRESULT(__stdcall* EndSceneFn)(IDirect3DDevice9*);
EndSceneFn Original_EndScene = NULL;

typedef HRESULT(__stdcall* ResetFn)(IDirect3DDevice9*, D3DPRESENT_PARAMETERS*);
ResetFn Original_Reset = NULL;

extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

HWND window;
bool active = true;

int sceneId;

int lastFrameId;

#define LATENCIES_COUNT 360
float latencies[LATENCIES_COUNT];

LARGE_INTEGER frameTimes[LATENCIES_COUNT];

bool isFrameScene() {
	switch (SokuLib::sceneId) {
	case SokuLib::Scene::SCENE_SELECTSV:
	case SokuLib::Scene::SCENE_SELECTCL:
	case SokuLib::Scene::SCENE_BATTLESV:
	case SokuLib::Scene::SCENE_BATTLECL:
		return true;
	default:
		return false;
	}
}

void init(void *_) {

}

void load() {

}

void render() {
	if(!isFrameScene()) {
		return;
	}
	
	ImGui::Begin("Network Information", NULL, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoInputs);
	
	float max = 0;
	float mean = 0;
	for (int i = 0; i < IM_ARRAYSIZE(latencies); i++) {
		mean += latencies[i];
		if(latencies[i] > max) {
			max = latencies[i];
		}
	}
	mean /= IM_ARRAYSIZE(latencies);
	char buf[64];
	sprintf(buf, "~%03.1fms", mean);
	
	if(ImPlot::BeginPlot("##netinfo", ImVec2(250, 100))) {
		ImPlot::SetupAxis(ImAxis_::ImAxis_X1, NULL, ImPlotAxisFlags_NoDecorations);
		ImPlot::SetupAxis(ImAxis_::ImAxis_Y1, "ms", ImPlotAxisFlags_None);
		ImPlot::SetupAxisLimits(ImAxis_::ImAxis_Y1, 0, max+20, ImGuiCond_Always);
		ImPlot::PlotLine("##latency", latencies, IM_ARRAYSIZE(latencies), 1, 0, lastFrameId % IM_ARRAYSIZE(latencies), sizeof(float));
		ImPlot::EndPlot();
	}
	
	
	ImGui::End();
}

int __stdcall Hooksendto(SOCKET s, char *buf, int len, int flags, sockaddr *to, int tolen) {
	int n = Original_sendto(s, buf, len, flags, to, tolen);
	if(n <= 0) {
		return n;
	}
	auto packet = *reinterpret_cast<SokuLib::Packet *>(buf);
	if(packet.type != SokuLib::PacketType::HOST_GAME && packet.type != SokuLib::PacketType::CLIENT_GAME) {
		return n;
	}
	auto packetGame = packet.game.event;
	if(packetGame.type != SokuLib::GameType::GAME_INPUT) {
		return n;
	}
	auto packetInput = packetGame.input;
	sceneId = packetInput.sceneId;
	lastFrameId = packetInput.frameId;
	LARGE_INTEGER frameTime;
	QueryPerformanceCounter(&frameTime);
	frameTimes[packetInput.frameId % LATENCIES_COUNT] = frameTime;
	return n;
}

int __stdcall Hookrecvfrom(SOCKET s, char *buf, int len, int flags, sockaddr *from, int *fromlen) {
	int n = Original_recvfrom(s, buf, len, flags, from, fromlen);
	if(n <= 0) {
		return n;
	}
	auto packet = *reinterpret_cast<SokuLib::Packet *>(buf);
	if(packet.type != SokuLib::PacketType::HOST_GAME && packet.type != SokuLib::PacketType::CLIENT_GAME) {
		return n;
	}
	auto packetGame = packet.game.event;
	if(packetGame.type != SokuLib::GameType::GAME_INPUT) {
		return n;
	}
	auto packetInput = packetGame.input;
	if(packetInput.sceneId != sceneId || packetInput.frameId > lastFrameId) {
		return n;
	}
	LARGE_INTEGER frameTime;
	QueryPerformanceCounter(&frameTime);
	LARGE_INTEGER elapsed;
	int i = packetInput.frameId % LATENCIES_COUNT;
	elapsed.QuadPart = frameTime.QuadPart - frameTimes[i].QuadPart;
	elapsed.QuadPart *= 1000;
	elapsed.QuadPart /= Frequency.QuadPart;
	latencies[i] = (int)elapsed.QuadPart;
	
	auto delay = static_cast<int>(packetInput.inputCount);
	
	return n;
}

LRESULT __stdcall HookWndProc(const HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	ImGuiIO& io = ImGui::GetIO();
	
	if (uMsg == WM_SIZE) {
		if (wParam == SIZE_MINIMIZED)
			active = false;
		else if (wParam == SIZE_RESTORED)
			active = true;
	}
	else if (uMsg == WM_ACTIVATEAPP && !D3DPresentParams->Windowed) {
		if(!wParam)
			active = false;
		else
			active = true;
	}

	LRESULT handled = CallWindowProc(Original_WndProc, hWnd, uMsg, wParam, lParam);

	if (!handled && active)
		if (ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam) || ImGui::GetIO().WantCaptureMouse)
			return true;
	return handled;
}

int __stdcall HookShimEndScene(IDirect3DDevice9* pDevice) {
	static bool init = true;
	
	if (active) {
		if (init) {
			init = false;
			ImGui::CreateContext();
			ImPlot::CreateContext();
			ImGuiIO& io = ImGui::GetIO();

			io.ConfigFlags |= ImGuiConfigFlags_NavNoCaptureKeyboard;
			io.ConfigFlags |= ImGuiConfigFlags_NoMouse;

			ImGui_ImplWin32_Init(window);
			ImGui_ImplDX9_Init(pDevice);

			Original_WndProc = (WNDPROC)SetWindowLongPtr(window, GWL_WNDPROC, (LONG_PTR)HookWndProc);

			load();
		}

		ImGui_ImplDX9_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();

		render();
		
		ImGui::EndFrame();
		ImGui::Render();

		ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
	}

	Original_EndScene(pDevice);
	return 0x8a0e14;
}

HRESULT __stdcall HookShimReset(IDirect3DDevice9* pDevice, D3DPRESENT_PARAMETERS* params) {
	ImGui_ImplDX9_InvalidateDeviceObjects();
	
	HRESULT hr = Original_Reset(pDevice, params);
	if (hr != S_OK)
		return hr;

	ImGui_ImplDX9_CreateDeviceObjects();

	return S_OK;
}

void HookSokuSetup(HWND hwnd) {
	window = hwnd;
	// Check if something already hooked DxReset.
	if (*((uint8_t*)ResetCallAddr) != 0xe8) {
		PatchMan::Patch(ResetPatchAddr, "\x68\x68\x0f\x8a\x00\x50\xe8\x00\x00\x00\x00\x90\x90", 13).Toggle(true);
		PatchMan::HookNear(ResetCallAddr, (DWORD)HookShimReset);
		Original_Reset = (*(ResetFn**)(*D3D9DevicePtr))[16];
	}
	else {
		Original_Reset = (ResetFn)PatchMan::HookNear(ResetCallAddr, (DWORD)HookShimReset);
	}
	// Check if something already hooked DxEndScene
	if (*((uint8_t*)EndSceneCallAddr) != 0xe8) {
		PatchMan::Patch(EndSceneCallAddr, "\xe8\x00\x00\x00\x00\x50\x90", 7).Toggle(true);
		PatchMan::HookNear(EndSceneCallAddr, (DWORD)HookShimEndScene);
		Original_EndScene = (*(EndSceneFn**)(*D3D9DevicePtr))[42];
	}
	else {
		Original_EndScene = (EndSceneFn)PatchMan::HookNear(EndSceneCallAddr, (DWORD)HookShimEndScene);
	}
}

bool __fastcall HookShimSokuSetup(void** DxWinHwnd, void* EDX, HWND* hwnd) {
	bool ret = Original_SokuSetup(DxWinHwnd, hwnd);
	HookSokuSetup(*hwnd);
	return ret;
}

extern "C" __declspec(dllexport) bool CheckVersion(const BYTE hash[16]) {
  return true;
}

extern "C" __declspec(dllexport) bool Initialize(HMODULE hMyModule, HMODULE hParentModule) {
	_beginthread(init, 0, NULL);
	
	QueryPerformanceFrequency(&Frequency);
	
	Original_SokuSetup = (SokuSetupFn)PatchMan::HookNear(0x7fb871, (DWORD)HookShimSokuSetup);
	Original_recvfrom = (recvfromFn)PatchMan::HookNear(0x41DAE5, (DWORD)Hookrecvfrom);
	Original_sendto = (sendtoFn)PatchMan::HookNear(0x4171CD, (DWORD)Hooksendto);

	return TRUE;
}

BOOL WINAPI DllMain(HINSTANCE hinst, DWORD dwReason, LPVOID reserved) {
	return TRUE;
}
