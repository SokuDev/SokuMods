--
-- Created by PinkySmile on 18/08/2021.
--

local dialogs = {
	"lcDAs a rabbit, shouldn't running be<br>a more effective strategy?",
	"rcAMy purpose is to protect the princess!",
	"lHAThen you should get stronger.",
	"rHDWhy did you attack me anyway?!",
	"lSDIt seems that I got carried away...",
	"lhDDid you go to our manor by any chance?",
	"rhANo I did not!<br>I wouldn't leave here without good reasons.",
	"lHAI see, you're a serious servant.",
	"lcAWell then it seems it was not you.<br>But here is one other suspect I can think about..."
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