--
-- Created by Yolotemperance and PinkySmile on 17/11/2021.
--

local bool = false

local dialogs = {
	"LWDSo? Will you tell me?",
	"RWD...",
	"RWHFine, It will add a bit more flavor to the story.",
	"LSHWhat do you mean?",
	"RScI do not know who it is that infiltrated the manor...<br>But I do know that this mysterious individual<br>passed by the hakurei shrine untouched.",
	"LccDid he beat Reimu?",
	"RcENot at all, he didn't fight her.",
	"LSEHow strange,<br>Reimu does have a sixth sense when it comes to trouble.",
	"LcEShe wouldn't let anyone suspicious go by,<br>however lazy she can be...",
	"LHEI was right, it must be someone quite stealthy...<br>Or someone able to manipulate others...",
	"LCEThere is one person,<br>and she fits the description quite nicely...",
	"RCDthat sure is a long day. At least it must be good for you?",
	"LCDIt is Perfect!",
	"RCcDo you know who she is assuming to be the culprit?",
	"LHcYes I do, truth is,<br>I have already made an arrangement with her.",
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
		battleMgr.rightChr.position.x = 50
		battleMgr.rightChr.position.y = 0
		battleMgr.rightChr.direction = enums.directions.RIGHT
		camera.translate.x = -300
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
		battleMgr.leftChr.action = enums.actions.ACTION_IDLE
		battleMgr.leftChr:initAnimation()
		currentStage = currentStage + 1
		battleMgr.leftChr.action = enums.actions.ACTION_FORWARD_JUMP
		playSfx(enums.sfxs.highJump)
		battleMgr.leftChr:initAnimation()
		battleMgr.leftChr.speed.x = 14
		battleMgr.leftChr.speed.y = 15
	end
end

local function stage1()
	battleMgr.leftChr:updateAnimation()
	battleMgr.leftChr.position = battleMgr.leftChr.position + battleMgr.leftChr.speed
	battleMgr.leftChr.speed.y = battleMgr.leftChr.speed.y - 0.7
	if battleMgr.leftChr.position.y <= 0 then
		playSfx(enums.sfxs.land)
		battleMgr.leftChr.position.y = 0
		battleMgr.leftChr.action = enums.actions.ACTION_LANDING
		battleMgr.leftChr:initAnimation()
		battleMgr.leftChr.speed.x = 0
		battleMgr.leftChr.speed.y = 0
		currentStage = currentStage + 1
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
		dialog.hidden = false
		ctr = 30
	end
	if keyPressed == true and battleMgr.leftChr.action == enums.actions.ACTION_IDLE then
		dialog.hidden = true
		battleMgr.rightChr:playSfx(000)
		battleMgr.leftChr.action = enums.actions.ACTION_GROUND_CRUSHED
		battleMgr.leftChr.direction = enums.directions.LEFT
		battleMgr.leftChr:initAnimation()
		currentStage = currentStage + 1
	end
end

local function stage3()
	battleMgr.leftChr:updateAnimation()
	battleMgr.rightChr:updateAnimation()
	camera.translate.x = camera.translate.x + 10
	camera.backgroundTranslate.x = camera.backgroundTranslate.x -10
	if camera.translate.x >= 20 then
		ctr = 25
		currentStage = currentStage + 1
	end
end

local function stage4()
	battleMgr.leftChr:updateAnimation()
	battleMgr.rightChr:updateAnimation()
	ctr = ctr - 1
	if ctr == 2 then
		battleMgr.leftChr:animate()
	end
	if ctr == 0 then
		battleMgr.leftChr.action = enums.actions.ACTION_IDLE
		battleMgr.leftChr:initAnimation()
		dialog.hidden = false
		dialog:onKeyPress()
	end
end

local anims = {
	stage0,
	stage1,
	stage2,
	stage3,
	stage4
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