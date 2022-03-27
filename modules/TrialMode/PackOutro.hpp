//
// Created by Gegel85 on 05/11/2021.
//

#ifndef SWRSTOYS_PACKOUTRO_HPP
#define SWRSTOYS_PACKOUTRO_HPP


#include <string>
#include <map>
#include <sol/forward.hpp>
#include <SokuLib.hpp>
#include <Animations/LuaBattleAnimation.hpp>

class PackOutro {
private:

	bool _hasOpeningObject = false;
	char _openingObject[0x98];
	std::string _packPath;
	unsigned _wait = 60;
	unsigned _textWait = 0;
	bool _finished = false;
	bool _ended = false;
	sol::state *_lua;
	SokuLib::Color _colorUp = SokuLib::Color::White;
	SokuLib::Color _colorDown = SokuLib::Color::White;
	SokuLib::DrawUtils::RectangleShape _rect;
	unsigned _currentSprite = 0;
	unsigned _currentCommand = 0;
	std::vector<std::string> _commands;
	SokuLib::DrawUtils::Sprite _background;
	std::unique_ptr<LuaBattleAnimation> _animation;
	std::vector<std::unique_ptr<SokuLib::DrawUtils::Sprite>> _text;
	std::vector<std::unique_ptr<SokuLib::DrawUtils::Sprite>> _sprites;

	static const std::map<std::string, void (PackOutro::*)(const std::string &args)> _commandsCallbacks;

	void _destroyOpeningObject();
	void _endCmd(const std::string &args);
	void _textCmd(const std::string &args);
	void _clearCmd(const std::string &args);
	void _colorCmd(const std::string &args);
	void _setBGMCmd(const std::string &args);
	void _playAnimCmd(const std::string &args);
	void _backgroundCmd(const std::string &args);
	void _generateAnimation(const std::string &args);
	void _initOpeningObject(const std::string &file);
	void _processNextCommand();

public:
	PackOutro(const std::string &packPath, const std::string &file);
	~PackOutro();
	void draw() const;
	bool update();
};


#endif //SWRSTOYS_PACKOUTRO_HPP
