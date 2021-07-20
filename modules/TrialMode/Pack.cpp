//
// Created by PinkySmile on 20/07/2021.
//

#include "Pack.hpp"

std::vector<Pack> loadedPacks;

void loadPacks()
{

}

Pack::Pack(const nlohmann::json &object)
{
	if (!object.is_object())
		return;

	this->category = object.contains("category") && object["category"].is_string() ? object["category"] : "no category";
	if (object.contains("icon")) {
		this->icon = std::make_unique<Icon>(object["icon"]);
		if (!this->icon->sprite.texture.hasTexture())
			this->icon.reset();
	}
	std::vector<Scenario> scenarios;
	SokuLib::DrawUtils::Sprite name;
	SokuLib::DrawUtils::Sprite error;
	SokuLib::DrawUtils::Sprite author;
}

Scenario::Scenario(const nlohmann::json &object)
{
	if (!object.is_object())
		return;
}

Icon::Icon(const nlohmann::json &object)
{
	if (!object.is_object())
		return;

	if (object.contains("offset") && object) {

	}
}
