--
-- Created by Yolotemperance and PinkySmile on 17/11/2021.
--

local YUKARI_START_LOCATION = 500

local playerInfo = PlayerInfo.new(enums.characters.yukari, 0, false)
local yukari
local portalObj
local stand = Sprite.new();

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
}

local dialogsVsYukari = {
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
local dialogVsYukari

local function init()
	yukari = Character.new(playerInfo)

	camera.translate.x = -320
	camera.translate.y = 420
	camera.backgroundTranslate.x = 640
	camera.backgroundTranslate.y = 0
	camera.scale = 1

	battleMgr.leftChr.direction = enums.directions.RIGHT
	battleMgr.leftChr.position.x = 420
	battleMgr.leftChr.action = enums.actions.ACTION_IDLE
	battleMgr.leftChr:initAnimation()

	battleMgr.rightChr.direction = enums.directions.LEFT
	battleMgr.rightChr.position.x = 800
	battleMgr.rightChr.position.y = 0
	battleMgr.rightChr.action = enums.actions.ACTION_KNOCKED_DOWN_STATIC
	battleMgr.rightChr:initAnimation()
	
	yukari.position.x = YUKARI_START_LOCATION
	yukari.position.y = 600
	yukari.action = enums.actions.ACTION_ALT2_SKILL4_C
	yukari:initAnimation()
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
	
	if keyPressed and #dialog == 0 then
		keyPressed = false
		dialog.hidden = true
		battleMgr.leftChr.action = enums.actions.ACTION_DEFAULT_SKILL4_B
		battleMgr.leftChr:initAnimation()
		currentStage = currentStage + 1
	end
end

local function stage6()
	battleMgr.leftChr:updateAnimation()
	battleMgr.leftChr.objects:update()

	if battleMgr.leftChr.actionBlockId == 1 and battleMgr.leftChr.animationCounter == 0 and battleMgr.leftChr.animationSubFrame == 0 and battleMgr.leftChr.frameCount == 0 then
		local start = #battleMgr.leftChr.objects + 1

		battleMgr.leftChr:createSubObject(0x32D, battleMgr.leftChr.position.x - 8, 115)
		battleMgr.leftChr:createSubObject(0x32D, battleMgr.leftChr.position.x - 8, 115)
		battleMgr.leftChr:createSubObject(0x32D, battleMgr.leftChr.position.x - 8, 115)
		battleMgr.leftChr:createSubObject(0x32D, battleMgr.leftChr.position.x - 8, 115)
		battleMgr.leftChr:createSubObject(0x32D, battleMgr.leftChr.position.x - 8, 115)
		battleMgr.leftChr:createSubObject(0x32D, battleMgr.leftChr.position.x - 8, 115)
		battleMgr.leftChr:createSubObject(0x32D, battleMgr.leftChr.position.x - 8, 115)
		for i = 5, 0, -1 do
			while battleMgr.leftChr.objects[start + i].actionBlockId ~= i do
				battleMgr.leftChr.objects[start + i]:animate()
			end
		end
	end
	if battleMgr.leftChr.actionBlockId == 1 and battleMgr.leftChr.animationCounter == 1 and battleMgr.leftChr.animationSubFrame == 1 and battleMgr.leftChr.frameCount == 10 then
		local start = #battleMgr.leftChr.objects + 1

		battleMgr.leftChr:createSubObject(0x32D, battleMgr.leftChr.position.x, 0)
		battleMgr.leftChr:createSubObject(0x32D, battleMgr.leftChr.position.x, 0)
		for i = 1, 0, -1 do
			battleMgr.leftChr.objects[start + i].speed.x = -10 * (i * 2 - 1)
			battleMgr.leftChr.objects[start + i].speed.y = 0
			battleMgr.leftChr.objects[start + i].renderInfos.xRotation = 0
			battleMgr.leftChr.objects[start + i].renderInfos.yRotation = 0
			battleMgr.leftChr.objects[start + i].renderInfos.zRotation = 0
			while battleMgr.leftChr.objects[start + i].actionBlockId ~= i + 6 do
				battleMgr.leftChr.objects[start + i]:animate()
			end
		end
		battleMgr.leftChr:playSfx(13)
		battleMgr.leftChr.position.x = -500
		currentStage = currentStage + 1
	end
	keyPressed = false
end

local function stage7()
	battleMgr.leftChr.objects:update()
	if #battleMgr.leftChr.objects == 0 then
		currentStage = currentStage + 1
		ctr = 30
	end
end

local function stage8()
	battleMgr.rightChr:updateAnimation()
	if ctr ~= 0 then
		ctr = ctr - 1
	else
        keyPressed = false

        -- Yukari arrives init
        yukari.position.y = 250
        yukari:playSfx(1)

        portalObj = yukari:createSubObject(0x33B, 200, 500)

        while portalObj.actionBlockId ~= 3 do
            portalObj:animate()
        end
        yukari:updateAnimation()
        yukari.objects:update()
        yukari.position.y = 800
        yukari:animate()
        currentStage = currentStage + 1
	end
end

local function stagey1()
	battleMgr.rightChr:updateAnimation()
	if ctr == 0 then
		yukari.objects:update()
	end
	if #yukari.objects ~= 0 and portalObj.animationCounter == 14 then
		if ctr == 0 then
			ctr = 30
		elseif ctr == 1 then
			yukari:playSfx(4)
			yukari:playSfx(5)
			yukari.position.y = 0
			currentStage = currentStage + 1
			ctr = 0
		else
			ctr = ctr - 1
		end
	end
end

local function stagey2()
	battleMgr.rightChr:updateAnimation()
	ctr = ctr ~ 1
	if ctr == 0 then
		yukari:updateAnimation()
	elseif yukari.actionBlockId == 0 then
		yukari.action = enums.actions.ACTION_IDLE
		yukari:initAnimation()
		dialogVsYukari.hidden = false
		keyPressed = false
		currentStage = currentStage + 1
	end
end

local function stagey3()
	battleMgr.rightChr:updateAnimation()
	yukari.objects:update()
	yukari:updateAnimation()
end

local anims = {
	stage0,
	stage1,
	stage4,
	stage5,
	stage6,
	stage7,
	stage8,
	stagey1,
	stagey2,
	stagey3
}


flashRect.fillColor = enums.colors.Transparent
flashRect.borderColor = enums.colors.Transparent
flashRect.size = Vector2u.new(640, 480)

dialog = StandDialog.new(dialogs)
dialogVsYukari = StandDialog.new(dialogsVsYukari, enums.characters.yukari, enums.characters.aya)

function update()
	dialog:update()
	dialogVsYukari:update()
	if currentStage < #anims then
		anims[currentStage + 1]()
	end

	if keyPressed then
		stop = stop or (not dialog:onKeyPress() and not dialogVsYukari:onKeyPress())
		if stop then
			dialog.hidden = true
		end
		keyPressed = false
	end
	return not stop or not dialog:isAnimationFinished() or not dialogVsYukari:isAnimationFinished()
end

function render()
	if yukari then
		-- Display the Chr subobjects
		yukari.objects:draw(-2)
		yukari.objects:draw(-1)

		-- Display the Chr
		yukari:draw()

		yukari.objects:draw(1)
		yukari.objects:draw(2)

		-- We redraw Remilia because Reisen's subobjects mess up the DirectX context and this will clean it up
		battleMgr.leftChr:draw()
	end
	dialog:render()
	dialogVsYukari:render()
    flashRect:draw()
end

function onKeyPressed()
	keyPressed = true
end