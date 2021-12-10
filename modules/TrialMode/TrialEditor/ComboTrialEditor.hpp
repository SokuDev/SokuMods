//
// Created by PinkySmile on 23/07/2021.
//

#ifndef SWRSTOYS_COMBOTRIALEDITOR_HPP
#define SWRSTOYS_COMBOTRIALEDITOR_HPP


#include <SokuLib.hpp>
#include "TrialEditor.hpp"
#include "Menu.hpp"

class ComboTrialEditor : public TrialEditor {
private:
	struct SpecialAction {
		bool optional;
		std::vector<SokuLib::Action> actions;
		std::vector<std::string> actionsStr;
		unsigned chargeTime;
		unsigned delay;
		std::vector<SokuLib::KeyInput> inputs;
		SokuLib::DrawUtils::Sprite sprite;
		unsigned int counter = 0;
		unsigned int chargeCounter = 0;
		std::string realMoveName;
		std::string moveName;
		std::string name;

		void parse();
	};

	struct RecordedAction {
		SokuLib::Inputs lastInput;
		unsigned frameCount;
		SokuLib::Action action;
		SokuLib::DrawUtils::Sprite sprite;
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

	//Init params
	bool _crouching;
	bool _leftWeather;
	bool _rightWeather;
	float _playerStartPos;
	unsigned _failTimer;
	SokuLib::Vector2f _dummyStartPos;
	std::vector<std::unique_ptr<SpecialAction>> _exceptedActions;
	SokuLib::Weather _weather;
	SokuLib::Skill _skills[15];
	bool _disableLimit;
	int _uniformCardCost;
	std::vector<unsigned short> _hand;
	std::vector<ScorePrerequisites> _scores;
	bool _playComboAfterIntro = false;
	std::vector<Doll> _dolls;
	bool _jump = false;
	bool _mpp = false;
	bool _orerries = false;
	bool _privateSquare = false;
	bool _stones = false;
	bool _clones = false;
	bool _tickTimer = false;
	unsigned _introRequ = 0;
	unsigned _outroRequ = 0;

	//State
	bool _needInit = false;
	unsigned char _dollAnim = 0;
	unsigned _freezeCounter = 0;
	unsigned _waitCounter = 0;
	unsigned _actionCounter = 0;
	unsigned _actionWaitCounter = 0;
	unsigned _timer = 0;
	float _rotation = 0;
	unsigned _firstFirst = 1;
	unsigned _currentDoll = 0;
	unsigned _lastSize = 0;
	bool _first = true;
	bool _isStart = true;
	bool _dummyHit = false;
	bool _playingIntro = false;
	bool _finished = false;
	bool _quit = false;
	SokuLib::Scene _next = SokuLib::SCENE_BATTLE;

	//Render
	SokuLib::DrawUtils::Sprite _score;
	SokuLib::DrawUtils::Sprite _pause;
	SokuLib::DrawUtils::Sprite _scoreEditFG;
	SokuLib::DrawUtils::Sprite _comboEditFG;
	SokuLib::DrawUtils::Sprite _comboEditBG;
	SokuLib::DrawUtils::Sprite _comboSprite;
	SokuLib::DrawUtils::Sprite _introSprite;
	SokuLib::DrawUtils::Sprite _outroSprite;
	SokuLib::DrawUtils::Sprite _trialEditorMockup;
	SokuLib::DrawUtils::Sprite _trialEditorPlayer;
	SokuLib::DrawUtils::Sprite _trialEditorDummy;
	SokuLib::DrawUtils::Sprite _trialEditorSystem;
	SokuLib::DrawUtils::Sprite _trialEditorMisc;
	SokuLib::DrawUtils::Sprite _damages[4];
	SokuLib::DrawUtils::Sprite _limits[4];
	SokuLib::DrawUtils::Sprite _hits[4];
	SokuLib::DrawUtils::Sprite _attempts[4];
	mutable SokuLib::DrawUtils::Sprite _attemptText;
	mutable SokuLib::DrawUtils::RectangleShape _rect;

	//Editor
	std::string _path;
	int _scoreCursorPos = 0;
	unsigned _musicTop = 0;
	unsigned _musicCursor = 0;
	unsigned _dollSelected = 0;
	unsigned _chrCursorPos = 0;
	unsigned _dollCursorPos = 0;
	unsigned _menuCursorPos = 0;
	unsigned _comboEditCursor = 0;
	unsigned _selectedSubcategory = 0;
	unsigned _stageCursor = 0;
	unsigned _scoreEdited = 0;
	float _fakePlayerPos = 0;
	bool _managingDolls = false;
	bool _changingPlayerPos = false;
	bool _changingDummyPos = false;
	bool _selectingStage = false;
	bool _selectingMusic = false;
	bool _selectingCharacters = false;
	bool _needReload = false;
	bool _comboPageDisplayed = false;
	bool _recordingCombo = false;
	bool _editingScore = false;
	bool _isRecordingScore = false;
	ScorePrerequisites _oldScore;
	std::vector<std::unique_ptr<RecordedAction>> _recordBuffer;
	SokuLib::Profile _fakeProfile;
	SokuLib::ProfileDeckEdit *_deckEditMenu = nullptr;
	SokuLib::Vector2f _dummyStartPosTmp;
	SokuLib::Character *_characterEdit = nullptr;
	SokuLib::Sprite _characterSprite;
	mutable SokuLib::DrawUtils::Sprite _numbers;
	mutable SokuLib::DrawUtils::Sprite _digits[6];
	mutable SokuLib::DrawUtils::Sprite _weathers;
	SokuLib::DrawUtils::RectangleShape _stageRect;
	SokuLib::DrawUtils::Sprite _weatherArrows;
	SokuLib::DrawUtils::Sprite _twilight;
	SokuLib::DrawUtils::Sprite _normal;
	SokuLib::Camera _oldCamera;
	std::vector<std::tuple<std::string, std::string, std::unique_ptr<SokuLib::DrawUtils::Sprite>>> _musics;
	std::map<unsigned, std::pair<std::unique_ptr<SokuLib::DrawUtils::Sprite>, std::unique_ptr<SokuLib::DrawUtils::Sprite>>> _stagesSprites;

	static const std::vector<unsigned> _stagesIds;
	static const std::map<unsigned, const char *> _stagesNames;

	void _refreshScoreSprites(int i);
	bool _selectScoreMenuItem();
	void _setupStageSprites();
	bool _copyDeckToPlayerSkills();
	bool _copyDeckToPlayerHand();
	bool _copyDeckToPlayerDeck();
	bool _copyDeckToDummyDeck();
	std::string _transformComboToString() const;
	std::vector<std::array<unsigned int, 2>> _getSkills() const;
	nlohmann::json _getScoresJson() const;
	void _selectingCharacterUpdate();
	void _selectingCharacterRender() const;
	void _openPause() const;
	void _typeNewCombo();

	//Menu callbacks
	bool notImplemented();
	bool returnToCharSelect();
	bool returnToGame();
	bool returnToTitleScreen();
	bool setPlayerCharacter();
	bool setPlayerPosition();
	bool setPlayerDeck();
	bool setPlayerSkills();
	bool setPlayerHand();
	bool setPlayerWeather();
	bool setPlayerDolls();
	bool setDummyCharacter();
	bool setDummyPosition();
	bool setDummyDeck();
	bool setDummyCrouch();
	bool setDummyJump();
	bool setDummyWeather();
	bool setCRankRequ();
	bool setBRankRequ();
	bool setARankRequ();
	bool setSRankRequ();
	bool setCounterHit();
	bool setLimitDisabled();
	bool setCardCosts();
	bool setCombo();
	bool setStage();
	bool setMusic();
	bool setWeather();
	bool setFailTimer();
	bool setOutro();
	bool setIntro();
	bool saveOnly();
	bool saveReturnToCharSelect();
	bool playIntro();
	bool playOutro();
	bool playPreview();
	static const std::vector<bool (ComboTrialEditor::*)()> callbacks[];

	void noRender() const;
	void playerRender() const;
	void dummyRender() const;
	void systemRender() const;
	void miscRender() const;
	static void (ComboTrialEditor::* const renderCallbacks[])() const;

	void _playIntro();
	void _initGameStart();
	void _loadExpected(const std::string &expected);

	static void _initVanillaGame();
	static SokuLib::Action _getMoveAction(SokuLib::Character chr, std::string &name);

public:
	ComboTrialEditor(const char *folder, const char *path, SokuLib::Character player, const nlohmann::json &json);
	~ComboTrialEditor() override;

	bool (ComboTrialEditor::*onDeckSaved)();
	void editPlayerInputs(SokuLib::KeyInput &originalInputs) override;
	SokuLib::KeyInput getDummyInputs() override;
	bool update(bool &canHaveNextFrame) override;
	void render() const override;
	int getScore() override;
	void onMenuClosed(MenuAction action) override;
	SokuLib::Scene getNextScene() override;
	int pauseOnUpdate() override;
	int pauseOnRender() const override;
	bool save() const;
};


#endif //SWRSTOYS_COMBOTRIALEDITOR_HPP
