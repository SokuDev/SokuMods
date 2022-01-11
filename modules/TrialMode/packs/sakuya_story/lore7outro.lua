--
-- Created by Yolotemperance and PinkySmile on 17/11/2021.
--

local dialogs = {
	"LWDSo? Will you tell me?",
	"RWD...",
	"R HFine, It will add a bit more flavor to the story.",
	"LSHWhat do you mean?",
	"RScI do not know who it is that infiltrated the manor...<br>But I do know that this mysterious individual<br>passed by the Hakurei shrine untouched.",
	"LccDid he beat Reimu?",
	"RcENot at all, he didn't fight her.",
	"LSEHow strange, Reimu does have a sixth sense<br>when it comes to trouble.",
	"LcEShe wouldn't let anyone suspicious go by,<br>however lazy she can be...",
	"LHEI was right, it must be someone quite <color FFFFFF>stealthy</color>...<br>Or someone able to <color FFFFFF>manipulate</color> others...",
	"LCEThere is one person,<br>and she fits the description quite nicely...",
	"RCDThat sure is a long day.<br>At least it must be good for you?",
	"LCDIt is perfect!",
	"RCcDo you know who she is assuming to be the culprit?",
	"LHcYes I do. Truth is, I have already made an<br>arrangement with her.",
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
	print("Init!")
	camera.translate.x = -320
	camera.translate.y = 420
	camera.backgroundTranslate.x = 640
	camera.backgroundTranslate.y = 0

	battleMgr.leftChr.direction = enums.directions.RIGHT
	battleMgr.leftChr.position.x = 420
	battleMgr.leftChr.action = enums.actions.ACTION_IDLE
	battleMgr.leftChr:initAnimation()

	battleMgr.rightChr.direction = enums.directions.LEFT
	battleMgr.rightChr.position.x = 800
	battleMgr.rightChr.position.y = 0
	battleMgr.rightChr.action = enums.actions.ACTION_KNOCKED_DOWN_STATIC
	battleMgr.rightChr:initAnimation()
	print("Init done")
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
	if keyPressed and #dialog == #dialogs - 2 then
		keyPressed = false
		dialog.hidden = true
		battleMgr.rightChr.action = enums.actions.ACTION_NEUTRAL_TECH
		battleMgr.rightChr:initAnimation()
		currentStage = currentStage + 1
	end
end

local function stage4()
	battleMgr.leftChr:updateAnimation()
	battleMgr.rightChr:updateAnimation()
	if ctr ~= 0 then
		ctr = ctr - 1
	end
	if battleMgr.rightChr.frameCount == 0 then
		battleMgr.rightChr.action = enums.actions.ACTION_IDLE
		battleMgr.rightChr:initAnimation()
		ctr = 30
	end
	if battleMgr.rightChr.action == enums.actions.ACTION_IDLE and ctr == 0 then
		dialog.hidden = false
		dialog:onKeyPress()
		currentStage = currentStage + 1
	end
end

local function stage5()
	battleMgr.leftChr:updateAnimation()
	battleMgr.rightChr:updateAnimation()
end

local anims = {
	stage0,
	stage1,
	stage4,
	stage5
}

print("Init intro.")

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
		print(stop and "Stop" or "Not stop")
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