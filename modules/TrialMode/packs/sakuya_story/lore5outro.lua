--
-- Created by yolotemperance on 06/11/2021.
--

local dialogs = {
"LcDWhat happened, you seemed quite troubled?",
"RcDWhy does lady Yuyuko give me a mission like this,<br>she implies that it is important,<br>yet she gives no clue and simply goes away.",
"LSDShe is quite lazy after all,<br>and she does seem to enjoy seeing you try.",
"RSAHow am I supposed to serve her,<br>when she never says what she truly wants?",
"LWAThough Your master is quite lazy, but she's not dumb,<br>I'm sure she has a good reason<br>for sending you on those missions.",
"LHAAs a servant you should always support your master.<br>Even if she is not perfect and her orders are not the best,<br>be confident that she will better herself in failure. ",
"LHAAnd if she continues,<br>it may simply be how she is and what she wants.",
"RHh...",
"LhhWhat I'm trying to say is,<br>you should believe in Yuyuko's decisions.",
"RhWI see, it seems it is my mind that needs some sharpening.",
"RhH I'll go investigate then.",
"LH They sure are complementary.",
"LW I should get going too."
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
	if #dialog == #dialogs - 9 and counter <= 20 and pressed then
		dialog.hidden = true 
		if counter == 0 then
			battleMgr.rightChr.action = enums.actions.ACTION_NEUTRAL_TECH
			battleMgr.rightChr:initAnimation()
		end
	end
	if #dialog <= #dialogs - 9 and battleMgr.rightChr.action ~= enums.actions.ACTION_KNOCKED_DOWN_STATIC and counter <=41 then
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
	if pressed and #dialog == #dialogs - 11 and counter == 42 and battleMgr.rightChr.position.x < 1500 then
		dialog:update()
		battleMgr.rightCharacterManager:updateAnimation()
		dialog.hidden = true
		battleMgr.rightChr.action = enums.actions.ACTION_FORWARD_DASH
		battleMgr.rightChr:initAnimation()
		playSfx(enums.sfxs.highJump)
		battleMgr.rightChr.speed.x = 12
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