//
// Created by PinkySmile on 02/09/2022.
//

#ifndef SWRSTOYS_MODULE_HPP
#define SWRSTOYS_MODULE_HPP


#include <windows.h>
#include <ModLoader.hpp>
#include <string>
#include <filesystem>

class Module {
private:
	HMODULE module = nullptr;
	std::filesystem::path path;
	byte hash[16];
	HMODULE this_module;
	bool _init = false;

	bool (*CheckVersion)(const BYTE hash[16]);
	bool (*Initialize)(HMODULE hMyModule, HMODULE hParentModule);
	bool (*hasChainedHooks)();
	void (*unHook)();

public:
	bool customPriority = false;
	int priority = 0;
	bool enabled = false;
	Module *prev = nullptr;
	Module *next = nullptr;
	std::wstring lastError;

	int (*getPriority)();
	SokuModLoader::IValue **(*getConfig)();
	void (*freeConfig)(SokuModLoader::IValue **v);
	bool (*commitConfig)(SokuModLoader::IValue *);
	const char *(*getFailureReason)();

	Module(const wchar_t *path, byte hash[16], HMODULE this_module);
	std::wstring getLastError();
	bool isInit() const;
	bool isLoaded() const;
	bool supportsUnHook() const;
	bool load();
	bool init();
	bool initChain();
	void setDefaultPriority();
	bool uninit(bool remove = true);
	bool unload(bool remove = true, bool freeLib = true, bool fromChain = false);
	std::wstring getPath() const;
	std::wstring getName() const;
	bool operator<(const Module &other) const;
};


#endif //SWRSTOYS_MODULE_HPP
