//
// Created by PinkySmile on 23/07/2021.
//

#ifndef SWRSTOYS_COMBOTRIAL_HPP
#define SWRSTOYS_COMBOTRIAL_HPP


#include <SokuLib.hpp>
#include "Trial.hpp"
#include "Menu.hpp"

class ComboTrial : public Trial {
private:
	struct SpecialAction {
		SokuLib::Action action;
		unsigned nbHits;
		unsigned delay;
		std::vector<SokuLib::KeyInput> inputs;
		SokuLib::DrawUtils::Sprite sprite;
		unsigned int counter = 0;
		std::string moveName;
		std::string name;

		void parse();
	};

	struct ScorePrerequisites {
		unsigned attempts = -1;
		unsigned hits = 0;
		int damage = INT32_MIN;
		unsigned minLimit = 0;
		unsigned maxLimit = -1;

		ScorePrerequisites() = default;
		ScorePrerequisites(const nlohmann::json &json, const ScorePrerequisites *other);
		bool met(unsigned currentAttempts) const;
	};

	//Init params
	float _playerStartPos;
	SokuLib::Vector2f _dummyStartPos;
	std::vector<std::unique_ptr<SpecialAction>> _exceptedActions;
	SokuLib::Weather _weather;
	SokuLib::Skill _skills[15];
	bool _disableLimit;
	int _uniformCardCost;
	std::vector<unsigned short> _hand;
	std::vector<ScorePrerequisites> _scores;
	bool _playComboAfterIntro;

	//State
	unsigned char _dollAnim = 0;
	unsigned _freezeCounter = 0;
	unsigned _waitCounter = 0;
	unsigned _actionCounter = 0;
	unsigned _actionWaitCounter = 0;
	unsigned _timer = 0;
	unsigned _attempts = 0;
	float _rotation = 0;
	bool _first = true;
	bool _isStart = true;
	bool _dummyHit = false;
	bool _playingIntro = false;
	bool _finished = false;
	SokuLib::Scene _next = SokuLib::SCENE_BATTLE;

	//Render
	mutable SokuLib::DrawUtils::Sprite _doll;
	mutable SokuLib::DrawUtils::Sprite _gear;
	mutable SokuLib::DrawUtils::Sprite _gearShadow;
	mutable SokuLib::DrawUtils::Sprite _attemptText;
	mutable SokuLib::DrawUtils::RectangleShape _rect;

	void _playIntro();
	void _initGameStart();
	void _loadExpected(const std::string &expected);

	static SokuLib::Action getMoveAction(SokuLib::Character chr, std::string &name);

public:
	ComboTrial(SokuLib::Character player, const nlohmann::json &json);

	void editPlayerInputs(SokuLib::KeyInput &originalInputs) override;
	SokuLib::KeyInput getDummyInputs() override;
	bool update(bool &canHaveNextFrame) override;
	void render() const override;
	int getScore() override;
	void onMenuClosed(MenuAction action) override;
	SokuLib::Scene getNextScene() override;

	friend class ComboTrialResult;
};

class ComboTrialResult : public ResultMenu {
private:
	struct ScorePart {
	private:
		int _ttlAttempts;
		SokuLib::DrawUtils::Sprite _score;
		SokuLib::DrawUtils::Sprite _hits;
		SokuLib::DrawUtils::Sprite _damages;
		SokuLib::DrawUtils::Sprite _attempts;
		SokuLib::DrawUtils::Sprite _limit;
		ComboTrial::ScorePrerequisites _prerequ;

	public:
		void load(int ttlattempts, const ComboTrial::ScorePrerequisites &prerequ, int index);
		void draw(float alpha);
	};

	ComboTrial &_parent;
	std::array<ScorePart, 4> _parts;

public:
	ComboTrialResult(ComboTrial &trial);
	~ComboTrialResult() override = default;
	int onRender() override;
};


#endif //SWRSTOYS_COMBOTRIAL_HPP
