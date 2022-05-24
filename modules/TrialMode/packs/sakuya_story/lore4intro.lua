--
-- Created by PinkySmile on 12/08/2021.
--

local dialogs = {
	"lc Where could she be...",
	--Reisen appears
	"r WHere you are again?",
	"r hIf you wish to meet our princess I<br>fear you'll need her approval.",
	"lhhDo not bother your master.",
	"lHhIt is you that I have business with.",
	"rHSMe? How curious?<br>And what do you have to do with me?",
	"lWSIllusion is your thing, isn't it?",
	"lcSIt must be quite easy to hide yourself<br>from your enemies and allies...",
	"rcCAnd what are you implying!?",
	"lCCThat you are a suspect!",
	"rCSAnd why would I possibly cause trouble alone?",
	"lhSWho knows? A Tewi prank perhaps? Or maybe<br>you had some schemes in mind? After all<br>you're still a moon rabbit at your core, aren't you?",
	"rhADon't underestimate my loyalty!",
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
		playBGM("data/bgm/ta01.ogg")
		battleMgr.leftChr.position.x = -100
		battleMgr.rightChr.position.x = 1000
		battleMgr.rightChr.renderInfos.color.a = 0
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
		ctr = 30
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
		battleMgr.leftChr.actionBlockId = 0
		battleMgr.leftChr.animationCounter = 0
		battleMgr.leftChr.animationSubFrame = 0
		battleMgr.leftChr.action = enums.actions.ACTION_IDLE
		battleMgr.leftChr:initAnimation()
		currentStage = currentStage + 1
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
		battleMgr.rightChr.action = enums.actions.ACTION_WALK_FORWARD
		battleMgr.rightChr:initAnimation()
		battleMgr.rightChr:playSfx(17)
	end
end

local function stage4()
	battleMgr.leftChr:updateAnimation()
	battleMgr.rightChr:updateAnimation()
	if battleMgr.rightChr.renderInfos.color.a ~= 0xFF then
		battleMgr.rightChr.renderInfos.color.a = battleMgr.rightChr.renderInfos.color.a + 5
	else
		currentStage = currentStage + 1
		dialog.hidden = false
		dialog:onKeyPress()
		keyPressed = false
		battleMgr.rightChr.action = enums.actions.ACTION_IDLE
		battleMgr.rightChr:initAnimation()
		return
	end
	battleMgr.rightChr.position.x = battleMgr.rightChr.position.x - 4
	if camera.translate.x ~= -420 then
		camera.translate.x = camera.translate.x - 5
		camera.backgroundTranslate.x = camera.backgroundTranslate.x + 5
	end
end

local function stage5()
	battleMgr.leftChr:updateAnimation()
	battleMgr.rightChr:updateAnimation()
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

stageBg.texture:loadFromFile(packPath.."/lore4intro.png")
stageBg.size = stageBg.texture.size
stageBg.position = Vector2i.new(
	math.floor(320 - stageBg.texture.size.x / 2),
	-50
)
stageBg.rect.width  = stageBg.texture.size.x
stageBg.rect.height = stageBg.texture.size.y
stageBg.tint.a = 0

stageBottom.texture:loadFromGame("data/scenario/effect/Stage4.png")
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