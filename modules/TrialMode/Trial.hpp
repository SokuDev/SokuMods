//
// Created by PinkySmile on 23/07/2021.
//

#ifndef SWRSTOYS_TRIAL_HPP
#define SWRSTOYS_TRIAL_HPP


#include <map>
#include <functional>
#include <nlohmann/json.hpp>
#include <SokuLib.hpp>
#include "Animations/BattleAnimation.hpp"

class Trial {
private:
	std::string music;
	std::unique_ptr<BattleAnimation> _intro;
	static const std::map<std::string, std::function<Trial *(const char *folder, SokuLib::Character player, const nlohmann::json &json)>> _factory;

protected:
	std::unique_ptr<BattleAnimation> _outro;
	bool _introPlayed = false;
	bool _outroPlayed = false;

	void _introOnRender() const;
	void _outroOnRender() const;
	void _introOnUpdate();
	void _outroOnUpdate();

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

	Trial(const char *folder, const nlohmann::json &json);
	virtual ~Trial();
	virtual bool update(bool &canHaveNextFrame) = 0;
	virtual void render() const = 0;
	virtual int getScore() = 0;
	virtual void editPlayerInputs(SokuLib::KeyInput &originalInputs) = 0;
	virtual SokuLib::KeyInput getDummyInputs() = 0;
	virtual void onMenuClosed(MenuAction action) = 0;
	virtual SokuLib::Scene getNextScene() = 0;

	static Trial *create(const char *folder, SokuLib::Character player, const nlohmann::json &json);
};



#endif //SWRSTOYS_TRIAL_HPP
