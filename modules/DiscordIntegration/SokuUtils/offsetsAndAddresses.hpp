//
// Created by Gegel85 on 31/10/2020.
//

#ifndef TOUHOUUNLDISCORDINTEGRATION_OFFSETSANDADDRESSES_HPP
#define TOUHOUUNLDISCORDINTEGRATION_OFFSETSANDADDRESSES_HPP

#include "SokuState.hpp"

#define CLogo_Process(p) Ccall(p, s_origCLogo_OnProcess, int, ())()
#define CBattle_Process(p) Ccall(p, s_origCBattle_OnProcess, int, ())()
#define CBattleSV_Process(p, o) Ccall(p, s_origCBattleSV_OnProcess, int, (void *))(o)
#define CBattleCL_Process(p, o) Ccall(p, s_origCBattleCL_OnProcess, int, (void *))(o)
#define CTitle_Process(p) Ccall(p, s_origCTitle_OnProcess, int, ())()
#define CSelect_Process(p) Ccall(p, s_origCSelect_OnProcess, int, ())()
#define PLAYER_POS_X_OFFSET 0xEC
#define PLAYER_POS_Y_OFFSET 0xF0

typedef void *(__thiscall *Init_fun)(void *);

#define LoadGraphicsFunAddr 0x408410
#define LGThreadAddr 0x089fff4
#define LPThreadId 0x089fff8
#define ActivateMenuAddr 0x43e130
#define NewAddr 0x81fbdc
#define NetworkMenuInitAddr 0x0448760

#define NetworkMenuBufferSize 0x118C

#define New(size) (((void *(__cdecl *)(DWORD))NewAddr)(size))
#define LGThread ((HANDLE *)LGThreadAddr)
#define ActivateMenu(addr) (((void (*)(void *))ActivateMenuAddr)(addr))
#define NetworkMenuInit() (((Init_fun)NetworkMenuInitAddr)(New(NetworkMenuBufferSize)))

#endif // TOUHOUUNLDISCORDINTEGRATION_OFFSETSANDADDRESSES_HPP
