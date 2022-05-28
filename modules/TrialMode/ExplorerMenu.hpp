//
// Created by Gegel85 on 26/05/2022.
//

#ifndef SWRSTOYS_EXPLORERMENU_HPP
#define SWRSTOYS_EXPLORERMENU_HPP


#include <vector>
#include <memory>
#include <SokuLib.hpp>
#include "Pack.hpp"

class ExplorerMenu : public SokuLib::IMenu {
private:
	std::vector<std::shared_ptr<Pack>> _loadedPacks;
	SokuLib::DrawUtils::Sprite _title;
	SokuLib::DrawUtils::Sprite _lockedText;
	unsigned _shownPack = 0;
	int _currentPack = -3;
	int _currentEntry = -1;
	bool _loading = true;
	int _nameFilter = -1;
	int _modeFilter = -1;
	int _topicFilter = -1;
	unsigned _packStart = 0;
	std::vector<std::string> _uniqueNames;
	std::vector<std::string> _uniqueModes;
	std::vector<std::string> _uniqueCategories;
	std::map<std::string, std::vector<std::shared_ptr<Pack>>> _packsByName;
	std::map<std::string, std::vector<std::shared_ptr<Pack>>> _packsByMode;
	std::map<std::string, std::vector<std::shared_ptr<Pack>>> _packsByCategory;
	std::string _link = "https://www.server.example";
	std::thread _loadingThread;

	void _loadPacks();
	bool _isLocked(int entry);
	void _renderOnePackBack(Pack &pack, SokuLib::Vector2<float> &pos, bool deployed);
	void _renderOnePack(Pack &pack, SokuLib::Vector2<float> &pos, bool deployed);
public:
	ExplorerMenu();
	~ExplorerMenu() override;
	void _() override;
	int onProcess() override;
	int onRender() override;
};

#endif //SWRSTOYS_EXPLORERMENU_HPP
