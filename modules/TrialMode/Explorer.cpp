//
// Created by PinkySmile on 12/31/2021.
//

#include <map>
#include <SokuLib.hpp>
#include <dinput.h>
#include <direct.h>
#include "Menu.hpp"
#include "Explorer.hpp"

#define EXPLORER_SCREEN_SIZE 17

struct Data {
	const char *resourceName;
	int packageIndex;
	int size;
	int offset;
	int unknown;
};

struct DataTree {
	std::string resourceName;
	std::string ext;
	std::map<std::string, struct DataTree> subfiles;
};

struct ExplorerFileEntry {
	std::string name;
	std::string fullPath;
	std::string nameCap;
	SokuLib::DrawUtils::Sprite pathSprite;
	std::map<std::string, DataTree> *entry;
	unsigned extension;
};

struct ExplorerReturnEntry {
	std::string path;
	unsigned cursor;
	unsigned top;
	std::map<std::string, DataTree> *entry;
};

bool explorerShown = false;

static std::map<std::string, std::map<std::string, DataTree>> gameAssets;
static SokuLib::DrawUtils::Sprite fileExplorerSeat;
static SokuLib::DrawUtils::Sprite explorerFileName;
static SokuLib::DrawUtils::Sprite explorerFileSize;
static SokuLib::DrawUtils::Sprite explorerPathSprite;
static SokuLib::DrawUtils::Sprite explorerImage;
static std::string explorerPath;
static std::function<void()> defaultMusic;
static unsigned explorerCursor;
static unsigned explorerTop;
static std::vector<std::unique_ptr<ExplorerFileEntry>> explorerEntries;
static std::vector<ExplorerReturnEntry> explorerReturnEntries;
static std::map<std::string, DataTree> *currentFolder;
static std::function<void (std::string)> explorerLoader;
static SokuLib::DrawUtils::Sprite extSprites[10];
static SokuLib::DrawUtils::Sprite extType[10];
static SokuLib::DrawUtils::RectangleShape explorerImageRect;
static bool explorerPlayingMusic = false;
static const std::map<std::string, int> exts = {
	{"cv0", 1}, // Dialog
	{"cv1", 0}, // Csv
	{"cv2", 3}, // Image
	{"cv3", 6}, // Sfx
	{"ogg", 4}, // Music
	{"pal", 5}, // Palette
	{"pat", 7}, // Player framedata
	{"dat", 8}, // Menu layout
};
static const char *extTypeStrs[] = {
	"Type: Csv",
	"Type: Dialog",
	"Type: Folder",
	"Type: Image",
	"Type: Music",
	"Type: Palette",
	"Type: Sfx",
	"Type: Character Data",
	"Type: Menu Layout",
	"Type: Unknown"
};


void openFileDialog(const char *types, const std::string &startDir, const std::function<void(const std::string &path)> &fct)
{
	OPENFILENAME ofn;       // common dialog box structure
	char szFile[MAX_PATH];  // buffer for file name
	char szDir[MAX_PATH];   // buffer for dir name
	char szCWD[MAX_PATH];   // buffer for dir name
	bool result;

	// Initialize OPENFILENAME
	getcwd(szCWD, sizeof(szCWD));
	memset(&ofn, 0, sizeof(ofn));
	memset(&szFile, 0, sizeof(szFile));
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = SokuLib::window;
	ofn.lpstrFile = szFile;
	ofn.nMaxFile = sizeof(szFile);
	ofn.lpstrFilter = types;
	ofn.nFilterIndex = 1;
	ofn.lpstrFileTitle = nullptr;
	ofn.nMaxFileTitle = 0;
	GetFullPathNameA(startDir.c_str(), sizeof(szDir), szDir, nullptr);
	ofn.lpstrInitialDir = szDir;
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
	ofn.lpstrTitle = "Open image file";

	// Display the Open dialog box.
	result = GetOpenFileName(&ofn);
	chdir(szCWD);
	if (!result)
		return SokuLib::playSEWaveBuffer(0x29);
	if (fct)
		fct(ofn.lpstrFile);
}


static void addFileToTree(const char *elem)
{
	std::vector<std::string> path;

	while (auto p = strchr(elem, '/')) {
		path.emplace_back(elem, p);
		elem = p + 1;
	}

	std::string ext{strchr(elem, '.') + 1, elem + strlen(elem)};
	auto *tree = &gameAssets[ext];
	auto *tree2 = &gameAssets["All files"];

	for (auto &folder : path) {
		tree  = &(*tree )[folder].subfiles;
		tree2 = &(*tree2)[folder].subfiles;
	}
	(*tree )[elem].resourceName = elem;
	(*tree2)[elem].resourceName = elem;
	(*tree )[elem].ext = ext;
	(*tree2)[elem].ext = ext;
}

static void getAssetTree()
{
	auto mapA = reinterpret_cast<SokuLib::Map<int, Data> *>(0x8a0078);

	if (!gameAssets.empty())
		return;
	puts("Fetching asset list");
	for (auto &v : *mapA)
		addFileToTree(v.second.resourceName);
}

static void playDefaultMusic()
{
	if (!explorerPlayingMusic)
		return;
	explorerPlayingMusic = false;
	if (!defaultMusic)
		SokuLib::playBGM("data/bgm/op2.ogg");
	else
		defaultMusic();
}

static void renderExplorerPreview()
{
	SokuLib::Vector2i size;

	explorerFileName.texture.createFromText(explorerEntries[explorerCursor]->nameCap.c_str(), defaultFont12, {400, 16}, &size);
	explorerFileName.setSize(explorerFileName.texture.getSize());
	explorerFileName.rect.width = explorerFileName.texture.getSize().x;
	explorerFileName.rect.height = explorerFileName.texture.getSize().y;
	explorerFileName.setPosition({447 - size.x / 2, 86});

	explorerImage.texture.destroy();
	explorerFileSize.texture.destroy();
	if (explorerEntries[explorerCursor]->extension == 4) {
		explorerPlayingMusic = true;
		SokuLib::playBGM(explorerEntries[explorerCursor]->fullPath.c_str());
		return;
	} else
		playDefaultMusic();
	if (explorerEntries[explorerCursor]->extension == 3 && explorerImage.texture.loadFromGame(explorerEntries[explorerCursor]->fullPath.c_str())) {
		explorerFileSize.texture.createFromText(
			(std::to_string(explorerImage.texture.getSize().x) + "x" + std::to_string(explorerImage.texture.getSize().y)).c_str(),
			defaultFont12, {400, 16}, &size
		);
		explorerFileSize.setPosition({447 - size.x / 2, 236});
		explorerFileSize.setSize(explorerFileSize.texture.getSize());
		explorerFileSize.rect.width = explorerFileSize.getSize().x;
		explorerFileSize.rect.height = explorerFileSize.getSize().y;

		SokuLib::Vector2f optimalSize{188, 128};
		SokuLib::Vector2f scale{
			optimalSize.x / explorerImage.texture.getSize().x,
			optimalSize.y / explorerImage.texture.getSize().y
		};
		float best = min(scale.x, scale.y);

		explorerImage.setSize((explorerImage.texture.getSize() * best).to<unsigned>());
		explorerImage.setPosition({
			static_cast<int>(354 + optimalSize.x / 2 - explorerImage.getSize().x / 2),
			static_cast<int>(106 + optimalSize.y / 2 - explorerImage.getSize().y / 2)
		});
		explorerImage.rect.width = explorerImage.texture.getSize().x;
		explorerImage.rect.height = explorerImage.texture.getSize().y;
		explorerImageRect.setSize(explorerImage.getSize());
		explorerImageRect.setPosition(explorerImage.getPosition());
	}
}

void loadExplorerEntries()
{
	explorerTop = 0;
	explorerCursor = 0;
	explorerEntries.clear();
	explorerEntries.reserve(currentFolder->size());
	explorerPathSprite.texture.createFromText(explorerPath.c_str(), defaultFont12, {300, 16});
	explorerPathSprite.setPosition({109, 56});
	explorerPathSprite.setSize(explorerPathSprite.texture.getSize());
	explorerPathSprite.rect.width = explorerPathSprite.texture.getSize().x;
	explorerPathSprite.rect.height = explorerPathSprite.texture.getSize().y;
	for (auto &pair : *currentFolder) {
		auto entry = new ExplorerFileEntry();

		explorerEntries.emplace_back(entry);
		entry->name = pair.first;
		entry->nameCap = pair.first;
		for (auto &c : entry->name)
			c = tolower(c);
		entry->fullPath = explorerPath + pair.first;
		if (pair.second.subfiles.empty())
			entry->extension = exts.find(pair.second.ext) == exts.end() ? 9 : exts.at(pair.second.ext);
		else
			entry->extension = 2;
		entry->pathSprite.texture.createFromText(pair.first.c_str(), defaultFont12, {300, 16});
		entry->pathSprite.setSize(entry->pathSprite.texture.getSize());
		entry->pathSprite.rect.width = entry->pathSprite.texture.getSize().x;
		entry->pathSprite.rect.height = entry->pathSprite.texture.getSize().y;
		entry->entry = &pair.second.subfiles;
	}
	std::sort(explorerEntries.begin(), explorerEntries.end(), [](const std::unique_ptr<ExplorerFileEntry> &entry1, const std::unique_ptr<ExplorerFileEntry> &entry2){
		if ((entry1->extension == 2 || entry2->extension == 2) && entry1->extension != entry2->extension)
			return entry1->extension == 2;
		return entry1->name < entry2->name;
	});
	renderExplorerPreview();
}

void loadExplorerRoot(const std::string &root)
{
	SokuLib::playSEWaveBuffer(0x28);
	getAssetTree();
	explorerReturnEntries.clear();
	explorerPath.clear();
	currentFolder = &gameAssets[root];
	loadExplorerEntries();
	explorerShown = true;
}

void loadExplorerFile(const std::string &path, const std::string &root)
{
	std::string str = path;

	loadExplorerRoot(root);
	while (!str.empty()) {
		auto pos = str.find('/');
		std::string token;

		if (pos == std::string::npos) {
			token = str;
			str.clear();
		} else {
			token = str.substr(0, pos);
			str = str.substr(pos + 1);
		}

		auto it = std::find_if(explorerEntries.begin(), explorerEntries.end(), [&token](std::unique_ptr<ExplorerFileEntry> &entry) {
			return entry->name == token;
		});

		if (it == explorerEntries.end())
			break;
		explorerCursor = it - explorerEntries.begin();
		if ((*it)->extension == 2) {
			explorerReturnEntries.push_back({explorerPath, explorerCursor, explorerTop, currentFolder});
			explorerPath = explorerEntries[explorerCursor]->fullPath + "/";
			currentFolder = explorerEntries[explorerCursor]->entry;
			loadExplorerEntries();
		} else {
			if (explorerCursor >= explorerTop + EXPLORER_SCREEN_SIZE)
				explorerTop = explorerCursor - (EXPLORER_SCREEN_SIZE - 1);
			break;
		}
	}
	renderExplorerPreview();
}

void explorerRender()
{
	if (!explorerShown)
		return;

	fileExplorerSeat.draw();
	explorerPathSprite.draw();
	for (int i = explorerTop, j = 0; i < explorerEntries.size() && j < EXPLORER_SCREEN_SIZE; j++, i++) {
		if (i == explorerCursor)
			displaySokuCursor({130, 86 + 20 * j}, {300, 16});
		explorerEntries[i]->pathSprite.setPosition({153, 88 + 20 * j});
		explorerEntries[i]->pathSprite.draw();
		extSprites[explorerEntries[i]->extension].setPosition({133, 86 + 20 * j});
		extSprites[explorerEntries[i]->extension].draw();
	}
	extType[explorerEntries[explorerCursor]->extension].draw();
	explorerFileName.draw();
	if (explorerImage.texture.hasTexture()) {
		explorerImage.draw();
		explorerImageRect.draw();
	}
	if (explorerFileSize.texture.hasTexture())
		explorerFileSize.draw();
}

void explorerUpdate()
{
	if (!explorerShown)
		return;

	if (SokuLib::checkKeyOneshot(DIK_ESCAPE, false, false, false)) {
		SokuLib::playSEWaveBuffer(0x29);
		explorerShown = false;
		playDefaultMusic();
		return;
	}
	if (SokuLib::inputMgrs.input.b == 1) {
		SokuLib::playSEWaveBuffer(0x29);
		if (!explorerReturnEntries.empty()) {
			explorerPath = explorerReturnEntries.back().path;
			currentFolder = explorerReturnEntries.back().entry;
			loadExplorerEntries();
			explorerCursor = explorerReturnEntries.back().cursor;
			explorerTop = explorerReturnEntries.back().top;
			renderExplorerPreview();
			explorerReturnEntries.pop_back();
		} else  {
			explorerShown = false;
			playDefaultMusic();
			return;
		}
	}
	if (!explorerEntries.empty()) {
		if (SokuLib::inputMgrs.input.a == 1) {
			SokuLib::playSEWaveBuffer(0x28);
			if (explorerEntries[explorerCursor]->extension == 2) {
				explorerReturnEntries.push_back({explorerPath, explorerCursor, explorerTop, currentFolder});
				explorerPath = explorerEntries[explorerCursor]->fullPath + "/";
				currentFolder = explorerEntries[explorerCursor]->entry;
				loadExplorerEntries();
			} else {
				explorerLoader(explorerEntries[explorerCursor]->fullPath);
				explorerShown = false;
				playDefaultMusic();
				return;
			}
		}
		if (SokuLib::inputMgrs.input.verticalAxis == -1 || (SokuLib::inputMgrs.input.verticalAxis < -36 && SokuLib::inputMgrs.input.verticalAxis % 6 == 0)) {
			SokuLib::playSEWaveBuffer(0x27);
			if (explorerCursor == 0)
				explorerCursor = explorerEntries.size() - 1;
			else
				explorerCursor--;
			renderExplorerPreview();
		} else if (SokuLib::inputMgrs.input.verticalAxis == 1 || (SokuLib::inputMgrs.input.verticalAxis > 36 && SokuLib::inputMgrs.input.verticalAxis % 6 == 0)) {
			SokuLib::playSEWaveBuffer(0x27);
			explorerCursor++;
			explorerCursor %= explorerEntries.size();
			renderExplorerPreview();
		}
		if (SokuLib::inputMgrs.input.horizontalAxis == -1 || (SokuLib::inputMgrs.input.horizontalAxis < -36 && SokuLib::inputMgrs.input.horizontalAxis % 6 == 0)) {
			SokuLib::playSEWaveBuffer(0x27);
			if (explorerCursor < EXPLORER_SCREEN_SIZE)
				explorerCursor = 0;
			else
				explorerCursor -= EXPLORER_SCREEN_SIZE;
			renderExplorerPreview();
		} else if (SokuLib::inputMgrs.input.horizontalAxis == 1 || (SokuLib::inputMgrs.input.horizontalAxis > 36 && SokuLib::inputMgrs.input.horizontalAxis % 6 == 0)) {
			SokuLib::playSEWaveBuffer(0x27);
			explorerCursor += EXPLORER_SCREEN_SIZE;
			if (explorerCursor > explorerEntries.size() - 1)
				explorerCursor = explorerEntries.size() - 1;
			renderExplorerPreview();
		}
		if (explorerCursor < explorerTop)
			explorerTop = explorerCursor;
		if (explorerCursor >= explorerTop + EXPLORER_SCREEN_SIZE)
			explorerTop = explorerCursor - (EXPLORER_SCREEN_SIZE - 1);
	}
}


void explorerLoadAssets()
{
	for (int i = 0; i < sizeof(extSprites) / sizeof(*extSprites); i++) {
		extSprites[i].texture.loadFromResource(myModule, MAKEINTRESOURCE(408 + i * 4));
		extSprites[i].setSize(extSprites[i].texture.getSize());
		extSprites[i].rect.width = extSprites[i].texture.getSize().x;
		extSprites[i].rect.height = extSprites[i].texture.getSize().y;
	}

	for (int i = 0; i < sizeof(extType) / sizeof(*extType); i++) {
		SokuLib::Vector2i size;

		extType[i].texture.createFromText(extTypeStrs[i], defaultFont12, {400, 16}, &size);
		extType[i].setSize(extType[i].texture.getSize());
		extType[i].rect.width = extType[i].texture.getSize().x;
		extType[i].rect.height = extType[i].texture.getSize().y;
		extType[i].setPosition({447 - size.x / 2, 266});
	}

	fileExplorerSeat.texture.loadFromResource(myModule, MAKEINTRESOURCE(404));
	fileExplorerSeat.setSize(fileExplorerSeat.texture.getSize());
	fileExplorerSeat.rect.width = fileExplorerSeat.texture.getSize().x;
	fileExplorerSeat.rect.height = fileExplorerSeat.texture.getSize().y;
	fileExplorerSeat.setPosition({
		(640 - static_cast<int>(fileExplorerSeat.getSize().x)) / 2,
		(480 - static_cast<int>(fileExplorerSeat.getSize().y)) / 2
	});

	explorerImageRect.setFillColor(SokuLib::Color::Transparent);
	explorerImageRect.setBorderColor(SokuLib::Color::Black);
}

void explorerUnloadAssets()
{
	fileExplorerSeat.texture.destroy();
	explorerFileName.texture.destroy();
	explorerFileSize.texture.destroy();
	explorerPathSprite.texture.destroy();
	explorerImage.texture.destroy();
}

void setExplorerCallback(const std::function<void (const std::string &path)> &fct)
{
	explorerLoader = fct;
}

void setExplorerDefaultMusic(const std::function<void ()> &player)
{
	defaultMusic = player;
}