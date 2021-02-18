//
// Created by PinkySmile on 18/02/2021.
//

#ifndef SWRSTOYS_GUI_HPP
#define SWRSTOYS_GUI_HPP


#include <TGUI/TGUI.hpp>

namespace Practice
{
	extern tgui::Gui gui;
	void loadAllGuiElements(LPCSTR profilePath);
	void updateGuiState();
	void init(LPCSTR profilePath);
}


#endif //SWRSTOYS_GUI_HPP
