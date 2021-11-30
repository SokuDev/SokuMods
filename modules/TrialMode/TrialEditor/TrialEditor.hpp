//
// Created by PinkySmile on 23/07/2021.
//

#ifndef SWRSTOYS_TRIALEDITOR_HPP
#define SWRSTOYS_TRIALEDITOR_HPP


#include <map>
#include <functional>
#include <nlohmann/json.hpp>
#include <SokuLib.hpp>
#include "TrialBase.hpp"
#include "Animations/BattleAnimation.hpp"

extern bool reloadRequest;

class TrialEditor : public TrialBase {
protected:
	int (SokuLib::PauseMenu::*_ogOnUpdate)();
	int (SokuLib::PauseMenu::*_ogOnRender)();
	std::string _music;
	std::string _musicReal;
	std::string _folder;
	std::string _introPath;
	std::string _outroPath;
	std::string _introRelPath;
	std::string _outroRelPath;
	double _loopStart = 0;
	double _loopEnd = 0;
	static const std::map<std::string, std::function<TrialEditor *(const char *folder, const char *path, SokuLib::Character player, const nlohmann::json &json)>> _factory;

	std::unique_ptr<BattleAnimation> _intro;
	std::unique_ptr<BattleAnimation> _outro;
	bool _introPlayed = false;
	bool _outroPlayed = false;

	void _introOnRender() const;
	void _outroOnRender() const;
	void _introOnUpdate();
	void _outroOnUpdate();
	void _initAnimations(bool intro = true, bool outro = true);
	void _playBGM();

public:
	TrialEditor(const char *folder, const nlohmann::json &json);
	~TrialEditor() override;

	virtual int pauseOnUpdate() = 0;
	virtual int pauseOnRender() const = 0;
	static TrialBase *create(const char *folder, const char *path, SokuLib::Character player, const nlohmann::json &json);
};



#endif //SWRSTOYS_TRIAL_HPP
