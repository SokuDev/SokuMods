--
-- Created by PinkySmile on 12/08/2021.
--

local bool = false

local dialogs = {
	"Lc She should be around here.",
	"R CStop right here!",
	"LHCI'm no criminal.",
	"RHSSo you've come right to me!<br>I have some questions to ask you.",
	"LWSGood, so do I.",
	"RWWWhat is going on in the Scarlet Mansion?",
	"LSWHmmm",
	"LEWI hoped you would be the one to answer this question...",
	"REEAren't you the one supposed to guard the place?",
	"LSEWell actually I'm not a guard, I'm a maid but...",
	"LcEI don't think you're lying to me,<br>and you're not the sneaky type,<br>but why are you interested in our business?",
	"RcHLady Yuyuko told me to investigate.<br>She said it was important.",
	"RccI think...",
	"LhcWell then, may I speak with her.",
	"RhAI won't let you pass!",
	"LCAToo bad!",
	"RCCWhatever this incident is, you must be the culprit!",
	"LECYou really are terrible at investigating aren't you?"
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
		playBGM("data/bgm/ta03.ogg")
		battleMgr.leftChr.position.x = -100
		battleMgr.rightChr.position.x = 800
		battleMgr.rightChr.position.y = 600
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
		battleMgr.leftChr:initAnimation()
		battleMgr.leftChr.speed.x = 15
		battleMgr.leftChr.speed.y = 15
	end
end

local function stage1()
	battleMgr.leftChr:updateAnimation()
	battleMgr.leftChr.position = battleMgr.leftChr.position + battleMgr.leftChr.speed
	battleMgr.leftChr.speed.y = battleMgr.leftChr.speed.y - 0.7
	if battleMgr.leftChr.position.y <= 0 then
		currentStage = currentStage + 1
		playSfx(enums.sfxs.land)
		battleMgr.leftChr.position.y = 0
		battleMgr.leftChr.action = enums.actions.ACTION_LANDING
		battleMgr.leftChr:initAnimation()
		battleMgr.leftChr.speed.x = 0
		battleMgr.leftChr.speed.y = 0
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
		currentStage = currentStage + 1
		ctr = 30
	end
end

local function stage3()
	ctr = ctr - 1
	battleMgr.leftChr:updateAnimation()
	if ctr == 0 then
		dialog.hidden = false
	end
	if keyPressed then
		currentStage = currentStage + 1
		dialog.hidden = true
		battleMgr.rightChr.action = enums.actions.ACTION_j2A
		playSfx(enums.sfxs.longMelee)
		battleMgr.rightChr:initAnimation()
	end
end

local function stage4()
	battleMgr.leftChr:updateAnimation()
	battleMgr.rightChr:updateAnimation()
	battleMgr.rightChr.position.y = battleMgr.rightChr.position.y - 30
	if battleMgr.rightChr.position.y <= 0 then
		battleMgr.rightChr.position.y = 0
		battleMgr.rightChr:animate()
		battleMgr.rightChr:playSfx(012)
		currentStage = currentStage + 1
	end
end

local function stage5()
	battleMgr.leftChr:updateAnimation()
	battleMgr.rightChr:updateAnimation()
	if battleMgr.rightChr.actionBlockId == 0 then
		battleMgr.rightChr.action = enums.actions.ACTION_DEFAULT_SKILL4_B
		battleMgr.rightChr:initAnimation()
		playSfx(enums.sfxs.mediumMelee)
		currentStage = currentStage + 1
	end
end

local function stage6()
	battleMgr.leftChr:updateAnimation()
	battleMgr.rightChr:updateAnimation()
	while battleMgr.rightChr.actionBlockId ~= 1 do
		battleMgr.rightChr:animate()
	end
	if dialog.hidden == true then
		dialog.hidden = false
		onKeyPressed()
	end
	if #dialog == #dialogs - 6 then
		currentStage = currentStage + 1
	end
end

local function stage7()
	battleMgr.leftChr:updateAnimation()
	battleMgr.rightChr:updateAnimation()
	if battleMgr.rightChr.action == enums.actions.ACTION_DEFAULT_SKILL4_B and battleMgr.rightChr.actionBlockId == 0 then
		battleMgr.rightChr.action = enums.actions.ACTION_IDLE
		battleMgr.rightChr:initAnimation()
	end
	if bool == true then
		bool = false
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
	stage7
}

stageBg.texture:loadFromFile(packPath.."/lore5intro.png")
stageBg.size = stageBg.texture.size
stageBg.position = Vector2i.new(
	math.floor(320 - stageBg.texture.size.x / 2),
	-50
)
stageBg.rect.width  = stageBg.texture.size.x
stageBg.rect.height = stageBg.texture.size.y
stageBg.tint.a = 0

stageBottom.texture:loadFromGame("data/scenario/effect/Stage5.png")
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
	flashRect:draw()
	dialog:render()
end

function onKeyPressed()
	keyPressed = true
end