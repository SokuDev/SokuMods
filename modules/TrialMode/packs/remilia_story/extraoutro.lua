--
-- Created by PinkySmile on 07/05/2021.
--

local dialogs = {
	"r DAs a detective, shouldn't you find proof before<br>catching anyone?",
	"lSDReally? That seems rather ineffective.",
	"rSEYou really aren't suited for detective work.<br>However you might make a rather convincing policewoman.",
	"r cAnyway, do you wish to know more about the situation?",
	"lhcNo, actually I don't really care about seeing through<br>your plans. I just want to participate.",
	"lHcYour secret party that you're hosting is still in my<br>home, so I'm feeling a bit left out.",
	"rHhIt was a bit rude of me, I'll admit. But don't worry,<br>I'll find you a nice role in this play of ours. ",
	"rHWIn fact, I already have one task for you in mind.",
	--Battle here
}

local flashRect = RectangleShape.new()
local side = false
local ctr = 60
local currentStage = 0
local dialog
local keyPressed = false
local stop = false

local function init()
	camera.translate.x = -320
	camera.translate.y = 420
	camera.backgroundTranslate.x = 640
	camera.backgroundTranslate.y = 0

	battleMgr.leftChr.position.x = 420
	battleMgr.leftChr.action = enums.actions.ACTION_IDLE
	battleMgr.leftChr:initAnimation()

	battleMgr.rightChr.position.x = 800
	battleMgr.rightChr.position.y = 0
	battleMgr.rightChr.action = enums.actions.ACTION_KNOCKED_DOWN_STATIC
	battleMgr.rightChr:initAnimation()

	battleMgr.leftChr.direction = enums.directions.RIGHT;
	battleMgr.rightChr.direction = enums.directions.LEFT;
end

local function stage0()
	local color = flashRect.fillColor

	battleMgr.leftChr:updateAnimation()
	battleMgr.rightChr:updateAnimation()
	if color.a == 0 and ctr == 0 and side then
		dialog.hidden = false
		currentStage = currentStage + 1
	end
	if not side then
		color.a = color.a + 0x11
		if color.a == 0xFF then
			side = true
			init()
		end
	elseif flashRect.fillColor.a ~= 0 then
		color.a = color.a - 0x11
	elseif ctr ~= 0 then
		ctr = ctr - 1
	end
	flashRect.fillColor = color
end

local function stage1()
	battleMgr.leftChr:updateAnimation()
	battleMgr.rightChr:updateAnimation()
	if #dialog == #dialogs - 3 and keyPressed then
		currentStage = currentStage + 1
		dialog.hidden = true
		battleMgr.rightChr.position.x = 800
		battleMgr.rightChr.position.y = 0
		battleMgr.rightChr.action = enums.actions.ACTION_KNOCKED_DOWN_STATIC
		battleMgr.rightChr:initAnimation()
	end
end

local function stage2()
	battleMgr.leftChr:updateAnimation()
	battleMgr.rightChr:updateMove()
	battleMgr.rightChr.objects:update()
	if battleMgr.rightChr.action == enums.actions.ACTION_IDLE then
		currentStage = currentStage + 1
		dialog.hidden = false
		dialog:onKeyPress()
	end
end

local function stage3()
	battleMgr.leftChr:updateAnimation()
	battleMgr.rightChr:updateAnimation()
end

local anims = {
	stage0,
	stage1,
	stage2,
	stage3
}


flashRect.fillColor = enums.colors.Transparent
flashRect.borderColor = enums.colors.Transparent
flashRect.size = Vector2u.new(640, 480)

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
	dialog:render()
	flashRect:draw()
end

function onKeyPressed()
	keyPressed = true
end