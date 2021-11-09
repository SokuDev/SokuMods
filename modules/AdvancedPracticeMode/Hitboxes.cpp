//
// Created by PinkySmile on 10/03/2021.
//

#include <SokuLib.hpp>
#include <cmath>
#include "DrawUtils.hpp"
#include "Hitboxes.hpp"
#include "State.hpp"
#include <list>

namespace Practice
{
#define BOXES_ALPHA 0.5
#define CHECK_CONFIG_VAL(var, app, key, path) { \
	int __ret = GetPrivateProfileInt((app), (key), -1, (path));\
\
	if (__ret < 0) {\
                MessageBox(SokuLib::window, (std::string("File ") + (path) + ": [" + (app) + "]: " + (key) + " either have an a negative value or no value.").c_str(), "Config error", MB_ICONERROR);\
		abort();\
        }\
	(var) = __ret;\
}

	struct FillerConfig {
		unsigned short size;
		unsigned short normalOffset;
		unsigned short grayedOffset;
	};

	struct EdgeConfig {
		unsigned short size;
		unsigned short offset;
	};

	struct BarConfig {
		EdgeConfig fillerStart;
		FillerConfig filler;
		EdgeConfig fillerEnd;
		EdgeConfig borderStart;
		EdgeConfig border;
		EdgeConfig borderEnd;
	};

	class BorderedBar {
	private:
		Sprite &_sprite;
		BarConfig _config;

		void _drawBorder(Vector2<int> pos, Vector2<unsigned> size, bool reversed)
		{
			this->_sprite.tint = DxSokuColor::White;
			this->_sprite.rect.top = 0;
			this->_sprite.rect.height = this->_sprite.texture.getSize().y;

			if (!reversed) {
				this->_sprite.setSize({this->_config.borderStart.size, size.y});
				this->_sprite.setPosition(pos);
				this->_sprite.rect.left = this->_config.borderStart.offset;
				this->_sprite.rect.width = this->_config.borderStart.size;
				this->_sprite.draw();

				pos.x += this->_config.borderStart.size;
				this->_sprite.setSize({this->_config.border.size, size.y});
				this->_sprite.rect.left = this->_config.border.offset;
				this->_sprite.rect.width = this->_config.border.size;
				while (size.x != 0) {
					this->_sprite.setPosition(pos);
					if (size.x < this->_config.border.size) {
						this->_sprite.setSize(size);
						this->_sprite.rect.left = this->_config.border.offset;
						this->_sprite.rect.width = size.x;
						this->_sprite.draw();
						break;
					}
					size.x -= this->_config.border.size;
					pos.x += this->_config.border.size;
					this->_sprite.draw();
				}

				this->_sprite.setSize({this->_config.borderEnd.size, size.y});
				this->_sprite.setPosition(pos);
				this->_sprite.rect.left = this->_config.borderEnd.offset;
				this->_sprite.rect.width = this->_config.borderEnd.size;
				this->_sprite.draw();
				return;
			}

			pos.x -= this->_config.borderStart.size;
			pos.x -= this->_config.borderEnd.size;
			this->_sprite.setSize({this->_config.borderEnd.size, size.y});
			this->_sprite.setPosition(pos);
			this->_sprite.rect.left = this->_config.borderEnd.offset + this->_config.borderEnd.size;
			this->_sprite.rect.width = -this->_config.borderEnd.size;
			this->_sprite.draw();

			pos.x += this->_config.borderEnd.size;
			this->_sprite.setSize({this->_config.border.size, size.y});
			this->_sprite.rect.left = this->_config.border.offset + this->_config.border.size;
			this->_sprite.rect.width = -this->_config.border.size;
			while (size.x != 0) {
				this->_sprite.setPosition(pos);
				if (size.x < this->_config.border.size) {
					this->_sprite.setSize(size);
					this->_sprite.rect.left = this->_config.border.offset + size.x;
					this->_sprite.rect.width = -size.x;
					this->_sprite.draw();
					break;
				}
				size.x -= this->_config.border.size;
				pos.x += this->_config.border.size;
				this->_sprite.draw();
			}

			this->_sprite.setSize({this->_config.borderStart.size, size.y});
			this->_sprite.setPosition(pos);
			this->_sprite.rect.left = this->_config.borderStart.offset + this->_config.borderStart.size;
			this->_sprite.rect.width = -this->_config.borderStart.size;
			this->_sprite.draw();
		}

		void _drawFiller(Vector2<int> pos, unsigned size, Vector2<unsigned> maxSize, bool reversed, unsigned short offset)
		{
			if (reversed) {
				pos.x -= this->_config.borderEnd.size;
				pos.x += maxSize.x;
				this->_sprite.rect.left = this->_config.fillerStart.offset + this->_config.fillerStart.size;
				if (size <= this->_config.fillerStart.size) {
					pos.x -= size;
					this->_sprite.setPosition(pos);
					this->_sprite.setSize({size, maxSize.y});
					this->_sprite.rect.width = -size;
					this->_sprite.draw();
					return;
				}
				pos.x -= this->_config.fillerStart.size;
				this->_sprite.setPosition(pos);
				this->_sprite.setSize({this->_config.fillerStart.size, maxSize.y});
				this->_sprite.rect.width = -this->_config.fillerStart.size;
				this->_sprite.draw();

				this->_sprite.setSize({this->_config.filler.size, maxSize.y});
				this->_sprite.rect.left = offset + this->_config.filler.size;
				this->_sprite.rect.width = -this->_config.filler.size;
				while (size > this->_config.fillerEnd.size) {
					if (size < this->_config.fillerEnd.size + this->_config.filler.size) {
						this->_sprite.setSize({size - this->_config.fillerEnd.size, maxSize.y});
						this->_sprite.rect.left = this->_config.fillerEnd.offset + size - this->_config.fillerEnd.size;
						this->_sprite.rect.width = -(size - this->_config.fillerEnd.size);
						size = this->_config.fillerEnd.size;
						pos.x -= size - this->_config.fillerEnd.size;
						this->_sprite.setPosition(pos);
						this->_sprite.draw();
						break;
					}
					size -= this->_config.filler.size;
					pos.x -= this->_config.filler.size;
					this->_sprite.setPosition(pos);
					this->_sprite.draw();
				}

				pos.x -= min(this->_config.fillerEnd.size, size);
				this->_sprite.setSize({min(this->_config.fillerEnd.size, size), maxSize.y});
				this->_sprite.setPosition(pos);
				this->_sprite.rect.left = this->_config.fillerEnd.offset - min(this->_config.fillerEnd.size, size);
				this->_sprite.rect.width = -min(this->_config.fillerEnd.size, size);
				this->_sprite.draw();
				return;
			}

			pos.x += this->_config.borderStart.size;
			this->_sprite.setPosition(pos);
			this->_sprite.rect.left = this->_config.fillerStart.offset;
			if (size <= this->_config.fillerStart.size) {
				this->_sprite.setSize({size, maxSize.y});
				this->_sprite.rect.width = size;
				this->_sprite.draw();
				return;
			}
			this->_sprite.setSize({this->_config.fillerStart.size, maxSize.y});
			this->_sprite.rect.width = this->_config.fillerStart.size;
			this->_sprite.draw();

			pos.x += this->_config.fillerStart.size;
			this->_sprite.setSize({this->_config.filler.size, maxSize.y});
			this->_sprite.rect.left = offset;
			this->_sprite.rect.width = this->_config.filler.size;
			while (size > this->_config.fillerEnd.size) {
				this->_sprite.setPosition(pos);
				if (size < this->_config.fillerEnd.size + this->_config.filler.size) {
					this->_sprite.setSize({size - this->_config.fillerEnd.size, maxSize.y});
					this->_sprite.rect.left = this->_config.fillerEnd.offset;
					this->_sprite.rect.width = size - this->_config.fillerEnd.size;
					this->_sprite.draw();
					size = this->_config.fillerEnd.size;
					break;
				}
				size -= this->_config.filler.size;
				pos.x += this->_config.filler.size;
				this->_sprite.draw();
			}

			this->_sprite.setSize({min(this->_config.fillerEnd.size, size), maxSize.y});
			this->_sprite.setPosition(pos);
			this->_sprite.rect.left = this->_config.fillerEnd.offset;
			this->_sprite.rect.width = min(this->_config.fillerEnd.size, size);
			this->_sprite.draw();
		}

	public:
		BorderedBar(Sprite &sprite, LPCSTR configPath, LPCSTR configApp) :
			_sprite(sprite)
		{
			CHECK_CONFIG_VAL(this->_config.fillerStart.size,    configApp, "FILLER_START_SIZE",    configPath);
			CHECK_CONFIG_VAL(this->_config.fillerStart.offset,  configApp, "FILLER_START_OFFSET",  configPath);
			CHECK_CONFIG_VAL(this->_config.filler.size,         configApp, "FILLER_SIZE",          configPath);
			CHECK_CONFIG_VAL(this->_config.filler.normalOffset, configApp, "FILLER_NORMAL_OFFSET", configPath);
			CHECK_CONFIG_VAL(this->_config.filler.grayedOffset, configApp, "FILLER_GRAYED_OFFSET", configPath);
			CHECK_CONFIG_VAL(this->_config.fillerEnd.offset,    configApp, "FILLER_END_SIZE",      configPath);
			CHECK_CONFIG_VAL(this->_config.fillerEnd.offset,    configApp, "FILLER_END_OFFSET",    configPath);
			CHECK_CONFIG_VAL(this->_config.borderStart.size,    configApp, "BORDER_START_SIZE",    configPath);
			CHECK_CONFIG_VAL(this->_config.borderStart.offset,  configApp, "BORDER_START_OFFSET",  configPath);
			CHECK_CONFIG_VAL(this->_config.border.size,         configApp, "BORDER_SIZE",          configPath);
			CHECK_CONFIG_VAL(this->_config.border.offset,       configApp, "BORDER_OFFSET",        configPath);
			CHECK_CONFIG_VAL(this->_config.borderEnd.size,      configApp, "BORDER_END_SIZE",      configPath);
			CHECK_CONFIG_VAL(this->_config.borderEnd.offset,    configApp, "BORDER_END_OFFSET",    configPath);
		}

		void draw(Vector2<int> pos, unsigned size, Vector2<unsigned> maxSize, bool reversed)
		{
			this->_drawBorder(pos, maxSize, reversed);
			this->_drawFiller(pos, size, maxSize, reversed, this->_config.filler.normalOffset);
		}

		void draw(Vector2<int> pos, unsigned size, Vector2<unsigned> maxSize, bool reversed, DxSokuColor color)
		{
			this->_drawBorder(pos, maxSize, reversed);
			this->_sprite.tint = color;
			this->_drawFiller(pos, size, maxSize, reversed, this->_config.filler.grayedOffset);
		}
	};

	enum SpriteOffsets {
		GRAZE_SPRITE_OFF,
		GRAZABLE_SPRITE_OFF,
		HJC_SPRITE_OFF,
		AUB_SPRITE_OFF,
		GRAB_INVUL_SPRITE_OFF,
		UNBLOCKABLE_SPRITE_OFF,
		PROJ_INVUL_SPRITE_OFF,
		GRAB_SPRITE_OFF,
		MELEE_INVUL_SPRITE_OFF,
		MILLENIUM_SPRITE_OFF,
		CAN_BLOCK_SPRITE_OFF,
		DROP_SPRITE_OFF,
		SUPERARMOR_SPRITE_OFF,
		HEALTH_SPRITE_OFF,
		FANTASY_HEAVEN_SPRITE_OFF,
		PHILOSOPHERS_STONE_OFF,
		TIME_SPRITE_OFF,
		CLONES_SPRITE_OFF,
		HEALING_CHARM_SPRITE_OFF,
		STOP_WATCH_SPRITE_OFF,
		MAGIC_POTION_SPRITE_OFF,
		TIME_STOP_SPRITE_OFF,
		S_TIER_DENSITY_SPRITE_OFF,
		A_TIER_DENSITY_SPRITE_OFF,
		B_TIER_DENSITY_SPRITE_OFF,
		C_TIER_DENSITY_SPRITE_OFF,
		TIGER_RELEASE_SPRITE_OFF,
		GIANT_SUIKA_SPRITE_OFF,
		FUTURISTIC_DOLL_SPRITE_OFF,
		AYA_SPEED_BOOST_SPRITE_OFF,
		CIRNO_ICE_SPRITE_OFF,
		CONFUSED_DEBUFF_SPRITE_OFF,
		IKU_HANGEKI_SPRITE_OFF,
		IKU_STICKLE_SPRITE_OFF,
		IKU_VEIL_SPRITE_OFF,
		ORERRIES_SPRITE_OFF,
		PARASOL_SPRITE_OFF,
		PARRY_SPRITE_OFF,
		GREEN_LIGHT_SPRITE_OFF,
		FARSIGHT_SPRITE_OFF,
		REISEN_GAS_SPRITE_OFF,
		ULTRARED_FIELD_SPRITE_OFF,
		ULTRAVIOLET_FIELD_SPRITE_OFF,
		XWAVE_SPRITE_OFF,
		CHAIN_DEBUFF_SPRITE_OFF,
		ROCK_HARD_SPRITE_OFF,
		WEATHER_DEBUFF_SPRITE_OFF,
		TOKAMAK_SPRITE_OFF
	};

	struct MaxAttributes {
		unsigned short drop;
		unsigned short star;
		unsigned short clones;
		unsigned short stopWatch;
	};

	static RectangleShape rectangle;
	static Sprite flagsSprite;
	static Sprite borderSprite;
	static Sprite iconBgSprite;
	static Sprite iconCoverSprite;
	static std::unique_ptr<BorderedBar> healthBorder;
	static std::unique_ptr<BorderedBar> installBorder;
	static std::pair<MaxAttributes, MaxAttributes> allMaxAttributes;

	void initBoxDisplay(LPCSTR profilePath)
	{
		if (flagsSprite.texture.hasTexture())
			return;

		std::string profile = profilePath;

		profile += "/assets/";
		Texture::loadFromFile(flagsSprite.texture, (profile + "flags.png").c_str());
		flagsSprite.setSize({32, 32});
		flagsSprite.rect.height = 32;

		Texture::loadFromFile(iconBgSprite.texture, (profile + "iconBg.png").c_str());
		iconBgSprite.setSize({40, 40});
		iconBgSprite.rect.width = 40;
		iconBgSprite.rect.height = 40;
		iconBgSprite.rect.top = 0;
		iconBgSprite.rect.left = 0;

		Texture::loadFromFile(iconCoverSprite.texture, (profile + "iconCover.png").c_str());
		iconCoverSprite.setSize({40, 40});
		iconCoverSprite.rect.width = 40;
		iconCoverSprite.rect.height = 40;
		iconCoverSprite.rect.top = 0;
		iconCoverSprite.rect.left = 0;
		//iconCoverSprite.tint = DxSokuColor::Transparent;

		Texture::loadFromFile(borderSprite.texture, (profile + "borders.png").c_str());
		profile += "borders.ini";
		healthBorder .reset(new BorderedBar(borderSprite, profile.c_str(), "HEALTH"));
		installBorder.reset(new BorderedBar(borderSprite, profile.c_str(), "INSTALL"));
	}

	static void drawBox(const SokuLib::Box &box, const SokuLib::RotationBox *rotation, DxSokuColor borderColor, DxSokuColor fillColor)
	{
		if (!rotation) {
			FloatRect rect{};

			rect.x1 = SokuLib::camera.scale * (SokuLib::camera.translate.x + box.left);
			rect.x2 = SokuLib::camera.scale * (SokuLib::camera.translate.x + box.right);
			rect.y1 = SokuLib::camera.scale * (SokuLib::camera.translate.y + box.top);
			rect.y2 = SokuLib::camera.scale * (SokuLib::camera.translate.y + box.bottom);
			rectangle.setRect(rect);
		} else {
			Rect<Vector2<float>> rect;

			rect.x1.x = SokuLib::camera.scale * (SokuLib::camera.translate.x + box.left);
			rect.x1.y = SokuLib::camera.scale * (SokuLib::camera.translate.y + box.top);

			rect.y1.x = SokuLib::camera.scale * (SokuLib::camera.translate.x + box.left + rotation->pt1.x);
			rect.y1.y = SokuLib::camera.scale * (SokuLib::camera.translate.y + box.top + rotation->pt1.y);

			rect.x2.x = SokuLib::camera.scale * (SokuLib::camera.translate.x + box.left + rotation->pt1.x + rotation->pt2.x);
			rect.x2.y = SokuLib::camera.scale * (SokuLib::camera.translate.y + box.top + rotation->pt1.y + rotation->pt2.y);

			rect.y2.x = SokuLib::camera.scale * (SokuLib::camera.translate.x + box.left + rotation->pt2.x);
			rect.y2.y = SokuLib::camera.scale * (SokuLib::camera.translate.y + box.top + rotation->pt2.y);
			rectangle.rawSetRect(rect);
		}

		rectangle.setFillColor(fillColor);
		rectangle.setBorderColor(borderColor);
		rectangle.draw();
	}

	static void drawCollisionBox(const SokuLib::ObjectManager &manager)
	{
		FloatRect rect{};
		const SokuLib::Box &box = *manager.frameData->collisionBox;

		if (!manager.frameData->collisionBox)
			return;

		rect.x1 = SokuLib::camera.scale * (std::ceil(manager.position.x) + manager.direction * box.left + SokuLib::camera.translate.x);
		rect.x2 = SokuLib::camera.scale * (std::ceil(manager.position.x) + manager.direction * box.right + SokuLib::camera.translate.x);
		rect.y1 = SokuLib::camera.scale * (box.top - std::ceil(manager.position.y) + SokuLib::camera.translate.y);
		rect.y2 = SokuLib::camera.scale * (box.bottom - std::ceil(manager.position.y) + SokuLib::camera.translate.y);

		rectangle.setRect(rect);
		rectangle.setFillColor(DxSokuColor::Yellow * BOXES_ALPHA);
		rectangle.setBorderColor(DxSokuColor::Yellow);
		rectangle.draw();
	}

	static void drawPositionBox(const SokuLib::ObjectManager &manager)
	{
		rectangle.setPosition({
			static_cast<int>(SokuLib::camera.scale * (manager.position.x - 2 + SokuLib::camera.translate.x)),
			static_cast<int>(SokuLib::camera.scale * (-manager.position.y - 2 + SokuLib::camera.translate.y))
		});
		rectangle.setSize({
			static_cast<unsigned int>(SokuLib::camera.scale * 5),
			static_cast<unsigned int>(SokuLib::camera.scale * 5)
		});
		rectangle.setFillColor(DxSokuColor::White);
		rectangle.setBorderColor(DxSokuColor::White + DxSokuColor::Black);
		rectangle.draw();
	}

	static void drawHurtBoxes(const SokuLib::ObjectManager &manager)
	{
		if (manager.hurtBoxCount > 5)
			return;

		for (int i = 0; i < manager.hurtBoxCount; i++)
			drawBox(
				manager.hurtBoxes[i],
				manager.hurtBoxesRotation[i],
				DxSokuColor::Green,
				(manager.frameData->frameFlags.chOnHit ? DxSokuColor::Cyan : DxSokuColor::Green) * BOXES_ALPHA
			);
	}

	static void drawHitBoxes(const SokuLib::ObjectManager &manager)
	{
		if (manager.hitBoxCount > 5)
			return;

		for (int i = 0; i < manager.hitBoxCount; i++)
			drawBox(manager.hitBoxes[i], manager.hitBoxesRotation[i], DxSokuColor::Red, DxSokuColor::Red * BOXES_ALPHA);
	}

	static void displayPlayerFrameFlag(SpriteOffsets textureOffset, Vector2<int> &barPos, bool reverse)
	{
		auto size = static_cast<int>(std::ceil(flagsSprite.texture.getSize().x / 32.f));

		flagsSprite.rect.left = textureOffset % size * 32;
		flagsSprite.rect.top = textureOffset / size * 32;
		if (reverse) {
			barPos.x -= 32;
			flagsSprite.rect.width = -32;
			flagsSprite.rect.left += 32;
		} else
			flagsSprite.rect.width = 32;

		auto bgPos = barPos;

		bgPos.x -= 4;
		bgPos.y -= 4;
		iconBgSprite.setSize({40, 40});
		iconBgSprite.setPosition(bgPos);
		iconBgSprite.draw();

		flagsSprite.setSize({32, 32});
		flagsSprite.setPosition(barPos);
		flagsSprite.draw();

		iconCoverSprite.setSize({40, 40});
		iconCoverSprite.setPosition(bgPos);
		iconCoverSprite.draw();

		if (!reverse)
			barPos.x += 40;
		else
			barPos.x -= 8;
	}

	static void displayPlayerBar(SpriteOffsets textureOffset, Vector2<int> basePos, Vector2<int> &barPos, unsigned value, unsigned max, bool reverse, DxSokuColor color)
	{
		if (value > max)
			printf("Showing out of bound box %u / %u\n", value, max);
		if (barPos.x != basePos.x || barPos.y != basePos.y) {
			barPos.x = basePos.x;
			barPos.y += 40;
		}
		displayPlayerFrameFlag(textureOffset, barPos, reverse);

		if (reverse)
			installBorder->draw({barPos.x - 201, barPos.y + 4}, 200 * value / max, {200, 24}, reverse, color);
		else
			installBorder->draw({barPos.x, barPos.y + 4}, 200 * value / max, {200, 24}, reverse, color);
	}

	static void displayObjectFrameFlag(SpriteOffsets textureOffset, Vector2<int> &barPos)
	{
		auto scaled = 32 * SokuLib::camera.scale;
		auto scaled2 = 40 * SokuLib::camera.scale;
		auto size = static_cast<int>(std::ceil(flagsSprite.texture.getSize().x / 32.f));

		flagsSprite.rect.left = textureOffset % size * 32;
		flagsSprite.rect.top = textureOffset / size * 32;
		flagsSprite.rect.width = 32;
		auto bgPos = barPos;

		bgPos.x -= 4 * SokuLib::camera.scale;
		bgPos.y -= 4 * SokuLib::camera.scale;
		iconBgSprite.setSize({
			static_cast<unsigned int>(scaled2),
			static_cast<unsigned int>(scaled2)
		});
		iconBgSprite.setPosition(bgPos);
		iconBgSprite.draw();

		flagsSprite.setSize({
			static_cast<unsigned int>(scaled),
			static_cast<unsigned int>(scaled)
		});
		flagsSprite.setPosition(barPos);
		flagsSprite.draw();

		iconCoverSprite.setSize({
			static_cast<unsigned int>(scaled2),
			static_cast<unsigned int>(scaled2)
		});
		iconCoverSprite.setPosition(bgPos);
		iconCoverSprite.draw();
		barPos.x += scaled;
	}

	static void displayObjectBar(SpriteOffsets textureOffset, Vector2<int> basePos, Vector2<int> &barPos, unsigned value, unsigned max, DxSokuColor color)
	{
		auto &barObj = (textureOffset == HEALTH_SPRITE_OFF || textureOffset == TIME_SPRITE_OFF ? *healthBorder : *installBorder);

		if (barPos.x != basePos.x || barPos.y != basePos.y) {
			barPos.x = basePos.x;
			barPos.y += 32 * SokuLib::camera.scale;
		}
		displayObjectFrameFlag(textureOffset, barPos);
		barObj.draw(
			{barPos.x, barPos.y + static_cast<int>(4 * SokuLib::camera.scale)},
			100 * SokuLib::camera.scale * value / max,
			{
				static_cast<unsigned>(200 * SokuLib::camera.scale),
				static_cast<unsigned>(24 * SokuLib::camera.scale)
			},
			false,
			color
		);
	}

	static float getSuperArmorRatio(const SokuLib::CharacterManager &manager, SokuLib::Character character, MaxAttributes &maxAttributes)
	{
		if (character == SokuLib::CHARACTER_PATCHOULI) {
			if (manager.diamondHardEffectLeft)
				return manager.diamondHardEffectLeft / 720.f;
		}
		if (manager.dragonStarTimeLeft) {
			maxAttributes.star = max(maxAttributes.star, manager.dragonStarTimeLeft);
			return manager.dragonStarTimeLeft / static_cast<float>(maxAttributes.star);
		} else
			maxAttributes.star = 0;
		return 0;
	}

	static unsigned getMaxSuperArmor(const SokuLib::CharacterManager &manager, SokuLib::Character character, MaxAttributes &maxAttributes)
	{
		if (character == SokuLib::CHARACTER_SUIKA) {
			if (manager.objectBase.action == SokuLib::ACTION_DEFAULT_SKILL1_B || manager.objectBase.action == SokuLib::ACTION_DEFAULT_SKILL1_C)
				switch (manager.skillLevels[0]) {
				case 0:
					return 0;
				case 1:
				case 2:
					return 250;
				case 3:
					return 500;
				default:
					return 750;
				}
		}
		if (character == SokuLib::CHARACTER_SANAE)
			if (manager.objectBase.action == SokuLib::ACTION_USING_SC_ID_210)
				return 2500;
		return 0;
	}

	static void displayPlayerStatusFlags(const SokuLib::CharacterManager &manager, SokuLib::Character character, MaxAttributes &maxAttributes, Vector2<int> barPos, bool reverse)
	{
		auto basePos = barPos;
		auto &base = manager.objectBase;
		auto &flags = base.frameData->frameFlags;
		auto superArmorRatio = getSuperArmorRatio(manager, character, maxAttributes);
		auto maxSuperArmor = getMaxSuperArmor(manager, character, maxAttributes);

		if (flags.graze)
			displayPlayerFrameFlag(GRAZE_SPRITE_OFF, barPos, reverse);
		if (flags.highJumpCancellable)
			displayPlayerFrameFlag(HJC_SPRITE_OFF, barPos, reverse);
		if (flags.grabInvincible || manager.grabInvulTimer)
			displayPlayerFrameFlag(GRAB_INVUL_SPRITE_OFF, barPos, reverse);
		if (flags.projectileInvincible || manager.projectileInvulTimer)
			displayPlayerFrameFlag(PROJ_INVUL_SPRITE_OFF, barPos, reverse);
		if (flags.meleeInvincible || manager.meleeInvulTimer)
			displayPlayerFrameFlag(MELEE_INVUL_SPRITE_OFF, barPos, reverse);
		if (flags.guardAvailable)
			displayPlayerFrameFlag(CAN_BLOCK_SPRITE_OFF, barPos, reverse);
		if (!maxAttributes.stopWatch && manager.timeStop && (character != SokuLib::CHARACTER_SAKUYA || !manager.sakuyasWorldTime))
			displayPlayerFrameFlag(STOP_WATCH_SPRITE_OFF, barPos, reverse);
		if (superArmorRatio || manager.noSuperArmor == 0 || flags.superArmor) {
			if (superArmorRatio > 0)
				displayPlayerBar(SUPERARMOR_SPRITE_OFF, basePos, barPos, 10000 * superArmorRatio, 10000, reverse, DxSokuColor::Blue);
			else if (maxSuperArmor)
				displayPlayerBar(SUPERARMOR_SPRITE_OFF, basePos, barPos, max(0, static_cast<int>(maxSuperArmor - manager.objectBase.superarmorDamageTaken)), maxSuperArmor, reverse, DxSokuColor::Red);
			else
				displayPlayerFrameFlag(SUPERARMOR_SPRITE_OFF, barPos, reverse);
		}
		if (manager.dropInvulTimeLeft) {
			maxAttributes.drop = max(maxAttributes.drop, manager.dropInvulTimeLeft);
			displayPlayerBar(
				DROP_SPRITE_OFF,
				basePos,
				barPos,
				manager.dropInvulTimeLeft,
				maxAttributes.drop,
				reverse,
				{
					manager.objectBase.renderInfos.shaderColor.r,
					manager.objectBase.renderInfos.shaderColor.g,
					manager.objectBase.renderInfos.shaderColor.b,
					0xFF
				}
			);
		} else
			maxAttributes.drop = 0;
		if (manager.magicPotionTimeLeft && (!manager.dropInvulTimeLeft || manager.magicPotionTimeLeft != 1))
			displayPlayerBar(MAGIC_POTION_SPRITE_OFF, basePos, barPos, manager.magicPotionTimeLeft, 360, reverse, DxSokuColor::Blue);
		if (manager.healingCharmTimeLeft)
			displayPlayerBar(HEALING_CHARM_SPRITE_OFF, basePos, barPos, manager.healingCharmTimeLeft, 250, reverse, DxSokuColor::Cyan);
		if (character == SokuLib::CHARACTER_SAKUYA && manager.sakuyasWorldTime)
			displayPlayerBar(STOP_WATCH_SPRITE_OFF, basePos, barPos, manager.sakuyasWorldTime, 300, reverse, DxSokuColor{0x44, 0x44, 0x44, 0xFF});
		else if (maxAttributes.stopWatch)
			displayPlayerBar(STOP_WATCH_SPRITE_OFF, basePos, barPos, 51 - maxAttributes.stopWatch, 50, reverse, DxSokuColor{0x44, 0x44, 0x44, 0xFF});

		switch (character) {
		case SokuLib::CHARACTER_PATCHOULI:
			if (manager.philosophersStoneTime)
				displayPlayerBar(PHILOSOPHERS_STONE_OFF, basePos, barPos, manager.philosophersStoneTime, 1200, reverse, DxSokuColor::Magenta);
			break;
		case SokuLib::CHARACTER_REMILIA:
			if (manager.milleniumVampireTime)
				displayPlayerBar(MILLENIUM_SPRITE_OFF, basePos, barPos, manager.milleniumVampireTime, 600, reverse, DxSokuColor::Red);
			break;
		case SokuLib::CHARACTER_YOUMU:
			if (manager.youmuCloneTimeLeft) {
				maxAttributes.clones = max(maxAttributes.clones, manager.youmuCloneTimeLeft);
				displayPlayerBar(CLONES_SPRITE_OFF, basePos, barPos, manager.youmuCloneTimeLeft, maxAttributes.clones, reverse, DxSokuColor{0xAA, 0xAA, 0xAA, 0xFF});
			} else
				maxAttributes.clones = 0;
			break;
		case SokuLib::CHARACTER_REIMU:
			if (manager.fantasyHeavenTimeLeft)
				displayPlayerBar(FANTASY_HEAVEN_SPRITE_OFF, basePos, barPos, manager.fantasyHeavenTimeLeft, 900, reverse, DxSokuColor::Red);
			break;
		default:
			break;
		}
	}

	static void displayHitProperties(SokuLib::Character baseCharacter, const SokuLib::ObjectManager &manager, bool showHp)
	{
		int maxX = 640 - (32 + 50) * SokuLib::camera.scale;
		int maxY = 480 - 32 * SokuLib::camera.scale;
		Vector2<int> leftBound = {maxX, 0};
		Vector2<int> base;
		int rightBound = 0;
		auto &attack = manager.frameData->attackFlags;
		Rect<Vector2<float>> rect;

		for (int i = 0; i < manager.hurtBoxCount; i++) {
			auto &box = manager.hurtBoxes[i];
			auto rotation = manager.hurtBoxesRotation[i];

			rect.x1.x = SokuLib::camera.scale * (SokuLib::camera.translate.x + box.left);
			rect.x1.y = SokuLib::camera.scale * (SokuLib::camera.translate.y + box.top);
			if (!rotation) {
				rect.x2.x = SokuLib::camera.scale * (SokuLib::camera.translate.x + box.right);
				rect.x2.y = SokuLib::camera.scale * (SokuLib::camera.translate.y + box.top);

				rect.y1.x = SokuLib::camera.scale * (SokuLib::camera.translate.x + box.left);
				rect.y1.y = SokuLib::camera.scale * (SokuLib::camera.translate.y + box.bottom);

				rect.y2.x = SokuLib::camera.scale * (SokuLib::camera.translate.x + box.right);
				rect.y2.y = SokuLib::camera.scale * (SokuLib::camera.translate.y + box.bottom);
			} else {
				rect.y1.x = SokuLib::camera.scale * (SokuLib::camera.translate.x + box.left + rotation->pt1.x);
				rect.y1.y = SokuLib::camera.scale * (SokuLib::camera.translate.y + box.top +  rotation->pt1.y);

				rect.x2.x = SokuLib::camera.scale * (SokuLib::camera.translate.x + box.left + rotation->pt1.x + rotation->pt2.x);
				rect.x2.y = SokuLib::camera.scale * (SokuLib::camera.translate.y + box.top +  rotation->pt1.y + rotation->pt2.y);

				rect.y2.x = SokuLib::camera.scale * (SokuLib::camera.translate.x + box.left + rotation->pt2.x);
				rect.y2.y = SokuLib::camera.scale * (SokuLib::camera.translate.y + box.top +  rotation->pt2.y);
			}
			rightBound  = min(maxX, max(rightBound, max(rect.x1.x, max(rect.x2.x, max(rect.y1.x, rect.x2.x)))));
			leftBound.x = max(0, min(leftBound.x, min(rect.x1.x, min(rect.x2.x, min(rect.y1.x, rect.x2.x)))));
			leftBound.y = min(maxY, max(leftBound.y, max(rect.x1.y, max(rect.x2.y, max(rect.y1.y, rect.x2.y)))));
		}

		for (int i = 0; i < manager.hitBoxCount; i++) {
			auto &box = manager.hitBoxes[i];
			auto rotation = manager.hitBoxesRotation[i];

			rect.x1.x = SokuLib::camera.scale * (SokuLib::camera.translate.x + box.left);
			rect.x1.y = SokuLib::camera.scale * (SokuLib::camera.translate.y + box.top);
			if (!rotation) {
				rect.x2.x = SokuLib::camera.scale * (SokuLib::camera.translate.x + box.right);
				rect.x2.y = SokuLib::camera.scale * (SokuLib::camera.translate.y + box.top);

				rect.y1.x = SokuLib::camera.scale * (SokuLib::camera.translate.x + box.left);
				rect.y1.y = SokuLib::camera.scale * (SokuLib::camera.translate.y + box.bottom);

				rect.y2.x = SokuLib::camera.scale * (SokuLib::camera.translate.x + box.right);
				rect.y2.y = SokuLib::camera.scale * (SokuLib::camera.translate.y + box.bottom);
			} else {
				rect.y1.x = SokuLib::camera.scale * (SokuLib::camera.translate.x + box.left + rotation->pt1.x);
				rect.y1.y = SokuLib::camera.scale * (SokuLib::camera.translate.y + box.top +  rotation->pt1.y);

				rect.x2.x = SokuLib::camera.scale * (SokuLib::camera.translate.x + box.left + rotation->pt1.x + rotation->pt2.x);
				rect.x2.y = SokuLib::camera.scale * (SokuLib::camera.translate.y + box.top +  rotation->pt1.y + rotation->pt2.y);

				rect.y2.x = SokuLib::camera.scale * (SokuLib::camera.translate.x + box.left + rotation->pt2.x);
				rect.y2.y = SokuLib::camera.scale * (SokuLib::camera.translate.y + box.top +  rotation->pt2.y);
			}
			rightBound = min(maxX, max(rightBound, max(rect.x1.x, max(rect.x2.x, max(rect.y1.x, rect.x2.x)))));
			leftBound.x = max(0, min(leftBound.x, min(rect.x1.x, min(rect.x2.x, min(rect.y1.x, rect.x2.x)))));
			leftBound.y = min(maxY, max(leftBound.y, max(rect.x1.y, max(rect.x2.y, max(rect.y1.y, rect.x2.y)))));
		}

		if (!manager.hurtBoxCount && !manager.hitBoxCount)
			leftBound = Vector2<int>{
				static_cast<int>(manager.position.x),
				static_cast<int>(manager.position.y)
			};

		base = leftBound;
		if (manager.hitBoxCount) {
			if (attack.grab)
				displayObjectFrameFlag(GRAB_SPRITE_OFF, base);
			else if (attack.unblockable)
				displayObjectFrameFlag(UNBLOCKABLE_SPRITE_OFF, base);
			else if (!attack.airBlockable)
				displayObjectFrameFlag(AUB_SPRITE_OFF, base);
			if (attack.grazable)
				displayObjectFrameFlag(GRAZABLE_SPRITE_OFF, base);
		}
		if (manager.hurtBoxCount) {
			if (showHp) {
				auto realHp = manager.hp;
				auto maxHp = 10000;

				if ( //This is Patchouli's bubble
					baseCharacter == SokuLib::CHARACTER_PATCHOULI &&
					manager.action == 0x358 &&
					(manager.image->number == 0x154 || manager.image->number == 0x1B3)
				) {
					displayObjectBar(TIME_SPRITE_OFF, leftBound, base, 360 - manager.frameCount, 360, DxSokuColor::Green);
					realHp += 3000;
					maxHp = 3000;
				}

				if ( //This is Alice's doll (C)
					baseCharacter == SokuLib::CHARACTER_ALICE &&
					manager.action == 805 &&
					(manager.image->number >= 304 && manager.image->number <= 313)
				) {
					realHp += 600;
					maxHp = 600;
				}

				if ( //This is Alice's doll (d22)
					baseCharacter == SokuLib::CHARACTER_ALICE &&
					manager.action == 825 &&
					(manager.image->number >= 322 && manager.image->number <= 325)
				) {
					realHp += 700;
					maxHp = 700;
				}

				if (realHp && maxHp > 1)
					displayObjectBar(HEALTH_SPRITE_OFF, leftBound, base, realHp, maxHp, DxSokuColor::Green);
			}
		}
	}

	static void drawPlayerBoxes(const SokuLib::CharacterManager &manager, SokuLib::Character character, MaxAttributes &maxAttributes, Vector2<int> barPos, bool reverse, const HitBoxParams &params)
	{
		if (params.showCollisionBox)
			drawCollisionBox(manager.objectBase);
		if (params.showHurtboxes)
			drawHurtBoxes(manager.objectBase);
		if (params.showHitboxes)
			drawHitBoxes(manager.objectBase);
		if (params.showPosition)
			drawPositionBox(manager.objectBase);
		if (params.showBuffProperties)
			displayPlayerStatusFlags(manager, character, maxAttributes, barPos, reverse);
		if (params.showHitProperties)
			displayHitProperties(character, manager.objectBase, false);

		auto array = manager.objects.list.vector();

		for (const auto _elem : array) {
			auto elem = reinterpret_cast<const SokuLib::ProjectileManager *>(_elem);

			if ((elem->isActive && elem->objectBase.hitCount > 0) || elem->objectBase.frameData->attackFlags.value > 0) {
				if (params.showSubObjectHurtboxes)
					drawHurtBoxes(elem->objectBase);
				if (params.showSubObjectHitboxes)
					drawHitBoxes(elem->objectBase);
				if (params.showSubObjectPosition)
					drawPositionBox(elem->objectBase);
				if (params.showSubObjectProperties)
					displayHitProperties(character, elem->objectBase, true);
			}
		}
	}

	void drawAllBoxes()
	{
		auto &battle = SokuLib::getBattleMgr();

		drawPlayerBoxes(battle.leftCharacterManager,  SokuLib::leftChar,  allMaxAttributes.first,  {20, 70}, false, settings.leftHitboxSettings);
		drawPlayerBoxes(battle.rightCharacterManager, SokuLib::rightChar, allMaxAttributes.second, {620, 70}, true, settings.rightHitboxSettings);
	}

	void updateCharacter(const SokuLib::CharacterManager &manager, MaxAttributes &maxAttributes)
	{
		if (manager.timeStop && (manager.objectBase.action < SokuLib::ACTION_USING_SC_ID_200 || manager.objectBase.action > SokuLib::ACTION_SC_ID_219_ALT_EFFECT))
			maxAttributes.stopWatch++;
		else
			maxAttributes.stopWatch = 0;
	}

	void onUpdate()
	{
		updateCharacter(SokuLib::getBattleMgr().leftCharacterManager, allMaxAttributes.first);
		updateCharacter(SokuLib::getBattleMgr().rightCharacterManager, allMaxAttributes.second);
	}
}