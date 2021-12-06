--
-- Created by Yolotemperance and PinkySmile on 17/11/2021.
--

local bool = false

local dialogs = {
	"LA Where could she be?",
	"RAWAre you looking for someone?",
	"LSWYuyuko?",
	"RSHI've seen you spare with Youmu,<br>it was quite interesting!",
	"LHHIt seems you truly like messing with her don't you?",
	"RHhWhat can I say,she is just a peculiar plant<br>who needs more wind than water.",
	"RHCBut perhaps you are of the same kind?",
	"LCCAnyway, I need answers from you.",
	"RChI'm sorry but I'm in a hurry, come back later!",
	"LAhYou won't get away!"
	--Battle here
}

local flashRect = RectangleShape.new()
local stageBg = Sprite.new()
local stageBottom = Sprite.new()
local ctr = 240
local currentStage = 0
local dialog
local keyPressed = false
local stop = false

-- CHR 26 -> 1

local function stage0()
	if ctr == 240 then
		playBGM("data/bgm/ta03.ogg")
		battleMgr.leftChr.position.x = -100
		battleMgr.leftChr.direction = enums.directions.RIGHT
		battleMgr.rightChr.position.x = 1200
		battleMgr.rightChr.position.y = 0
		battleMgr.rightChr.direction = enums.directions.LEFT
	end
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
		currentStage = currentStage + 1
		battleMgr.leftChr.action = enums.actions.ACTION_FORWARD_JUMP
		playSfx(enums.sfxs.highJump)
		battleMgr.leftChr:initAnimation()
		battleMgr.leftChr.speed.x = 15
		battleMgr.leftChr.speed.y = 15

	end
end

local function stage1()
	battleMgr.leftChr:updateAnimation()
	battleMgr.leftChr.position = battleMgr.leftChr.position + battleMgr.leftChr.speed
	battleMgr.leftChr.speed.y = battleMgr.leftChr.speed.y - 0.7
	if battleMgr.leftChr.position.y <= 0 then
		currentStage = currentStage + 1
		playSfx(enums.sfxs.land)
		battleMgr.leftChr.position.y = 0
		battleMgr.leftChr.action = enums.actions.ACTION_LANDING
		battleMgr.leftChr:initAnimation()
		battleMgr.leftChr.speed.x = 0
		battleMgr.leftChr.speed.y = 0
	end
end

local function stage2()
	battleMgr.leftChr:updateAnimation()
	if
		battleMgr.leftChr.action == enums.actions.ACTION_LANDING and
		battleMgr.leftChr.actionBlockId == 0 and
		battleMgr.leftChr.animationCounter == 0 and
		battleMgr.leftChr.animationSubFrame == 0
	then
		battleMgr.leftChr.action = enums.actions.ACTION_IDLE
		battleMgr.leftChr:initAnimation()
		currentStage = currentStage + 1
		ctr = 30
	end
end

local function stage3()
	ctr = ctr - 1
	battleMgr.leftChr:updateAnimation()
	if ctr == 0 then
		dialog.hidden = false
	end
	if keyPressed then
		currentStage = currentStage + 1
		dialog.hidden = true
		battleMgr.rightChr.action = enums.actions.ACTION_FORWARD_DASH
		playSfx(enums.sfxs.dash)
		battleMgr.rightChr:initAnimation()
	end
end

local function stage4()
	battleMgr.leftChr:updateAnimation()
	battleMgr.rightChr:updateAnimation()
	if battleMgr.rightChr.position.y <= 0 then
		battleMgr.rightChr.position.y = 0
		battleMgr.rightChr:animate()
		battleMgr.rightChr:playSfx(012)
		currentStage = currentStage + 1
	end
end

local function stage5()
	battleMgr.leftChr:updateAnimation()
	battleMgr.rightChr:updateAnimation()
	battleMgr.rightChr.position.x = battleMgr.rightChr.position.x - 15
	if battleMgr.rightChr.position.x == 900 then
		battleMgr.rightChr.action = enums.actions.ACTION_IDLE
		battleMgr.rightChr:initAnimation()
		currentStage = currentStage + 1
	end
end

local function stage6()
	battleMgr.leftChr:updateAnimation()
	battleMgr.rightChr:updateAnimation()
	if dialog.hidden == true then
		dialog.hidden = false
		onKeyPressed()
	end
end

local anims = {
	stage0,
	stage1,
	stage2,
	stage3,
	stage4,
	stage5,
	stage6
}

stageBg.texture:loadFromFile(packPath.."/lore6intro.png")
stageBg.size = stageBg.texture.size
stageBg.position = Vector2i.new(
	math.floor(320 - stageBg.texture.size.x / 2),
	-50
)
stageBg.rect.width  = stageBg.texture.size.x
stageBg.rect.height = stageBg.texture.size.y
stageBg.tint.a = 0

stageBottom.texture:loadFromGame("data/scenario/effect/Stage6.png")
stageBottom.size = stageBottom.texture.size
stageBottom.position = Vector2i.new(
	math.floor(320 - stageBg.texture.size.x / 2),
	math.floor(-50 + stageBg.texture.size.y)
)
stageBottom.rect.width  = stageBottom.texture.size.x
stageBottom.rect.height = stageBottom.texture.size.y
stageBottom.tint.a = 0

flashRect.size = Vector2u.new(640, 480)
flashRect.borderColor = enums.colors.Transparent
flashRect.fillColor   = enums.colors.Transparent

dialog = StandDialog.new(dialogs)

function update()
	dialog:update()
	if currentStage < #anims then
		anims[currentStage + 1]()
	end
	battleMgr.rightChr.objects:update()
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
	stageBg:draw()
	stageBottom:draw()
	flashRect:draw()
	dialog:render()
end

function onKeyPressed()
	keyPressed = true
end