--
-- Created by PinkySmile on 12/08/2021.
--

local dialogs = {
	--Remilia passing by
	--Cirno standing proudly in her way,
	"r WHey!",
	--Remilia ignoring and confusing her (animation turning on herself?),
	"r AStop right here!",
	"lAAA maid like you shall not stop her mistress...<br>Didn't Sakuya teach you anything?",
	"rAEI'm not sure what you're talking about but...",
	"rAhI'm going to beat you!",
	"lSh...",
	"lhhI didn't know we had a jester.",
	"rhAI'm not a jester, I'm a fairy!",
	"rhWNow prepare yourself!",
	"lHWYou're pretty good!",
	"lWWBut you should be careful what you wish for.",
	--Battle here
}

local stageBg = Sprite.new()
local stageBottom = Sprite.new()
local ctr = 240
local currentStage = 0
local dialog
local keyPressed = false
local stop = false

local function init()
	playBGM("data/bgm/ta04.ogg")

	camera.translate.x = -20
	camera.translate.y = 840
	camera.scale = 0.5

	battleMgr.leftChr.position.x = 1450
	battleMgr.leftChr.action = enums.actions.ACTION_WALK_FORWARD
	battleMgr.leftChr.direction = enums.directions.LEFT
	battleMgr.leftChr:initAnimation()

	battleMgr.rightChr.position.x = 650
	battleMgr.rightChr.action = enums.actions.ACTION_WALK_FORWARD
	battleMgr.rightChr:initAnimation()
end

local function stage0()
	if ctr == 240 then
		init()
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

	battleMgr.rightChr:updateAnimation()
	battleMgr.rightChr.position.x = battleMgr.rightChr.position.x + battleMgr.rightChr.direction * 2.5
	if battleMgr.rightChr.direction == enums.directions.LEFT and battleMgr.rightChr.position.x <= 550 then
		battleMgr.rightChr.direction = enums.directions.RIGHT
	elseif battleMgr.rightChr.direction == enums.directions.RIGHT and battleMgr.rightChr.position.x >= 750 then
		battleMgr.rightChr.direction = enums.directions.LEFT
	end

	if ctr % 2 == 1 then
		stageBottom.position = stageBottom.position + Vector2i.new(1, 0)
	end
	ctr = ctr - 1
	if ctr == 0 then
		currentStage = currentStage + 1
	end
end

local function stage1()
	battleMgr.rightChr:updateAnimation()
	battleMgr.rightChr.position.x = battleMgr.rightChr.position.x + battleMgr.rightChr.direction * 2.5
	if battleMgr.rightChr.position.x >= 850 then
		battleMgr.rightChr.action = enums.actions.ACTION_IDLE
		battleMgr.rightChr:initAnimation()
		currentStage = currentStage + 1
	end
end

local function stage2()
	keyPressed = false
	battleMgr.leftChr:updateAnimation()
	battleMgr.rightChr:updateAnimation()
	battleMgr.leftChr.position.x = battleMgr.leftChr.position.x - 6
	if battleMgr.leftChr.position.x == 1240 then
		dialog.hidden = false
	end
	if battleMgr.leftChr.position.x == 1024 then
		dialog.hidden = true
	end
	if battleMgr.leftChr.position.x == 874 then
		battleMgr.rightChr.action = enums.actions.ACTION_STAND_GROUND_HIT_HUGE_HITSTUN
		battleMgr.rightChr:initAnimation()
	end
	if battleMgr.leftChr.position.x == 634 then
		battleMgr.rightChr.action = enums.actions.ACTION_IDLE
		battleMgr.rightChr.direction = enums.directions.LEFT
		battleMgr.rightChr:initAnimation()
	end
	if battleMgr.leftChr.position.x == 574 then
		dialog.hidden = false
		dialog:onKeyPress()
	end
	if battleMgr.leftChr.position.x == 424 then
		currentStage = currentStage + 1
		battleMgr.leftChr.action = enums.actions.ACTION_IDLE
		battleMgr.leftChr.direction = enums.directions.RIGHT
		battleMgr.leftChr:initAnimation()
	end
end

local function stage3()
	battleMgr.leftChr:updateAnimation()
	battleMgr.rightChr:updateAnimation()
	if camera.translate.x > -320 then
		camera.scale = camera.scale + 0.01
		camera.translate.x = 320 / camera.scale - 640
		camera.translate.y = 420 / camera.scale
	else
		camera.translate.x = -320
		camera.translate.y = 420
		camera.scale = 1
		currentStage = currentStage + 1
	end
end

local function stage4()
	battleMgr.leftChr:updateAnimation()
	battleMgr.rightChr:updateAnimation()
end

local function stage5()
end

local function stage6()
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

print("Init intro.")

stageBg.texture:loadFromFile(packPath.."/bnbcorn2intro.png")
stageBg.size = stageBg.texture.size
stageBg.position = Vector2i.new(
	math.floor(320 - stageBg.texture.size.x / 2),
	-50
)
stageBg.rect.width  = stageBg.texture.size.x
stageBg.rect.height = stageBg.texture.size.y
stageBg.tint.a = 0

stageBottom.texture:loadFromGame("data/scenario/effect/Stage7.png")
stageBottom.size = stageBottom.texture.size
stageBottom.position = Vector2i.new(
	math.floor(320 - stageBg.texture.size.x / 2),
	math.floor(-50 + stageBg.texture.size.y)
)
stageBottom.rect.width  = stageBottom.texture.size.x
stageBottom.rect.height = stageBottom.texture.size.y
stageBottom.tint.a = 0

dialog = StandDialog.new(dialogs)

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
	stageBg:draw()
	stageBottom:draw()
	dialog:render()
end

function onKeyPressed()
	keyPressed = true
end