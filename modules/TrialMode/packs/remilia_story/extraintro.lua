--
-- Created by PinkySmile on 07/05/2021.
--

local dialogs = {
	"lh It seems I have finally found you, Yukari!",
	"lW Or should I call you the partner in crime?",
	"rWS... Hello Remilia.",
	"lhSI can read the surprise on your face!<br>Perhaps you wonder how I found out?",
	"lcSYou see, Remilia Scarlet had finally found the<br>culprit through her fight with the shrine maiden,<br>but this answer alone was insufficient.",
	"lcSThe \"how\" and the \"why\" were still unanswered.",
	"rcEOh I get it... You're doing the armchair detective<br>thing again.",
	"rcHWell, after all, I do have my fair share of secrets.",
	"lAHBut I don't have the full truth yet. Before my final<br>revelation speech I still need to see through<br>your lies and secrets...",
	"rAAHow exciting. Will you interrogate me then?",
	"lCANo...",
	"lhAI will beat you in a danmaku battle!",
	--Battle here
}

local stageBg = Sprite.new()
local stageBottom = Sprite.new()
local ctr = 240
local currentStage = 0
local dialog = StandDialog.new(dialogs)
local keyPressed = false
local stop = false

print("Init intro.")
stageBg.texture:loadFromFile(packPath.."\\extraintro.png")
stageBg.size = stageBg.texture.size
stageBg.position = Vector2i.new(
	math.floor(320 - stageBg.texture.size.x / 2),
	-50
)
stageBg.rect.width  = stageBg.texture.size.x
stageBg.rect.height = stageBg.texture.size.y
stageBg.tint.a = 0

stageBottom.texture:loadFromResource(484)
stageBottom.size = stageBottom.texture.size
stageBottom.position = Vector2i.new(
	math.floor(320 - stageBg.texture.size.x / 2),
	math.floor(-50 + stageBg.texture.size.y)
)
stageBottom.rect.width  = stageBottom.texture.size.x
stageBottom.rect.height = stageBottom.texture.size.y
stageBottom.tint.a = 0


local function init()
	battleMgr.rightChr.direction = enums.directions.LEFT

	battleMgr.leftChr.direction = enums.directions.LEFT
	battleMgr.leftChr.position.x = 800 + 380
	battleMgr.leftChr.action = enums.actions.ACTION_ALT1_SKILL4_C
	battleMgr.leftChr:initAnimation()
	playBgm("data/bgm/ta07.ogg")
end

local function stage0()
	if ctr == 240 then
		init()
	end
	battleMgr.rightChr:updateAnimation()
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
	end
end

local function stage1()
	battleMgr.leftChr:updateMove()
	battleMgr.leftChr.objects:update()
	battleMgr.leftChr.position = battleMgr.leftChr.position + battleMgr.leftChr.speed
	battleMgr.rightChr:updateAnimation()
	if battleMgr.leftChr.actionBlockId == 1 and battleMgr.leftChr.frameCount == 12 then
		battleMgr.rightChr.direction = enums.directions.RIGHT
	end
	if battleMgr.leftChr.actionBlockId == 6 and battleMgr.leftChr.frameCount == 0 then
		battleMgr.rightChr.action = enums.actions.ACTION_IDLE
		battleMgr.rightChr:initAnimation()
	end
	if battleMgr.leftChr.actionBlockId == 4 then
		if battleMgr.leftChr.frameCount == 6 then
			battleMgr.rightChr.direction = enums.directions.LEFT
			battleMgr.rightChr.action = enums.actions.ACTION_GROUND_CRUSHED
			battleMgr.rightChr:initAnimation()
		end
		if battleMgr.leftChr.speed.x < 0 then
			battleMgr.leftChr.speed.x = 0
			battleMgr.leftChr.direction = enums.directions.RIGHT
			ctr = 60
		end
	elseif battleMgr.leftChr.speed.x > 0 then
		battleMgr.leftChr.speed.x = -battleMgr.leftChr.speed.x
	end
	if ctr ~= 0 then
		ctr = ctr - 1
		camera.translate.y = 420 + math.sin(ctr * math.pi / 2) * ctr / 5
		camera.backgroundTranslate.y = 0 - math.sin(ctr * math.pi / 2) * ctr / 5
	end
	if battleMgr.leftChr.action == enums.actions.ACTION_IDLE then
		currentStage = currentStage + 1
		dialog.hidden = false
	end
end

local function stage2()
	battleMgr.leftChr:updateAnimation()
	battleMgr.leftChr.objects:update()
	battleMgr.rightChr:updateAnimation()
end

local function stage3()
end

local anims = {
	stage0,
	stage1,
	stage2,
	stage3
}

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
	if ctr ~= 0 then
		stageBg:draw()
		stageBottom:draw()
	end
	dialog:render()
end

function onKeyPressed()
	keyPressed = true
end