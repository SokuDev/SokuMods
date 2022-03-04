-- Modpack created by PinkySmile
--
-- Template Created by DaBomb
--
-- The characters and stages are pulled from your .json files that describe your combo. To add this completed animation to your combo, 
-- add      "intro": "your_filename_here.lua",     for intros and          "outro": "your_filename_here.lua",         for outros.
-- These are added above a field that says   "type":   
--
-- Example format:
-- "intro": "your_filename_here.lua",
-- "outro": "your_other_filename_here.lua",
-- "type": "combo",
--
--
-- Dialogue Instructions:
-- 	The first three characters after the " mark who is speaking.
--	The first character is either l or r. This marks whether the character on the left or right is talking.
--	The second character is the character's sprite. The sprite codes are pinned in the TrialMode community server, in the #ask-help channel.
--	The third character is the other character's sprite. Putting a space in this field will not add any sprite for the other character.
--	For example, the dialogue below shows the left character's happy sprite and no sprite for the right character. "rH " would show no sprite for the left character and a happy sprite for the right character.
--	For no sprites at all, use "l  " or "r  ". The two spaces in the sprite fields means both sprites are not included. 
--	To add a line, put    "",    in a new line and add your dialogue between the quotes. <br> adds a line break.

local dialogs = {
	"lt During MPP there are a lot of AUB options available to you <br>5a 5c and j5a are all AUB",
	"lH Make sure to use them to catch jumping opponents!",
	"l  Note:Even though the jump hitbox of MPP is not AUB <br>it will always lead into j5a on block."
	--Battle here
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
--	Adjusting the camera and character positions is not required. You can play with the values to align the cutscene with your combo's starting position if you wish.
-- 		The next 4 fields point the camera at the scene. The following values point the camera at the left corner. Changing the y values is not recommended. Negative values are possible if needed.
--		The current values place the characters and camera similarly to a normal round start.
			camera.translate.x = -300
			camera.translate.y = 420
			camera.backgroundTranslate.x = 640
			camera.backgroundTranslate.y = 0

--		Use the position.x fields to change where the characters are standing.
			battleMgr.leftCharacterManager.position.x = 420
			battleMgr.leftCharacterManager.position.y = 0
			battleMgr.leftCharacterManager.actionBlockId = 0
			battleMgr.leftCharacterManager.animationCounter = 0
			battleMgr.leftCharacterManager.animationSubFrame = 0
			battleMgr.leftCharacterManager.action = enums.actions.ACTION_IDLE
			battleMgr.leftCharacterManager:initAnimation()

			battleMgr.rightCharacterManager.position.x = 800
			battleMgr.rightCharacterManager.position.y = 0
			battleMgr.rightCharacterManager.animationSubFrame = 0
			battleMgr.rightCharacterManager.action = enums.actions.ACTION_IDLE
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