//
// Created by PinkySmile on 12/08/2021.
//

#ifndef SWRSTOYS_LUABATTLEANIMATION_HPP
#define SWRSTOYS_LUABATTLEANIMATION_HPP


#include <sol/forward.hpp>
#include "BattleAnimation.hpp"

class LuaBattleAnimation : public BattleAnimation {
private:
	mutable std::unique_ptr<sol::state> _lua;
	mutable bool _hasError = false;

public:
	LuaBattleAnimation(const char *packPath, const char *script);

	bool update() override;
	void render() const override;
	void onKeyPressed() override;
};


#endif //SWRSTOYS_LUABATTLEANIMATION_HPP
