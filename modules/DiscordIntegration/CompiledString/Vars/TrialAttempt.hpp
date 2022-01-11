//
// Created by PinkySmile on 15/01/2021.
//

#ifndef SWRSTOYS_TRIALATTEMPT_HPP
#define SWRSTOYS_TRIALATTEMPT_HPP

#include "../CompiledString.hpp"

class TrialAttempt : public CompiledString {
public:
	std::string getString() const override;
};

#endif // SWRSTOYS_TRIALATTEMPT_HPP
