--
-- Created by PinkySmile on 12/08/2021.
--

local dialogs = {
	--Sakuya walking to Meiling on the ground
	"lc Here you are...",
	--Meiling moving in her sleep?
	"rcSAh Miss... Sakuya?",
	"rcDSo it was just a dream.",
	"lcDHaving a hard time Meiling?",
	"rchMiss Sakuya, did you come for help?",
	"lEhSort of.",
	"rEHReally?",
	"lhHI'm collecting information on the matter.",
	"rhEOh... Well it would certainly help in the long run.",
	"lHEBefore that, I have to praise you.<br>You still tried despite your odds, you would deserve<br>a raise... If we were paid that is.",
	"rHSReally?",
	"rHhThank you miss Sakuya!",
	"lChHowever, your failure still deserves punishment.",
	--Battle here
}

local flashRect = RectangleShape.new()
local stageBg = Sprite.new()
local stageBottom = Sprite.new()
local ctr = 240
local currentStage = 0
local dialog
local keyPressed = false
local stop = false

-- CHR 26 -> 1

local function stage0()
	if ctr == 240 then
		camera.translate.x = 0
		camera.backgroundTranslate.x = 400
		battleMgr.leftChr.position.x = -200
		battleMgr.rightChr.position.x = 1000

		playBGM("data/bgm/ta01.ogg")
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
		battleMgr.leftChr.action = enums.actions.ACTION_WALK_FORWARD
		battleMgr.leftChr:initAnimation()
		battleMgr.rightChr.action = enums.actions.ACTION_KNOCKED_DOWN_STATIC
		battleMgr.rightChr:initAnimation()
	end
end

local function stage1()
	battleMgr.leftChr:updateAnimation()
	battleMgr.leftChr.position.x = battleMgr.leftChr.position.x + 5
	if battleMgr.leftChr.position.x > 200 then
		camera.translate.x = camera.translate.x - 5
		camera.backgroundTranslate.x = camera.backgroundTranslate.x + 5
		if battleMgr.leftChr.position.x >= 800 then
			dialog.hidden = false
			currentStage = currentStage + 1
			battleMgr.leftChr.action = enums.actions.ACTION_IDLE
			battleMgr.leftChr:initAnimation()
		end
	end
end

local function stage2()
	if #dialog == #dialogs - 2 and keyPressed then
		keyPressed = false
		currentStage = currentStage + 1
		battleMgr.rightChr.action = enums.actions.ACTION_NEUTRAL_TECH
		battleMgr.rightChr:initAnimation()
	end
end

local function stage3()
	keyPressed = false
	battleMgr.leftChr:updateAnimation()
	battleMgr.rightChr:updateAnimation()
	if battleMgr.rightChr.animationCounter == 7 then
		dialog:onKeyPress()
		currentStage = currentStage + 1
		battleMgr.rightChr.actionBlockId = 0
		battleMgr.rightChr.animationCounter = 0
		battleMgr.rightChr.animationSubFrame = 0
		battleMgr.rightChr.action = enums.actions.ACTION_IDLE
		battleMgr.rightChr:initAnimation()
	end
end

local function stage4()
	battleMgr.leftChr:updateAnimation()
	battleMgr.rightChr:updateAnimation()
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

print("Init intro.")

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