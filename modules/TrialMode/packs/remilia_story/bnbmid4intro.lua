--
-- Created by PinkySmile on 12/08/2021.
--

local dialogs = {
	"lH As easy to find as ever...",
        --walk onto the reading Patchouly.
        "rHHAnd you are surprisingly upbeat.",
        "rHCDo you enjoy your home being ransacked that much?",
        "lHCThey are merely passers by.",
        "lCCThough it is unsettling how easily people invite<br>themselves...",
        "rCSThe mansion's cats have been quite ineffective this<br>time...",
        "rCWNonetheless you have come for information. Am I correct?",
        "rCAI fear that this time I won't be able to help.<br>The havoc in the mansion makes it impossible to know<br>for certain who this mysterious intruder could be...",
        "lcAI see...",
        "lAAI've gone as far as to give you a gift and you<br>still hold information from me?<br>You need more motivation to help me it seems...",
        "rAE*sigh* Please be wary of my health.",
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
		battleMgr.leftChr.position.x = -200
		battleMgr.rightChr.position.x = 900
		battleMgr.rightChr.action = enums.actions.ACTION_RIGHTBLOCK_LOW_MEDIUM_BLOCKSTUN
		battleMgr.rightChr:initAnimation()
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
		dialog.hidden = false
		ctr = 10
	end
end

local function stage1()
	if keyPressed then
		currentStage = currentStage + 1
		dialog.hidden = true
	end
end

local function stage2()
	battleMgr.leftChr:updateAnimation()
	battleMgr.leftChr.position.x = battleMgr.leftChr.position.x + 6
	if ctr ~= 0 then
		ctr = ctr - 1
	else
		battleMgr.rightChr:updateAnimation()
		if battleMgr.rightChr.action == enums.actions.ACTION_RIGHTBLOCK_LOW_MEDIUM_BLOCKSTUN and battleMgr.rightChr.animationCounter == 1 and battleMgr.rightChr.animationSubFrame == 4 then
			battleMgr.rightChr.action = enums.actions.ACTION_STANDING_UP
			battleMgr.rightChr:initAnimation()
		elseif battleMgr.rightChr.action == enums.actions.ACTION_STANDING_UP and battleMgr.rightChr.animationCounter == 2 and battleMgr.rightChr.animationSubFrame == 4 then
			battleMgr.rightChr.action = enums.actions.ACTION_IDLE
			battleMgr.rightChr:initAnimation()
		end
	end
	if battleMgr.leftChr.position.x >= 400 then
		currentStage = currentStage + 1
		dialog.hidden = false
		dialog:onKeyPress()
		battleMgr.leftChr.action = enums.actions.ACTION_IDLE
		battleMgr.leftChr:initAnimation()
	end
end

local function stage3()
	battleMgr.leftChr:updateAnimation()
	battleMgr.rightChr:updateAnimation()
end

local function stage4()
end

local function stage5()
end

local function stage6()
	battleMgr.leftChr:updateAnimation()
	battleMgr.rightChr:updateAnimation()
	if #dialog == #dialogs - 14 then
		currentStage = currentStage + 1
	end
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

stageBg.texture:loadFromFile(packPath.."/bnbmid4intro.png")
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