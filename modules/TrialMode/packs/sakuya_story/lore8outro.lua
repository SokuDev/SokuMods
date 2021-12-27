--
-- Created by Yolotemperance and PinkySmile on 17/11/2021.
--

local dialogs = {
	"lH And now my job is finished! You won't attract anyone<br>to our mansion anymore right?",
	"rHDYep I will definitely stop doing that.",
	"lSDWell... I guess this is the end...",
	"rSHYou are the true hero of Gensokyo! You've dealt with this<br>problem entirely by yourself, how impressive!",
	"lEHAre you hiding something by any chance...",
	"rEAOf course not!",
	"lhAI guess it is time to come back home for me then.",
	"rhhIndeed you should come back, there is much<br>to come after all.",
	"lchYou were saying something?",
	"rchI want to prepare another party! Want to help me?",
	"lEhNo, I have better things to do.",
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

	battleMgr.leftChr.position.x = 420
	battleMgr.leftChr.action = enums.actions.ACTION_IDLE
	battleMgr.leftChr:initAnimation()

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