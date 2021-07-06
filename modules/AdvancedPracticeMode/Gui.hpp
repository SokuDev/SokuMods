//
// Created by PinkySmile on 18/02/2021.
//

#ifndef SWRSTOYS_GUI_HPP
#define SWRSTOYS_GUI_HPP


#include <TGUI/TGUI.hpp>
#include <SokuLib.hpp>

namespace Practice
{
	extern tgui::Gui gui;
	extern tgui::Panel::Ptr panel;
	extern tgui::Tabs::Ptr tab;

	void loadAllGuiElements(LPCSTR profilePath);
	void updateGuiState();
	void init(LPCSTR profilePath);
	void setElem(const std::string &elem, const SokuLib::ElemProperty &property);
}


#endif //SWRSTOYS_GUI_HPP
