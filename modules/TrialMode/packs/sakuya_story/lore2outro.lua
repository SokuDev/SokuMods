--
-- Created by PinkySmile on 18/08/2021.
--

local dialogs = {
	"rWDIs it over?",
	"lWDYes it is.",
	"lcDSo, did you notice anything?",
	"rcEI could feel someone before the rumor spread!<br>But I never manage to catch a glimpse of her appearance.",
	"lSEI see... Whatever I can say about your work ethics,<br>you still have really keen senses...",
	"rSDBut then so many came here, maybe the Taisui Xingjun...",
	"lHDThank you for your precious information.<br>Oh, and try to remain awake this time.<br>There is still much to do...",
	--Sakuya gets away and meiling and falls to the ground, sleeping.
}

local flashRect = RectangleShape.new()
local stageBg = Sprite.new()
local stageBottom = Sprite.new()
local ctr = 30
local currentStage = 0
local dialog
local keyPressed = false
local stop = false

local function init()
	camera.translate.x = -500
	camera.translate.y = 420
	camera.scale = 1
	camera.backgroundTranslate.x = 900
	camera.backgroundTranslate.y = 0

	battleMgr.leftChr.position.x = 800
	battleMgr.leftChr.position.y = 0
	battleMgr.leftChr.direction = enums.directions.RIGHT
	battleMgr.leftChr.action = enums.actions.ACTION_IDLE
	battleMgr.leftChr:initAnimation()

	battleMgr.rightChr.position.x = 1000
	battleMgr.rightChr.position.y = 0
	battleMgr.rightChr.animationSubFrame = 0
	battleMgr.rightChr.direction = enums.directions.LEFT
	battleMgr.rightChr.action = enums.actions.ACTION_GROUND_CRUSHED
	battleMgr.rightChr:initAnimation()
end

local function stage0()
	local color = flashRect.fillColor

	battleMgr.leftChr:updateAnimation()
	battleMgr.rightChr:updateAnimation()
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
		battleMgr.rightChr.action = enums.actions.ACTION_IDLE
		battleMgr.rightChr:initAnimation()
	end
end

local function stage1()
	battleMgr.leftChr:updateAnimation()
	battleMgr.rightChr:updateAnimation()
	if #dialog == 0 and keyPressed then
		currentStage = currentStage + 1
		dialog.hidden = true
		battleMgr.leftChr.direction = enums.directions.LEFT
		battleMgr.leftChr.action = enums.actions.ACTION_WALK_FORWARD
		battleMgr.leftChr:initAnimation()
	end
end

local function stage2()
	keyPressed = false
	battleMgr.leftChr:updateAnimation()
	battleMgr.leftChr.position.x = battleMgr.leftChr.position.x - 5
	battleMgr.rightChr:updateAnimation()
	if battleMgr.leftChr.position.x < 200 then
		currentStage = currentStage + 1
		battleMgr.rightChr.action = enums.actions.ACTION_KNOCKED_DOWN
		battleMgr.rightChr:initAnimation()
	end
end

local function stage3()
	battleMgr.rightChr:updateAnimation()
	if battleMgr.rightChr.frameCount == 5 and battleMgr.rightChr.actionBlockId <= 1 then
		if battleMgr.rightChr.actionBlockId == 0 then
			playSfx(enums.sfxs.knockdown)
		end
		battleMgr.rightChr:animate()
		if battleMgr.rightChr.actionBlockId == 2 then
			battleMgr.rightChr:animate()
		end
	elseif battleMgr.rightChr.frameCount == 12 then
		currentStage = currentStage + 1
		ctr = 0
		battleMgr.rightChr.action = enums.actions.ACTION_KNOCKED_DOWN_STATIC
		battleMgr.rightChr:initAnimation()
	end
end

local function stage4()
	ctr = ctr + 1
	if ctr == 120 then
		stop = true
	end
end

local function stage5()
end

local function stage6()
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

print("Init outro.")

stageBg.texture:loadFromFile(packPath.."\\lore2intro.png")
stageBg.size = stageBg.texture.size
stageBg.position = Vector2i.new(
	math.floor(320 - stageBg.texture.size.x / 2),
	-50
)
stageBg.rect.width  = stageBg.texture.size.x
stageBg.rect.height = stageBg.texture.size.y
stageBg.tint.a = 0

stageBottom.texture:loadFromGame("data/scenario/effect/Stage2.png")
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