//
// Created by Gegel85 on 31/10/2020.
//

#ifndef TOUHOUUNLDISCORDINTEGRATION_OFFSETSANDADDRESSES_HPP
#define TOUHOUUNLDISCORDINTEGRATION_OFFSETSANDADDRESSES_HPP

#include "globals.hpp"

#define CLogo_Process(p)   Ccall(p, s_origCLogo_OnProcess,   int, ())()
#define CBattle_Process(p) Ccall(p, s_origCBattle_OnProcess, int, ())()
#define CTitle_Process(p)  Ccall(p, s_origCTitle_OnProcess,  int, ())()
#define CSelect_Process(p) Ccall(p, s_origCSelect_OnProcess, int, ())()
#define PLAYER_POS_X_OFFSET 0xEC
#define PLAYER_POS_Y_OFFSET 0xF0

#endif //TOUHOUUNLDISCORDINTEGRATION_OFFSETSANDADDRESSES_HPP
