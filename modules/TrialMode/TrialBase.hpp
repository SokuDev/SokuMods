//
// Created by PinkySmile on 16/09/2021.
//

#ifndef SWRSTOYS_TRIALBASE_HPP
#define SWRSTOYS_TRIALBASE_HPP


#include <SokuLib.hpp>
#include <array>
#include <string>
#include <map>

extern const std::map<std::string, SokuLib::Weather> weathers;

class TrialBase {
public:
	enum MenuAction {
		GO_TO_NEXT_TRIAL,
		RETRY,
		RETURN_TO_TRIAL_SELECT,
		RETURN_TO_TITLE_SCREEN,
		NB_MENU_ACTION
	};

	static const std::array<std::string, NB_MENU_ACTION> menuActionText;
	static void hook();

	virtual ~TrialBase() = default;
	virtual bool update(bool &canHaveNextFrame) = 0;
	virtual void render() const = 0;
	virtual unsigned getAttempt() = 0;
	virtual int getScore() = 0;
	virtual void editPlayerInputs(SokuLib::KeyInput &originalInputs) = 0;
	virtual SokuLib::KeyInput getDummyInputs() = 0;
	virtual void onMenuClosed(MenuAction action) = 0;
	virtual SokuLib::Scene getNextScene() = 0;
};


#endif //SWRSTOYS_TRIALBASE_HPP
