//
// Created by PinkySmile on 15/01/2021.
//

#ifndef SWRSTOYS_STAGEIMGSTRING_HPP
#define SWRSTOYS_STAGEIMGSTRING_HPP

#include "../CompiledString.hpp"

class StageImgString: public CompiledString {
public:
	std::string getString() const override;
};

#endif // SWRSTOYS_STAGEIMGSTRING_HPP
