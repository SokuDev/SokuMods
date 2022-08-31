//
// Created by PinkySmile on 26/05/2022.
//

#ifndef SWRSTOYS_EXPLORERMENU_HPP
#define SWRSTOYS_EXPLORERMENU_HPP


#include <vector>
#include <memory>
#include <SokuLib.hpp>
#include "Pack.hpp"
#include "Menu.hpp"

#define MAX_STEP 4
#define str2(x) #x
#define str(x) str2(x)

class ExplorerMenu : public SokuLib::IMenu {
private:
	std::vector<std::shared_ptr<Pack>> _loadedPacks;
	SokuLib::Vector2i _nameFilterSize;
	SokuLib::Vector2i _modeFilterSize;
	SokuLib::Vector2i _topicFilterSize;
	SokuLib::DrawUtils::Sprite _title;
	SokuLib::DrawUtils::Sprite _lockedText;
	SokuLib::DrawUtils::Sprite _nameFilterText;
	SokuLib::DrawUtils::Sprite _modeFilterText;
	SokuLib::DrawUtils::Sprite _topicFilterText;
	SokuLib::DrawUtils::Sprite _loadingText[MAX_STEP + 1];
	SokuLib::DrawUtils::Sprite _messageBox;
	unsigned _shownPack = 0;
	unsigned _loadingStep = 0;
	int _currentPack = -3;
	int _currentEntry = -1;
	bool _loading = true;
	bool _message = false;
	bool _error = false;
	int _nameFilter = -1;
	int _modeFilter = -1;
	int _topicFilter = -1;
	bool _expended = false;
	unsigned _entryStart = 0;
	unsigned _packStart = 0;
	std::mutex _downloadMutex;
	std::vector<std::string> _uniqueNames;
	std::vector<std::string> _uniqueModes;
	std::vector<std::string> _uniqueCategories;
	std::map<std::string, std::vector<std::shared_ptr<Pack>>> _packsByName;
	std::map<std::string, std::vector<std::shared_ptr<Pack>>> _packsByMode;
	std::map<std::string, std::vector<std::shared_ptr<Pack>>> _packsByCategory;
	std::thread _loadingThread;
	std::string _linkDownload;
	Guide _guides[2];

	void _loadPacks();
	bool _isLocked(int entry);
	void _renderOnePackBack(Pack &pack, SokuLib::Vector2<float> &pos, bool deployed);
	void _renderOnePack(Pack &pack, SokuLib::Vector2<float> &pos, bool deployed);
	std::string _downloadPackList();
	void _handlePlayerInputs(const SokuLib::KeyInput &input);
	void _checkScrollDown();
	void _checkScrollUp();
	void _handleGoLeft();
	void _handleGoRight();
	void _handleGoUp();
	void _handleGoDown();
	void _displayFilters();
	void _downloadPack(const std::string &link);
public:
	ExplorerMenu();
	~ExplorerMenu() override;
	void _() override;
	int onProcess() override;
	int onRender() override;
};

#endif //SWRSTOYS_EXPLORERMENU_HPP
