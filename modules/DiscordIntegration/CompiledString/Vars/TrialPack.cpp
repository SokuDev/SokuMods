//
// Created by PinkySmile on 15/01/2021.
//

#include <windows.h>
#include "TrialPack.hpp"

std::string TrialPack::getString() const {
	HMODULE handle  = LoadLibraryA("TrialMode");

	if (!handle)
		handle = LoadLibraryA("TrialModeDebug");
	if (!handle)
		return "TrialMode not detected";

	auto getTrialPack = reinterpret_cast<const char *(*)()>(GetProcAddress(handle, "getTrialPack"));

	return getTrialPack ? getTrialPack() : "getTrialPack not found in TrialMode dll";
}
