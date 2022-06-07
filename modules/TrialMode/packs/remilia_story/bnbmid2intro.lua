--
-- Created by PinkySmile on 18/08/2021.
--

local dialogs = {
	"lh Sakuya?",
	"rhHYes, mistress?",
	"lCHDo you know something about a mysterious intruder?<br>One that has been hidding in the mansion.",
	"rCSMysterious? No not that I know...<br>Still, considering the state of our \"gate keeper\"<br>anyone could have come...",
	"lASYou, too, are in charge of security, you know?",
	"lhSIt seems I need to teach you how to do it."
	--Battle here
}

local stageBg = Sprite.new()
local stageBottom = Sprite.new()
local ctr = 240
local currentStage = 0
local dialog
local keyPressed = false
local stop = false

function stage0()
	if ctr == 240 then
		battleMgr.leftChr.actionBlockId = 0
		battleMgr.leftChr.animationCounter = 0
		battleMgr.leftChr.animationSubFrame = 0
		battleMgr.leftChr.action = enums.actions.ACTION_IDLE
		battleMgr.leftChr:initAnimation()
		battleMgr.rightChr.position.x = 1200
		playBgm("data/bgm/ta00.ogg")
	end

	battleMgr.leftChr:updateAnimation()
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
		dialog.hidden = false
		currentStage = currentStage + 1
	end
end

function stage1()
	battleMgr.leftChr:updateAnimation()
	battleMgr.rightChr:updateAnimation()
	if keyPressed then
		battleMgr.rightChr.actionBlockId = 0
		battleMgr.rightChr.animationCounter = 0
		battleMgr.rightChr.animationSubFrame = 0
		battleMgr.rightChr.action = enums.actions.ACTION_FORWARD_JUMP
		battleMgr.rightChr:initAnimation()
		battleMgr.rightChr.speed.x = -10
		battleMgr.rightChr.speed.y = 15
		battleMgr.rightChr.position.x = 1240
		currentStage = currentStage + 1
		keyPressed = false
	end
end

function stage2()
	keyPressed = false
	battleMgr.leftChr:updateAnimation()
	battleMgr.rightChr:updateAnimation()
	battleMgr.rightChr.position = battleMgr.rightChr.position + battleMgr.rightChr.speed
	battleMgr.rightChr.speed.y = battleMgr.rightChr.speed.y - 0.7
	if battleMgr.rightChr.position.y <= 0 and battleMgr.rightChr.speed.y < 0 then
		battleMgr.rightChr.position.y = 0
		battleMgr.rightChr.speed.x = 0
		battleMgr.rightChr.speed.y = 0
		playSfx(enums.sfxs.land)
		battleMgr.rightChr.actionBlockId = 0
		battleMgr.rightChr.animationCounter = 0
		battleMgr.rightChr.animationSubFrame = 0
		battleMgr.rightChr.action = enums.actions.ACTION_LANDING
		battleMgr.rightChr:initAnimation()
		currentStage = currentStage + 1
		dialog:onKeyPress()
	end
end

function stage3()
	battleMgr.leftChr:updateAnimation()
	battleMgr.rightChr:updateAnimation()
	if
		battleMgr.rightChr.action == enums.actions.ACTION_LANDING and
		battleMgr.rightChr.actionBlockId == 0 and
		battleMgr.rightChr.animationCounter == 0 and
		battleMgr.rightChr.animationSubFrame == 0
	then
		battleMgr.rightChr.actionBlockId = 0
		battleMgr.rightChr.animationCounter = 0
		battleMgr.rightChr.animationSubFrame = 0
		battleMgr.rightChr.action = enums.actions.ACTION_IDLE
		battleMgr.rightChr:initAnimation()
	end
end

local anims = {
	stage0,
	stage1,
	stage2,
	stage3
}

print("Init intro.")

stageBg.texture:loadFromFile(packPath.."\\bnbmid2intro.png")
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
	if ctr ~= 0 then
		stageBg:draw()
		stageBottom:draw()
	end
	dialog:render()
end

function onKeyPressed()
	keyPressed = true
end