//
// Created by PinkySmile on 11/11/2020.
//

#ifndef HISOUTENSOKUDISCORDINTEGRATION_SHIFTJISDECODER_HPP
#define HISOUTENSOKUDISCORDINTEGRATION_SHIFTJISDECODER_HPP

#include <map>
#include <string>

std::string convertShiftJisToUTF8(const char *str);
extern const std::map<unsigned short, unsigned short> convTable;

#endif // HISOUTENSOKUDISCORDINTEGRATION_SHIFTJISDECODER_HPP
