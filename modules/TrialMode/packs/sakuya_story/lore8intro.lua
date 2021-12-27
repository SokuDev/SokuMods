--
-- Created by Yolotemperance and PinkySmile on 17/11/2021.
--

local bool = false

local dialogs = {
	"lH It's you right?",
	"rH ???: ...",
	"lC The one behind all of this!",
	--Suika arrives
	"r SYou don't have to scream you know!",
	"lhSSo now tell me. Why?",
	"rhEWell, don't you know how much my head is<br>hurting right now?",
	"lSENo, I'm not talking about why you have a hangover.<br>I'm asking why you came to our mansion.",
	"rSWIsn't anyone doing that today?",
	"rSHShouldn't you welcome them by the way?",
	"lWHBut you were the first one and you are the one<br>who spread the rumor isn't it?",
	"rWE...",
	"rWhOh! That!",
	"rWhYeah that was me. Congratulations for finding<br>the culprit! Well I have to go, see you soon!",
	--Sakuya teleports on the other side
	"lC Wait a minute, I didn't punish you for breaking in and<br>for the mess you've done yet...",
	"rCWExactly what I needed to clear my mind!"
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

local function round(x)
	return math.floor(x + 0.5)
end

local function calcAngle(pt, center)
	local point1 = Vector2f.new(pt.x, pt.y)
	local point2 = Vector2f.new(center.x, center.y)
	local diff = point2 - point1
	local distance = math.sqrt(math.pow(diff.x, 2) + math.pow(diff.y, 2))
	local vec2 = Vector2f.new(diff.x / distance, diff.y / distance)
	
	return math.atan2(vec2.y, vec2.x) * 180 / math.pi
end

local function getCirclePoints(points, pos)
	local newArray = {}
	local radius
	local center = Vector2f.new(0, 0)
	local goClockwise
	local angles = {}
	local arcAngle

	if #points ~= 2 then
		error("Invalid number of points provided for a perfect circle slider. (3 expected but "..tostring(#points).." found)\n")
	end

	local pt1 = pos
	local pt2 = points[1]
	local pt3 = points[2]

	center.y = (
		(pt2.x - pt3.x) * (math.pow(pt2.x, 2) + math.pow(pt2.y, 2) - math.pow(pt1.x, 2) - math.pow(pt1.y, 2)) -
		(pt2.x - pt1.x) * (math.pow(pt2.x, 2) + math.pow(pt2.y, 2) - math.pow(pt3.x, 2) - math.pow(pt3.y, 2))
	) / (
		((pt2.x - pt1.x) * (pt3.y - pt2.y) + (pt2.y - pt1.y) * (pt2.x - pt3.x)) * 2
	)

	center.x = (
		(pt2.y - pt1.y) * (math.pow(pt2.x, 2) + math.pow(pt2.y, 2) - math.pow(pt3.x, 2) - math.pow(pt3.y, 2)) -
		(pt2.y - pt3.y) * (math.pow(pt2.x, 2) + math.pow(pt2.y, 2) - math.pow(pt1.x, 2) - math.pow(pt1.y, 2))
	) / (
		((pt2.x - pt1.x) * (pt3.y - pt2.y) + (pt2.y - pt1.y) * (pt2.x - pt3.x)) * 2
	)

	radius = math.sqrt(math.pow(pt1.x - center.x, 2) + math.pow(pt1.y - center.y, 2))

	angles[1] = (360 - calcAngle(pt1, center)) % 360
	angles[2] = (360 - calcAngle(pt2, center)) % 360
	angles[3] = (360 - calcAngle(pt3, center)) % 360

	assert(radius ~= 1/0, "All 3 points are on the same line")
	goClockwise = ((angles[2] - angles[1] + 360) % 360) > ((angles[3] - angles[1] + 360) % 360)
	arcAngle = ((goClockwise and angles[1] - angles[3] or angles[3] - angles[1]) + 360) % 360

	local len = (math.pi / 2 * radius * arcAngle / 360) + 1

	assert(len < 60000, "The circle is too big")
	for i = 1, len do
		local angle = ((goClockwise and angles[3] or angles[1]) + (arcAngle * i / len)) * math.pi / 180

		newArray[i] = Vector2f.new(
			round(-math.cos(angle) * radius + center.x),
			round( math.sin(angle) * radius + center.y)
		)
	end
	return newArray
end

local startPos = Vector2f.new(700, 25)
local endPos = Vector2f.new(1000, -125)
local points = getCirclePoints({
	Vector2f.new(
		startPos.x + (endPos.x - startPos.x) * 0.7,
		startPos.y + (endPos.y - startPos.y) * 0.7
	),
	startPos
}, endPos)
local index = #points

local function stage0()
	if ctr == 240 then
		playBGM("data/bgm/ta04.ogg")
		battleMgr.leftChr.position.x = -100
		battleMgr.leftChr.direction = enums.directions.RIGHT
		battleMgr.rightChr.position.x = 900
		battleMgr.rightChr.position.y = -100
		battleMgr.rightChr.direction = enums.directions.LEFT
		battleMgr.rightChr.direction = 0
		camera.translate.x = camera.translate.x - 70
		camera.backgroundTranslate.x = camera.backgroundTranslate.x + 70
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
	end
	ctr = ctr - 1
	if ctr == 0 then
		currentStage = currentStage + 1
		battleMgr.leftChr.action = enums.actions.ACTION_FORWARD_JUMP
		playSfx(enums.sfxs.highJump)
		battleMgr.leftChr:initAnimation()
		battleMgr.leftChr.speed.x = 14
		battleMgr.leftChr.speed.y = 15
	end
end

local function stage1()
	battleMgr.leftChr:updateAnimation()
	battleMgr.leftChr.position = battleMgr.leftChr.position + battleMgr.leftChr.speed
	battleMgr.leftChr.speed.y = battleMgr.leftChr.speed.y - 0.7
	if battleMgr.leftChr.position.y <= 0 then
		playSfx(enums.sfxs.land)
		battleMgr.leftChr.position.y = 0
		battleMgr.leftChr.action = enums.actions.ACTION_LANDING
		battleMgr.leftChr:initAnimation()
		battleMgr.leftChr.speed.x = 0
		battleMgr.leftChr.speed.y = 0
		currentStage = currentStage + 1
	end
end

local function stage2()
	battleMgr.leftChr:updateAnimation()
	if
		battleMgr.leftChr.action == enums.actions.ACTION_LANDING and
		battleMgr.leftChr.actionBlockId == 0 and
		battleMgr.leftChr.animationCounter == 0 and
		battleMgr.leftChr.animationSubFrame == 0
	then
		battleMgr.leftChr.action = enums.actions.ACTION_IDLE
		battleMgr.leftChr:initAnimation()
		dialog.hidden = false
		ctr = 30
	end
	if #dialog == #dialogs - 3 and keyPressed and battleMgr.leftChr.action == enums.actions.ACTION_IDLE then
		dialog.hidden = true
		battleMgr.rightChr:playSfx(19)
		battleMgr.rightChr.action = enums.actions.ACTION_ALT1_SKILL3_B
		battleMgr.rightChr:initAnimation()
		currentStage = currentStage + 1
	end
end

local function stage3()
	battleMgr.leftChr:updateAnimation()
	repeat
		battleMgr.rightChr:updateAnimation()
	until battleMgr.rightChr.actionBlockId ~= 0
	battleMgr.rightChr.direction = enums.directions.LEFT
	battleMgr.rightChr.objects:update()
	if battleMgr.rightChr.actionBlockId == 0 or battleMgr.rightChr.actionBlockId == 1 then
		if (battleMgr.rightChr.frameCount & 1) == 0 then
			local xPos = math.random(0, 0x3C) + battleMgr.rightChr.position.x - 30.00000000
			local yPos = math.random(0, 0x3C) + battleMgr.rightChr.position.y + 30.00000000

			battleMgr.rightChr:createSubObject(0x331, xPos, yPos, battleMgr.rightChr.direction)
		end
	end
	if points[index] then
		print(index, points[index], points[index].x, points[index].y)
	else
		print(index, points[index])
	end
	battleMgr.rightChr.position = points[index]
	index = index - 1
	if index <= 0 then
		currentStage = currentStage + 1
		print(battleMgr.rightChr.position.x, battleMgr.rightChr.position.y)
		battleMgr.rightChr:animate()
	end
end

local function stage4()
	battleMgr.leftChr:updateAnimation()
	battleMgr.rightChr:updateAnimation()
	battleMgr.rightChr.speed.y = battleMgr.rightChr.speed.y - 0.5
	battleMgr.rightChr.position = battleMgr.rightChr.position + battleMgr.rightChr.speed
	if battleMgr.rightChr.position.y <= 0 then
		currentStage = currentStage + 1
		battleMgr.rightChr.position.y = 0
		battleMgr.rightChr.action = enums.actions.ACTION_CROUCHING
		battleMgr.rightChr:initAnimation()
	end
end

local function stage5()
	battleMgr.leftChr:updateAnimation()
	battleMgr.rightChr:updateAnimation()
	if battleMgr.rightChr.frameCount == 21 then
		if battleMgr.rightChr.action == enums.actions.ACTION_CROUCHING then
			battleMgr.rightChr.action = enums.actions.ACTION_CROUCHED
			battleMgr.rightChr:initAnimation()
			ctr = 30
		end
	end
	if battleMgr.rightChr.action == enums.actions.ACTION_CROUCHED and ctr == 1 then
		dialog.hidden = false
		dialog:onKeyPress()
	end
	if ctr ~= 0 then
		ctr = ctr - 1
	end
	if keyPressed and #dialog == 2 then
		dialog.hidden = true
		battleMgr.rightChr.action = enums.actions.ACTION_STANDING_UP
		battleMgr.rightChr:initAnimation()
		currentStage = currentStage + 1
	end
end

local function stage6()
	battleMgr.leftChr:updateAnimation()
	battleMgr.rightChr:updateAnimation()
	if battleMgr.rightChr.action == enums.actions.ACTION_STANDING_UP and battleMgr.rightChr.frameCount == 12 then
		battleMgr.rightChr.action = enums.actions.ACTION_WALK_FORWARD
		battleMgr.rightChr:initAnimation()
		battleMgr.rightChr.direction = enums.directions.RIGHT
	elseif battleMgr.rightChr.action == enums.actions.ACTION_WALK_FORWARD then
		battleMgr.rightChr.position.x = battleMgr.rightChr.position.x + 5
		if battleMgr.rightChr.position.x > 800 then
			battleMgr.leftChr.action = enums.actions.ACTION_DEFAULT_SKILL4_B
			battleMgr.leftChr:initAnimation()
			currentStage = currentStage + 1
		end
	end
end

local function stage7()
	battleMgr.leftChr:updateAnimation()
	battleMgr.leftChr.objects:update()
	battleMgr.rightChr:updateAnimation()
	if battleMgr.rightChr.action == enums.actions.ACTION_WALK_FORWARD then
		battleMgr.rightChr.position.x = battleMgr.rightChr.position.x + 5
	end
	if battleMgr.leftChr.actionBlockId >= 1 and camera.translate.x > -650 then
		camera.translate.x = camera.translate.x - 30
		camera.backgroundTranslate.x = camera.backgroundTranslate.x + 30
	end
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
			print("To ", i)
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
			print("To ", i + 6)
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
		battleMgr.leftChr.position.x = 1200
		battleMgr.leftChr.direction = enums.directions.LEFT
		battleMgr.leftChr:animate()
	end
	if battleMgr.leftChr.actionBlockId == 3 and battleMgr.leftChr.animationCounter == 0 and battleMgr.leftChr.animationSubFrame == 0 and battleMgr.leftChr.frameCount == 0 then
		battleMgr.leftChr:createSubObject(0x32D, battleMgr.leftChr.position.x, battleMgr.leftChr.position.y + 100, battleMgr.leftChr.position.direction, 3, 3, 10, 0, 8)
		battleMgr.leftChr:createSubObject(0x32D, battleMgr.leftChr.position.x, battleMgr.leftChr.position.y, battleMgr.leftChr.position.direction, 3, 3, -10, 0, 9)
		battleMgr.leftChr:createSubObject(0x32D, battleMgr.leftChr.position.x, battleMgr.leftChr.position.y, battleMgr.leftChr.position.direction, 3, 3, 0, 0, 10)
		battleMgr.rightChr.action = enums.actions.ACTION_GROUND_CRUSHED
		battleMgr.rightChr:initAnimation()
		currentStage = currentStage + 1
	end
end

local function stage8()
	battleMgr.leftChr:updateAnimation()
	battleMgr.leftChr.objects:update()
	battleMgr.rightChr:updateAnimation()
	if battleMgr.leftChr.action == enums.actions.ACTION_DEFAULT_SKILL4_B then
		if battleMgr.leftChr.frameCount == 21 and battleMgr.leftChr.actionBlockId == 3 then
			battleMgr.leftChr:animate()
		end
		if battleMgr.leftChr.actionBlockId == 0 then
			battleMgr.leftChr.action = enums.actions.ACTION_IDLE
			battleMgr.leftChr:initAnimation()
			ctr = 15
		end
	elseif ctr ~= 0 then
		ctr = ctr - 1
	elseif battleMgr.rightChr.action ~= enums.actions.ACTION_IDLE then
		dialog.hidden = false
		dialog:onKeyPress()
		battleMgr.rightChr.action = enums.actions.ACTION_IDLE
		battleMgr.rightChr:initAnimation()
	end
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
	stage10
}

stageBg.texture:loadFromFile(packPath.."/lore8intro.png")
stageBg.size = stageBg.texture.size
stageBg.position = Vector2i.new(
	math.floor(320 - stageBg.texture.size.x / 2),
	-50
)
stageBg.rect.width  = stageBg.texture.size.x
stageBg.rect.height = stageBg.texture.size.y
stageBg.tint.a = 0

stageBottom.texture:loadFromGame("data/scenario/effect/StageFinal.png")
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
	battleMgr.rightChr.objects:update()
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