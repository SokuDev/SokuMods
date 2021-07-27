//
// Created by PinkySmile on 19/07/2021.
//

#ifndef SWRSTOYS_MENU_HPP
#define SWRSTOYS_MENU_HPP


#include <SokuLib.hpp>
#include "Trial.hpp"

extern char profilePath[1024 + MAX_PATH];
extern char profileFolderPath[1024 + MAX_PATH];
extern HMODULE myModule;
extern SokuLib::SWRFont defaultFont10;
extern SokuLib::SWRFont defaultFont12;
extern SokuLib::SWRFont defaultFont16;
extern bool loadRequest;
extern bool wasPressed;
extern std::unique_ptr<Trial> loadedTrial;
extern bool editorMode;

void menuLoadAssets();
int menuOnProcess(SokuLib::MenuResult *This);
void menuOnRender(SokuLib::MenuResult *This);
void menuUnloadAssets();


#endif //SWRSTOYS_MENU_HPP
