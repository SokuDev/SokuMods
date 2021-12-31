//
// Created by PinkySmile on 12/31/2021.
//

#ifndef SWRSTOYS_EXPLORER_HPP
#define SWRSTOYS_EXPLORER_HPP

#include <functional>

void openFileDialog(const char *types, const std::string &startDir, const std::function<void(const std::string &path)> &fct);
void explorerLoadAssets();
void explorerUnloadAssets();
void loadExplorerRoot(const std::string &root);
void loadExplorerFile(const std::string &path, const std::string &root);
void setExplorerCallback(const std::function<void (const std::string &path)> &fct);
void explorerRender();
void explorerUpdate();
void setExplorerDefaultMusic(const std::function<void ()> &player);

extern bool explorerShown;

#endif //SWRSTOYS_EXPLORER_HPP
