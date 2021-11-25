//
// Created by PinkySmile on 19/07/2021.
//

#ifndef SWRSTOYS_MENU_HPP
#define SWRSTOYS_MENU_HPP


#include <memory>
#include <SokuLib.hpp>
#include <nlohmann/json.hpp>
#include "TrialBase.hpp"
#include "PackOutro.hpp"

class ResultMenu : public SokuLib::IMenu {
private:
	bool _done = true;
	int _selected = 0;
	SokuLib::DrawUtils::Sprite _title;
	SokuLib::DrawUtils::Sprite _score;
	SokuLib::DrawUtils::Sprite _resultTop;
	std::array<SokuLib::DrawUtils::Sprite, TrialBase::NB_MENU_ACTION> _text;

protected:
	bool _disabled = false;

public:
	ResultMenu(int score);
	~ResultMenu() override = default;
	void _() override;
	int onProcess() override;
	int onRender() override;
};

extern char profilePath[1024 + MAX_PATH];
extern char profileFolderPath[1024 + MAX_PATH];
extern HMODULE myModule;
extern SokuLib::SWRFont defaultFont10;
extern SokuLib::SWRFont defaultFont12;
extern SokuLib::SWRFont defaultFont16;
extern bool loadRequest;
extern std::unique_ptr<TrialBase> loadedTrial;
extern std::unique_ptr<PackOutro> loadedOutro;
extern bool editorMode;
extern unsigned loading;

void menuLoadAssets();
int menuOnProcess(SokuLib::MenuResult *This);
void menuOnRender(SokuLib::MenuResult *This);
void menuUnloadAssets();
std::vector<unsigned> getCurrentPackScores();

template<typename T, typename ...Args>
T getField(nlohmann::json json, T defaultValue, bool (nlohmann::json::*checker)() const, std::string field, Args... fields)
{
	std::vector<std::string> vec{fields...};

	if (!json.is_object())
		return defaultValue;
	for (auto &val : vec) {
		if (!json.contains(val) || !json[val].is_object())
			return defaultValue;
		json = json[val];
	}
	if (!json.contains(field) || !(json[field].*checker)())
		return defaultValue;
	return json[field];
}

#endif //SWRSTOYS_MENU_HPP
