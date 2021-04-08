//
// Created by PinkySmile on 18/02/2021.
//

#ifndef SWRSTOYS_GUI_HPP
#define SWRSTOYS_GUI_HPP


#include <TGUI/TGUI.hpp>

namespace Practice
{
	extern std::array<std::string, SokuLib::CHARACTER_RANDOM> names;
	extern tgui::Gui gui;

	union ElemProperty {
		float value;
		int selected;
		bool checked;
		char text[4];
	};

	void loadAllGuiElements(LPCSTR profilePath);
	void updateGuiState();
	void init(LPCSTR profilePath);
	void setElem(const std::string &elem, const ElemProperty &property);
}


#endif //SWRSTOYS_GUI_HPP
