//
// Created by Gegel85 on 05/04/2022.
//

#ifndef SWRSTOYS_INPUTBOX_HPP
#define SWRSTOYS_INPUTBOX_HPP

#include <functional>

void inputBoxRender();
void inputBoxUpdate();
void inputBoxLoadAssets();
void inputBoxUnloadAssets();
void openInputDialog(const char *title, const char *defaultValue);
void setInputBoxCallbacks(const std::function<void (const std::string &value)> &onAccept);

extern bool inputBoxShown;

#endif //SWRSTOYS_INPUTBOX_HPP
