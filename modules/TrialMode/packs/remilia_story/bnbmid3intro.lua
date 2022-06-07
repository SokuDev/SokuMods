--
-- Created by PinkySmile on 12/08/2021.
--

local dialogs = {
	"lH What perfect weather to go outside.",
	"lA Still sleeping aren't you?",
	"rADI wish...",
	"lcDIt seems that many uninvited guests have come.",
	"lIDAny unusual ones?",
	"rISUnusual ones?",
	"rIcThere were too many to count!",
	"lccJudging by your state, it seems you are<br>telling the truth...",
	"lccAnd that is without counting the ones who used your<br>fights to sneak in...",
	"lEcI see... Coming here was useless after all...",
	"rEDI'm sorry...",
	"lhDDon't worry, I didn't expect the answer<br>to be so simple, so it is fine.",
	"rhHReally?",
	"lHHOf course!<br>It is nothing a little punishment can't absolve!",
	"rHE..."
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

local function stage0()
	if ctr == 240 then
		playBGM("data/bgm/ta05.ogg")
	end
	camera.translate.x = 0
	camera.backgroundTranslate.x = 400
	battleMgr.leftChr.position.x = -200
	battleMgr.rightChr.position.x = 1000
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
		battleMgr.leftChr.actionBlockId = 0
		battleMgr.leftChr.animationCounter = 0
		battleMgr.leftChr.animationSubFrame = 0
		battleMgr.leftChr.action = enums.actions.ACTION_WALK_FORWARD
		battleMgr.leftChr:initAnimation()

		battleMgr.rightChr.actionBlockId = 0
		battleMgr.rightChr.animationCounter = 0
		battleMgr.rightChr.animationSubFrame = 0
		battleMgr.rightChr.action = enums.actions.ACTION_KNOCKED_DOWN_STATIC
		battleMgr.rightChr:initAnimation()

		ctr = 3
	end
end

local function stage1()
	battleMgr.leftChr:updateAnimation()
	battleMgr.leftChr.position.x = battleMgr.leftChr.position.x + 6
	if battleMgr.leftChr.position.x > 200 then
		dialog.hidden = false
		currentStage = currentStage + 1
		battleMgr.leftChr.actionBlockId = 0
		battleMgr.leftChr.animationCounter = 0
		battleMgr.leftChr.animationSubFrame = 0
		battleMgr.leftChr.action = enums.actions.ACTION_IDLE
		battleMgr.leftChr:initAnimation()
	end
end

local function stage2()
	battleMgr.leftChr:updateAnimation()
	if keyPressed then
		keyPressed = false
		dialog.hidden = true
		currentStage = currentStage + 1
		battleMgr.leftChr.actionBlockId = 0
		battleMgr.leftChr.animationCounter = 0
		battleMgr.leftChr.animationSubFrame = 0
		battleMgr.leftChr.action = enums.actions.ACTION_WALK_FORWARD
		battleMgr.leftChr:initAnimation()
	end
end

local function stage3()
	battleMgr.leftChr:updateAnimation()
	battleMgr.leftChr.position.x = battleMgr.leftChr.position.x + 6
	camera.translate.x = camera.translate.x - 5
	camera.backgroundTranslate.x = camera.backgroundTranslate.x + 5
	if battleMgr.leftChr.position.x >= 800 then
		dialog.hidden = false
		dialog:onKeyPress()
		currentStage = currentStage + 1
		battleMgr.leftChr.actionBlockId = 0
		battleMgr.leftChr.animationCounter = 0
		battleMgr.leftChr.animationSubFrame = 0
		battleMgr.leftChr.action = enums.actions.ACTION_IDLE
		battleMgr.leftChr:initAnimation()
	end
end

local function stage4()
	battleMgr.leftChr:updateAnimation()
	if keyPressed then
		currentStage = currentStage + 1
		battleMgr.rightChr.actionBlockId = 0
		battleMgr.rightChr.animationCounter = 0
		battleMgr.rightChr.animationSubFrame = 0
		battleMgr.rightChr.action = enums.actions.ACTION_NEUTRAL_TECH
		battleMgr.rightChr:initAnimation()
		keyPressed = false
	end
end

local function stage5()
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

local function stage6()
	battleMgr.leftChr:updateAnimation()
	battleMgr.rightChr:updateAnimation()
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

stageBg.texture:loadFromFile(packPath.."\\bnbmid3intro.png")
stageBg.size = stageBg.texture.size
stageBg.position = Vector2i.new(
	math.floor(320 - stageBg.texture.size.x / 2),
	-50
)
stageBg.rect.width  = stageBg.texture.size.x
stageBg.rect.height = stageBg.texture.size.y
stageBg.tint.a = 0

stageBottom.texture:loadFromGame("data/scenario/effect/Stage3.png")
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