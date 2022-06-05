local function loadBackgroundType1(id)
	local sprites = {}
	local i = 0

	while i < 30 do
		repeat
			local sprite = Sprite.new()

			if sprite.texture:loadFromGame("data/background/bg"..(id < 10 and "0"..tostring(id) or tostring(id)).."/0000_"..(i < 10 and "0"..tostring(i) or tostring(i))..".cv2") then
				local size = (sprite.texture.size * 0.5)

				sprite.size = Vector2u.new(math.floor(size.x), math.floor(size.y))
				sprite.rect.width  = sprite.texture.size.x
				sprite.rect.height = sprite.texture.size.y
				sprite.position = Vector2i.new(math.floor(-30 + i % 6 * 128 - offset / 2), -60 + math.floor(i / 6) * 128)
			end
			sprites[#sprites + 1] = sprite
			i = i + 1
		until i % 6 == 0
	end
	return sprites
end

local function loadBackgroundType2(id)
	local sprites = {}

	for y = 1, 5 do
		for x = 1, 6 do
			local sprite = Sprite.new()

			if sprite.texture:loadFromGame("data/background/bg"..tostring(id).."/0000[0"..tostring(y).."][0"..tostring(x).."].cv2") then
				local size = (sprite.texture.size * 0.5)

				sprite.size = Vector2u.new(math.floor(size.x), math.floor(size.y))
				sprite.rect.width  = sprite.texture.size.x
				sprite.rect.height = sprite.texture.size.y
				sprite.position = Vector2i.new(math.floor(-30 + (x - 1) * 128 - offset / 2), -60 + (y - 1) * 128)
			end
			sprites[#sprites + 1] = sprite
		end
	end
	return sprites
end

local function loadSprite(path)
	local sprite = Sprite.new()

	sprite.texture:loadFromGame(path)

	local size = (sprite.texture.size * 2)

	sprite.size = Vector2u.new(math.floor(size.x), math.floor(size.y))
	sprite.rect.width  = sprite.texture.size.x
	sprite.rect.height = sprite.texture.size.y
	sprite.position = Vector2i.new(math.floor(-offset / 2), 0)
	return sprite
end

local function render(self)
	for _, k in pairs(self.sprites) do
		k:draw()
	end
end

local function renderHall(self)
	render(self)
end

return {
	backgrounds = {
		[0] = {
			sprites = loadBackgroundType1(0),
			draw = render,
			down = 400
		},
		[1] = {
			sprites = loadBackgroundType1(1),
			draw = render,
			down = 400
		},
		[2] = {
			sprites = loadBackgroundType1(2),
			draw = render,
			down = 400,
			broken = "somewhat"
		},
		[3] = {
			sprites = loadBackgroundType1(3),
			draw = render,
			down = 360
		},
		[4] = {
			sprites = loadBackgroundType1(4),
			draw = render,
			down = 400
		},
		[5] = {
			sprites = loadBackgroundType1(5),
			draw = render,
			down = 400
		},
		[6] = {
			sprites = loadBackgroundType1(6),
			draw = render,
			down = 400
		},
		[10] = {
			sprites = loadBackgroundType1(10),
			draw = render,
			down = 430
		},
		[11] = {
			sprites = loadBackgroundType1(11),
			draw = render,
			down = 430
		},
		[12] = {
			sprites = loadBackgroundType1(12),
			draw = render,
			down = 430,
			broken = "somewhat"
		},
		[13] = {
			sprites = loadBackgroundType1(13),
			draw = render,
			down = 430
		},
		[14] = {
			sprites = loadBackgroundType1(14),
			draw = render,
			down = 400
		},
		[15] = {
			sprites = loadBackgroundType1(15),
			draw = render,
			down = 430
		},
		[16] = {
			sprites = loadBackgroundType1(16),
			draw = renderHall,
			down = 400,
			broken = true
		},
		[17] = {
			sprites = loadBackgroundType1(17),
			draw = render,
			down = 430
		},
		[18] = {
			sprites = loadBackgroundType1(18),
			draw = render,
			down = 430
		},
		[30] = {
			sprites = loadBackgroundType2(30),
			draw = render,
			down = 430
		},
		[31] = {
			sprites = loadBackgroundType2(31),
			draw = render,
			down = 430
		},
		[32] = {
			sprites = loadBackgroundType2(32),
			draw = render,
			down = 400
		},
		[33] = {
			sprites = loadBackgroundType2(33),
			draw = render,
			down = 400,
			broken = true
		},
		[34] = {
			sprites = loadBackgroundType2(34),
			draw = render,
			down = 400,
			broken = true
		},
		[35] = {
			sprites = loadBackgroundType2(35),
			draw = render,
			down = 400,
			broken = true
		},
		[36] = {
			sprites = loadBackgroundType2(36),
			draw = render,
			down = 430,
			broken = "somewhat"
		},
		[37] = {
			sprites = loadBackgroundType2(37),
			draw = render,
			down = 430,
			broken = "somewhat"
		},
		[38] = {
			sprites = loadBackgroundType2(38),
			draw = render,
			down = 400,
			broken = true
		}
	},
	skies = {
		loadSprite("data/weather/00.cv0"),
		loadSprite("data/weather/01.cv0"),
		loadSprite("data/weather/02.cv0"),
		loadSprite("data/weather/03.cv0"),
		loadSprite("data/weather/04.cv0"),
		loadSprite("data/weather/05.cv0"),
		loadSprite("data/weather/06.cv0"),
		loadSprite("data/weather/07.cv0"),
		loadSprite("data/weather/08.cv0"),
		loadSprite("data/weather/09.cv0"),
		loadSprite("data/weather/10.cv0"),
		loadSprite("data/weather/11.cv0"),
		loadSprite("data/weather/12.cv0"),
		loadSprite("data/weather/13.cv0"),
		loadSprite("data/weather/14.cv0"),
		loadSprite("data/weather/15.cv0"),
		loadSprite("data/weather/16.cv0"),
		loadSprite("data/weather/17.cv0"),
		loadSprite("data/weather/18.cv0"),
		loadSprite("data/weather/19.cv0"),
	}
}