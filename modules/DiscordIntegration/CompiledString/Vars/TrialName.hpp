//
// Created by PinkySmile on 15/01/2021.
//

#ifndef SWRSTOYS_TRIALNAME_HPP
#define SWRSTOYS_TRIALNAME_HPP

#include "../CompiledString.hpp"

class TrialName: public CompiledString {
public:
	std::string getString() const override;
};

#endif // SWRSTOYS_TRIALNAME_HPP
