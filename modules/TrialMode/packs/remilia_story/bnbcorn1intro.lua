--
-- Created by PinkySmile on 12/08/2021.
--

local dialogs = {
	--Flandre walking happily
        --Remilia coming in
        "r SOh, hello",
        "r HI didn't know you were making a party!",
        "lEHWell, me neither.",
        "lCHLet me guess, you're here to \"play\" with the guests?",
        "rChOf course!",
        "lchCould you not involve yourself in this?",
        "lChIt is already hard to keep track of the situation",
        "rCEBut, sister...",
        "lhE*sigh* Fine, I will play with you.",
        "rhHReally?",
        "lHHOf course! I'm not a bad sister after all.",
        "lAHNow don't move."
	--Battle here
}

local stageBg = Sprite.new()
local stageBottom = Sprite.new()
local ctr = 240
local currentStage = 0
local dialog
local keyPressed = false
local stop = false

local function init()
	playBGM("data/bgm/ta08.ogg")

	battleMgr.leftChr.position.x = 0
	battleMgr.leftChr.action = enums.actions.ACTION_DEFAULT_SKILL1_B

	battleMgr.rightChr.position.x = 800
	battleMgr.rightChr.action = enums.actions.ACTION_IDLE
	battleMgr.rightChr:initAnimation()
end

local function stage0()
	if ctr == 240 then
		init()
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
	battleMgr.leftChr:updateAnimation()
	battleMgr.rightChr:updateAnimation()
	if ctr % 2 == 1 then
		stageBottom.position = stageBottom.position + Vector2i.new(1, 0)
	end
	ctr = ctr - 1
	if ctr == 0 then
		currentStage = currentStage + 1
	end
end

local function stage1()
	--TODO: Make battle against Reisen
	battleMgr.leftChr:updateAnimation()
	if true then
		currentStage = currentStage + 1
		battleMgr.leftChr:initAnimation()
		battleMgr.leftChr:animate()
		battleMgr.leftChr.speed = Vector2f.new(30, 12)
		battleMgr.leftChr.position.y = 0
		battleMgr.leftChr:playSfx(10)
	end
end

local function stage2()
	battleMgr.leftChr:updateAnimation()
	battleMgr.rightChr:updateAnimation()
	battleMgr.leftChr.position = battleMgr.leftChr.position + battleMgr.leftChr.speed
	if battleMgr.leftChr.position.y <= 0 then
		if battleMgr.leftChr.speed.y < 0 then
			print(battleMgr.leftChr.position.y, battleMgr.leftChr.speed.y)
			battleMgr.leftChr.speed.x = 20
			battleMgr.leftChr.speed.y = 0
			battleMgr.leftChr:animate()
		end
		if battleMgr.leftChr.speed.x ~= 0 then
			battleMgr.leftChr.speed.x = battleMgr.leftChr.speed.x - 2
		elseif battleMgr.leftChr.actionBlockId == 0 then
			currentStage = currentStage + 1
			battleMgr.leftChr.action = enums.actions.ACTION_IDLE
			battleMgr.leftChr:initAnimation()
			dialog.hidden = false
		elseif battleMgr.leftChr.actionBlockId ~= 2 then
			battleMgr.leftChr:animate()
		end
	else
		battleMgr.leftChr.speed.y = battleMgr.leftChr.speed.y - 2
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

stageBg.texture:loadFromFile(packPath.."/bnbcorn1intro.png")
stageBg.size = stageBg.texture.size
stageBg.position = Vector2i.new(
	math.floor(320 - stageBg.texture.size.x / 2),
	-50
)
stageBg.rect.width  = stageBg.texture.size.x
stageBg.rect.height = stageBg.texture.size.y
stageBg.tint.a = 0

stageBottom.texture:loadFromGame("data/scenario/effect/Stage6.png")
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