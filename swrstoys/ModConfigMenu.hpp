//
// Created by PinkySmile on 02/09/2022.
//

#ifndef SWRSTOYS_MODCONFIGMENU_HPP
#define SWRSTOYS_MODCONFIGMENU_HPP


#include <deque>
#include <SokuLib.hpp>

class ModConfigMenu : public SokuLib::IMenu {
private:
	typedef SokuLib::DrawUtils::Sprite SpritePair[2];

	SokuLib::Vector2i _lastErrorSize;
	SokuLib::Vector2i _pathSize;
	bool _expended = false;
	int _selected = 0;
	int _exselected = 0;
	int _scrollStart = 0;
	int _exscrollStart = 0;
	SpritePair _path;
	SpritePair _priority;
	SpritePair _enabled;
	SokuLib::DrawUtils::Sprite _lastError;
#ifndef _NOTEX
	SokuLib::DrawUtils::Sprite _title;
#endif
	std::deque<SokuLib::DrawUtils::Sprite> _text;

	bool _normalProcess();
	bool _expendedProcess();

public:
	ModConfigMenu();
	~ModConfigMenu() override;

	void _() override;
	void prepareRightPanel();
	void renderRightPanel();
	void addNewModule();
	int onProcess() override;
	int onRender() override;
};


#endif //SWRSTOYS_MODCONFIGMENU_HPP
