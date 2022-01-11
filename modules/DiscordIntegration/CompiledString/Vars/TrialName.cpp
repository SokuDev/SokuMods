//
// Created by PinkySmile on 15/01/2021.
//

#include <windows.h>
#include "TrialName.hpp"

std::string TrialName::getString() const {
	HMODULE handle  = LoadLibraryA("TrialMode");

	if (!handle)
		handle = LoadLibraryA("TrialModeDebug");
	if (!handle)
		return "TrialMode not detected";

	auto getTrialName = reinterpret_cast<std::string (*)()>(GetProcAddress(handle, "getTrialName"));

	return getTrialName ? getTrialName() : "getTrialName not found in TrialMode dll";
}
