--
-- Created by PinkySmile on 18/08/2021.
--

local dialogs = {
    "lH So, moon rabbit, what are you all hiding?",
    "rHAI'm no moon rabbit anymore!",
    "rHEBut when I see their leaders<br>I don't think I'm an earth rabbit either...",
    "rHANonetheless I won't say anything!",
    "lcAQuite uptight aren't you?<br>How strange, a moon rabbit this loyal to Eientei.",
    "rcHWell, this place is one that I can call home now.",
    "rcWBut what about you who is human<br>yet still serves a vampire?",
    "lSWI guess it must seem strange without context huh?<br>Fine, I won't ask further questions.<br>I shouldn't have doubted you it seems.",
    "rShThank you.",
    "lHhAs an apology let me give you this piece of advice. You<br>should learn to let go. If you call this place your home,<br>you shouldn't be too on edge yet to be earth rabbit?",
    "rHEI appreciate your concern,<br>but I already have one superior that is far too relaxed.",
    "lcEYou shouldn't say that, it's important<br>to appreciate your superiors for who they are.",
    "lhEBesides, I wonder if Lady Patchouly has recovered?",
    "rhS?",
    "lWSWell I have to go, farewell future earth rabbit.",
}

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
					battleMgr.leftChr.speed.x = 20
					battleMgr.leftChr.speed.y = 20
				end
				battleMgr.leftChr.position = battleMgr.leftChr.position + battleMgr.leftChr.speed
				battleMgr.leftChr.speed.y = battleMgr.leftChr.speed.y - 0.7
			end
		end
		return battleMgr.leftChr.position.x < 1500
	end

	local color = flashRect.fillColor

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

			battleMgr.rightCharacterManager.position.x = 1000
			battleMgr.rightCharacterManager.position.y = 0
			battleMgr.rightCharacterManager.animationSubFrame = 0
			battleMgr.rightCharacterManager.action = enums.actions.ACTION_KNOCKED_DOWN_STATIC
			battleMgr.rightCharacterManager:initAnimation()

			battleMgr.leftChr.direction = enums.directions.RIGHT;
			battleMgr.rightChr.direction = enums.directions.LEFT;
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