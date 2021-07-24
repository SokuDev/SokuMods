//
// Created by PinkySmile on 23/07/2021.
//

#ifndef SWRSTOYS_TRIAL_HPP
#define SWRSTOYS_TRIAL_HPP


#include <map>
#include <functional>
#include <nlohmann/json.hpp>
#include <SokuLib.hpp>

class Trial {
private:
	static const std::map<std::string, std::function<Trial *(SokuLib::Character player, const nlohmann::json &json)>> _factory;

public:
	virtual ~Trial() = default;
	virtual bool update(bool &canHaveNextFrame) = 0;
	virtual void render() = 0;
	virtual int getScore() = 0;

	static Trial *create(SokuLib::Character player, const nlohmann::json &json);
};



#endif //SWRSTOYS_TRIAL_HPP
