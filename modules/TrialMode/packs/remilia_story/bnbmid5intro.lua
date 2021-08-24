--
-- Created by PinkySmile on 12/08/2021.
--

local dialogs = {
	--Remilia coming in fast (animation 22b or 6a?)
        --Marisa walking (slow animation to manifest overload?)
        "lS Oh?",
        "lC You're here too...",
        "rCEThanks for the intel.",
        "rCcOf all people it had to be you huh?",
        "lWcYou don't appreciate a good old friend visiting you?<br>Well, it is more you visiting me I presume...",
        "lccAre you here for the intruder too?",
        "rWS...",
        "rWEYes...?",
        --taking a step back
        "lcE...",
        "rchYes of course, why else would I be there?!",
        --Remilia taking two step forward,
        --Marisa scared?
        "lAhYou won't be able to evade any attack<br>if you're carrying too much stuff, you know?"
	--Battle here
}

local stageBg = Sprite.new()
local stageBottom = Sprite.new()
local ctr = 240
local currentStage = 0
local dialog
local keyPressed = false
local stop = false

-- CHR 26 -> 1

local function init()
	playBGM("data/bgm/ta00.ogg")
	camera.translate.y = 525
	camera.scale = 0.8

	battleMgr.leftChr.position.x = 0
	battleMgr.leftChr.position.y = 320
	battleMgr.leftChr.action = enums.actions.ACTION_j6A
	battleMgr.leftChr:initAnimation()
	while battleMgr.leftChr.animationCounter ~= 4 do
		battleMgr.leftChr:updateAnimation()
	end

	battleMgr.rightChr.position.x = 700
	battleMgr.rightChr.action = enums.actions.ACTION_WALK_BACKWARD
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
	if ctr % 2 == 1 then
		stageBottom.position = stageBottom.position + Vector2i.new(1, 0)
		battleMgr.rightChr:updateAnimation()
	end
	battleMgr.rightChr.position.x = battleMgr.rightChr.position.x + 1
	ctr = ctr - 1
	if ctr == 0 then
		currentStage = currentStage + 1
		battleMgr.leftChr:playSfx(14)
	end
end

local function stage1()
	if ctr % 2 == 1 then
		battleMgr.rightChr:updateAnimation()
	end
	if battleMgr.leftChr.position.x > 480 and camera.scale < 1 then
		camera.translate.x = camera.translate.x - 40
		camera.scale = camera.scale + 0.05
		camera.translate.y = 420 / camera.scale
	end
	if battleMgr.leftChr.position.y > 0 then
		battleMgr.leftChr.position = battleMgr.leftChr.position + Vector2f.new(40, -20)
	else
		battleMgr.leftChr.position.y = 0
		battleMgr.leftChr:animate()
		battleMgr.rightChr.action = enums.actions.ACTION_GROUND_CRUSHED
		battleMgr.rightChr:initAnimation()
		currentStage = currentStage + 1
	end
	ctr = ctr + 1
	battleMgr.rightChr.position.x = battleMgr.rightChr.position.x + 1
end

local function stage2()
	ctr = ctr + 1
	if battleMgr.leftChr.animationCounter ~= 2 or ctr % 8 == 0 then
		battleMgr.leftChr:updateAnimation()
	end
	battleMgr.rightChr:updateAnimation()
	if battleMgr.leftChr.actionBlockId ~= 1 then
		currentStage = currentStage + 1
		battleMgr.leftChr.action = enums.actions.ACTION_IDLE
		battleMgr.leftChr:initAnimation()
		battleMgr.rightChr.action = enums.actions.ACTION_IDLE
		battleMgr.rightChr:initAnimation()
		dialog.hidden = false
	end
end

local function stage3()
	battleMgr.leftChr:updateAnimation()
	battleMgr.rightChr:updateAnimation()
end

local function stage4()
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

stageBg.texture:loadFromFile(packPath.."/bnbmid5intro.png")
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