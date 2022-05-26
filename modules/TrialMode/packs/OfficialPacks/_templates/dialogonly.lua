--
-- Created by PinkySmile on 07/05/2021.
--

local dialog = StandDialog.new(dialogs)
local keyPressed = false
local dialogs = {
	"r HDialog 1",
	"r EDialog 2",
	"r SDialog 3",
	"r CDialog 4",
	"r ADialog 5",
	"r DDialog 6",
	"r WDialog 7",
	"r cDialog 8",
	"r hDialog 9",
	"lH Dialog 10",
	"lE Dialog 11",
	"lS Dialog 12",
	"lC Dialog 13",
	"lA Dialog 14",
	"lD Dialog 15",
	"lW Dialog 16",
	"lc Dialog 17",
	"lh Dialog 18"
}

function update()
	dialog:update()
	battleMgr.leftChr:updateAnimation()
	battleMgr.rightChr:updateAnimation()
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
	dialog:render()
end

function onKeyPressed()
	keyPressed = true
end