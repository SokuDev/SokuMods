//
// Created by 2deg and PinkySmile on 20/07/2021.
//

#include <utility>
#include "BattleAnimation.hpp"
#include "LuaBattleAnimation.hpp"

class ExternalBattleAnimation : public BattleAnimation {
private:
	HMODULE _module;
	BattleAnimation *_real;

public:
	ExternalBattleAnimation(HMODULE module, BattleAnimation *val) :
		_module(module),
		_real(val)
	{
	}

	~ExternalBattleAnimation() override
	{
		delete this->_real;
		FreeLibrary(this->_module);
	}

	bool update() override
	{
		return this->_real->update();
	}

	void render() const override
	{
		this->_real->render();
	}

	void onKeyPressed() override
	{
		this->_real->onKeyPressed();
	}
};

std::string getLastErrorAsString(DWORD errorMessageID)
{
	if (errorMessageID == 0)
		return "No error";

	LPSTR messageBuffer = nullptr;

	FormatMessageA(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		nullptr,
		errorMessageID,
		MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US),
		reinterpret_cast<LPSTR>(&messageBuffer),
		0,
		nullptr
	);

	if (!messageBuffer)
		return "Unknown error " + std::to_string(errorMessageID);

	std::string str = messageBuffer;

	LocalFree(messageBuffer);
	return str;
}

ExternalBattleAnimation *loadBattleAnimationFromExternalModule(const char *path, bool isIntro)
{
	HMODULE handle = LoadLibraryA(path);

	if (handle == nullptr) {
		auto err = GetLastError();

		throw std::invalid_argument("LoadLibrary failed with code " + std::to_string(err) + ": " + getLastErrorAsString(err));
	}

	std::string name = isIntro ? "getIntro" : "getOutro";
	auto fct = reinterpret_cast<BattleAnimation *(*)()>(GetProcAddress(handle, name.c_str()));

	if (!fct) {
		FreeLibrary(handle);
		throw std::invalid_argument("GetProcAddress(\"" + name + "\") failed.");
	}

	BattleAnimation *val = fct();

	if (!val) {
		FreeLibrary(handle);
		throw std::invalid_argument(name + " returned nullptr");
	}
	return new ExternalBattleAnimation(handle, val);
}

BattleAnimation *loadBattleAnimation(const char *packPath, const char *path, bool isIntro)
{
	if (!path)
		return nullptr;
	if (strlen(path) >= strlen(".lua") && strcmp(&path[strlen(path) - strlen(".lua")], ".lua") == 0)
		return new LuaBattleAnimation(packPath, path);
	if (strlen(path) >= strlen(".dll") && strcmp(&path[strlen(path) - strlen(".dll")], ".dll") == 0)
		return loadBattleAnimationFromExternalModule(path, isIntro);
	throw std::invalid_argument("Unknown animation format");
}
