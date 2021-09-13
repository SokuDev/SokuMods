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

local TRANSLATE_MAX = 120
local REISEN_START_LOCATION = 570
local CAM_START_LOCATION = -20
local BG_START_LOCATION = 850
local HIT_STOP = 7
local PUSH_RATIO = 2

local stageBg = Sprite.new()
local stageBottom = Sprite.new()
local falling = true
local remiIn = false
local fightCtr = 120
local fightStage = 0
local ctr = 240
local ctr2 = 0
local ctr3 = 0
local currentStage = 0
local dialog = new
local keyPressed = false
local stop = false
local playerInfo = PlayerInfo.new(enums.characters.reisen, 0, false)
local reisen = nil

function fightStage0()
	reisen.position = reisen.position + reisen.speed
	reisen.speed.y = reisen.speed.y - 0.6
	battleMgr.rightChr.position = battleMgr.rightChr.position + battleMgr.rightChr.speed
	battleMgr.rightChr.speed.y = battleMgr.rightChr.speed.y - 0.6
	if reisen.position.y <= 300 and reisen.action ~= enums.actions.ACTION_j5A then
		reisen.action = enums.actions.ACTION_j5A
		reisen:initAnimation()
	end
	if battleMgr.rightChr.position.y <= 200 then
		battleMgr.rightChr.speed = Vector2f.new(0, 0)
		battleMgr.rightChr.action = enums.actions.ACTION_FORWARD_AIRDASH
		battleMgr.rightChr:initAnimation()
		fightStage = fightStage + 1
	end
	if REISEN_START_LOCATION - reisen.position.x + CAM_START_LOCATION < TRANSLATE_MAX then
		camera.translate.x = REISEN_START_LOCATION - reisen.position.x + CAM_START_LOCATION
		camera.backgroundTranslate.x = reisen.position.x - REISEN_START_LOCATION + BG_START_LOCATION
	else
		camera.translate.x = TRANSLATE_MAX
		camera.backgroundTranslate.x = CAM_START_LOCATION - TRANSLATE_MAX + BG_START_LOCATION
	end
end

function fightStage1()
	reisen.position = reisen.position + reisen.speed
	battleMgr.rightChr.position = battleMgr.rightChr.position + battleMgr.rightChr.speed
	if falling then
		reisen.speed.y = reisen.speed.y - 0.6
	end
	if reisen.action == enums.actions.ACTION_j5A then
		if reisen.frameCount == 8 then
			playSfx(enums.sfxs.shortMelee)
		end
		if reisen.position.y <= 0 then
			playSfx(enums.sfxs.land)
			reisen.speed.x = -4
			reisen.speed.y = 0
			reisen.position.y = 0
			reisen.action = enums.actions.ACTION_WALK_BACKWARD
			reisen:initAnimation()
			falling = false
		end
	elseif reisen.action == enums.actions.ACTION_RIGHTBLOCK_HIGH_MEDIUM_BLOCKSTUN or reisen.action == enums.actions.ACTION_RIGHTBLOCK_LOW_BIG_BLOCKSTUN then
		if reisen.speed.x > 0 then
			reisen.speed.x = reisen.speed.x + 0.6
		else
			reisen.speed.x = 0
		end
	elseif reisen.action == enums.actions.ACTION_BE2 then
		if reisen.actionBlockId == 1 and reisen.frameCount == 0 then
			reisen.speed.y = 20
			playSfx(43)
		elseif reisen.actionBlockId >= 1 then
			reisen.speed.y = reisen.speed.y - 0.75
		end
	end

	if remiIn then
		battleMgr.leftChr:updateAnimation()
		battleMgr.leftChr:updateAnimation()
		battleMgr.leftChr.position = battleMgr.leftChr.position + battleMgr.leftChr.speed
		if battleMgr.leftChr.position.y <= 0 then
			if battleMgr.leftChr.speed.y < 0 then
				battleMgr.leftChr.speed.x = 20
				battleMgr.leftChr.speed.y = 0
				battleMgr.leftChr:animate()
			end
			if battleMgr.leftChr.speed.x ~= 0 then
				battleMgr.leftChr.speed.x = battleMgr.leftChr.speed.x - 2
			elseif battleMgr.leftChr.actionBlockId == 0 then
				fightStage = fightStage + 1
				dialog.hidden = false
				battleMgr.leftChr.action = enums.actions.ACTION_IDLE
				battleMgr.leftChr:initAnimation()
			elseif battleMgr.leftChr.actionBlockId ~= 2 then
				battleMgr.leftChr:animate()
			end
		else
			battleMgr.leftChr.speed.y = battleMgr.leftChr.speed.y - 2
		end
	end

	if reisen.action == enums.actions.ACTION_NEUTRAL_HIGH_JUMP then
		if reisen.actionBlockId == 1 and reisen.frameCount == 0 then
			reisen.speed.y = 20
			playSfx(enums.sfxs.highJump)
		elseif reisen.actionBlockId >= 1 then
			reisen.speed.y = reisen.speed.y - 0.75
		end
	end

	if battleMgr.rightChr.position.x < reisen.position.x + 75 then
		reisen.position.x = reisen.position.x + battleMgr.rightChr.speed.x / PUSH_RATIO
		if battleMgr.rightChr.position.x < reisen.position.x + 75 then
			battleMgr.rightChr.position.x = reisen.position.x + 75
		end
	end

	if battleMgr.rightChr.action == enums.actions.ACTION_NEUTRAL_HIGH_JUMP then
		if battleMgr.rightChr.actionBlockId == 1 and battleMgr.rightChr.frameCount == 0 then
			battleMgr.rightChr.speed.y = 23
			playSfx(43)
		elseif battleMgr.rightChr.actionBlockId >= 1 then
			battleMgr.rightChr.speed.y = battleMgr.rightChr.speed.y - 1
		end
	elseif battleMgr.rightChr.action == enums.actions.ACTION_3A then
		ctr2 = ctr2 + 1
		if ctr2 == 15 then
			battleMgr.leftChr:initAnimation()
			battleMgr.leftChr:animate()
			battleMgr.leftChr.speed = Vector2f.new(30, 12)
			battleMgr.leftChr.position.y = 0
			battleMgr.leftChr:playSfx(10)
			remiIn = true
		end
		if battleMgr.rightChr.frameCount == 10 then
			playSfx(enums.sfxs.longMelee)
			battleMgr.rightChr.speed.x = -13
		elseif battleMgr.rightChr.frameCount == 14 then
			playSfx(enums.sfxs.block)
			fightCtr = HIT_STOP
			reisen.speed.x = -13
			reisen.action = enums.actions.ACTION_RIGHTBLOCK_LOW_BIG_BLOCKSTUN
			reisen:initAnimation()
		elseif ctr2 == 20 then
			reisen.speed.x = 0
			reisen.action = enums.actions.ACTION_BE2
			reisen:initAnimation()
			playSfx(enums.sfxs.borderEscape)
		elseif ctr2 == 25 then
			ctr2 = 0
			battleMgr.rightChr.speed.x = 0
			battleMgr.rightChr.action = enums.actions.ACTION_NEUTRAL_HIGH_JUMP
			battleMgr.rightChr:initAnimation()
		end
	elseif battleMgr.rightChr.action == enums.actions.ACTION_2A then
		if battleMgr.rightChr.frameCount == 8 then
			playSfx(enums.sfxs.block)
			playSfx(enums.sfxs.shortMelee)
			fightCtr = HIT_STOP
			reisen.action = enums.actions.ACTION_RIGHTBLOCK_LOW_SMALL_BLOCKSTUN
			reisen:initAnimation()
		elseif battleMgr.rightChr.frameCount == 9 then
			battleMgr.rightChr.action = enums.actions.ACTION_3A
			battleMgr.rightChr:initAnimation()
			ctr2 = 0
		end
	elseif battleMgr.rightChr.action == enums.actions.ACTION_j5AA then
		if battleMgr.rightChr.frameCount == 5 then
			playSfx(enums.sfxs.block)
			playSfx(enums.sfxs.shortMelee)
			fightCtr = HIT_STOP
			reisen.speed.x = -10
			reisen.action = enums.actions.ACTION_RIGHTBLOCK_HIGH_MEDIUM_BLOCKSTUN
			reisen:initAnimation()
		end
		if battleMgr.rightChr.position.y <= 0 then
			battleMgr.rightChr.speed = Vector2f.new(0, 0)
			battleMgr.rightChr.position.y = 0
			battleMgr.rightChr.action = enums.actions.ACTION_2A
			battleMgr.rightChr:initAnimation()
			playSfx(enums.sfxs.land)
		end
	elseif battleMgr.rightChr.action == enums.actions.ACTION_j5A then
		if battleMgr.rightChr.frameCount == 8 then
			playSfx(enums.sfxs.block)
			playSfx(enums.sfxs.shortMelee)
			fightCtr = HIT_STOP
			reisen.speed.x = -10
			reisen.action = enums.actions.ACTION_RIGHTBLOCK_HIGH_MEDIUM_BLOCKSTUN
			reisen:initAnimation()
		elseif battleMgr.rightChr.frameCount == 9 then
			battleMgr.rightChr.action = enums.actions.ACTION_j5AA
			battleMgr.rightChr:initAnimation()
		end
	else
		if battleMgr.rightChr.actionBlockId == 1 and battleMgr.rightChr.frameCount == 0 then
			battleMgr.rightChr.speed = Vector2f.new(-30, -4)
			playSfx(enums.sfxs.dash)
		elseif battleMgr.rightChr.actionBlockId == 1 and battleMgr.rightChr.frameCount == 8 then
			battleMgr.rightChr.speed.x = -10
		elseif battleMgr.rightChr.actionBlockId == 2 then
			battleMgr.rightChr.speed.y = -4.5
			battleMgr.rightChr.action = enums.actions.ACTION_j5A
			battleMgr.rightChr:initAnimation()
		end
	end
	if REISEN_START_LOCATION - reisen.position.x + CAM_START_LOCATION < TRANSLATE_MAX then
		camera.translate.x = REISEN_START_LOCATION - reisen.position.x + CAM_START_LOCATION
		camera.backgroundTranslate.x = reisen.position.x - REISEN_START_LOCATION + BG_START_LOCATION
	else
		camera.translate.x = TRANSLATE_MAX
		camera.backgroundTranslate.x = CAM_START_LOCATION - TRANSLATE_MAX + BG_START_LOCATION
	end
end

local function fightStage2()
	reisen.position = reisen.position + reisen.speed
	battleMgr.rightChr.position = battleMgr.rightChr.position + battleMgr.rightChr.speed

	ctr3 = ctr3 + 1
	if ctr3 == 5 then
		battleMgr.rightChr.speed.y = 2.66
		battleMgr.rightChr.action = enums.actions.ACTION_j6A
		battleMgr.rightChr:initAnimation()
		reisen.action = enums.actions.ACTION_j2B
		reisen:initAnimation()
	end

	if reisen.action == enums.actions.ACTION_BE2 then
		if reisen.actionBlockId == 1 and reisen.frameCount == 0 then
			reisen.speed.y = 20
			playSfx(enums.sfxs.highJump)
		elseif reisen.actionBlockId >= 1 then
			reisen.speed.y = reisen.speed.y - 0.75
		end
	elseif reisen.position.x < -500 then
		battleMgr.rightChr.action = enums.actions.ACTION_FLY
		battleMgr.rightChr:initAnimation()
		battleMgr.rightChr.speed = Vector2f.new(0, 0)
		reisen.speed = Vector2f.new(0, 0)
		reisen.position.x = -490
	end

	battleMgr.leftChr:updateAnimation()
	if battleMgr.rightChr.action == enums.actions.ACTION_NEUTRAL_HIGH_JUMP then
		if battleMgr.rightChr.actionBlockId == 1 and battleMgr.rightChr.frameCount == 0 then
			battleMgr.rightChr.speed.y = 23
			playSfx(enums.sfxs.highJump)
		elseif battleMgr.rightChr.actionBlockId >= 1 then
			battleMgr.rightChr.speed.y = battleMgr.rightChr.speed.y - 1
		end
	elseif battleMgr.rightChr.action == enums.actions.ACTION_3A then
		ctr2 = ctr2 + 1
		if battleMgr.rightChr.frameCount == 10 then
			playSfx(enums.sfxs.longMelee)
			battleMgr.rightChr.speed.x = -13
		elseif battleMgr.rightChr.frameCount == 14 then
			playSfx(enums.sfxs.block)
			fightCtr = HIT_STOP
			reisen.speed.x = -13
			reisen.action = enums.actions.ACTION_RIGHTBLOCK_LOW_BIG_BLOCKSTUN
			reisen:initAnimation()
		elseif ctr2 == 20 then
			reisen.speed.x = 0
			reisen.action = enums.actions.ACTION_BE2
			reisen:initAnimation()
			playSfx(enums.sfxs.borderEscape)
		elseif ctr2 == 25 then
			ctr2 = 0
			battleMgr.rightChr.speed.x = 0
			battleMgr.rightChr.action = enums.actions.ACTION_NEUTRAL_HIGH_JUMP
			battleMgr.rightChr:initAnimation()
		end
	elseif battleMgr.rightChr.position.y < 0 and battleMgr.rightChr.action ~= enums.actions.ACTION_LANDING then
		battleMgr.rightChr.action = enums.actions.ACTION_LANDING
		battleMgr.rightChr:initAnimation()
		battleMgr.rightChr.position.y = 0
		battleMgr.rightChr.speed = Vector2f.new(0, 0)
		playSfx(enums.sfxs.land)
	elseif
		battleMgr.rightChr.action == enums.actions.ACTION_LANDING and
		battleMgr.rightChr.actionBlockId == 0 and
		battleMgr.rightChr.frameCount == 0
	then
		battleMgr.rightChr.action = enums.actions.ACTION_IDLE
		battleMgr.rightChr:initAnimation()
		dialog:onKeyPress()
		fightStage = fightStage + 1
	elseif battleMgr.rightChr.action == enums.actions.ACTION_FALLING then
		battleMgr.rightChr.speed.y = battleMgr.rightChr.speed.y - 0.6
	elseif battleMgr.rightChr.action == enums.actions.ACTION_FLY then
		if battleMgr.rightChr.actionBlockId == 1 and battleMgr.rightChr.frameCount == 0 then
			battleMgr.rightChr.speed.y = -25
			playSfx(enums.sfxs.dash)
		elseif battleMgr.rightChr.actionBlockId == 1 and battleMgr.rightChr.frameCount == 12 then
			battleMgr.rightChr.speed.y = -4
		elseif battleMgr.rightChr.actionBlockId > 1 or battleMgr.rightChr.frameCount == 14 then
			battleMgr.rightChr.speed.y = battleMgr.rightChr.speed.y - 0.6
		end
	elseif battleMgr.rightChr.action == enums.actions.ACTION_j6A then
		if battleMgr.rightChr.actionBlockId == 0 and battleMgr.rightChr.frameCount == 12 then
			playSfx(enums.sfxs.counterHit)
			playSfx(enums.sfxs.longMelee)
			battleMgr.rightChr.speed = Vector2f.new(7.5, 9.86)
			reisen.action = enums.actions.ACTION_AIR_HIT_BIG_HITSTUN
			reisen:initAnimation()
			reisen.damageLimited = true
			reisen.realLimit = 100
			reisen.speed.x = -20
			fightCtr = HIT_STOP * 2
		elseif battleMgr.rightChr.actionBlockId ~= 0 or battleMgr.rightChr.frameCount > 12 then
			battleMgr.rightChr.speed.y = battleMgr.rightChr.speed.y - 0.6
		end
	end
end

local fightStages = {
	fightStage0,
	fightStage1,
	fightStage2
}

local function updateFight()
	if fightCtr ~= 0 then
		fightCtr = fightCtr - 1
		return true
	end
	keyPressed = false
	reisen.objects:update()
	reisen:updateAnimation()
	battleMgr.rightChr:updateAnimation()
	fightStages[fightStage + 1]()
	return fightStage < #fightStages
end

local function stage0()
	if ctr == 240 then
		reisen = Character.new(playerInfo)

		camera.scale = 0.8
		camera.translate.x = CAM_START_LOCATION
		camera.translate.y = 525
		camera.backgroundTranslate.x = BG_START_LOCATION

		battleMgr.leftChr.position.x = -500
		battleMgr.leftChr.action = enums.actions.ACTION_DEFAULT_SKILL1_B

		battleMgr.rightChr.position.y = 525
		battleMgr.rightChr.action = enums.actions.ACTION_FALLING
		battleMgr.rightChr:initAnimation()

		reisen.position.x = REISEN_START_LOCATION
		reisen.position.y = 525
		reisen.action = enums.actions.ACTION_FALLING
		reisen:initAnimation()

		playBgm("data/bgm/st20.ogg")
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

	updateFight()
	if ctr % 2 == 1 then
		stageBottom.position = stageBottom.position + Vector2i.new(1, 0)
	end
	ctr = ctr - 1
	if ctr == 0 then
		currentStage = currentStage + 1
	end
end

local function stage1()
	if not updateFight() then
		currentStage = currentStage + 1
		playBgm("data/bgm/ta08.ogg")
	end
end

local function stage2()
	battleMgr.leftChr:updateAnimation()
	battleMgr.rightChr:updateAnimation()
end

anims = {
	stage0,
	stage1,
	stage2
}

print("Init intro.")

stageBg.texture:loadFromFile(packPath.."\\bnbcorn1intro.png")
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
	if currentStage <= 1 and reisen then
		-- Display the CharacterManager
		reisen:draw()

		-- Display the CharacterManager subobjects
		reisen.objects:draw(-1)
		reisen.objects:draw(1)

		-- We redraw Remilia because Reisen's subobjects mess up the DirectX context and this will clean it up
		battleMgr.leftChr:draw()
	end
	if ctr ~= 0 then
		stageBg:draw()
		stageBottom:draw()
	end
	dialog:render()
end

function onKeyPressed()
	keyPressed = true
end