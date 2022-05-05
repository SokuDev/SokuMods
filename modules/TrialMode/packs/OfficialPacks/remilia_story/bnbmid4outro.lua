--
-- Created by PinkySmile on 12/08/2021.
--

local dialogs = {
	"lADNow tell me what lead you have!",
	"rADNothing in any of my books can tell what is<br>happening here.",
	"lcDAre you sure?",
	"rchNow that I think about it, there was something<br>suspicious.",
	"rcSReimu came by and she was acting...<br>Awkwardly I would say.",
	"lWhSo she is here too...",
	"rWAIt was the first time I felt like she already knew<br>what she was looking for, and not<br>just following her instinct.",
	"rWEThough it is baseless information...",
	"lhEYet it is plenty enough.",
	"lHEI shall commend you for being the first<br>one to have any kind of idea!",
	"rHCThank you...",
	--Remilia get away...
	"r AWait!",
	"r ECould you help me stand up?"
}
local dialog  = StandDialog.new(dialogs)
local pressed = false
local side = false
local ctr = 60
local finish = false
local flashRect = RectangleShape.new()

dialog.hidden = true
flashRect.borderColor = enums.colors.Transparent
flashRect.fillColor = enums.colors.Transparent
flashRect.size = Vector2u.new(640, 480)

function update()
	battleMgr.leftChr:updateAnimation()
	if pressed and #dialog == #dialogs - 11 and battleMgr.leftChr.direction ~= enums.directions.LEFT then
		dialog.hidden = true
		battleMgr.leftChr.direction = enums.directions.LEFT
		battleMgr.leftChr.action = enums.actions.ACTION_WALK_FORWARD
		battleMgr.leftChr:initAnimation()
		ctr = 20
	end
	if side and battleMgr.leftChr.direction == enums.directions.LEFT and ctr ~= 0 then
		dialog:update()
		if ctr == 1 then
			dialog.hidden = false
		end
		ctr = ctr - 1
	else
		local color = flashRect.fillColor

		if color.a and ctr == 0 and side then
			dialog:update()
			if pressed then
				if not dialog:onKeyPress() then
					dialog.hidden = true
					finish = true
				end
				pressed = false
			end
		end
		if not side then
			color.a = color.a + 0x11;
			if color.a == 0xFF then
				side = true;
				dialog.hidden = false
				camera.translate.x = -500
				camera.translate.y = 420
				camera.backgroundTranslate.x = 900
				camera.backgroundTranslate.y = 0

				battleMgr.leftChr.direction = enums.directions.RIGHT
				battleMgr.leftChr.position.x = 850
				battleMgr.leftChr.position.y = 0
				battleMgr.leftChr.actionBlockId = 0
				battleMgr.leftChr.animationCounter = 0
				battleMgr.leftChr.animationSubFrame = 0
				battleMgr.leftChr.action = enums.actions.ACTION_IDLE
				battleMgr.leftChr:initAnimation()

				battleMgr.rightChr.direction = enums.directions.LEFT
				battleMgr.rightChr.position.x = 1000
				battleMgr.rightChr.position.y = 0
				battleMgr.rightChr.animationSubFrame = 0
				battleMgr.rightChr.action = enums.actions.ACTION_KNOCKED_DOWN_STATIC
				battleMgr.rightChr:initAnimation()
			end
		elseif flashRect.fillColor.a ~= 0 then
			color.a = color.a - 0x11;
		elseif ctr ~= 0 then
			ctr = ctr - 1
		end
		flashRect.fillColor = color
	end
	if battleMgr.leftChr.direction == enums.directions.LEFT and ctr <= 10 then
		battleMgr.leftChr.position.x = battleMgr.leftChr.position.x - 4
	end
	return not finish or battleMgr.leftChr.position.x > 400
end

function render()
	flashRect:draw()
	dialog:render()
end

function onKeyPressed()
	if flashRect.fillColor.a and ctr == 0 and side then
		pressed = true
	end
end