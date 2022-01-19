//
// Created by PinkySmile on 23/07/2021.
//

#ifndef SWRSTOYS_TRIAL_HPP
#define SWRSTOYS_TRIAL_HPP


#include <map>
#include <functional>
#include <nlohmann/json.hpp>
#include <SokuLib.hpp>
#include "TrialBase.hpp"
#include "Animations/BattleAnimation.hpp"

class Trial : public TrialBase {
private:
	std::string _music;
	std::string _folder;
	std::string _introPath;
	std::string _outroPath;
	double _loopStart = 0;
	double _loopEnd = 0;
	static const std::map<std::string, std::function<Trial *(const char *folder, SokuLib::Character player, const nlohmann::json &json)>> _factory;
	static const std::map<std::string, std::function<Trial *(const char *folder, SokuLib::Character player, const nlohmann::json &json)>> _editorFactory;

	void _playBGM();

protected:
	int (SokuLib::PauseMenu::*_ogOnUpdate)();
	int (SokuLib::PauseMenu::*_ogOnRender)();
	std::unique_ptr<BattleAnimation> _intro;
	std::unique_ptr<BattleAnimation> _outro;
	bool _introPlayed = false;
	bool _outroPlayed = false;

	void _introOnRender() const;
	void _outroOnRender() const;
	void _introOnUpdate();
	void _outroOnUpdate();
	void _initAnimations(bool intro = true, bool outro = true);

public:
	Trial(const char *folder, const nlohmann::json &json);
	~Trial() override;

	static bool isTypeValid(const std::string &type);
	static TrialBase *create(const char *folder, SokuLib::Character player, const nlohmann::json &json);
};



#endif //SWRSTOYS_TRIAL_HPP
