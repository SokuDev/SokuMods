//
// Created by PinkySmile on 15/01/2021.
//

#ifndef SWRSTOYS_EXCEPTIONS_HPP
#define SWRSTOYS_EXCEPTIONS_HPP

#include <exception>

class InvalidString: public std::exception {
private:
	std::string _msg;

public:
	InvalidString(const std::string &&msg): _msg(msg){};
	const char *what() const noexcept override {
		return this->_msg.c_str();
	};
};

#endif // SWRSTOYS_EXCEPTIONS_HPP
