#include <windows.h>
#include <asm.h>
#include "dummy.h"

__declspec(naked) void WINAPI d_Direct3DShaderValidatorCreate9() {
	_asm { JUMP(p_Direct3DShaderValidatorCreate9) }
}
__declspec(naked) void WINAPI d_PSGPError() {
	_asm { JUMP(p_PSGPError) }
}
__declspec(naked) void WINAPI d_PSGPSampleTexture() {
	_asm { JUMP(p_PSGPSampleTexture) }
}
__declspec(naked) void WINAPI d_D3DPERF_BeginEvent() {
	_asm { JUMP(p_D3DPERF_BeginEvent) }
}
__declspec(naked) void WINAPI d_D3DPERF_EndEvent() {
	_asm { JUMP(p_D3DPERF_EndEvent) }
}
__declspec(naked) void WINAPI d_D3DPERF_GetStatus() {
	_asm { JUMP(p_D3DPERF_GetStatus) }
}
__declspec(naked) void WINAPI d_D3DPERF_QueryRepeatFrame() {
	_asm { JUMP(p_D3DPERF_QueryRepeatFrame) }
}
__declspec(naked) void WINAPI d_D3DPERF_SetMarker() {
	_asm { JUMP(p_D3DPERF_SetMarker) }
}
__declspec(naked) void WINAPI d_D3DPERF_SetOptions() {
	_asm { JUMP(p_D3DPERF_SetOptions) }
}
__declspec(naked) void WINAPI d_D3DPERF_SetRegion() {
	_asm { JUMP(p_D3DPERF_SetRegion) }
}
__declspec(naked) void WINAPI d_DebugSetLevel() {
	_asm { JUMP(p_DebugSetLevel) }
}
__declspec(naked) void WINAPI d_DebugSetMute() {
	_asm { JUMP(p_DebugSetMute) }
}
__declspec(naked) void WINAPI d_Direct3DCreate9() {
	_asm { JUMP(p_Direct3DCreate9) }
}
__declspec(naked) void WINAPI d_Direct3DCreate9Ex() {
	_asm { JUMP(p_Direct3DCreate9Ex) }
}
