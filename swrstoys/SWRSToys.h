#include <SokuLib.hpp>
#include <windows.h>
#include "Module.hpp"

bool Hook(HMODULE this_module);
void saveSettings();
void displaySokuCursor(SokuLib::Vector2i pos, SokuLib::Vector2u size);

extern SokuLib::SWRFont font;
extern HMODULE myModule;
extern BYTE gameHash[16];
extern std::vector<Module> modules;
extern Module *head;