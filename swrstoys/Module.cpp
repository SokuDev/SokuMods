//
// Created by PinkySmile on 02/09/2022.
//

#include <fstream>
#include "Module.hpp"
#include "../MemoryModule/MemoryModule.h"


#define GET_MANDATORY_FUNCTION(name, path, module, modName) \
	do{if (!(this->name = (decltype(name))GetProcAddress(module, #name))) { \
		this->lastError = L"Failed loading " + modName + L": " #name " not found"; \
		printf("%S\n", this->lastError.c_str());\
		return false; \
	}}while(0)
#define GET_FUNCTION(name, module) this->name = (decltype(name))GetProcAddress(module, #name)


extern wchar_t app_path[MAX_PATH];

std::wstring GetLastErrorAsString(DWORD errorMessageID)
{
	if (errorMessageID == 0)
		return L"No Error";

	LPWSTR messageBuffer = nullptr;
	std::wstring m;

	printf("Finding message for error %lx\n", errorMessageID);
	if (FormatMessageW(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		nullptr,
		errorMessageID,
		MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US),
		(LPWSTR)&messageBuffer,
		0,
		nullptr
	)) {
		m = messageBuffer;
		LocalFree(messageBuffer);
		return m;
	}
	printf("Error %lx in FormatMessageW (EN)\n", GetLastError());

	if (!FormatMessageW(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		nullptr,
		errorMessageID,
		0,
		(LPWSTR)&messageBuffer,
		0,
		nullptr
	)) {
		printf("Error %lx in FormatMessageW (LOCAL)\n", GetLastError());
		return L"(null)\n";
	}
	m = messageBuffer;
	LocalFree(messageBuffer);
	return m;
}

Module::Module(const wchar_t *path, byte hash[16], HMODULE this_module) : path(path), this_module(this_module)
{
	if (this->path.is_relative())
		this->path = std::filesystem::path(app_path) / this->path;
	memcpy(this->hash, hash, 16);
}

std::wstring Module::getLastError()
{
	return this->lastError;
}

bool Module::isInit() const
{
	return this->_init;
}

bool Module::isLoaded() const
{
	return this->module != nullptr;
}

bool Module::supportsUnHook() const
{
	return this->unHook != nullptr;
}

bool Module::load()
{
	this->lastError = L"";
	if (this->isLoaded()) {
		puts("Not loading module because it is already loaded");
		return true;
	}

	printf("Loading module %S\n", this->path.c_str());

	auto name = this->getName();

	printf("Loading module %S\n", path.c_str());
	this->module = LoadLibraryW(path.c_str());
	if (this->module == nullptr) {
		auto err = GetLastError();

		if (err == ERROR_MOD_NOT_FOUND) {
			HANDLE hFile = CreateFileW(path.c_str(), GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
			LARGE_INTEGER size;
			char *buffer;
			DWORD real;

			if (hFile == INVALID_HANDLE_VALUE)
				goto fallback;

			if (!GetFileSizeEx(hFile, &size) || size.HighPart) {
				CloseHandle(hFile);
				puts("Invalid size.");
				goto fallback;
			}
			buffer = new(std::nothrow) char[size.LowPart];
			if (!buffer) {
				CloseHandle(hFile);
				puts("Allocation failure.");
				goto fallback;
			}
			ReadFile(hFile, buffer, size.LowPart, &real, nullptr);
			CloseHandle(hFile);

			auto result = MemoryLoadLibrary(buffer, real);

			free(buffer);
			if (result) {
				// ?????
				puts("WTF??");
				MemoryFreeLibrary(result);
				goto fallback;
			}
			err = GetLastError();
			if (GetLastError() != ERROR_MOD_NOT_FOUND) {
				printf("Failed memory loading %S: GetLastError() 0x%lx: %S", name.c_str(), err, GetLastErrorAsString(err).c_str());
				MemoryFreeLibrary(result);
				err = ERROR_MOD_NOT_FOUND;
				goto fallback;
			}

			size_t needed = _snwprintf(nullptr, 0, L"Failed loading %s: GetLastError() 0x%lx: %sModule's dependency '%hs' was not found.", name.c_str(), err, GetLastErrorAsString(err).c_str(), MemoryModuleMissingDependency());
			auto buf = (wchar_t *)malloc((needed + 1) * sizeof(wchar_t));

			_snwprintf(buf, (needed + 1), L"Failed loading %s: GetLastError() 0x%lx: %sModule's dependency '%hs' was not found.", name.c_str(), err, GetLastErrorAsString(err).c_str(), MemoryModuleMissingDependency());
			printf("%S\n", buf);
			this->lastError = buf;
			free(buf);
		} else {
		fallback:
			size_t needed = _snwprintf(nullptr, 0, L"Failed loading %s: GetLastError() 0x%lx: %s", name.c_str(), err, GetLastErrorAsString(err).c_str());
			auto buf = (wchar_t *)malloc((needed + 1) * sizeof(wchar_t));

			_snwprintf(buf, (needed + 1), L"Failed loading %s: GetLastError() 0x%x: %s", name.c_str(), err, GetLastErrorAsString(err).c_str());
			printf("%S\n", buf);
			this->lastError = buf;
			free(buf);
		}
		return false;
	}

	GET_MANDATORY_FUNCTION(CheckVersion, path, this->module, name);
	GET_MANDATORY_FUNCTION(Initialize,   path, this->module, name);
	GET_FUNCTION(getPriority,            this->module);
	GET_FUNCTION(hasChainedHooks,        this->module);
	GET_FUNCTION(unHook,                 this->module);
	GET_FUNCTION(getConfig,              this->module);
	GET_FUNCTION(freeConfig,             this->module);
	GET_FUNCTION(commitConfig,           this->module);
	GET_FUNCTION(getFailureReason,       this->module);
	if (!this->customPriority)
		this->priority = this->getPriority ? this->getPriority() : 0;
	return true;
}

bool Module::init()
{
	if (this->isInit()) {
		puts("Not initializing module because it is already initialized");
		return true;
	}
	if (!this->isLoaded() && !this->load())
		return false;
	printf("Init module %S\n", this->path.c_str());

	auto name = this->getName();

	if (!this->CheckVersion(this->hash)) {
		this->lastError = L"Failed loading " + name + L": CheckVersion failed";
		printf("%S\n", this->lastError.c_str());
		FreeLibrary(this->module);
		this->module = nullptr;
		return false;
	}
	if (!this->Initialize(static_cast<HMODULE>(this->module), this->this_module)) {
		this->lastError = L"Failed loading " + name + L": Initialize failed";
		printf("%S\n", this->lastError.c_str());
		FreeLibrary(this->module);
		this->module = nullptr;
		return false;
	}
	this->_init = true;
	return true;
}

bool Module::initChain()
{
	if (!this->init())
		return false;
	return !this->next || this->next->load();
}

void Module::setDefaultPriority()
{
	if (!this->isLoaded())
		this->priority = 0;
	else
		this->priority = this->getPriority ? this->getPriority() : 0;
	this->customPriority = false;
}

bool Module::uninit(bool remove)
{
	return this->unload(remove, false);
}

bool Module::unload(bool remove, bool freeLib, bool fromChain)
{
	printf("Un%s module %S\n", (freeLib ? "load" : "init"), this->path.c_str());
	if (!this->isLoaded())
		return true;
	if (!this->unHook) {
		this->lastError = L"Hook revert is not supported by this mod.<br>Restart your game for the changes to have effect.";
		return false;
	}

	auto chain = !this->hasChainedHooks || this->hasChainedHooks();

	if (chain && this->next)
		if (this->next->isLoaded() && !this->next->unload(false, false, true)) {
			this->lastError = L"Next chain unloading failed";
			return false;
		}
	if (!chain && fromChain)
		return true;
	this->unHook();
	if (freeLib) {
		FreeLibrary(this->module);
		this->module = nullptr;
	}
	this->_init = false;
	if (remove) {
		if (this->prev)
			this->prev->next = this->next;
		if (this->next) {
			this->next->prev = this->prev;
			if (chain && !this->next->initChain())
				return false;
		}
	}
	return true;
}

std::wstring Module::getPath() const
{
	if (this->path.is_relative())
		return std::filesystem::absolute(this->path);

	auto result = std::filesystem::relative(this->path, app_path);

	if (result.empty() || result.native()[0] == '.')
		return std::filesystem::absolute(this->path);
	return result;
}

std::wstring Module::getName() const
{
	auto path = this->path.wstring();
	auto pos1 = path.find_last_of('/');
	auto pos2 = path.find_last_of('\\');
	auto pos = pos1 == std::string::npos ? pos2 : pos2 == std::string::npos ? pos1 : max(pos1, pos2);
	auto name = pos != std::string::npos ? path.substr(pos + 1) : path;

	return name;
}

bool Module::operator<(const Module &other) const {
	if (other.isLoaded() != this->isLoaded())
		return this->isLoaded();
	if (other.enabled != this->enabled)
		return this->enabled;
	if (!this->enabled && this->customPriority != other.customPriority)
		return this->customPriority;
	if (this->enabled || (this->priority != other.priority && this->customPriority))
		return this->priority > other.priority;
	if (!this->enabled)
		return this->getName() < other.getName();
	return false;
}