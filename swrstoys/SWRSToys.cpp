#include <windows.h>
#include <cstdio>
#include <shlwapi.h>
#include <ModLoader.hpp>
#include <nlohmann/json.hpp>
#include <locale>
#include <codecvt>
#include <fstream>
#include <filesystem>
#include <SokuLib.hpp>
#include <directx/dinput.h>
#include <direct.h>

#define GET_MANDATORY_FUNCTION(name, path, module, modName) \
	do{if (!(this->name = (decltype(name))GetProcAddress(module, #name))) { \
		this->lastError = L"Failed loading " + modName + L": " #name " not found"; \
		return false; \
	}}while(0)
#define GET_FUNCTION(name, module) this->name = (decltype(name))GetProcAddress(module, #name)
#define CRenderer_Unknown1 ((void (__thiscall *)(int, int))0x404AF0)
#define REAL_VERSION_STR "alpha 0.1"
#ifdef _DEBUG
#define VERSION_STR "version " REAL_VERSION_STR " DEBUG"
#else
#define VERSION_STR "version " REAL_VERSION_STR
#endif
#define hasEnglishPatch (*(int *)0x411c64 == 1)

static int (SokuLib::Title::*ogTitleOnProcess)();
static int (SokuLib::Title::*ogTitleOnRender)();

static bool loaded = false;
static SokuLib::SWRFont font;
static SokuLib::DrawUtils::Sprite help;
static SokuLib::DrawUtils::Sprite version;
static HMODULE myModule;

void saveSettings();

void loadAssets()
{
	if (loaded)
		return;
	loaded = true;
	SokuLib::FontDescription desc;

	desc.r1 = 255;
	desc.r2 = 255;
	desc.g1 = 255;
	desc.g2 = 255;
	desc.b1 = 255;
	desc.b2 = 255;
	desc.height = 12 + hasEnglishPatch * 2;
	desc.weight = FW_NORMAL;
	desc.italic = 0;
	desc.shadow = 1;
	desc.bufferSize = 1000000;
	desc.charSpaceX = 0;
	desc.charSpaceY = 0;
	desc.offsetX = 0;
	desc.offsetY = 0;
	desc.useOffset = 0;
	strcpy(desc.faceName, "MonoSpatialModSWR");
	desc.weight = FW_REGULAR;
	font.create();
	font.setIndirect(desc);

	SokuLib::Vector2i size;

	version.texture.createFromText(("SokuModLoader " + std::string(VERSION_STR)).c_str(), font, {300, 150}, &size);
	version.setSize({300, 150});
	version.rect.width = 300;
	version.rect.height = 150;
	version.setPosition({635 - size.x, 5});

	int height = size.y;

	help.texture.createFromText("Press D or Pad 5 to access mod settings", font, {300, 150}, &size);
	help.setSize({300, 150});
	help.rect.width = 300;
	help.rect.height = 150;
	help.setPosition({635 - size.x, 5 + height});
}

static void displaySokuCursor(SokuLib::Vector2i pos, SokuLib::Vector2u size)
{
	SokuLib::Sprite (&CursorSprites)[3] = *(SokuLib::Sprite (*)[3])0x89A6C0;

	//0x443a50 -> Vanilla display cursor
	//We have this here because the vanilla cursor display function has a fixed size
	CursorSprites[0].scale.x = size.x * 0.00195313f;
	CursorSprites[0].scale.y = size.y / 16.f;
	pos.x -= 7;
	CursorSprites[0].render(pos.x, pos.y);
	CRenderer_Unknown1(0x896B4C, 2);
	CursorSprites[1].rotation = *(float *)0x89A450 * 4.00000000f;
	CursorSprites[1].render(pos.x, pos.y + 8.00000000f);
	CursorSprites[2].rotation = -*(float *)0x89A450 * 4.00000000f;
	CursorSprites[2].render(pos.x - 14.00000000f, pos.y - 1.00000000f);
	CRenderer_Unknown1(0x896B4C, 1);
}

LPWSTR GetLastErrorAsString(DWORD errorMessageID)
{
	if (errorMessageID == 0) {
		return nullptr;
	}

	LPWSTR messageBuffer = nullptr;

	FormatMessageW(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		nullptr,
		errorMessageID,
		MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US),
		(LPWSTR)&messageBuffer,
		0,
		nullptr
	);

	return messageBuffer;
}

class Module {
private:
	HMODULE module = nullptr;
	std::wstring path;
	std::wstring lastError;
	byte hash[16];
	HMODULE this_module;
	bool _init = false;

	int (*getPriority)();
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

	SokuModLoader::IValue **(*getConfig)();
	void (*freeConfig)(SokuModLoader::IValue **v);
	bool (*commitConfig)(SokuModLoader::IValue *);
	const char *(*getFailureReason)();

	Module(const wchar_t *path, byte hash[16], HMODULE this_module) : path(path), this_module(this_module)
	{
		memcpy(this->hash, hash, 16);
	}

	std::wstring getLastError()
	{
		return this->lastError;
	}

	bool isInit() const
	{
		return this->_init;
	}

	bool isLoaded() const
	{
		return this->module != nullptr;
	}

	bool supportsUnHook() const
	{
		return this->unHook != nullptr;
	}

	bool load()
	{
		this->lastError = L"";
		if (this->isLoaded()) {
			puts("Not loading module because it is already loaded");
			return true;
		}

		auto name = this->getName();

		printf("Loading module %S\n", this->path.c_str());
		this->module = LoadLibraryW(this->path.c_str());
		if (this->module == nullptr) {
			auto err = GetLastError();
			size_t needed = _snwprintf(nullptr, 0, L"Failed loading %s: GetLastError() 0x%x: %s", name.c_str(), err, GetLastErrorAsString(err));
			auto buf = (wchar_t *)malloc((needed + 1) * sizeof(wchar_t));

			_snwprintf(buf, (needed + 1), L"Failed loading %s: GetLastError() 0x%x: %s", name.c_str(), err, GetLastErrorAsString(err));
			this->lastError = buf;
			free(buf);
			return false;
		}

		GET_MANDATORY_FUNCTION(CheckVersion, this->path, this->module, name);
		GET_MANDATORY_FUNCTION(Initialize,   this->path, this->module, name);
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

	bool init()
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
			FreeLibrary(this->module);
			this->module = nullptr;
			return false;
		}
		if (!this->Initialize(this->module, this->this_module)) {
			this->lastError = L"Failed loading " + name + L": Initialize failed";
			FreeLibrary(this->module);
			this->module = nullptr;
			return false;
		}
		this->_init = true;
		return true;
	}

	bool initChain()
	{
		if (!this->init())
			return false;
		return !this->next || this->next->load();
	}

	void setDefaultPriority()
	{
		if (!this->isLoaded())
			this->load();
		this->priority = this->getPriority ? this->getPriority() : 0;
	}

	bool uninit(bool remove = true)
	{
		return this->unload(remove, false);
	}

	bool unload(bool remove = true, bool freeLib = true, bool fromChain = false)
	{
		printf("Un%s module %S\n", (freeLib ? "load" : "init"), this->path.c_str());
		if (!this->isLoaded())
			return true;
		if (!this->unHook) {
			this->lastError = L"Hook revert is not supported";
			return false;
		}

		auto chain = this->hasChainedHooks && this->hasChainedHooks();

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
				if (!this->next->initChain())
					return false;
			}
		}
		return true;
	}

	std::wstring getPath() const
	{
		return this->path;
	}

	std::wstring getName() const
	{
		auto pos1 = this->path.find_last_of('/');
		auto pos2 = this->path.find_last_of('\\');
		auto pos = pos1 == std::string::npos ? pos2 : pos2 == std::string::npos ? pos1 : max(pos1, pos2);
		auto name = pos != std::string::npos ? this->path.substr(pos + 1) : this->path;

		return name;
	}

	bool operator<(const Module &other) const {
		if (other.isLoaded() != this->isLoaded())
			return this->isLoaded();
		if (other.enabled != this->enabled)
			return this->enabled;
		return this->priority > other.priority;
	}
};

class CryptHash {
public:
	CryptHash(): prov(NULL), hash(NULL) {}
	~CryptHash() {
		if (hash != NULL)
			::CryptDestroyHash(hash);
		if (prov != NULL)
			::CryptReleaseContext(prov, 0);
	}
	bool Initialize() {
		if (FALSE == ::CryptAcquireContextW(&prov, nullptr, nullptr, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT))
			return false;
		if (FALSE == ::CryptCreateHash(prov, CALG_MD5, 0, 0, &hash))
			return false;
		return true;
	}
	bool AddData(const BYTE *buffer, unsigned int buffer_size) {
		return ::CryptHashData(hash, buffer, buffer_size, 0);
	}
	bool GetValue(BYTE *buffer, DWORD buffer_size) const {
		DWORD ret_size = buffer_size;
		if (FALSE == ::CryptGetHashParam(hash, HP_HASHVAL, nullptr, &ret_size, 0) || ret_size != buffer_size)
			return false;
		if (FALSE == ::CryptGetHashParam(hash, HP_HASHVAL, buffer, &ret_size, 0))
			return false;
		return true;
	}

protected:
	HCRYPTPROV prov;
	HCRYPTHASH hash;
};

#define scrollSize 15

BYTE gameHash[16];
std::vector<Module> modules;
Module *head = nullptr;

// convert UTF-8 string to wstring
std::wstring utf8_to_wstring(const std::string& str)
{
	std::wstring_convert<std::codecvt_utf8<wchar_t>> myconv;

	return myconv.from_bytes(str);
}

// convert wstring to UTF-8 string
std::string wstring_to_utf8(const std::wstring &str)
{
	std::wstring_convert<std::codecvt_utf8<wchar_t>> myconv;

	return myconv.to_bytes(str);
}

void reloadModules()
{
	Module *last = nullptr;
	std::ifstream ifs{"currentModule.txt"};
	std::string line;

	std::getline(ifs, line);
	ifs.close();
	for (auto &module : modules) {
		if (!module.enabled)
			continue;

		if (line == wstring_to_utf8(module.getPath())) {
			MessageBoxW(nullptr, (module.getName() + L" has been disabled because it crashed the game last time.\nYou can enable it again in the mod config menu.").c_str(), L"Module disabled", MB_ICONWARNING);
			module.enabled = false;
			saveSettings();
			continue;
		}

		std::ofstream of{"currentModule.txt"};

		of << wstring_to_utf8(module.getPath()) << std::endl;
		of.close();
		if (!module.load())
			MessageBoxW(nullptr, module.getLastError().c_str(), L"Module prepare error", MB_ICONWARNING);
	}
	std::sort(modules.begin(), modules.end());
	for (auto &module : modules) {
		if (!module.enabled)
			break;
		if (!module.getLastError().empty())
			continue;

		std::ofstream of{"currentModule.txt"};

		of << wstring_to_utf8(module.getPath()) << std::endl;
		of.close();
		if (!module.init()) {
			MessageBoxW(nullptr, module.getLastError().c_str(), L"Module init error", MB_ICONERROR);
			continue;
		}

		module.next = nullptr;
		module.prev = last;
		if (last)
			last->next = &module;
		else
			head = &module;
		last = &module;
	}
	if (unlink("currentModule.txt") < 0 && errno != ENOENT)
		MessageBoxA(nullptr, ("Could not delete currentModule.txt: " + std::string(strerror(errno))).c_str(), "Error", MB_ICONERROR);
}

class ModConfigMenu : public SokuLib::IMenu {
private:
	typedef SokuLib::DrawUtils::Sprite SpritePair[2];

	SokuLib::Vector2i _lastErrorSize;
	SokuLib::Vector2i _pathSize;
	bool _expended = false;
	int _selected = 0;
	int _scrollStart = 0;
	SpritePair _path;
	SpritePair _priority;
	SpritePair _enabled;
	SokuLib::DrawUtils::Sprite _lastError;
	SokuLib::DrawUtils::Sprite _title;
	std::list<SokuLib::DrawUtils::Sprite> _text;

public:
	ModConfigMenu()
	{
		this->_title.texture.loadFromResource(myModule, MAKEINTRESOURCE(4));
		this->_title.setSize(this->_title.texture.getSize());
		this->_title.rect.width = this->_title.texture.getSize().x;
		this->_title.rect.height = this->_title.texture.getSize().y;

		this->_enabled[0].texture.createFromText("Disabled", font, {300, 20});
		this->_enabled[0].setSize({200, 20});
		this->_enabled[0].rect.width = 200;
		this->_enabled[0].rect.height = 20;
		this->_enabled[0].tint = SokuLib::Color::Red;
		this->_enabled[0].setPosition({218, 96});

		this->_enabled[1].texture.createFromText("Enabled", font, {300, 20});
		this->_enabled[1].setSize({200, 20});
		this->_enabled[1].rect.width = 200;
		this->_enabled[1].rect.height = 20;
		this->_enabled[1].setPosition({218, 96});

		this->_path[0].texture.createFromText("Path", font, {300, 20});
		this->_path[0].setSize({200, 20});
		this->_path[0].rect.width = 200;
		this->_path[0].rect.height = 20;
		this->_path[0].setPosition({218, 120});

		this->_text.emplace_back();
		this->_text.back().texture.createFromText("Add new module", font, {300, 20});
		this->_text.back().setSize({200, 20});
		this->_text.back().rect.width = 200;
		this->_text.back().rect.height = 20;
		this->_text.back().setPosition({38, 102});

		for (auto &module : modules) {
			std::string str;
			auto name = module.getName();
			auto pos = name.find_last_of('.');

			if (pos != std::string::npos)
				name = name.substr(0, pos);
			str.reserve(name.size());
			for (auto &c : name)
				str += c > 127 ? '?' : c;
			this->_text.emplace_back();
			this->_text.back().texture.createFromText(str.c_str(), font, {300, 20});
			this->_text.back().setSize({200, 20});
			this->_text.back().rect.width = 200;
			this->_text.back().rect.height = 20;
			if (module.enabled)
				this->_text.back().tint = module.isLoaded() ? SokuLib::Color::Green : SokuLib::Color{0xFF, 0x80, 0x00, 0xFF};
			else
				this->_text.back().tint = SokuLib::Color::Red;
		}

		this->_lastError.tint = SokuLib::Color::Red;
	};

	~ModConfigMenu() override = default;

	void _() override
	{
		*(int *)0x882a94 = 0x16;
	}

	void prepareRightPanel()
	{
		if (this->_selected == 0)
			return;

		std::string s;
		std::string l;
		auto &module = modules[this->_selected - 1];
		auto path = module.getPath();
		auto err = module.getLastError();

		s.reserve(path.size() + path.size() / 57);
		for (unsigned i = 0, j = 0; i < path.size(); i++) {
			s += path[i] == '\\' ? '/' : path[i] > 127 ? '?' : path[i];
			j++;
			if (j && j % 57 == 0)
				s += "<br>";
		}
		l.reserve(err.size() + err.size() / 57);
		for (unsigned i = 0, j = 0; i < err.size(); i++) {
			if (err[i] < 32 || (err[i] == ' ' && j == 0))
				continue;
			j++;
			l += err[i] == '\\' ? '/' : err[i] > 127 ? '?' : err[i];
			if (j && j % 57 == 0)
				l += "<br>";
		}

		this->_lastError.texture.createFromText(l.c_str(), font, {400, 200}, &this->_lastErrorSize);
		this->_lastError.setSize({356, 200});
		this->_lastError.rect.width = 356;
		this->_lastError.rect.height = 200;
		this->_lastError.setPosition({284, 96});

		if (this->_lastErrorSize.y == 0)
			this->_lastErrorSize.y = 12;

		this->_path[0].setPosition({218, 106 + this->_lastErrorSize.y});
		this->_path[1].texture.createFromText(s.c_str(), font, {400, 200}, &this->_pathSize);
		this->_path[1].setSize({356, 200});
		this->_path[1].rect.width = 356;
		this->_path[1].rect.height = 200;
		this->_path[1].setPosition({284, 106 + this->_lastErrorSize.y});
	}

	void renderRightPanel()
	{
		if (this->_selected == 0)
			return;

		auto &module = modules[this->_selected - 1];

		this->_path[0].draw();
		this->_path[1].draw();

		if (module.enabled)
			this->_enabled[1].tint = module.isLoaded() ? SokuLib::Color::Green : SokuLib::Color{0xFF, 0x80, 0x00, 0xFF};
		this->_enabled[module.enabled].draw();
		this->_lastError.draw();
	}

	void addNewModule()
	{
		OPENFILENAMEW ofn;         // common dialog box structure
		wchar_t szFile[MAX_PATH];  // buffer for file name
		wchar_t szDir[MAX_PATH];   // buffer for dir name
		char szCWD[MAX_PATH];      // buffer for dir name
		bool result;

		// Initialize OPENFILENAME
		getcwd(szCWD, sizeof(szCWD));
		memset(&ofn, 0, sizeof(ofn));
		memset(&szFile, 0, sizeof(szFile));
		ofn.lStructSize = sizeof(ofn);
		ofn.hwndOwner = SokuLib::window;
		ofn.lpstrFile = szFile;
		ofn.nMaxFile = sizeof(szFile) / sizeof(*szFile);
		ofn.lpstrFilter =
			L"Hisoutensoku module\0*.DLL\0"
			"Module archive\0*.ZIP\0"
			"All files\0*.*\0";
		ofn.nFilterIndex = 1;
		ofn.lpstrFileTitle = nullptr;
		ofn.nMaxFileTitle = 0;
		GetFullPathNameW(L"./Modules", sizeof(szDir) / sizeof(*szDir), szDir, nullptr);
		ofn.lpstrInitialDir = szDir;
		ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
		ofn.lpstrTitle = L"Open module";

		// Display the Open dialog box.
		result = GetOpenFileNameW(&ofn);
		chdir(szCWD);
		if (!result)
			return SokuLib::playSEWaveBuffer(0x29);

		auto ptr = wcschr(ofn.lpstrFile, '.');

		for (int i = 0; ofn.lpstrFile[i]; i++)
			if (ofn.lpstrFile[i] == '\\')
				ofn.lpstrFile[i] = '/';
		for (int i = 0; ptr[i]; i++)
			if (ptr && ptr[i] < 0x80)
				ptr[i] = tolower(ptr[i]);
		if (ptr && wcscmp(ptr, L".zip") == 0) {
			MessageBoxA(nullptr, "Archive files are not supported yet.", "Error", MB_ICONERROR);
			return SokuLib::playSEWaveBuffer(0x29);
		}
		modules.emplace_back(ofn.lpstrFile, gameHash, myModule);

		auto &module = modules.back();
		std::string str;
		auto name = module.getName();
		auto pos = name.find_last_of('.');

		if (pos != std::string::npos)
			name = name.substr(0, pos);
		str.reserve(name.size());
		for (auto &c : name)
			str += c > 127 ? '?' : c;
		this->_text.emplace_back();
		this->_text.back().texture.createFromText(str.c_str(), font, {300, 20});
		this->_text.back().setSize({200, 20});
		this->_text.back().rect.width = 200;
		this->_text.back().rect.height = 20;
		this->_text.back().tint = module.enabled ? SokuLib::Color::Green : SokuLib::Color::Red;
		this->_selected = this->_text.size() - 1;
		if (this->_selected >= this->_scrollStart + scrollSize)
			this->_scrollStart = this->_selected - scrollSize;
		this->prepareRightPanel();
		SokuLib::playSEWaveBuffer(0x28);
		saveSettings();
	}

	int onProcess() override
	{
		if (SokuLib::inputMgrs.input.b == 1 || SokuLib::checkKeyOneshot(DIK_ESCAPE, 0, 0, 0)) {
			SokuLib::playSEWaveBuffer(0x29);
			return false;
		}
		if (SokuLib::inputMgrs.input.a == 1) {
			SokuLib::playSEWaveBuffer(0x28);
			if (this->_selected == 0)
				this->addNewModule();
		}
		if (SokuLib::inputMgrs.input.verticalAxis == -1 || (SokuLib::inputMgrs.input.verticalAxis <= -36 && SokuLib::inputMgrs.input.verticalAxis % 6 == 0)) {
			SokuLib::playSEWaveBuffer(0x27);
			this->_selected--;
			this->_selected += this->_text.size();
			this->_selected %= this->_text.size();
			if (this->_selected <= this->_scrollStart)
				this->_scrollStart = this->_selected ? this->_selected - 1 : 0;
			if (this->_selected >= this->_scrollStart + scrollSize)
				this->_scrollStart = this->_selected - scrollSize;
			this->prepareRightPanel();
		} else if (SokuLib::inputMgrs.input.verticalAxis == 1 || (SokuLib::inputMgrs.input.verticalAxis >= 36 && SokuLib::inputMgrs.input.verticalAxis % 6 == 0)) {
			SokuLib::playSEWaveBuffer(0x27);
			this->_selected++;
			this->_selected %= this->_text.size();
			if (this->_selected < this->_scrollStart)
				this->_scrollStart = this->_selected;
			if (this->_selected >= this->_scrollStart + scrollSize)
				this->_scrollStart = this->_selected - scrollSize;
			this->prepareRightPanel();
		}
		return true;
	}

	int onRender() override
	{
		this->_title.draw();
		//Display the green gradiant cursor bar
		displaySokuCursor(
			{38 + (this->_selected != 0) * 10, 101 + (this->_selected - this->_scrollStart) * 20},
			{200, 16}
		);

		auto it = this->_text.begin();

		it->setPosition({38, 102});
		it->draw();
		for (int i = 0; i <= this->_scrollStart; i++)
			it++;
		for (int i = 0; it != this->_text.end() && i < scrollSize; i++, it++) {
			it->setPosition({48, 122 + i * 20});
			it->draw();
		}
		renderRightPanel();
		return 0;
	}
};

bool getSWRSHash(BYTE retVal[16]) {
	CryptHash hash;
	if (!hash.Initialize()) {
		return false;
	}

	wchar_t swrs_path[MAX_PATH];
	if (FALSE == ::GetModuleFileNameW(nullptr, swrs_path, MAX_PATH)) {
		return false;
	}

	HANDLE file = ::CreateFileW(swrs_path, GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, 0, nullptr);
	if (!file) {
		return false;
	}

	while (true) {
		BYTE buff[1024];
		DWORD readSize;
		if (FALSE == ::ReadFile(file, buff, sizeof(buff), &readSize, nullptr)) {
			::CloseHandle(file);
			return false;
		}
		if (readSize == 0) {
			break;
		}

		if (!hash.AddData(buff, readSize)) {
			::CloseHandle(file);
			return false;
		}
	}
	::CloseHandle(file);

	if (!hash.GetValue(retVal, 16)) {
		return false;
	}
	return true;
}

void saveSettings()
{
	nlohmann::json json;
	nlohmann::json moduleObj;

	for (auto &module : modules) {
		nlohmann::json moduleJson;

		moduleJson["enabled"] = module.enabled;
		if (module.customPriority)
			moduleJson["priority"] = module.priority;
		moduleObj[wstring_to_utf8(module.getPath())] = moduleJson;
	}
	json["modules"] = moduleObj;

	auto str = json.dump(4);
	std::ofstream stream("ModLoaderSettings.json");

	stream << str;
}

void loadSettings(byte hash[16], HMODULE this_module)
{
	std::ifstream stream("ModLoaderSettings.json");
	nlohmann::json json;

	if (!stream.is_open()) {
		MessageBoxA(nullptr, ("ModLoaderSettings.json could not be loaded: " + std::string(strerror(errno))).c_str(), "Settings loading error", MB_ICONERROR);
		return;
	}

	stream >> json;
	for (auto &module : json["modules"].items()) {
		auto val = module.value();
		auto path = utf8_to_wstring(module.key());
		auto enabled = val.contains("enabled") && val["enabled"].is_boolean() && val["enabled"];
		auto moduleIt = std::find_if(modules.begin(), modules.end(), [&path](const Module &module) {
			return module.getPath() == path;
		});

		if (moduleIt == modules.end()) {
			DWORD attrs = GetFileAttributesW(path.c_str());

			if (attrs == INVALID_FILE_ATTRIBUTES || (attrs & FILE_ATTRIBUTE_DIRECTORY))
				continue;
			modules.emplace_back(path.c_str(), hash, this_module);
			moduleIt = std::prev(modules.end());
		}
		moduleIt->enabled = enabled;
		if (val.contains("priority") && val["priority"].is_number_integer()) {
			moduleIt->customPriority = true;
			moduleIt->priority = val["priority"];
		}
	}
}

void convertIni(byte hash[16], HMODULE this_module)
{
	wchar_t app_path[MAX_PATH];
	wchar_t setting_path[MAX_PATH];

	GetModuleFileNameW(this_module, app_path, MAX_PATH);
	PathRemoveFileSpecW(app_path);
	if (GetEnvironmentVariableW(L"SWRSTOYS", setting_path, sizeof(setting_path)) <= 0) {
		int argc;
		wchar_t **arg_list = CommandLineToArgvW(GetCommandLineW(), &argc);

		if (arg_list && argc > 1 && StrStrIW(arg_list[1], L"ini")) {
			wcscpy(setting_path, arg_list[1]);
			LocalFree(arg_list);
		} else {
			wcscpy(setting_path, app_path);
			PathAppendW(setting_path, L"\\SWRSToys.ini");
		}
		if (arg_list)
			LocalFree(arg_list);
	}

	wchar_t moduleKeys[1024];
	wchar_t moduleValue[MAX_PATH];

	GetPrivateProfileStringW(L"Module", nullptr, nullptr, moduleKeys, sizeof(moduleKeys), setting_path);
	for (wchar_t *key = moduleKeys; *key; key += wcslen(key) + 1) {
		GetPrivateProfileStringW(L"Module", key, nullptr, moduleValue, sizeof(moduleValue), setting_path);
		while (auto ptr = wcschr(moduleValue, L'/'))
			*ptr = L'\\';

		auto module = std::find_if(modules.begin(), modules.end(), [&](const Module &m) {
			try {
				return std::filesystem::equivalent(m.getPath(), moduleValue);
			} catch(std::filesystem::filesystem_error const& ex) {
				return false;
			}
		});

		if (module == modules.end()) {
			DWORD attrs = GetFileAttributesW(moduleValue);

			if (attrs == INVALID_FILE_ATTRIBUTES || (attrs & FILE_ATTRIBUTE_DIRECTORY)) {
				MessageBoxW(nullptr, (L"Module " + std::wstring(moduleValue) + L" specified in old INI is not a valid file\nIt will be ignored").c_str(), L"Module loading error", MB_ICONWARNING);
				continue;
			}
			modules.emplace_back(Module(moduleValue, hash, this_module));
			module = std::prev(modules.end());
		}
		module->enabled = true;
	}
	saveSettings();
}

bool exists(const char *path)
{
	DWORD attrs = GetFileAttributesA(path);

	return (attrs != INVALID_FILE_ATTRIBUTES && !(attrs & FILE_ATTRIBUTE_DIRECTORY));
}

void loadModuleList(byte hash[16], HMODULE this_module)
{
	WIN32_FIND_DATAW findData;
	auto ret = FindFirstFileW(L"Modules/*", &findData);

	if (ret == INVALID_HANDLE_VALUE)
		return;
	do {
		if (!(findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
			continue;

		wchar_t module_path[MAX_PATH];

		wcscpy(module_path, L"Modules\\");
		PathAppendW(module_path, findData.cFileName);
		PathAppendW(module_path, findData.cFileName);
		wcscat(module_path, L".dll");

		DWORD attrs = GetFileAttributesW(module_path);

		if (attrs != INVALID_FILE_ATTRIBUTES && !(attrs & FILE_ATTRIBUTE_DIRECTORY))
			modules.emplace_back(module_path, hash, this_module);
	} while (FindNextFileW(ret, &findData));
}

int __fastcall myTitleOnProcess(SokuLib::Title *This)
{
	int ret = (This->*ogTitleOnProcess)();

	loadAssets();
	if (SokuLib::inputMgrs.input.spellcard == 1 && SokuLib::menuManager.empty()) {
		SokuLib::playSEWaveBuffer(0x3D);
		SokuLib::activateMenu(new ModConfigMenu());
	}
	return ret;
}

int __fastcall myTitleOnRender(SokuLib::Title *This)
{
	int ret = (This->*ogTitleOnRender)();

	version.draw();
	help.draw();
	return ret;
}

bool Hook(HMODULE this_module)
{
#ifdef _DEBUG
	FILE *_;

	AllocConsole();
	freopen_s(&_, "CONOUT$", "w", stdout);
	freopen_s(&_, "CONOUT$", "w", stderr);
#endif

	myModule = this_module;
	puts("SokuModLoader " VERSION_STR);
	try {
		DWORD old;

		if (!getSWRSHash(gameHash))
			return false;
		puts("Discovering modules in Modules folder");
		loadModuleList(gameHash, this_module);
		if (!exists("ModLoaderSettings.json")) {
			puts("No settings file found, creating one from SWRSToys.ini");
			convertIni(gameHash, this_module);
		}
		puts("Loading settings file");
		loadSettings(gameHash, this_module);
		puts("Placing hooks");
		VirtualProtect((PVOID)RDATA_SECTION_OFFSET, RDATA_SECTION_SIZE, PAGE_EXECUTE_WRITECOPY, &old);
		ogTitleOnProcess  = SokuLib::TamperDword(&SokuLib::VTable_Title.onProcess, myTitleOnProcess);
		ogTitleOnRender   = SokuLib::TamperDword(&SokuLib::VTable_Title.onRender,  myTitleOnRender);
		VirtualProtect((PVOID)RDATA_SECTION_OFFSET, RDATA_SECTION_SIZE, old, &old);
		reloadModules();
	} catch (std::exception &e) {
		MessageBoxA(nullptr, e.what(), "Exception", MB_ICONERROR);
		return false;
	}
	return true;
}
