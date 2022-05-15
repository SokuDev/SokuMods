--
-- Created by PinkySmile on 12/08/2021.
--

local flashRect = RectangleShape.new()
local stageBg = Sprite.new()
local stageBottom = Sprite.new()
local ctr = 240
local currentStage = 0
local dialog
local keyPressed = false
local stop = false
local loc = getLocal()
local dialogs, err = pcall(dofile, packPath.."/bnbmid1intro_"..loc..".dialogs")

if not dialogs then
	if err == "cannot open "..packPath.."/bnbmid1intro_"..loc..".dialogs: No such file or directory" then
		dialogs = dofile(packPath.."/bnbmid1intro.dialogs")
	else
		error(err)
	end
else
	dialogs = err
end

-- CHR 26 -> 1

local function stage0()
	if ctr == 240 then
		playBGM("data/bgm/ta01.ogg")
	end
	camera.translate.y = 820
	camera.backgroundTranslate.y = -400
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
		battleMgr.leftChr.position.x = 400
		battleMgr.leftChr.actionBlockId = 0
		battleMgr.leftChr.animationCounter = 0
		battleMgr.leftChr.animationSubFrame = 0
		battleMgr.leftChr.action = enums.actions.ACTION_WALK_FORWARD
		battleMgr.leftChr:initAnimation()
		battleMgr.rightChr.position.x = 1200
		ctr = 3
	end
end

local function stage1()
	battleMgr.leftChr:updateAnimation()
	if camera.backgroundTranslate.y ~= 0 then
		camera.backgroundTranslate.y = camera.backgroundTranslate.y + 1
		camera.translate.y = camera.translate.y - 1
	end
	battleMgr.leftChr.position.x = battleMgr.leftChr.position.x + 5 * battleMgr.leftChr.direction
	if battleMgr.leftChr.direction == enums.directions.RIGHT and battleMgr.leftChr.position.x > 800 then
		battleMgr.leftChr.direction = enums.directions.LEFT
	elseif battleMgr.leftChr.direction == enums.directions.LEFT and battleMgr.leftChr.position.x < 400 then
		battleMgr.leftChr.direction = enums.directions.RIGHT
		ctr = ctr - 1
	end
	if ctr == 0 then
		battleMgr.leftChr.action = enums.actions.ACTION_IDLE
		battleMgr.leftChr:initAnimation()
		currentStage = currentStage + 1
		dialog.hidden = false
	end
end

local function stage2()
	battleMgr.leftChr:updateAnimation()
	if  keyPressed then
		currentStage = currentStage + 1
		dialog.hidden = true
		dialog:finishAnimations()
		battleMgr.rightChr.position.x = 800
		battleMgr.rightChr.actionBlockId = 0
		battleMgr.rightChr.animationCounter = 0
		battleMgr.rightChr.animationSubFrame = 0
		battleMgr.rightChr.action = enums.actions.ACTION_IDLE
		battleMgr.rightChr:initAnimation()
		battleMgr.leftChr.actionBlockId = 0
		battleMgr.leftChr.animationCounter = 0
		battleMgr.leftChr.animationSubFrame = 0
		battleMgr.leftChr.action = enums.actions.ACTION_STAND_GROUND_HIT_HUGE_HITSTUN
		battleMgr.leftChr:initAnimation()
		flashRect.fillColor = enums.colors.White
		playSfx(enums.sfxs.mishagujiSamaHit)
	end
end

local function stage3()
	battleMgr.rightChr:updateAnimation()
	if flashRect.fillColor.a > 1 then
		local color = flashRect.fillColor

		color.a = color.a - 0x2
		flashRect.fillColor = color
	else
		flashRect.fillColor = enums.colors.Transparent
		currentStage = currentStage + 1
		battleMgr.leftChr.actionBlockId = 0
		battleMgr.leftChr.animationCounter = 0
		battleMgr.leftChr.animationSubFrame = 0
		battleMgr.leftChr.action = enums.actions.ACTION_IDLE
		battleMgr.leftChr:initAnimation()
		ctr = 12
	end
end

local function stage4()
	battleMgr.leftChr:updateAnimation()
	battleMgr.rightChr:updateAnimation()
	if ctr ~= 0 then
		ctr = ctr - 1
	elseif not stop and dialog.hidden then
		dialog.hidden = false
		dialog:onKeyPress()
	end
	if #dialog == #dialogs - 6 and keyPressed then
		dialog.hidden = true
		currentStage = currentStage + 1
		battleMgr.leftChr.actionBlockId = 0
		battleMgr.leftChr.animationCounter = 0
		battleMgr.leftChr.animationSubFrame = 0
		battleMgr.leftChr.action = enums.actions.ACTION_ALT2_SKILL3_B
		battleMgr.leftChr:initAnimation()
	end
end

function block0(old)
	if old == 4 and battleMgr.leftChr.animationCounter == 5 then
		battleMgr.leftChr:playSfx(26)
	end
	if battleMgr.leftChr.animationCounter >= 5 then
		battleMgr.leftChr.position.x = battleMgr.leftChr.position.x + 15
		if battleMgr.rightChr.position.x - battleMgr.leftChr.position.x < 60 then
			battleMgr.leftChr.animationCounter = 0
			battleMgr.leftChr.animationSubFrame = 0
			battleMgr.leftChr:animate()

			battleMgr.rightChr.actionBlockId = 0
			battleMgr.rightChr.animationCounter = 0
			battleMgr.rightChr.animationSubFrame = 0
			battleMgr.rightChr.action = enums.actions.ACTION_STAND_GROUND_HIT_SMALL_HITSTUN
			battleMgr.rightChr:initAnimation()
		end
	end
end

function block1()
	if battleMgr.leftChr.animationCounter == 3 then
		battleMgr.leftChr.animationCounter = 0
		battleMgr.leftChr.animationSubFrame = 0
		battleMgr.leftChr:animate()
		battleMgr.leftChr:animate()
		playSfx(6)
		battleMgr.leftChr.speed = Vector2f.new(-10, 12.5)
		battleMgr.leftChr.position = battleMgr.leftChr.position + battleMgr.leftChr.speed
	end
end

function block2()
	error("Bad animation block")
end

function block3(old)
	battleMgr.leftChr.speed = battleMgr.leftChr.speed + Vector2f.new(0.1, -0.75)
	battleMgr.leftChr.position = battleMgr.leftChr.position + battleMgr.leftChr.speed
	if battleMgr.leftChr.animationCounter == 9 and battleMgr.leftChr.animationSubFrame == 2 then
		battleMgr.leftChr.animationCounter = 0
		battleMgr.leftChr.animationSubFrame = 0
		battleMgr.leftChr:animate()
	end
end

function block4()
	battleMgr.leftChr.position = battleMgr.leftChr.position + battleMgr.leftChr.speed
	battleMgr.leftChr.speed = battleMgr.leftChr.speed + Vector2f.new(0.1, -0.75)
	if battleMgr.leftChr.position.y <= 0 and battleMgr.leftChr.speed.y < 0 then
		battleMgr.leftChr.position.y = 0
		battleMgr.leftChr.speed.x = 0
		battleMgr.leftChr.speed.y = 0
		playSfx(enums.sfxs.land)

		battleMgr.leftChr.animationCounter = 0
		battleMgr.leftChr.animationSubFrame = 0
		battleMgr.leftChr:animate()

		battleMgr.rightChr.actionBlockId = 0
		battleMgr.rightChr.animationCounter = 0
		battleMgr.rightChr.animationSubFrame = 0
		battleMgr.rightChr.action = enums.actions.ACTION_IDLE
		battleMgr.rightChr:initAnimation()
	end
end

function block5()
	if battleMgr.leftChr.animationCounter == 6 and battleMgr.leftChr.animationSubFrame == 4 then
		battleMgr.leftChr.actionBlockId = 0
		battleMgr.leftChr.animationCounter = 0
		battleMgr.leftChr.animationSubFrame = 0
		battleMgr.leftChr.action = enums.actions.ACTION_IDLE
		battleMgr.leftChr:initAnimation()
		dialog.hidden = false
		dialog:onKeyPress()
		currentStage = currentStage + 1
	end
end

local animBlocks = {
	block0,
	block1,
	block2,
	block3,
	block4,
	block5
}

local function stage5()
	local old = battleMgr.leftChr.animationCounter

	battleMgr.leftChr:updateAnimation()
	if battleMgr.rightChr.animationCounter ~= 1 or battleMgr.rightChr.action ~= enums.actions.ACTION_STAND_GROUND_HIT_SMALL_HITSTUN then
		battleMgr.rightChr:updateAnimation()
	end
	animBlocks[battleMgr.leftChr.actionBlockId + 1](old)
end

local function stage6()
	battleMgr.leftChr:updateAnimation()
	battleMgr.rightChr:updateAnimation()
	if #dialog == #dialogs - 14 then
		currentStage = currentStage + 1
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

print("Init intro.")

if not stageBg.texture:loadFromFile(packPath.."\\bnbmid1intro_"..loc..".png") then
	stageBg.texture:loadFromFile(packPath.."\\bnbmid1intro.png")
end
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

flashRect.size = Vector2u.new(640, 480)
flashRect.borderColor = enums.colors.Transparent
flashRect.fillColor   = enums.colors.Transparent

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
	flashRect:draw()
	dialog:render()
end

function onKeyPressed()
	keyPressed = true
end