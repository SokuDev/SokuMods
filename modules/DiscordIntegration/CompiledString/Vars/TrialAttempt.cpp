//
// Created by PinkySmile on 15/01/2021.
//

#include <windows.h>
#include "TrialAttempt.hpp"

std::string TrialAttempt::getString() const {
	HMODULE handle  = LoadLibraryA("TrialMode");

	if (!handle)
		handle = LoadLibraryA("TrialModeDebug");
	if (!handle)
		return "TrialMode not detected";

	auto getCurrentAttempts = reinterpret_cast<unsigned (*)()>(GetProcAddress(handle, "getCurrentAttempts"));

	return getCurrentAttempts ? std::to_string(getCurrentAttempts()) : "getCurrentAttempts not found in TrialMode dll";
}
