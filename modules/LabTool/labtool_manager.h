#pragma once

#include "stdafx.h"

#define RET_BOOL_IF_FALSE(Func, res) \
	res = Func; \
	if (!res) { \
		return res; \
	}

#define RET_VOID_IF_FALSE(Func) \
	if (!Func) { \
		return; \
	}

extern bool enable_practice;
extern bool enable_versus;
extern bool enable_replays;

class PlayerImpl {
public:
	void setData(unsigned offset);
	void *data();
	const std::string &getGirlName() const;
	short &currentSeq();
	bool isCurrSeqChange() const;

private:
	void *_data;
	int _index = 0;
	short _currentSeq = 0;
};

using PlayerImplPtr = std::unique_ptr<PlayerImpl>;

class LabToolConsole {
public:
	LabToolConsole();
	~LabToolConsole();

	LabToolConsole(const LabToolConsole &) = delete;
	LabToolConsole(LabToolConsole &&) = delete;
	LabToolConsole &operator=(const LabToolConsole &) = delete;
	LabToolConsole &operator=(LabToolConsole &&) = delete;

private:
	std::streambuf *_cinBuffer = nullptr;
	std::streambuf *_coutBuffer = nullptr;
	std::streambuf *_cerrBuffer = nullptr;
	std::fstream _consoleInput;
	std::fstream _consoleOutput;
	std::fstream _consoleError;
};

// Class manager
class LabToolManager {
public:
	LabToolManager() = default;
	~LabToolManager() = default;
	LabToolManager(const LabToolManager &) = delete;
	LabToolManager(LabToolManager &&) = delete;
	LabToolManager &operator=(const LabToolManager &) = delete;
	LabToolManager &operator=(LabToolManager &&) = delete;

	static LabToolManager &getInstance();
	static HWND getSokuHandle();
	static bool isHisoutensokuOnTop();

	void create();
	void destruct();
	bool isValidMode() const;

	const PlayerImplPtr &getPlayerMain();
	const PlayerImplPtr &getPlayerSecond();

private:
	enum class EMode { eUndefined = 0, ePractice, eVsPlayer, eReplay };

private:
	bool fetchCurrentMode();

private:
	PlayerImplPtr _pPlayerMain;
	PlayerImplPtr _pPlayerSecond;
	std::unique_ptr<LabToolConsole> _pConsole;
	EMode _currentMode = EMode::eUndefined;
};

void welcomeScreen();