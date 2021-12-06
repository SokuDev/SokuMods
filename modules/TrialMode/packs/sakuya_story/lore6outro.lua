--
-- Created by PinkySmile on 12/08/2021.
--

local bool = false

local dialogs = {
	"LHDSo now will you tell me what is going on?",
	"RHDYou should come back to your first clue,<br>that will help you I'm sure.",
	"LcDCan't you just explain to me right now?",
	"REEI could,<br>but I believe it would be best to find out by yourself.",
	"LHEIt is fine,<br>your explanations are always too cryptic anyway.",
	"LWEWell I have someone else to ask questions thanks to you now,<br>so I must take my leave.",
	"R CIt seems she is going for the right source...",
	"R CSadly for her it is the flow,<br>and not the spring that matters this time...",
	"R CSadly for her it is the flow,<br>and not the spring that matters this time...<br>isn't it?",
	"R ASadly for her it is the flow,<br>and not the spring that matters this time...<br>isn't it?",
	"R CSadly for her it is the flow,<br>and not the spring that matters this time...<br>isn't it?",
	"   Here you are!"
	--Battle here
}

local flashRect = RectangleShape.new()
local stageBg = Sprite.new()
local stageBottom = Sprite.new()
local ctr = 0
local currentStage = 0
local dialog
local keyPressed = false
local stop = false
local gravity = 0
local playerInfo = PlayerInfo.new(enums.characters.yukari, 0, false)
-- CHR 26 -> 1

local function stage0()
	playBGM("data/bgm/ta03.ogg")
	battleMgr.leftChr.position.x = 500
	battleMgr.rightChr.position.x = 800
	battleMgr.leftChr.action = enums.actions.ACTION_IDLE
	battleMgr.leftChr:initAnimation()
	battleMgr.leftChr.direction = enums.directions.RIGHT
	battleMgr.rightChr.action = enums.actions.ACTION_KNOCKED_DOWN_STATIC
	battleMgr.rightChr:initAnimation()
	battleMgr.rightChr.direction = enums.directions.LEFT
	currentStage = currentStage + 1
end

local function stage1()
	battleMgr.leftChr:updateAnimation()
	battleMgr.rightChr:updateAnimation()
	dialog.hidden = false
	if #dialog == #dialogs - 7 then
		currentStage = currentStage + 1
	end
end

local function stage2()
	battleMgr.rightChr:updateAnimation()
	battleMgr.leftChr:updateAnimation()
	if dialog:onKeyPress() == true then
		dialog.hidden = true
		battleMgr.leftChr.action = enums.actions.ACTION_FORWARD_HIGH_JUMP
		battleMgr.leftChr:initAnimation()
		playSfx(enums.sfxs.highJump)
		battleMgr.leftChr.direction = enums.directions.LEFT
		currentStage = currentStage + 1
	end
end

local function stage3()
	battleMgr.rightChr:updateAnimation()
	battleMgr.leftChr:updateAnimation()
	gravity = gravity + 0.5
	battleMgr.leftChr.position.x = battleMgr.leftChr.position.x - 15
	battleMgr.leftChr.position.y = battleMgr.leftChr.position.y + 15 - gravity
	if 	battleMgr.leftChr.position.x <= -100 and battleMgr.leftChr.position.x >= -115 then
		battleMgr.rightChr.action = enums.actions.ACTION_NEUTRAL_TECH
		battleMgr.rightChr:initAnimation()
	end
	if 	battleMgr.leftChr.position.x <= -400 then
		currentStage = currentStage + 1
	end
end

local function stage4()
	battleMgr.leftChr:updateAnimation()
	battleMgr.rightChr:updateAnimation()
	if battleMgr.rightChr.animationCounter == 0 then
		battleMgr.rightChr.action = enums.actions.ACTION_IDLE
		battleMgr.rightChr:initAnimation()
		print("test2")
		currentStage = currentStage + 1
	end
end

local function stage5()
	battleMgr.leftChr:updateAnimation()
	battleMgr.rightChr:updateAnimation()
	dialog.hidden = false
end

local function stage6()
	battleMgr.leftChr:updateAnimation()
	battleMgr.rightChr:updateAnimation()
	while battleMgr.rightChr.actionBlockId ~= 1 do
		battleMgr.rightChr:animate()
	end
	if dialog.hidden == true then
		dialog.hidden = false
		onKeyPressed()
	end
	if #dialog == #dialogs - 6 then
		currentStage = currentStage + 1
	end
end

local function stage7()
	battleMgr.leftChr:updateAnimation()
	battleMgr.rightChr:updateAnimation()
	if battleMgr.rightChr.action == enums.actions.ACTION_DEFAULT_SKILL4_B and battleMgr.rightChr.actionBlockId == 0 then
		battleMgr.rightChr.action = enums.actions.ACTION_IDLE
		battleMgr.rightChr:initAnimation()
	end
	if bool == true then
		bool = false
	end
end

local anims = {
	stage0,
	stage1,
	stage2,
	stage3,
	stage4,
	stage5,
	stage6,
	stage7
}

stageBg.texture:loadFromFile(packPath.."/lore5intro.png")
stageBg.size = stageBg.texture.size
stageBg.position = Vector2i.new(
	math.floor(320 - stageBg.texture.size.x / 2),
	-50
)
stageBg.rect.width  = stageBg.texture.size.x
stageBg.rect.height = stageBg.texture.size.y
stageBg.tint.a = 0

stageBottom.texture:loadFromGame("data/scenario/effect/Stage5.png")
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