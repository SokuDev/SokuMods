//
// Created by PinkySmile on 15/01/2021.
//

#ifndef SWRSTOYS_TRIALPACK_HPP
#define SWRSTOYS_TRIALPACK_HPP

#include "../CompiledString.hpp"

class TrialPack : public CompiledString {
public:
	std::string getString() const override;
};

#endif // SWRSTOYS_TRIALPACK_HPP
