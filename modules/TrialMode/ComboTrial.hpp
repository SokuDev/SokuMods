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
		std::vector<SokuLib::KeyInput> inputs;
		unsigned int counter = 0;
		std::string name;

		void parse();
	};

	//Init params
	float _playerStartPos;
	SokuLib::Vector2f _dummyStartPos;
	std::vector<SpecialAction> _exceptedActions;
	SokuLib::Weather _weather;
	SokuLib::Skill _skills[15];
	bool _disableLimit;
	int _uniformCardCost;
	std::vector<unsigned short> _hand;
	bool _playComboAfterIntro;

	//State
	unsigned _waitCounter = 0;
	unsigned _actionCounter = 0;
	unsigned _actionWaitCounter = 0;
	bool _first = true;
	bool _isStart = true;
	bool _dummyHit = false;
	bool _playingIntro = false;

	void _playIntro();
	void _initGameStart();
	void _loadExpected(const std::string &expected);

public:
	void editPlayerInputs(SokuLib::KeyInput &originalInputs) override;

	SokuLib::KeyInput getDummyInputs() override;

	ComboTrial(SokuLib::Character player, const nlohmann::json &json);
	bool update(bool &canHaveNextFrame) override;
	void render() override;
	int getScore() override;
};


#endif //SWRSTOYS_COMBOTRIAL_HPP
