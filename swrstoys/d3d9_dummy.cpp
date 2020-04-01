#include <windows.h>
#include "dummy.h"

__declspec(naked) void WINAPI d_Direct3DShaderValidatorCreate9() {
	_asm { jmp p_Direct3DShaderValidatorCreate9 }
}
__declspec(naked) void WINAPI d_PSGPError() {
	_asm { jmp p_PSGPError }
}
__declspec(naked) void WINAPI d_PSGPSampleTexture() {
	_asm { jmp p_PSGPSampleTexture }
}
__declspec(naked) void WINAPI d_D3DPERF_BeginEvent() {
	_asm { jmp p_D3DPERF_BeginEvent }
}
__declspec(naked) void WINAPI d_D3DPERF_EndEvent() {
	_asm { jmp p_D3DPERF_EndEvent }
}
__declspec(naked) void WINAPI d_D3DPERF_GetStatus() {
	_asm { jmp p_D3DPERF_GetStatus }
}
__declspec(naked) void WINAPI d_D3DPERF_QueryRepeatFrame() {
	_asm { jmp p_D3DPERF_QueryRepeatFrame }
}
__declspec(naked) void WINAPI d_D3DPERF_SetMarker() {
	_asm { jmp p_D3DPERF_SetMarker }
}
__declspec(naked) void WINAPI d_D3DPERF_SetOptions() {
	_asm { jmp p_D3DPERF_SetOptions }
}
__declspec(naked) void WINAPI d_D3DPERF_SetRegion() {
	_asm { jmp p_D3DPERF_SetRegion }
}
__declspec(naked) void WINAPI d_DebugSetLevel() {
	_asm { jmp p_DebugSetLevel }
}
__declspec(naked) void WINAPI d_DebugSetMute() {
	_asm { jmp p_DebugSetMute }
}
__declspec(naked) void WINAPI d_Direct3DCreate9() {
	_asm { jmp p_Direct3DCreate9 }
}
__declspec(naked) void WINAPI d_Direct3DCreate9Ex() {
	_asm { jmp p_Direct3DCreate9Ex }
}
