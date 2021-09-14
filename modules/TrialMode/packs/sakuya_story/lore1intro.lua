--
-- Created by PinkySmile on 12/08/2021.
--

local dialogs = {
	--Remilia going out
	--Sakuya standing up
	"lH The mistress is quite lively today.",
	"lc If she likes visits that much, maybe I should<br>let them in?",
	"lH Well that wouldn't be appropriate from a maid.",
	"lS Now, I should begin my instigations. I fear that the<br>mystery of this intruder might not be solved without<br>confronting the sun...",
	--Marisa coming and seeing sakuya and try to sneak out
	"lC Stop right here!",
	"rCEShoot...",
	"rChI'm just passing by.",
	"lAhYou know, you shouldn't be here in the first place.",
	"rACCan I just stay here for a while? ",
	"rAhI won't steal!",
	"lhhI will let you live here as long as sodium does.",
	"rhHOk.",
	"r EHow long is that?",
	"lW Time's up!",
	--Battle here
}

local stageBg = Sprite.new()
local stageBottom = Sprite.new()
local ctr = 240
local currentStage = 0
local dialog
local keyPressed = false
local stop = false
local remilia
local playerInfo = PlayerInfo.new(enums.characters.remilia, 0, true)


-- CHR 26 -> 1

local function stage0()
	if ctr == 240 then
		remilia = Character.new(playerInfo)

		battleMgr.leftChr.position.x = 800
		battleMgr.leftChr.action = enums.actions.ACTION_IDLE
		battleMgr.leftChr:initAnimation()
		battleMgr.leftChr.direction = enums.directions.LEFT

		battleMgr.rightChr.position.x = 1300
		battleMgr.rightChr.action = enums.actions.ACTION_WALK_FORWARD
		battleMgr.rightChr:initAnimation()
		battleMgr.rightChr.direction = enums.directions.LEFT

		remilia.position.x = 480
		remilia.position.y = 0
		remilia.action = enums.actions.ACTION_IDLE
		remilia:initAnimation()
		remilia.direction = enums.directions.RIGHT
		playBGM("data/bgm/ta00.ogg")
	end
	battleMgr.leftChr:updateAnimation()
	remilia:updateAnimation()
	if ctr == 120 then
		remilia.direction = enums.directions.LEFT
		remilia.action = enums.actions.ACTION_WALK_FORWARD
		remilia:initAnimation()
	elseif ctr < 120 then
		remilia.position.x = remilia.position.x - 6
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
		dialog.hidden = false
	end
end

local function stage1()
	battleMgr.leftChr:updateAnimation()
	if #dialog == #dialogs - 4 and keyPressed then
		keyPressed = false
		dialog.hidden = true
		currentStage = currentStage + 1
	elseif camera.translate.x > -480 then
		camera.translate.x = camera.translate.x - 2
		camera.backgroundTranslate.x = camera.backgroundTranslate.x + 2
	end
end

local function stage2()
	battleMgr.leftChr:updateAnimation()
	battleMgr.rightChr:updateAnimation()
	if camera.translate.x > -480 then
		camera.translate.x = camera.translate.x - 4
		camera.backgroundTranslate.x = camera.backgroundTranslate.x + 4
	else
		if battleMgr.rightChr.action == enums.actions.ACTION_WALK_FORWARD then
			if battleMgr.rightChr.position.x > 1100 then
				battleMgr.rightChr.position.x = battleMgr.rightChr.position.x - 4
			else
				battleMgr.rightChr.action = enums.actions.ACTION_GROUND_CRUSHED
				battleMgr.rightChr:initAnimation()
				ctr = 0
			end
		elseif battleMgr.rightChr.action == enums.actions.ACTION_GROUND_CRUSHED then
			ctr = ctr + 1
			if ctr == 30 then
				battleMgr.rightChr.action = enums.actions.ACTION_WALK_BACKWARD
				battleMgr.rightChr:initAnimation()
			end
		elseif battleMgr.rightChr.action == enums.actions.ACTION_WALK_BACKWARD then
			keyPressed = false
			if battleMgr.rightChr.position.x == 1150 then
				battleMgr.leftChr.direction = enums.directions.RIGHT
			end
			if battleMgr.rightChr.position.x == 1170 then
				dialog.hidden = false
				dialog:onKeyPress()
			end
			if battleMgr.rightChr.position.x < 1225 then
				battleMgr.rightChr.position.x = battleMgr.rightChr.position.x + 2
			else
				battleMgr.rightChr.action = enums.actions.ACTION_IDLE
				battleMgr.rightChr:initAnimation()
				currentStage = currentStage + 1
			end
		end
		if camera.translate.x > -640 then
			camera.translate.x = camera.translate.x - 8
			camera.backgroundTranslate.x = camera.backgroundTranslate.x + 8
		end
	end
end

local function stage3()
	battleMgr.leftChr:updateAnimation()
	battleMgr.rightChr:updateAnimation()
	if #dialog == 2 and keyPressed then
		currentStage = currentStage + 1
		dialog.hidden = true
		ctr = 0
	end
end

local function stage4()
	ctr = ctr + 1
	battleMgr.leftChr:updateAnimation()
	battleMgr.rightChr:updateAnimation()
	if ctr == 60 then
		dialog.hidden = false
		dialog:onKeyPress()
	end
	if keyPressed then
		currentStage = currentStage + 1
		dialog.hidden = true
		ctr = 0
	end
end

local function stage5()
	ctr = ctr + 1
	battleMgr.leftChr:updateAnimation()
	battleMgr.rightChr:updateAnimation()
	if ctr == 120 then
		dialog.hidden = false
		dialog:onKeyPress()
	end
end

local anims = {
	stage0,
	stage1,
	stage2,
	stage3,
	stage4,
	stage5
}

print("Init intro.")

stageBg.texture:loadFromFile(packPath.."\\lore1intro.png")
stageBg.size = stageBg.texture.size
stageBg.position = Vector2i.new(
	math.floor(320 - stageBg.texture.size.x / 2),
	-50
)
stageBg.rect.width  = stageBg.texture.size.x
stageBg.rect.height = stageBg.texture.size.y
stageBg.tint.a = 0

stageBottom.texture:loadFromGame("data/scenario/effect/Stage1.png")
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
	if currentStage == 0 and remilia then
		remilia:draw()
		remilia.objects:draw(-1)
		remilia.objects:draw(1)
	end
	stageBg:draw()
	stageBottom:draw()
	dialog:render()
end

function onKeyPressed()
	keyPressed = true
end