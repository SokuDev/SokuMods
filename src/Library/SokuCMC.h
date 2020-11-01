// Made by S-len https://github.com/S-len
#pragma once
#include <windows.h>
#include <wchar.h>

#define UNKNOWN_GLOBAL 0x0089a888
#define IN_MENU *(char*)(0x089a888 + 4)

#define CMENU_OBJ (*(*((*(void****)UNKNOWN_GLOBAL)+1)+2))

typedef unsigned uint;

typedef struct {
    void* vftable;
    void* CNetworkBasePtr;
    byte Choice;
    byte Subchoice;
    byte UNKNOWN_0[2];
    void* CDesignBasePtr;
    byte UNKNOWN_1[48];
    void* MenuItemSprites[7];
    byte UNKNOWN_2[860];
    byte NotSpectateFlag;
    byte Spectate;
    byte UNKNOWN_3[2];
    uint MenuItem_Count;
    byte UNKNOWN_4[8];
    uint CursorPos;
    uint CursorPos2;
    byte UNKNOWN_5[48];
    uint NumberInput_ArrowPos;
    byte UNKNOWN_6[36];
    uint Port;
    byte UNKNOWN_7[4];
    char IPString[20]; //Final/Used val
    char IPWString; //Unknown size
    byte UNKNOWN_8[2051];
    uint NotInSubMenuFlag;
    byte UNKNOWN_9[332];
    byte InSubMenuFlag;
    byte UNKNOWN_10[171];
    byte UnknownJoinFlag;
    byte UNKNOWN_12[835];
} CMenuConnect;

namespace SokuCMC {
    CMenuConnect* GetMenuObj() {
        return (CMenuConnect*)CMENU_OBJ;
    }

    void SetupHost(uint port, bool spectate) {
        typedef void(__thiscall* func)(void*);
        func HostFun = (func)0x0446a40;

        CMenuConnect *menu = GetMenuObj();
        menu->Port = port;
        menu->Spectate = spectate;
        menu->Choice = 1;
        menu->Subchoice = 2;

        HostFun(menu);
    }

    void JoinHost(const char* ip, uint port, bool spectate = false) {
        typedef void(__thiscall* func)(void*);
        func JoinFun = (func)0x0446b20;

        CMenuConnect* menu = GetMenuObj();
        if (ip != NULL) {
            //Unsafe
            strcpy(menu->IPString, ip);
            menu->Port = port;
        }
        menu->Choice = 2;
        menu->Subchoice = (spectate? 6 : 3);
        menu->UnknownJoinFlag = 1;
        menu->NotSpectateFlag = (byte)!spectate;

        JoinFun(menu);
    }

    //Resets choice/subchoice and clears any messagebox
    void ClearMenu() {
        GetMenuObj()->Choice = 0;
        GetMenuObj()->Subchoice = 0;
    }
}
