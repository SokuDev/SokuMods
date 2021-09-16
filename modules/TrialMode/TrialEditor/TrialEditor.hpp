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

class TrialEditor : public TrialBase {
private:
	std::string _music;
	std::string _folder;
	std::string _introPath;
	std::string _outroPath;
	static const std::map<std::string, std::function<TrialEditor *(const char *folder, SokuLib::Character player, const nlohmann::json &json)>> _factory;

protected:
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
	TrialEditor(const char *folder, const nlohmann::json &json);
	~TrialEditor() override;

	static TrialBase *create(const char *folder, SokuLib::Character player, const nlohmann::json &json);
};



#endif //SWRSTOYS_TRIAL_HPP
