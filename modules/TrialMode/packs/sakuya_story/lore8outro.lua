--
-- Created by Yolotemperance and PinkySmile on 17/11/2021.
--

local dialogs = {
	"lH And now my job is finished! You won't attract anyone<br>to our mansion anymore right?",
	"rHDYep. I will definitely stop doing that.",
	"lSDWell... I guess this is the end...",
	"rSHYou are the true hero of Gensokyo! You've dealt with<br>this problem entirely by yourself, how impressive!",
	"lEHAre you hiding something by any chance...",
	"rEAOf course not!",
	"lhAI guess it is time to come back home for me then.",
	"r hIndeed you should come back, there is much<br>to come after all.",
	"lcSYou were saying something?",
	"rcS...",
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
	if keyPressed and #dialog == 10 then
	    keyPressed = false
        dialog.hidden = true
        battleMgr.rightChr.action = enums.actions.ACTION_NEUTRAL_TECH
        battleMgr.rightChr:initAnimation()
        currentStage = currentStage + 1
	end
end

local function stage2()
	battleMgr.leftChr:updateAnimation()
	battleMgr.rightChr:updateAnimation()
    if battleMgr.rightChr.actionBlockId == 0 and battleMgr.rightChr.animationCounter == 0 and battleMgr.rightChr.animationSubFrame == 0 and battleMgr.rightChr.frameCount == 0 then
        battleMgr.rightChr.action = enums.actions.ACTION_CROUCHING
        battleMgr.rightChr:initAnimation()
        currentStage = currentStage + 1
	end
end

local function stage3()
    battleMgr.leftChr:updateAnimation()
    battleMgr.rightChr:updateAnimation()
        if battleMgr.rightChr.actionBlockId == 0 and battleMgr.rightChr.animationCounter == 0 and battleMgr.rightChr.animationSubFrame == 0 and battleMgr.rightChr.frameCount == 0 then
            battleMgr.rightChr.action = enums.actions.ACTION_CROUCHED
            battleMgr.rightChr:initAnimation()
            keyPressed = true
            dialog.hidden = false
            currentStage = currentStage + 1
    	end
end

local function stage4()
    battleMgr.rightChr:updateAnimation()
    battleMgr.leftChr:updateMove()
    battleMgr.leftChr.objects:update()
   	if keyPressed and #dialog == 5 then
   		keyPressed = false
   		dialog.hidden = true
   		battleMgr.leftChr.action = enums.actions.ACTION_DEFAULT_SKILL4_B
   		battleMgr.leftChr:initAnimation()
   		currentStage = currentStage + 1
   	end
end

local function stage5()
    battleMgr.rightChr:updateAnimation()
    battleMgr.leftChr:updateMove()
    battleMgr.leftChr.objects:update()
    if battleMgr.leftChr.actionBlockId == 2 and battleMgr.leftChr.animationCounter == 0 and battleMgr.leftChr.animationSubFrame == 0 and battleMgr.leftChr.frameCount == 0 then
        battleMgr.leftChr.position.x = -500
        ctr = 0
        currentStage = currentStage + 1
    	end
end

local function stage6()
    battleMgr.rightChr:updateAnimation()
    battleMgr.leftChr:updateMove()
    battleMgr.leftChr.objects:update()
    ctr = ctr + 1
    if ctr == 35 then
        keyPressed = true
        dialog.hidden = false
        currentStage = currentStage + 1
    end
end

local function stage7()
    battleMgr.leftChr:updateAnimation()
    battleMgr.rightChr:updateMove()
  	if keyPressed and #dialog == 4 then
   		keyPressed = false
   		dialog.hidden = true
   		battleMgr.leftChr.action = enums.actions.ACTION_DEFAULT_SKILL4_B
   		battleMgr.leftChr:initAnimation()
   		currentStage = currentStage + 1
   	end
end

local function stage8()
    battleMgr.rightChr:updateAnimation()
    battleMgr.leftChr:updateMove()
    battleMgr.leftChr.objects:update()
    if battleMgr.leftChr.actionBlockId == 1 and battleMgr.leftChr.animationCounter == 1 and battleMgr.leftChr.animationSubFrame == 1 and battleMgr.leftChr.frameCount == 10 then
        battleMgr.leftChr.position.x = 500
        battleMgr.rightChr.action = enums.actions.ACTION_GROUND_CRUSHED
        battleMgr.rightChr:initAnimation()
        ctr = 0
        currentStage = currentStage + 1
    end
end

local function stage9()
    battleMgr.rightChr:updateAnimation()
    battleMgr.leftChr:updateMove()
    battleMgr.leftChr.objects:update()
    ctr = ctr + 1
    if battleMgr.leftChr.actionBlockId == 4 and battleMgr.leftChr.animationCounter == 3 and battleMgr.leftChr.animationSubFrame == 1 and battleMgr.leftChr.frameCount == 7 then
        battleMgr.leftChr.action = enums.actions.ACTION_IDLE
        battleMgr.leftChr:initAnimation()
    end
    if ctr == 25 then
        battleMgr.rightChr.action = enums.actions.ACTION_IDLE
        battleMgr.rightChr:initAnimation()
       	keyPressed = true
       	dialog.hidden = false
       	currentStage = currentStage + 1
    end
end

local function stage10()
    battleMgr.leftChr:updateMove()
    battleMgr.leftChr.objects:update()
    battleMgr.rightChr:updateAnimation()
  	if keyPressed and #dialog == 0 then
    	dialog:update()
    	battleMgr.leftChr.action = enums.actions.ACTION_FORWARD_HIGH_JUMP
    	battleMgr.leftChr.direction = enums.directions.LEFT
    	battleMgr.leftChr:initAnimation()
    	currentStage = currentStage + 1
    end
end

local function stage11()
    battleMgr.leftChr:updateAnimation()
    battleMgr.rightChr:updateAnimation()
	if battleMgr.leftChr.actionBlockId >= 1 then
		playSfx(enums.sfxs.highJump)
		battleMgr.leftChr.speed.x = -13
		battleMgr.leftChr.speed.y = 17
		currentStage = currentStage + 1
	end
end

local function stage12()
    battleMgr.leftChr:updateAnimation()
    battleMgr.rightChr:updateAnimation()
    battleMgr.leftChr.position = battleMgr.leftChr.position + battleMgr.leftChr.speed
    battleMgr.leftChr.speed.y = battleMgr.leftChr.speed.y - 0.8
end

local anims = {
	stage0,
	stage1,
	stage2,
	stage3,
	stage4,
	stage5,
	stage6,
	stage7,
	stage8,
	stage9,
	stage10,
	stage11,
	stage12
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