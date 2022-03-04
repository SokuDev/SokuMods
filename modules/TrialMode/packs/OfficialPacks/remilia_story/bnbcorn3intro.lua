--
-- Created by PinkySmile on 12/08/2021.
--

local dialogs = {
	--Remilia moving with the camera zoomed on her
	"lh Here you finally are!",
	--Camera unzooming to reveal Reimu
	"lW Were you missing me that much?",
	"rWSWeren't you the one looking for me?",
	"lHSSo tell me...",
	"lcSWhat are you doing?",
	"rccWell there is a bunch of unusual youkai<br>here wrecking havoc...",
	"rchIs there anything strange with me being here?",
	"lChYes, I've seen many youkai here and rare were<br>the one to have been beaten specifically by you!",
	"rCSYou can recognize the ones that fought against me?",
	"lESWell you know, I fought you many times now...",
	"lASAnyway you are hiding something, I can feel it.<br>Now tell me everything.",
	"rAEWill you not harass me if I tell you?",
	"lhEHere I come Reimu!",
	"rhEAs I thought"
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
	playBGM("data/bgm/ta02.ogg")

	camera.backgroundTranslate.x = 100
	camera.translate.y = 210
	camera.scale = 2

	battleMgr.leftChr.position.x = 180
	battleMgr.leftChr.action = enums.actions.ACTION_WALK_FORWARD
	battleMgr.leftChr:initAnimation()

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

	if ctr % 2 == 1 then
		stageBottom.position = stageBottom.position + Vector2i.new(1, 0)
	end
	ctr = ctr - 1
	if ctr == 0 then
		currentStage = currentStage + 1
	end
end

local function stage1()
	battleMgr.leftChr:updateAnimation()
	if battleMgr.leftChr.position.x ~= 480 then
		battleMgr.leftChr.position.x = battleMgr.leftChr.position.x + 6
	elseif camera.backgroundTranslate.x ~= 640 then
		camera.backgroundTranslate.x = camera.backgroundTranslate.x + 6
	else
		currentStage = currentStage + 1
		dialog.hidden = false
		battleMgr.leftChr.action = enums.actions.ACTION_IDLE
		battleMgr.leftChr:initAnimation()
	end
end

local function stage2()
	battleMgr.leftChr:updateAnimation()
	if keyPressed then
		keyPressed = false
		currentStage = currentStage + 1
	end
end

local function stage3()
	battleMgr.leftChr:updateAnimation()
	battleMgr.rightChr:updateAnimation()
	keyPressed = false
	camera.scale = camera.scale - 0.025
	camera.translate.y = 420 / camera.scale
	if camera.scale <= 1 then
		camera.scale = 1
		dialog:onKeyPress()
		camera.translate.y = 420
		currentStage = currentStage + 1
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

stageBg.texture:loadFromFile(packPath.."/bnbcorn3intro.png")
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