//
// Created by PinkySmile on 19/07/2021.
//

#ifndef SWRSTOYS_MENU_HPP
#define SWRSTOYS_MENU_HPP


#include <SokuLib.hpp>

extern char profilePath[1024 + MAX_PATH];
extern HMODULE myModule;

void menuLoadAssets();
int menuOnProcess(SokuLib::MenuResult *This);
void menuOnRender(SokuLib::MenuResult *This);
void menuUnloadAssets();


#endif //SWRSTOYS_MENU_HPP
