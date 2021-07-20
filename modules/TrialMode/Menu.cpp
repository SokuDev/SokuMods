//
// Created by PinkySmile on 19/07/2021.
//

#include "Menu.hpp"
#include "Pack.hpp"

static bool loaded = false;
static SokuLib::DrawUtils::Sprite packContainer;
static SokuLib::DrawUtils::Sprite previewContainer;

SokuLib::SWRFont defaultFont8;
SokuLib::SWRFont defaultFont10;
SokuLib::SWRFont defaultFont16;
HMODULE myModule;
char profilePath[1024 + MAX_PATH];
char profileFolderPath[1024 + MAX_PATH];

void loadFont()
{
	SokuLib::FontDescription desc;

	desc.r1 = 255;
	desc.r2 = 255;
	desc.g1 = 255;
	desc.g2 = 255;
	desc.b1 = 255;
	desc.b2 = 255;
	desc.height = 8;
	desc.weight = FW_BOLD;
	desc.italic = 0;
	desc.shadow = 2;
	desc.bufferSize = 1000000;
	desc.charSpaceX = 0;
	desc.charSpaceY = 0;
	desc.offsetX = 0;
	desc.offsetY = 0;
	desc.useOffset = 0;
	strcpy(desc.faceName, SokuLib::defaultFontName);
	desc.weight = FW_REGULAR;
	defaultFont8.create();
	defaultFont8.setIndirect(desc);

	desc.height = 10;
	defaultFont10.create();
	defaultFont10.setIndirect(desc);

	desc.height = 16;
	defaultFont16.create();
	defaultFont16.setIndirect(desc);
}

void menuLoadAssets()
{
	if (loaded)
		return;
	loaded = true;
	puts("Loading assets");

	previewContainer.texture.loadFromGame("data/menu/profile_list_seat.bmp");
	previewContainer.rect = {
		0, 0,
		static_cast<int>(previewContainer.texture.getSize().x),
		static_cast<int>(previewContainer.texture.getSize().y),
	};
	previewContainer.setPosition({310, 92});
	previewContainer.setSize({365, 345});

	packContainer.texture.loadFromResource(myModule, MAKEINTRESOURCE(4));
	packContainer.rect = {
		0, 0,
		static_cast<int>(previewContainer.texture.getSize().x),
		static_cast<int>(previewContainer.texture.getSize().y),
	};
	packContainer.setSize(packContainer.texture.getSize());
	loadFont();
	loadPacks();
}

void menuUnloadAssets()
{
	if (!loaded)
		return;
	loaded = false;
	puts("Unloading assets");

	defaultFont8.destruct();
	defaultFont10.destruct();
	defaultFont16.destruct();
	previewContainer.texture.destroy();
	packContainer.texture.destroy();
	loadedPacks.clear();
}

int menuOnProcess(SokuLib::MenuResult *This)
{
	auto keys = reinterpret_cast<SokuLib::KeyManager *>(0x89A394);

	menuLoadAssets();
	if (keys->keymapManager->input.b) {
		SokuLib::playSEWaveBuffer(0x29);
		return 0;
	}
	return 1;
}

void menuOnRender(SokuLib::MenuResult *This)
{
	previewContainer.draw();
}