--
-- Created by PinkySmile on 18/08/2021.
--

local dialogs = {
	"lcDIt's boring...",
	"lADWhy aren't you fighting back?",
	--Reimu getting back on her feet
	"rADI won't retaliate, so stop attacking.",
	"lCDDoesn't this make for best opportunity to defeat you?",
	"rCWWell, if it pleases you in winning that way,<br>be my guest!",
	"lSWDid you grow a brain when I wasn't looking?",
	"lcWIt seems that Patchouli was on spot when she thought<br>you were acting weirdly...",
	"rcHSo can I go now?",
	"rcEBelieve me, I don't like settling things in that way<br>but I want it to be done quickly.",
	"lCEBefore you go, tell me.",
	"lAEWhat is going on with this mysterious intruder that<br>everyone is talking about?",
	"rAcWell...",
	"rAEIt's kind of weird to say.",
	"lWEIf you don't tell me I might take you on your offer and<br>beat you endlessly.",
	"rWAFine...",
	--Reimu looking left and right to see if anyone's here"
	"rWW*whisper whisper*",
	"lSW...",
	"lSWOh my.",
	"lcWThat is unexpected indeed.",
	"rchI'm taking care of it, but won't you do anything about<br>the curious ones.",
	"rcSThey should be ransacking your house as we speak.",
	"lWSReimu, you really have no hospitality...",
	"lHSWhat kind of host ends the party this early!",
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
	camera.translate.x = -320
	camera.translate.y = 420
	camera.backgroundTranslate.x = 640
	camera.backgroundTranslate.y = 0

	battleMgr.leftCharacterManager.position.x = 480
	battleMgr.leftCharacterManager.position.y = 0
	battleMgr.leftCharacterManager.direction = enums.directions.RIGHT
	battleMgr.leftCharacterManager.action = enums.actions.ACTION_IDLE
	battleMgr.leftCharacterManager:initAnimation()

	battleMgr.rightCharacterManager.position.x = 800
	battleMgr.rightCharacterManager.position.y = 0
	battleMgr.rightCharacterManager.direction = enums.directions.LEFT
	battleMgr.rightCharacterManager.action = enums.actions.ACTION_KNOCKED_DOWN_STATIC
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
		dialog.hidden = false
	end
end

local function stage1()
	battleMgr.leftCharacterManager:updateAnimation()
	if #dialogs - 3 == #dialog and keyPressed then
		keyPressed = false
		battleMgr.rightCharacterManager.direction = enums.directions.LEFT
		battleMgr.rightCharacterManager.action = enums.actions.ACTION_NEUTRAL_TECH
		battleMgr.rightCharacterManager:initAnimation()
		currentStage = currentStage + 1
	end
end

local function stage2()
	keyPressed = false
	battleMgr.leftCharacterManager:updateAnimation()
	battleMgr.rightCharacterManager:updateAnimation()
	if battleMgr.rightChr.frameCount == 0 and battleMgr.rightChr.actionBlockId == 0 then
		ctr = 0
		battleMgr.rightCharacterManager.action = enums.actions.ACTION_IDLE
		battleMgr.rightCharacterManager:initAnimation()
		playSfx(enums.sfxs.land)
	end
	ctr = ctr + 1
	if ctr == 10 and battleMgr.rightCharacterManager.action == enums.actions.ACTION_IDLE then
		dialog:onKeyPress()
		currentStage = currentStage + 1
	end
end

local function stage3()
	battleMgr.leftCharacterManager:updateAnimation()
	battleMgr.rightCharacterManager:updateAnimation()
	if #dialogs - 15 == #dialog and keyPressed then
		dialog.hidden = true
		currentStage = currentStage + 1
		ctr = 0
	end
end

local function stage4()
	battleMgr.leftCharacterManager:updateAnimation()
	battleMgr.rightCharacterManager:updateAnimation()
	ctr = ctr + 1
	if ctr == 30 then
		battleMgr.rightCharacterManager.direction = enums.directions.RIGHT
	elseif ctr == 60 then
		battleMgr.rightCharacterManager.direction = enums.directions.LEFT
	elseif ctr == 90 then
		dialog.hidden = false
		dialog:onKeyPress()
		currentStage = currentStage + 1
	end
end

local function stage5()
	battleMgr.leftCharacterManager:updateAnimation()
	battleMgr.rightCharacterManager:updateAnimation()
	if #dialog == 0 and keyPressed then
		keyPressed = false
		stop = true
	end
end

local anims = {
	stage0,
	stage1,
	stage2,
	stage3,
	stage4,
	stage5
}

print("Init outro.")

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