//
// Created by PinkySmile on 23/07/2021.
//

#ifndef SWRSTOYS_COMBOTRIAL_HPP
#define SWRSTOYS_COMBOTRIAL_HPP


#include <SokuLib.hpp>
#include "Trial.hpp"

class ComboTrial : public Trial {
private:
	struct SpecialAction {
		SokuLib::Action action;
		unsigned nbHits;
		unsigned delay;
	};

	//Init params
	float _playerStartPos;
	SokuLib::Vector2f _dummyStartPos;
	std::vector<SpecialAction> _exceptedActions;
	bool _counterHit;
	SokuLib::Skill _skills[15];
	bool _disableLimit;
	int _uniformCardCost;
	std::vector<unsigned short> _hand;
	bool _playComboAfterIntro;

	//State
	bool _isStart = true;
	bool _dummyHit = false;

	void _initGameStart();

public:
	ComboTrial(SokuLib::Character player, const nlohmann::json &json);
	bool update(bool &canHaveNextFrame) override;
	void render() override;
	int getScore() override;
};


#endif //SWRSTOYS_COMBOTRIAL_HPP
