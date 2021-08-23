//
// Created by PinkySmile on 19/07/2021.
//

#ifndef SWRSTOYS_MENU_HPP
#define SWRSTOYS_MENU_HPP


#include <SokuLib.hpp>
#include "Trial.hpp"

class ResultMenu : public SokuLib::IMenu {
private:
	int _selected = 0;
	SokuLib::DrawUtils::Sprite _title;
	SokuLib::DrawUtils::Sprite _score;
	SokuLib::DrawUtils::Sprite _resultTop;
	std::array<SokuLib::DrawUtils::Sprite, Trial::NB_MENU_ACTION> _text;

public:
	ResultMenu(int score);
	~ResultMenu() override = default;
	void _() override;
	int onProcess() override;
	int onRender() override;
};

extern char profilePath[1024 + MAX_PATH];
extern char profileFolderPath[1024 + MAX_PATH];
extern HMODULE myModule;
extern SokuLib::SWRFont defaultFont10;
extern SokuLib::SWRFont defaultFont12;
extern SokuLib::SWRFont defaultFont16;
extern bool loadRequest;
extern std::unique_ptr<Trial> loadedTrial;
extern bool editorMode;
extern bool drawMutex;
extern bool filterMutex;

void __lockMutex(volatile bool &mutex);
void __unlockMutex(bool &mutex);
void menuLoadAssets();
int menuOnProcess(SokuLib::MenuResult *This);
void menuOnRender(SokuLib::MenuResult *This);
void menuUnloadAssets();


#endif //SWRSTOYS_MENU_HPP
