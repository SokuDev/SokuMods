//
// Created by PinkySmile on 23/07/2021.
//

#ifndef SWRSTOYS_COMBOTRIAL_HPP
#define SWRSTOYS_COMBOTRIAL_HPP


#include <SokuLib.hpp>
#include "Trial.hpp"
#include "Menu.hpp"
#include "Images.hpp"

class ComboTrial : public Trial {
private:
	struct ScorePart {
	private:
		SokuLib::DrawUtils::Sprite _score;
		SokuLib::DrawUtils::Sprite _hits;
		SokuLib::DrawUtils::Sprite _damages;
		SokuLib::DrawUtils::Sprite _attempts;
		SokuLib::DrawUtils::Sprite _limit;
	};

	struct SpecialAction {
		bool optional;
		std::vector<SokuLib::Action> actions;
		unsigned chargeTime;
		unsigned delay;
		std::vector<SokuLib::KeyInput> inputs;
		SokuLib::DrawUtils::Sprite sprite;
		unsigned int counter = 0;
		unsigned int chargeCounter = 0;
		std::string moveName;
		std::string name;

		void parse();
	};

	struct ScorePrerequisites {
		unsigned attempts = -1;
		unsigned hits = 0;
		int damage = 0;
		unsigned minLimit = 0;

		ScorePrerequisites() = default;
		ScorePrerequisites(const nlohmann::json &json, const ScorePrerequisites *other);
		bool met(unsigned currentAttempts) const;
	};

	struct Doll {
		SokuLib::Vector2f pos;
		SokuLib::Direction dir;
	};

	typedef std::vector<std::pair<SokuLib::KeyInput, unsigned>> MacroData;

	//Init params
	bool _crouching;
	bool _leftWeather;
	bool _rightWeather;
	float _playerStartPos;
	unsigned _failTimer;
	SokuLib::Vector2f _dummyStartPos;
	std::vector<std::unique_ptr<SpecialAction>> _expectedActions;
	SokuLib::Weather _weather;
	SokuLib::Skill _skills[15];
	bool _disableLimit;
	int _uniformCardCost = 0;
	MacroData _previewInputs;
	std::vector<unsigned short> _hand;
	std::vector<ScorePrerequisites> _scores;
	bool _playComboAfterIntro = true;
	std::vector<Doll> _dolls;
	bool _jump = false;
	bool _mpp = false;
	bool _orerries = false;
	bool _privateSquare = false;
	bool _stones = false;
	bool _clones = false;
	bool _tickTimer = false;

	//State
	unsigned char _dollAnim = 0;
	unsigned _freezeCounter = 0;
	unsigned _waitCounter = 0;
	unsigned _actionCounter = 0;
	unsigned _actionWaitCounter = 0;
	unsigned _timer = 0;
	unsigned _attempts = 0;
	float _rotation = 0;
	unsigned _firstFirst = 1;
	unsigned _currentDoll = 0;
	unsigned _lastSize = 0;
	unsigned _inputPos = 0;
	unsigned _inputCurrent = 0;
	bool _first = true;
	bool _isStart = true;
	bool _dummyHit = false;
	bool _playingIntro = false;
	bool _finished = false;
	SokuLib::Scene _next = SokuLib::SCENE_BATTLE;

	//Render
	mutable SokuLib::DrawUtils::Sprite _name;
	mutable SokuLib::DrawUtils::Sprite _score;
	mutable std::array<ScorePart, 4> _parts;

	mutable SokuLib::DrawUtils::Sprite _doll;
	mutable SokuLib::DrawUtils::Sprite _gear;
	mutable SokuLib::DrawUtils::Sprite _gearShadow;
	mutable SokuLib::DrawUtils::Sprite _attemptText;
	mutable SokuLib::DrawUtils::RectangleShape _rect;

	//Pause
	bool _quit = false;
	bool _mirror = false;
	unsigned _cursorPos = 0;
	SokuLib::DrawUtils::Sprite _title;
	SokuLib::DrawUtils::Sprite _pause;
	mutable SokuLib::DrawUtils::Sprite _tick;

	void _playIntro();
	void _initGameStart();
	void _loadPauseAssets();
	bool _pauseOnKeyPressed();
	void _loadExpected(const std::string &expected);

	static SokuLib::Action _getMoveAction(SokuLib::Character chr, std::string &move, std::string &name);

public:
	ComboTrial(const char *folder, SokuLib::Character player, const nlohmann::json &json);
	void editPlayerInputs(SokuLib::KeyInput &originalInputs) override;
	SokuLib::KeyInput getDummyInputs() override;
	bool update(bool &canHaveNextFrame) override;
	void render() const override;
	int getScore() override;
	void onMenuClosed(MenuAction action) override;
	SokuLib::Scene getNextScene() override;
	unsigned getAttempt() override;
	int pauseOnUpdate() override;
	int pauseOnRender() const override;

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
