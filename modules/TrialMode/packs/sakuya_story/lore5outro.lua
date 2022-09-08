--
-- Created by yolotemperance on 06/11/2021.
--

local dialogs = {
	"Lc Something on your mind?<br>You're less fierce than I remember.",
	"RcDI don't know...",
	"RcAWhy am I still investigating?",
	"LSAWhat do you mean, Isn't it on lady Yuyuko's suggestion?",
	"RSDNo not really, but she did let me go easily this time.",
	"LEDSo isn't it fine then.",
	"REhIf she doesn't care then it shouldn't be a big deal,<br>yet I'm still here.",
	"REcThe whole thing might already be resolved<br>for all I know.",
	"LccThen why are you back again?",
	"RcEWell, to see if this incident could mean trouble...<br>why am I even here?",
	"LhEFor the same reason she does let you go I believe.",
	"RhhReally? I don't think she cares that much.",
	"LHhI do think she cares but you simply can't notice.<br>It takes time to know your master enough<br>to see those kinds of things.",
	"RHAThen how am I supposed to know now?",
	"LCAIt's simple really,<br>you just have to continue until you find out.",
	"RChI don't think that will change anything.",
	"RCHBut now that I'm here, I'll see it to the end.",
	--Youmu getting out.
	"LH They sure are complementary.",
	"LW I should get going too.",
}

local counter = 0
local dialog  = StandDialog.new(dialogs)
local pressed = false
local side = false
local ctr = 60
local flashRect = RectangleShape.new()

dialog.hidden = false
flashRect.borderColor = enums.colors.Transparent
flashRect.fillColor = enums.colors.Transparent
flashRect.size = Vector2u.new(640, 480)

function update()
	if #dialog == #dialogs - 16 and counter <= 20 and pressed then
		dialog.hidden = true 
		if counter == 0 then
			battleMgr.rightChr.action = enums.actions.ACTION_NEUTRAL_TECH
			battleMgr.rightChr:initAnimation()
		end
	end
	if #dialog <= #dialogs - 16 and battleMgr.rightChr.action ~= enums.actions.ACTION_KNOCKED_DOWN_STATIC and counter <=41 then
		counter = counter + 1
	end
	if battleMgr.rightChr.actionBlockId == 0 and battleMgr.rightChr.action == enums.actions.ACTION_NEUTRAL_TECH and counter > 20 then
		battleMgr.rightChr.action = enums.actions.ACTION_IDLE
		battleMgr.rightChr:initAnimation()
	end
	if counter == 41 and battleMgr.rightChr.action == enums.actions.ACTION_IDLE then
		dialog.hidden = false
		onKeyPressed()
	end
	if pressed and #dialog == #dialogs - 17 and counter == 42 and battleMgr.rightChr.position.x < 1500 then
		dialog:update()
		battleMgr.rightCharacterManager:updateAnimation()
		dialog.hidden = true
		battleMgr.rightChr.action = enums.actions.ACTION_FORWARD_DASH
		battleMgr.rightChr:initAnimation()
		playSfx(enums.sfxs.dash)
		battleMgr.rightChr.speed.x = 15
		counter = 0
	end
	if 	battleMgr.rightChr.action == enums.actions.ACTION_FORWARD_DASH and counter <= 42 then
		battleMgr.rightChr.direction = enums.directions.RIGHT
		battleMgr.rightChr.position.x = battleMgr.rightChr.position.x + battleMgr.rightChr.speed.x
		if battleMgr.rightChr.position.x >= 2200 and counter == 42 then
			dialog.hidden = false
			onKeyPressed()
			counter = counter + 1
		end
	end
	if #dialog < 2 then
		battleMgr.leftChr.direction = enums.directions.LEFT
	end
	if pressed and #dialog == 0 then
		dialog:update()
		battleMgr.leftCharacterManager:updateAnimation()
		if battleMgr.leftChr.action ~= enums.actions.ACTION_FORWARD_HIGH_JUMP then
			dialog.hidden = true
			battleMgr.leftChr.action = enums.actions.ACTION_FORWARD_HIGH_JUMP
			battleMgr.leftChr:initAnimation()
		else
			if battleMgr.leftChr.actionBlockId >= 1 then
				if battleMgr.leftChr.actionBlockId == 1 and battleMgr.leftChr.frameCount == 0 then
					playSfx(enums.sfxs.highJump)
					battleMgr.leftChr.speed.x = -25
					battleMgr.leftChr.speed.y = 20
				end
				battleMgr.leftChr.position = battleMgr.leftChr.position + battleMgr.leftChr.speed
				battleMgr.leftChr.speed.y = battleMgr.leftChr.speed.y - 0.7
			end
		end
		return battleMgr.leftChr.position.x > -300
	end

	local color = flashRect.fillColor
	battleMgr.rightCharacterManager:updateAnimation()
	battleMgr.leftCharacterManager:updateAnimation()
	if color.a and ctr == 0 and side then
		dialog:update()
		if pressed then
			if not dialog:onKeyPress() then
				return false
			end
			pressed = false
		end
	end
	if not side then
		color.a = color.a + 0x11;
		if color.a == 0xFF then
			side = true;
			camera.translate.x = -500
			camera.translate.y = 420
			camera.backgroundTranslate.x = 900
			camera.backgroundTranslate.y = 0

			battleMgr.leftCharacterManager.position.x = 800
			battleMgr.leftCharacterManager.position.y = 0
			battleMgr.leftCharacterManager.actionBlockId = 0
			battleMgr.leftCharacterManager.animationCounter = 0
			battleMgr.leftCharacterManager.animationSubFrame = 0
			battleMgr.leftCharacterManager.action = enums.actions.ACTION_IDLE
			battleMgr.leftCharacterManager:initAnimation()
			battleMgr.leftCharacterManager.direction = enums.directions.RIGHT

			battleMgr.rightCharacterManager.position.x = 1000
			battleMgr.rightCharacterManager.position.y = 0
			battleMgr.rightCharacterManager.animationSubFrame = 0
			battleMgr.rightCharacterManager.action = enums.actions.ACTION_KNOCKED_DOWN_STATIC
			battleMgr.rightCharacterManager:initAnimation()
			battleMgr.rightCharacterManager.direction = enums.directions.LEFT
		end
	elseif flashRect.fillColor.a ~= 0 then
		color.a = color.a - 0x11;
	elseif ctr ~= 0 then
		ctr = ctr - 1
	end
	flashRect.fillColor = color
	return true;
end

function render()
	flashRect:draw()
	if flashRect.fillColor.a == 0 and ctr == 0 and side then
		dialog:render()
	end
end

function onKeyPressed()
	if flashRect.fillColor.a and ctr == 0 and side then
		pressed = true
	end
end