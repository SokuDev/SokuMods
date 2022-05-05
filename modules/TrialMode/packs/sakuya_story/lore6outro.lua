local YUKARI_START_LOCATION = 500

local flashRect = RectangleShape.new()
local side = false
local ctr = 60
local currentStage = 0
local dialog
local keyPressed = false
local stop = false
local playerInfo = PlayerInfo.new(enums.characters.yukari, 0, false)
local yukari
local portalObj
local stand = Sprite.new();
local dialogs = {
	"LHDNow... Will you tell me what is going on?",
	"RHDYou should come back to your first clue,<br>that will help you I'm sure.",
	"LcDCan't you just explain to me right now?",
	"REEI could, but I believe it would<br>be best to find out by yourself.",
	"LHEIt is fine, your explanations<br>are always too cryptic anyway.",
	"LWEWell. I have someone else to ask questions thanks<br>to you now, so I must take my leave.",
	"R CIt seems she is going for the right source...",
	"R hSadly for her, it is the flow,<br>not the spring that matters this time...",
	"R CIsn't it?",
	"   ???: Here you are!"
}

local function init()
	yukari = Character.new(playerInfo)

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

	yukari.position.x = YUKARI_START_LOCATION
	yukari.position.y = 800
	yukari.action = enums.actions.ACTION_ALT2_SKILL4_C
	yukari:initAnimation()
	yukari.renderInfos.color = enums.colors.Black

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
	if keyPressed and #dialog == 4 then
		keyPressed = false
		dialog.hidden = true
		battleMgr.leftChr.action = enums.actions.ACTION_DEFAULT_SKILL4_B
		battleMgr.leftChr:initAnimation()
		currentStage = currentStage + 1
	end
end

local function stage2()
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

local function stage3()
	battleMgr.leftChr.objects:update()
	if #battleMgr.leftChr.objects == 0 then
		battleMgr.rightChr.action = enums.actions.ACTION_NEUTRAL_TECH
		battleMgr.rightChr:initAnimation()
		currentStage = currentStage + 1
	end
end

local function stage4()
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
	battleMgr.rightChr:updateAnimation()
	if #dialog == 1 and keyPressed then
		keyPressed = false
		dialog.hidden = true

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

local leftPos = Vector2i.new(-512, -32)
local leftExpectedPos = Vector2i.new(-96, -32)

local function updateStand()
	local leftStep = leftExpectedPos - leftPos;

	leftStep.x = math.floor(leftStep.x / 5)
	leftStep.y = math.floor(leftStep.y / 5)

	local x = leftPos.x + leftStep.x
	local y = leftPos.y + leftStep.y

	if
		(leftExpectedPos.x < x and leftStep.x >= 0) or
		(leftExpectedPos.x > x and leftStep.x <= 0)
	then
		x = leftExpectedPos.x;
	end
	if
		(leftExpectedPos.y < y and leftStep.y >= 0) or
		(leftExpectedPos.y > y and leftStep.y <= 0)
	then
		y = leftExpectedPos.y;
	end
	leftPos.x = x
	leftPos.y = y
end

local function stagey2()
	battleMgr.rightChr:updateAnimation()
	ctr = ctr ~ 1
	if ctr == 0 then
		yukari:updateAnimation()
	elseif yukari.actionBlockId == 0 then
		yukari.action = enums.actions.ACTION_IDLE
		yukari:initAnimation()
		dialog.hidden = false
		dialog:onKeyPress()
		currentStage = currentStage + 1
	end
end

local function stagey3()
	battleMgr.rightChr:updateAnimation()
	yukari.objects:update()
	yukari:updateAnimation()
	updateStand()
end

local anims = {
	stage0,
	stage1,
	stage2,
	stage3,
	stage4,
	stage5,
	stagey1,
	stagey2,
	stagey3
}


stand.texture:loadFromGame("data/character/yukari/stand/\x8a\xf0.bmp")
stand.size = stand.texture.size
--stand.position =
stand.rect.width  = stand.texture.size.x
stand.rect.height = stand.texture.size.y
stand.tint = Color.new(0x4, 0x4, 0x4, 0xFF)

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
	stand.position = leftPos
	stand:draw()
	dialog:render()
	flashRect:draw()
end

function onKeyPressed()
	keyPressed = true
end