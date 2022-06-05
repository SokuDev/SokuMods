--
-- Created by PinkySmile on 18/08/2021.
--

local dialogs = {
	"lcDAs a rabbit, shouldn't running be<br>a more effective strategy?",
	"rcAMy purpose is to protect the princess!",
	"lcABut isn't that strange?",
	"lcAI understand that they're helping you,<br>but why go this far for them?",
	"lSADo you remember? When we stormed in,<br>wouldn't it have been smarter to hide or run away?<br>It should have been the easier solution.",
	"rShIt seems you misunderstand,<br>I follow them by choice.",
	"rSEBesides I'm not much of a moon rabbit anymore.<br>Even though I might not be ready to consider myself an<br>earth rabbit yet, considering their leader...",
	"rSWBut aren't you in a much more singular situation than<br>mine? A human who is the personal maid of a vampire...<br>That is quite difficult to believe.",
	"lHWSeen in that way, it is a bit strange.",
	"lEWWell it seems I got carried away but now I know for<br>sure that it isn't you.",
	"lHWOn to the next suspect then.",
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