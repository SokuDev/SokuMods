//
// Created by PinkySmile on 19/07/2021.
//

#include "Menu.hpp"
#include "Pack.hpp"

static bool loaded = false;
static SokuLib::DrawUtils::Sprite packContainer;
static SokuLib::DrawUtils::Sprite previewContainer;

HMODULE myModule;
char profilePath[1024 + MAX_PATH];

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
}

void menuUnloadAssets()
{
	if (!loaded)
		return;
	loaded = false;
	puts("Unloading assets");

	previewContainer.texture.destroy();
	packContainer.texture.destroy();
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