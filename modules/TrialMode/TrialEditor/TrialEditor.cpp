//
// Created by PinkySmile on 23/07/2021.
//

#include "../Trial/Trial.hpp"
#include "TrialEditor.hpp"
#include "ComboTrialEditor.hpp"
#include "Patches.hpp"

#ifndef _DEBUG
#define puts(...)
#define printf(...)
#endif

bool reloadRequest = false;
static DWORD old;
static double _loopStart, _loopEnd;
static void (__stdcall *og)(int);

const std::map<std::string, std::function<TrialEditor *(const char *folder, const char *path, SokuLib::Character player, const nlohmann::json &json)>> TrialEditor::_factory{
	{ "combo", [](const char *folder, const char *path, SokuLib::Character player, const nlohmann::json &json){ return new ComboTrialEditor(folder, path, player, json); } }
};

static void __stdcall editLoop(int ptr) {
	//int samplePerSec = *reinterpret_cast<int*>(ptr+0x12fc);
	printf("EditLoop with loop params %f %f\n", ::_loopStart, ::_loopEnd);
	*reinterpret_cast<double*>(ptr+0x12e8) = _loopStart;
	*reinterpret_cast<double*>(ptr+0x12f0) = _loopEnd;
	SokuLib::TamperNearJmpOpr(0x418cc5, og);
	VirtualProtect((PVOID)0x418cc5, 5, old, &old);
}

int __fastcall onUpdate(SokuLib::PauseMenu *)
{
	return reinterpret_cast<TrialEditor &>(*loadedTrial).pauseOnUpdate();
}

int __fastcall onRender(SokuLib::PauseMenu *)
{
	return reinterpret_cast<TrialEditor &>(*loadedTrial).pauseOnRender();
}

TrialEditor::TrialEditor(const char *folder, const nlohmann::json &json)
{
	DWORD old;

	applyFileSystemPatch();
	this->_folder = folder;
	if (json.contains("intro") && json["intro"].is_string()) {
		this->_introPath = folder + json["intro"].get<std::string>();
		this->_introRelPath = json["intro"].get<std::string>();
	}
	if (json.contains("outro") && json["outro"].is_string()) {
		this->_outroPath = folder + json["outro"].get<std::string>();
		this->_outroRelPath = json["outro"].get<std::string>();
	}
	if (json.contains("music") && json["music"].is_number()) {
		unsigned t = json["music"];

		this->_music = (t < 10 ? "data/bgm/st0" : "data/bgm/st") + std::to_string(t) + ".ogg";
	} else
		this->_music = getField<std::string>(json, "data/bgm/op.ogg", &nlohmann::json::is_string, "music");
	this->_musicReal = this->_music;
	for (auto pos = this->_music.find("{{pack_path}}"); pos != std::string::npos; pos = this->_music.find("{{pack_path}}"))
		this->_music.replace(pos, strlen("{{pack_path}}"), folder);
	if (json.contains("counter_hit") && json["counter_hit"].is_boolean() && json["counter_hit"].get<bool>())
		this->_counterHit = true, applyCounterHitOnlyPatch();
	if (json.contains("music_loop_start") && json["music_loop_start"].is_number())
		this->_loopStart = json["music_loop_start"];
	if (json.contains("music_loop_end") && json["music_loop_end"].is_number())
		this->_loopEnd = json["music_loop_end"];
	this->_initAnimations();
	::VirtualProtect((PVOID)RDATA_SECTION_OFFSET, RDATA_SECTION_SIZE, PAGE_EXECUTE_READWRITE, &old);
	this->_ogOnUpdate = SokuLib::TamperDword(&SokuLib::VTable_PauseMenu.onProcess, onUpdate);
	this->_ogOnRender = SokuLib::TamperDword(&SokuLib::VTable_PauseMenu.onRender, onRender);
	::VirtualProtect((PVOID)RDATA_SECTION_OFFSET, RDATA_SECTION_SIZE, old, &old);
}

TrialEditor::~TrialEditor()
{
	DWORD old;

	removeFileSystemPatch();
	removeCounterHitOnlyPatch();
	::VirtualProtect((PVOID)RDATA_SECTION_OFFSET, RDATA_SECTION_SIZE, PAGE_EXECUTE_READWRITE, &old);
	SokuLib::TamperDword(&SokuLib::VTable_PauseMenu.onProcess, this->_ogOnUpdate);
	SokuLib::TamperDword(&SokuLib::VTable_PauseMenu.onRender, this->_ogOnRender);
	::VirtualProtect((PVOID)RDATA_SECTION_OFFSET, RDATA_SECTION_SIZE, old, &old);
}

TrialBase *TrialEditor::create(const char *folder, const char *path, SokuLib::Character player, const nlohmann::json &json)
{
	if (!json.contains("type") || !json["type"].is_string())
		return nullptr;
	return TrialEditor::_factory.at(json["type"])(folder, path, player, json);
}

void TrialEditor::_introOnRender() const
{
	if (!this->_intro)
		return;
	this->_intro->render();
}

void TrialEditor::_outroOnRender() const
{
	if (!this->_outro)
		return;
	this->_outro->render();
}

void TrialEditor::_introOnUpdate()
{
	(*reinterpret_cast<char **>(0x8985E8))[0x494] = 22; // Remove HUD
	if (!this->_intro) {
		this->_introPlayed = true;
		this->_playBGM();
		return;
	}

	auto result = this->_intro->update();

	if (!result && !this->_introPlayed)
		this->_playBGM();
	this->_introPlayed |= !result;
	if (SokuLib::inputMgrs.input.a == 1 || SokuLib::inputMgrs.input.b)
		this->_intro->onKeyPressed();
	if (SokuLib::inputMgrs.pause == 1) {
		SokuLib::playSEWaveBuffer(0x29);
		this->_introPlayed = true;
		this->_playBGM();
	};
}

void TrialEditor::_outroOnUpdate()
{
	if (!this->_outro) {
		this->_outroPlayed = true;
		return;
	}
	this->_outroPlayed |= !this->_outro->update();
	if (SokuLib::inputMgrs.input.a == 1 || SokuLib::inputMgrs.input.b)
		this->_outro->onKeyPressed();
	if (SokuLib::inputMgrs.pause == 1) {
		SokuLib::playSEWaveBuffer(0x29);
		this->_outroPlayed = true;
	};
}

void TrialEditor::_initAnimations(bool intro, bool outro)
{
	if (!this->_introPath.empty() && intro)
		try {
			this->_intro.reset(loadBattleAnimation(this->_folder.c_str(), this->_introPath.c_str(), true));
		} catch (std::exception &e) {
			throw std::invalid_argument("Cannot load intro file \"" + this->_introPath + "\":" + e.what());
		}
	if (!this->_outroPath.empty() && outro)
		try {
			this->_outro.reset(loadBattleAnimation(this->_folder.c_str(), this->_outroPath.c_str(), false));
		} catch (std::exception &e) {
			throw std::invalid_argument("Cannot load outro file \"" + this->_outroPath + "\":" + e.what());
		}
}

void TrialEditor::_playBGM()
{
	if (this->_loopEnd == 0 && this->_loopStart == 0)
		return SokuLib::playBGM(this->_music.c_str());
	::_loopStart = this->_loopStart;
	::_loopEnd = this->_loopEnd;
	VirtualProtect((PVOID)0x418cc5, 5, PAGE_EXECUTE_WRITECOPY, &old);
	og = SokuLib::TamperNearJmpOpr(0x418cc5, editLoop);
	SokuLib::playBGM(this->_music.c_str());
}
