--
-- Created by PinkySmile on 12/08/2021.
--

local dialogs = {
	--Sakuya coming in.
	--Patchouly already there doing something? (reading books or whatever) and noticing Sakuya
	"r SFinally here?",
	"lWSWere you waiting for me?",
	"rWCI knew you would want to investigate. Though, I<br>must say I do not know much if anything at all.",
	"rWWThe only thing I do know is that I felt presence<br>earlier today. But it was not a clear one, and I<br>can say for sure it's not here anymore.",
	"lHWI see... Well there is something I must give you<br>from the mistress.",
	"rHSA tengu feather? Strange...",
	"lWSI believe it has something to do with<br>the current situation.",
	"lcSAlso it is highly possible that the<br>mistress will come here for answers. Could you<br>not talk to her about the presence you felt?",
	"rcWI don't intend to hide anything from her.",
	"lhWI insist that you do. If she knew<br>she would realize that this mystery cannot be<br>resolved while staying inside.",
	"rhAI repeat what I've said. I won't hide anything from<br>her. She's not a child anymore...",
	"rhEWell... Actually, she is. But you know what I mean!",
	"lCEThen, forgive me.",
	--Sakuya attacking Patchouli,
	"r ASakuya... What are you doing?",
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
		playBGM("data/bgm/ta05.ogg")
		battleMgr.leftChr.position.x = -100
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
		ctr = 10
	end
end

local function stage1()
	battleMgr.leftChr:updateAnimation()
	battleMgr.leftChr.position.x = battleMgr.leftChr.position.x + 5
	if battleMgr.leftChr.position.x >= 400 then
		currentStage = currentStage + 1
		dialog.hidden = false
		ctr = 30
		battleMgr.leftChr.action = enums.actions.ACTION_IDLE
		battleMgr.leftChr:initAnimation()
		battleMgr.rightChr.action = enums.actions.ACTION_STANDING_UP
		battleMgr.rightChr:initAnimation()
	end
end

local function stage2()
	battleMgr.leftChr:updateAnimation()
	battleMgr.rightChr:updateAnimation()
	if battleMgr.rightChr.action == enums.actions.ACTION_STANDING_UP and battleMgr.rightChr.animationCounter == 2 and battleMgr.rightChr.animationSubFrame == 4 then
		battleMgr.rightChr.action = enums.actions.ACTION_IDLE
		battleMgr.rightChr:initAnimation()
		currentStage = currentStage + 1
	end
end

local function stage3()
	battleMgr.leftChr:updateAnimation()
	battleMgr.rightChr:updateAnimation()
	if #dialog == 1 and keyPressed then
		currentStage = currentStage + 1
		dialog.hidden = true
		ctr = 30
	end
end

local function stage4()
	battleMgr.leftChr:updateAnimation()
	battleMgr.rightChr:updateAnimation()
	battleMgr.leftChr.position.x  = battleMgr.leftChr.position.x  + battleMgr.leftChr.speed.x
	ctr = ctr - 1
	if ctr == 5 then
		battleMgr.leftChr.action = enums.actions.ACTION_66B
		battleMgr.leftChr:initAnimation()
		battleMgr.leftChr.speed.x = 12.5
	end
	if ctr == 0 then
		battleMgr.rightChr.action = enums.actions.ACTION_DEFAULT_SKILL3_B
		battleMgr.rightChr:initAnimation()
		currentStage = currentStage + 1
	end
end

local function stage5()
	battleMgr.leftChr:updateAnimation()
	---battleMgr.rightChr:updateAnimation()
	battleMgr.rightChr:updateMove()
	battleMgr.rightChr.objects:update()
	if battleMgr.leftChr.frameCount == 8 then
		playSfx(enums.sfxs.longMelee)
	elseif battleMgr.leftChr.frameCount > 16 then
		if battleMgr.leftChr.frameCount == 51 then
			battleMgr.leftChr.action = enums.actions.ACTION_IDLE
			battleMgr.leftChr:initAnimation()
		end
		if battleMgr.leftChr.speed.x ~= 0 then
			battleMgr.leftChr.speed.x = battleMgr.leftChr.speed.x - 0.5
		end
	end

	if battleMgr.rightChr.actionBlockId >= 1 then
		ctr = ctr + 1
		if battleMgr.rightChr.actionBlockId == 4 then
			ctr = ctr + 1
			if battleMgr.rightChr.frameCount == 5 then
				battleMgr.rightChr.action = enums.actions.ACTION_IDLE
				battleMgr.rightChr:initAnimation()
				currentStage = currentStage + 1
				ctr = 0
			end
		end
	end
	battleMgr.leftChr.position.x  = battleMgr.leftChr.position.x  + battleMgr.leftChr.speed.x
	battleMgr.leftChr.position.x  = battleMgr.leftChr.position.x  - ((ctr < 10 and ctr * 3 or ctr < 30 and ctr or ctr < 40 and (40 - ctr) * 3 or 0) / 2.5)
	battleMgr.rightChr.position.y = battleMgr.rightChr.position.y + battleMgr.rightChr.speed.y
end

local function stage6()
	battleMgr.leftChr:updateAnimation()
	battleMgr.rightChr:updateAnimation()
	battleMgr.rightChr.objects:update()
	ctr = ctr + 1
	if ctr == 30 then
		dialog.hidden = false
		keyPressed = false
		dialog:onKeyPress()
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

stageBg.texture:loadFromFile(packPath.."/lore3intro.png")
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