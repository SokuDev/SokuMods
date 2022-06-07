--
-- Created by PinkySmile on 12/08/2021.
--

local dialogs = {
        "r DI didn't like this game that much...",
        "r WWhy not play something else?",
        "lSWYou still want to play?",
        "lAWJust go back to your room, I'm done playing with you!",
        --Remilia leaves
        "r DBut I don't want to go back to my room..."
}

local side = false
local flashRect = RectangleShape.new()
local stageBg = Sprite.new()
local stageBottom = Sprite.new()
local ctr = 60
local currentStage = 0
local dialog = StandDialog.new(dialogs)
local keyPressed = false
local stop = false

-- CHR 26 -> 1

local function init()
	camera.translate.x = -500
	camera.translate.y = 420
	camera.backgroundTranslate.x = 900
	camera.backgroundTranslate.y = 0

	battleMgr.leftCharacterManager.position.x = 800
	battleMgr.leftCharacterManager.position.y = 0
	battleMgr.leftCharacterManager.actionBlockId = 0
	battleMgr.leftCharacterManager.animationCounter = 0
	battleMgr.leftCharacterManager.animationSubFrame = 0
	battleMgr.leftCharacterManager.direction = enums.directions.RIGHT
	battleMgr.leftCharacterManager.action = enums.actions.ACTION_IDLE
	battleMgr.leftCharacterManager:initAnimation()

	battleMgr.rightCharacterManager.position.x = 1000
	battleMgr.rightCharacterManager.position.y = 0
	battleMgr.rightCharacterManager.direction = enums.directions.LEFT
	battleMgr.rightCharacterManager.action = enums.actions.ACTION_GROUND_CRUSHED
	battleMgr.rightCharacterManager:initAnimation()
end

local function stage0()
	local color = flashRect.fillColor

	battleMgr.leftCharacterManager:updateAnimation()
	battleMgr.rightCharacterManager:updateAnimation()
	if not side then
		color.a = color.a + 0x11;
		if color.a == 0xFF then
			side = true;
			init()
		end
		flashRect.fillColor = color
	elseif flashRect.fillColor.a ~= 0 then
		color.a = color.a - 0x11;
		flashRect.fillColor = color
	elseif ctr ~= 0 then
		ctr = ctr - 1
	else
		currentStage = currentStage + 1
		battleMgr.rightCharacterManager.action = enums.actions.ACTION_3A
		battleMgr.rightCharacterManager:initAnimation()
	end
end

local function stage1()
	ctr = ctr + 1
	battleMgr.leftCharacterManager:updateAnimation()
	battleMgr.rightCharacterManager:updateAnimation()
	if ctr % 3 == 0 then
		battleMgr.rightCharacterManager:updateAnimation()
	end
	if battleMgr.rightCharacterManager.frameCount == 20 then
		playSfx(enums.sfxs.knockdown)
	end
	if battleMgr.rightCharacterManager.animationCounter == 11 and battleMgr.rightCharacterManager.frameCount == 52 then
		currentStage = currentStage + 1
		dialog.hidden = false
		battleMgr.rightCharacterManager.action = enums.actions.ACTION_IDLE
		battleMgr.rightCharacterManager:initAnimation()
	end
end

local function stage2()
	battleMgr.leftCharacterManager:updateAnimation()
	battleMgr.rightCharacterManager:updateAnimation()
	if #dialogs - 4 == #dialog and keyPressed then
		dialog.hidden = true
		battleMgr.leftCharacterManager.direction = enums.directions.LEFT
		battleMgr.leftCharacterManager.action = enums.actions.ACTION_WALK_FORWARD
		battleMgr.leftCharacterManager:initAnimation()
		currentStage = currentStage + 1
	end
end

local function stage3()
	battleMgr.leftCharacterManager:updateAnimation()
	battleMgr.rightCharacterManager:updateAnimation()
	battleMgr.leftCharacterManager.position.x = battleMgr.leftCharacterManager.position.x - 4
	if battleMgr.leftCharacterManager.position.x < 500 then
		dialog.hidden = false
		dialog:onKeyPress()
		currentStage = currentStage + 1
	end
end

local function stage4()
	battleMgr.leftCharacterManager:updateAnimation()
	battleMgr.rightCharacterManager:updateAnimation()
	battleMgr.leftCharacterManager.position.x = battleMgr.leftCharacterManager.position.x - 4
	if keyPressed then
		stop = true
		keyPressed = false
	end
end

local anims = {
	stage0,
	stage1,
	stage2,
	stage3,
	stage4
}

print("Init intro.")

flashRect.size = Vector2u.new(640, 480)
flashRect.borderColor = enums.colors.Transparent
flashRect.fillColor   = enums.colors.Transparent

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
	flashRect:draw()
	dialog:render()
end

function onKeyPressed()
	keyPressed = true
end