offset = 256

local chrs = dofile(packPath.."\\characterAnimations.lua")
local bg = dofile(packPath.."\\backgrounds.lua")
local bgs = bg.backgrounds
local characters = {}
local index = 0
local selectedSky = math.random(1, 20)
local scale = 2
local ctr = 0
local title = false
local popped = false
local creditsImages = {}
local titleImages = {
	{ image = "data/scene/title/15_back.cv2",     x = 0,   y = 0},  --<mutable id="150" xposition="0" yposition="0"/>
	{ image = "data/scene/title/14_back.cv2",     x = 0,   y = 352},--<mutable id="140" xposition="0" yposition="352"/>
	{ image = "data/scene/title/13_back.cv2",     x = 468, y = 360},--<mutable id="130" xposition="468" yposition="480"/>
	{ image = "data/scene/title/12_back.cv2",     x = 0,   y = 368},--<mutable id="120" xposition="0" yposition="368"/>
	{ image = "data/scene/title/11_back.cv2",     x = 0,   y = 269},--<mutable id="110" xposition="0" yposition="269"/>
	--<image name="2_menu_black.bmp" xposition="0" yposition="0" width="160" height="480"/>
	--<mutable id="200" xposition="0" yposition="60"/> (Probably the gear)
	--<image name="2_menu_hari.bmp" xposition="0" yposition="0" width="86" height="57"/>
	--<mutable id="300" xposition="7" yposition="88"/> (Probably the menu items)
	{ image = "data/scene/title/2_title.cv2",     x = 120, y = 60}, --<image name="2_title.bmp" xposition="24" yposition="19" width="352" height="161"/>
	{ image = "data/scene/title/2_copyright.cv2", x = 136, y = 458},--<image name="2_copyright.bmp" xposition="0" yposition="0" width="368" height="14"/>
}
local _sprite = Sprite.new()
local titleSprites = {}
local rightRect = RectangleShape.new()
local flashRect = RectangleShape.new()
local popRect = RectangleShape.new()
local backgrounds = {
	0, 0,
	1, 1,
	2, 2,
	3, 3,
	4, 4,
	5, 5,
	6,
	10,
	11,
	12, 12, 12, 12, 12, 12,
	13, 13,
	14, 14, 14, 14, 14, 14,
	15, 15,
	--16, 16, 16, 16, 16, 16, 16, 16,
	17, 17,
	18, 18,
	30, 30,
	31, 31,
	32, 32,
	36
}
local selectedBg = math.random(1, #backgrounds)
local objsOffsets = {
	alice =     { offset = { -16, -40 },  size = Vector2u.new(32, 32), rotation = 0, total = 0},
	aya =       { offset = { -16, -48 },  size = Vector2u.new(24, 24), rotation = 0, total = 0},
	chirno =    { offset = { -16, -40 },  size = Vector2u.new(32, 32), rotation = 0, total = 0},
	iku =       { offset = { -16, -40 },  size = Vector2u.new(32, 32), rotation = 0, total = 0},
	komachi =   { offset = { -16, -40 },  size = Vector2u.new(32, 32), rotation = 0, total = 0},
	marisa =    { offset = { -16, -48 },  size = Vector2u.new(32, 32), rotation = 0, total = 0},
	meirin =    { offset = { -16, -40 },  size = Vector2u.new(32, 32), rotation = 0, total = 0},
	patchouli = { offset = { -16, -40 },  size = Vector2u.new(32, 32), rotation = 0, total = 0},
	reimu =     { offset = { -16, -40 },  size = Vector2u.new(32, 32), rotation = 0, total = 0},
	remilia =   { offset = { -76, -125 }, size = Vector2u.new(96, 96), rotation = math.pi / 12, total = 0},
	sakuya =    { offset = { -16, -36 },  size = Vector2u.new(16, 16), rotation = 0, total = 0},
	sanae =     { offset = { -16, -40 },  size = Vector2u.new(32, 32), rotation = 0, total = 0},
	suika =     { offset = { -16, -40 },  size = Vector2u.new(32, 32), rotation = 0, total = 0},
	suwako =    { offset = { -16, -40 },  size = Vector2u.new(32, 32), rotation = 0, total = 0},
	tenshi =    { offset = { -16, -40 },  size = Vector2u.new(32, 32), rotation = 0, total = 0},
	udonge =    { offset = { -16, -40 },  size = Vector2u.new(32, 32), rotation = 0, total = 0},
	utsuho =    { offset = { -16, -64 },  size = Vector2u.new(32, 32), rotation = 0, total = 0},
	youmu =     { offset = { -16, -40 },  size = Vector2u.new(32, 32), rotation = 0, total = 0},
	yukari =    { offset = { -16, -40 },  size = Vector2u.new(32, 32), rotation = 0, total = 0},
	yuyuko =    { offset = { -16, -40 },  size = Vector2u.new(32, 32), rotation = 0, total = 0},
}
local forcedOrder = {
	"remilia",
	"sakuya",
	"patchouli",
	"meirin",
	"reimu",
	"marisa",
}
local randomOrder = {
	"alice",
	"aya",
	"chirno",
	"iku",
	"komachi",
	"sanae",
	"suika",
	"suwako",
	"tenshi",
	"udonge",
	"utsuho",
	"youmu",
	"yukari",
	"yuyuko",
}

flashRect.fillColor = enums.colors.Transparent
flashRect.borderColor = enums.colors.Transparent
flashRect.size = Vector2u.new(640, 480)
popRect.fillColor = Color.new(255, 255, 255, 0)
popRect.borderColor = enums.colors.Transparent
popRect.size = Vector2u.new(640, 480)
rightRect.fillColor = enums.colors.Black
rightRect.borderColor = enums.colors.Transparent
rightRect.size = Vector2u.new(offset, 480)
rightRect.position = Vector2i.new(640 - rightRect.size.x, 0)

local function shuffle(tbl)
	for i = #tbl, 2, -1 do
		local j = math.random(i)
		tbl[i], tbl[j] = tbl[j], tbl[i]
	end
	return tbl
end

local function loadTitleSprite(data)
	local sprite = Sprite.new()

	sprite.texture:loadFromGame(data.image)
	sprite.size = sprite.texture.size
	sprite.position = Vector2i.new(data.x, data.y)
	sprite.rect.width = sprite.texture.size.x
	sprite.rect.height= sprite.texture.size.y
	return sprite
end

local function loadObjectSprite(chr)
	local sprite = Sprite.new()

	if not sprite.texture:loadFromFile(packPath.."objects/"..chr..".png") then
		return nil
	end
	sprite.size = objsOffsets[chr].size
	sprite.rect.width  = sprite.texture.size.x
	sprite.rect.height = sprite.texture.size.y
	return sprite
end

local function loadSprites(chr, animations)
	loadPalette("data/character/"..chr.."/palette000.pal")
	for i, k in pairs(animations) do
		for _, frame in pairs(k) do
			local sprite = Sprite.new()

			sprite.texture:loadFromGame(frame.image)
			sprite.size = Vector2u.new(math.floor(frame.texwidth * 2 / scale), math.floor(frame.texheight * 2 / scale))
			sprite.rect.left  = frame.xtexoffset
			sprite.rect.top   = frame.ytexoffset
			sprite.rect.width = frame.texwidth
			sprite.rect.height= frame.texheight
			if i == "walk" then
				objsOffsets[chr].total = objsOffsets[chr].total + frame.duration
			end
			frame.sprite = sprite
		end
	end
	return animations
end

for _, k in pairs(titleImages) do
	titleSprites[#titleSprites + 1] = loadTitleSprite(k)
end

for i = 1, 11 do
	local sprite = Sprite.new()

	sprite.texture:loadFromGame("data/scene/staffroll/moji/moji["..(i < 10 and "0"..tostring(i) or tostring(i)).."].cv2")
	sprite.size = sprite.texture.size
	print(sprite.texture.size.x)
	if i > 1 then
		sprite.position = Vector2i.new(math.floor(rightRect.position.x + rightRect.size.x / 2 - sprite.texture.size.x / 2), creditsImages[i - 1].position.y + creditsImages[i - 1].size.y)
	else
		sprite.position = Vector2i.new(math.floor(rightRect.position.x + rightRect.size.x / 2 - sprite.texture.size.x / 2), 600)
	end
	sprite.rect.width = sprite.texture.size.x
	sprite.rect.height= sprite.texture.size.y
	creditsImages[i] = sprite
end

_sprite.texture:loadFromFile(packPath.."/credits.png")
_sprite.size = _sprite.texture.size
_sprite.position = Vector2i.new(math.floor(rightRect.position.x + rightRect.size.x / 2 - _sprite.texture.size.x / 2), creditsImages[11].position.y + creditsImages[11].size.y)
_sprite.rect.width = _sprite.texture.size.x
_sprite.rect.height= _sprite.texture.size.y
creditsImages[12] = _sprite

_sprite = Sprite.new()
_sprite.texture:loadFromGame("data/scene/staffroll/moji/moji[12].cv2")
_sprite.size = _sprite.texture.size
_sprite.position = Vector2i.new(math.floor(rightRect.position.x + rightRect.size.x / 2 - _sprite.texture.size.x / 2), creditsImages[12].position.y + creditsImages[12].size.y)
_sprite.rect.width = _sprite.texture.size.x
_sprite.rect.height= _sprite.texture.size.y
creditsImages[13] = _sprite

for _, i in pairs(shuffle(randomOrder)) do
	local k = chrs.animations[i]
	local scores = getTrialScores("Babylon's Aurora", chrs.names[i])

	if #scores >= 2 and scores[#scores - 1] ~= -1 then
		characters[#characters + 1] = {
			animArray = loadSprites(i, k),
			currentAnim = "walk",
			animation = 1,
			animCtr = 0,
			character = i,
			total = 0,
			extra = scores[#scores] == 3 and loadObjectSprite(i) or nil
		}
	end
end

for t = #forcedOrder, 1, -1 do
	local i = forcedOrder[t]
	local k = chrs.animations[i]
	local scores = getTrialScores("Babylon's Aurora", chrs.names[i])

	if #scores >= 2 and scores[#scores - 1] ~= -1 then
		characters[#characters + 1] = {
			animArray = loadSprites(i, k),
			currentAnim = "walk",
			animation = 1,
			animCtr = 0,
			character = i,
			total = 0,
			extra = scores[#scores] == 3 and loadObjectSprite(i) or nil
		}
	end
end
if #characters == 0 then
	error("Invalid state")
end

for _, chr in pairs(characters) do
	local anim = chr.animArray[chr.currentAnim][chr.animation]

	chr.position = {
		x = index * 60 - #characters * 60 - 240,
		y = bgs[backgrounds[selectedBg]].down
	}
	anim.sprite.position = Vector2i.new(math.floor(chr.position.x - anim.xoffset / scale), math.floor(chr.position.y - anim.yoffset / scale))
	index = index + 1
end

function update()
	local color2 = popRect.fillColor
	local color = flashRect.fillColor

	if title then
		if color2.a > 0 then
			color2.a = color2.a - 15
			popRect.fillColor = color2
		end

		local val = (ctr - 8000) / 300

		val = val < 0 and 0 or val < 30 and val or 30
		for _ = 0, val do
			if ctr % 3 == 0 then
				titleSprites[2].position = Vector2i.new((titleSprites[2].position.x + 1) % titleSprites[2].size.x, titleSprites[2].position.y)
			end
			titleSprites[3].rotation = math.sin(ctr % 240 * 2 * math.pi / 240) * math.pi / 64
			titleSprites[3].position = Vector2i.new(
				math.floor(468 + math.sin(ctr % 240 * 2 * math.pi / 240) * 10),
				math.floor(330 + math.abs(math.sin(ctr % 240 * 2 * math.pi / 240)) * 20)
			)
			if ctr % 3 <= 1 then
				titleSprites[4].position = Vector2i.new((titleSprites[4].position.x + 1) % titleSprites[4].size.x, titleSprites[4].position.y)
			end
			titleSprites[5].position = Vector2i.new((titleSprites[5].position.x + 1) % titleSprites[5].size.x, titleSprites[5].position.y)
			ctr = ctr + 1
		end
		return true
	end
	ctr = ctr + 1
	for _, k in pairs(creditsImages) do
		k.position = Vector2i.new(k.position.x, k.position.y - 1)
	end
	if ctr > 4170 then
		color2.a = color2.a + 3
		popRect.fillColor = color2
		title = color2.a == 0xFF
	elseif not popped then
		color2.a = color2.a + 5
		if color2.a == 0xFF then
			popped = true
		end
		popRect.fillColor = color2
		return true
	elseif color2.a ~= 0 then
		color2.a = color2.a - 3
		popRect.fillColor = color2
	end

	if color.a ~= 0 and characters[1].position.x < 0 then
		color.a = color.a - 5
		flashRect.fillColor = color
		return true
	elseif color.a == 0xFF then
		index = 0
		selectedSky = math.random(1, 20)

		local old = selectedBg

		repeat
			selectedBg = math.random(1, #backgrounds)
		until backgrounds[old] ~= backgrounds[selectedBg]
		for _, chr in pairs(characters) do
			chr.position = {
				x = index * 60 - #characters * 60 - 40,
				y = bgs[backgrounds[selectedBg]].down
			}
			index = index + 1
		end
		return true
	elseif color.a ~= 0xFF and characters[1].position.x > rightRect.position.x then
		color.a = color.a + 5
		flashRect.fillColor = color
	end

	for _, chr in pairs(characters) do
		local anim = chr.animArray[chr.currentAnim][chr.animation]

		chr.position.x = chr.position.x + 2
		chr.animCtr = chr.animCtr + 1
		chr.total = chr.total + 1
		if chr.animCtr == anim.duration then
			chr.animCtr = 0
			chr.animation = chr.animation + 1
			if chr.animation > #chr.animArray[chr.currentAnim] then
				chr.animation = 1
				chr.total = 0
			end
			anim = chr.animArray[chr.currentAnim][chr.animation]
		end
		if chr.extra then
			local offset = objsOffsets[chr.character]

			chr.extra.position = Vector2i.new(chr.position.x + offset.offset[1], chr.position.y + offset.offset[2])
			if chr.character == "remilia" then
				chr.extra.rotation = offset.rotation
			else
				chr.extra.rotation = math.sin(chr.total * 2 * math.pi / offset.total) * math.pi / 6 + offset.rotation
			end
		end
		anim.sprite.position = Vector2i.new(math.floor(chr.position.x - anim.xoffset / scale), math.floor(chr.position.y - anim.yoffset / scale))
	end
	return true
end

function render()
	if title then
		for i, k in pairs(titleSprites) do
			k:draw()
			if i == 2 or i == 4 or i == 5 then
				if k.position.x > 0 then
					k.position = Vector2i.new(k.position.x - k.size.x, k.position.y)
					k:draw()
					k.position = Vector2i.new(k.position.x + k.size.x, k.position.y)
				end
				if k.position.x + k.size.x < 640 then
					k.position = Vector2i.new(k.position.x + k.size.x, k.position.y)
					k:draw()
					k.position = Vector2i.new(k.position.x - k.size.x, k.position.y)
				end
			end
		end
	else
		if not popped then
			return popRect:draw()
		end
		bg.skies[selectedSky]:draw()
		bgs[backgrounds[selectedBg]]:draw()
		for _, chr in pairs(characters) do
			chr.animArray[chr.currentAnim][chr.animation].sprite:draw()
			if chr.extra then
				chr.extra:draw()
			end
		end
		flashRect:draw()
		rightRect:draw()
	end
	for _, k in pairs(creditsImages) do
		k:draw()
	end
	popRect:draw()
end

function onKeyPressed()
end