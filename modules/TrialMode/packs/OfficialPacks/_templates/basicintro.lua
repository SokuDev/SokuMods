--
-- Created by PinkySmile on 07/05/2021.
--

local dialogs = {
	"r H",
	"r E",
	"r S",
	"r C",
	"r A",
	"r D",
	"r W",
	"r c",
	"r h"
	--Battle here
}

local stageBg = Sprite.new()
local stageBottom = Sprite.new()
local ctr = 240
local currentStage = 0
local dialog = StandDialog.new(dialogs)
local keyPressed = false
local stop = false

print("Init intro.")
stageBg.texture:loadFromFile(packPath.."\\icon.png")
stageBg.size = stageBg.texture.size
stageBg.position = Vector2i.new(
	math.floor(320 - stageBg.texture.size.x / 2),
	-50
)
stageBg.rect.width  = stageBg.texture.size.x
stageBg.rect.height = stageBg.texture.size.y
stageBg.tint.a = 0

stageBottom.texture:loadFromGame("data/scenario/effect/Stage1.png")
stageBottom.size = stageBottom.texture.size
stageBottom.position = Vector2i.new(
	math.floor(320 - stageBg.texture.size.x / 2),
	math.floor(-50 + stageBg.texture.size.y)
)
stageBottom.rect.width  = stageBottom.texture.size.x
stageBottom.rect.height = stageBottom.texture.size.y
stageBottom.tint.a = 0


local function init()
	battleMgr.leftChr.actionBlockId = 0
	battleMgr.leftChr.animationCounter = 0
	battleMgr.leftChr.animationSubFrame = 0
	battleMgr.leftChr.action = enums.actions.ACTION_IDLE
	battleMgr.leftChr:initAnimation()
	battleMgr.rightChr.position.x = 1200
	playBgm("data/bgm/ta00.ogg")
end

local function stage0()
	if ctr == 240 then
		init()
	end
	battleMgr.leftChr:updateAnimation()
	battleMgr.rightChr:updateAnimation()
	if ctr < 60 then
		if stageBg.tint.a ~= 0 then
			stageBg.tint.a = stageBg.tint.a - 0xF
		end
		if stageBottom.tint.a ~= 0 then
			stageBottom.tint.a = stageBottom.tint.a - 0xF
		end
	else
		if stageBg.tint.a ~= 0xFF then
			stageBg.tint.a = stageBg.tint.a + 0xF
		end
		if stageBottom.tint.a ~= 0xFF then
			stageBottom.tint.a = stageBottom.tint.a + 0xF
		end
	end
	if ctr % 2 == 1 then
		stageBottom.position = stageBottom.position + Vector2i.new(1, 0)
	end
	ctr = ctr - 1
	if ctr == 0 then
		dialog.hidden = false
		currentStage = currentStage + 1
	end
end

local function stage1()
	battleMgr.leftChr:updateAnimation()
	battleMgr.rightChr:updateAnimation()
end

local function stage2()
end

local function stage3()
end

local anims = {
	stage0,
	stage1,
	stage2,
	stage3
}

function update()
	dialog:update()
	if currentStage < #anims then
		anims[currentStage + 1]()
	end
	if keyPressed then
		stop = stop or not dialog:onKeyPress()
		if stop then
			dialog.hidden = true
		end
		keyPressed = false
	end
	return not stop or not dialog:isAnimationFinished()
end

function render()
	if ctr ~= 0 then
		stageBg:draw()
		stageBottom:draw()
	end
	dialog:render()
end

function onKeyPressed()
	keyPressed = true
end