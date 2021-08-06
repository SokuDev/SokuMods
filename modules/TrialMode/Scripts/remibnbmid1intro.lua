print("Init intro.")

local rect = RectangleShape()
local stageBg = Sprite()
local stageBottom = Sprite()
local ctr = 240
local currentStage = 1
local dialogIntro = StandDialog(
	"lc Another boring day. Maybe I'll go duel Reimu<br>tonight? If I destroy her shrine, she<br>might get motivated a bit.",
	--Clic
	"rh*That should make a good cover. As for the title...*",
	"lWOh my, an intruder. You are on time!",
	"rHDon't mind me. I'm not here to invade your<br>privacy or anything.<br>Just here to take a few picture in secret...",
	"rEWell so long for the secret part...",
	"lWPatchouli told me she needs a new tengu feather,<br>might as well help her out!",
	"lHI'm sure even the vegetative Patchouli<br>shall be grateful.",
	"rDMy beautiful feather...",
	"lCAnyway what were you doing here?",
	"rhI was investigating a rumor that..",
	"lASurely You are too tough to talk aren't you?",
	"rANo it's not a se...",
	"lhRoughing you up shall make you talk."
)
local dialogOutro = StandDialog(
	"rWDWhy being a journalist is so hard?",
	"lWI was still pretty soft you know?",
	"lCNow talk!",
	"rDI was just investigating the rumor that someone<br>had infiltrated the scarlet manor.",
	"lcWell it's not anything new...",
	"rDI meant neither marisa nor me.",
	"lSNow that is surprising!<br>Sakuya must know something..."
)
local dialogHidden = true
local keyPressed = false


stageBg.texture.loadFromFile(packPath.."\\bnbmid1intro.png")
stageBg.size = stageBg.texture.size
stageBg.position = {
	320 - stageBg.texture.size.x / 2,
	-50
}
stageBg.rect.width  = stageBg.texture.size.x
stageBg.rect.height = stageBg.texture.size.y
stageBg.tint.a = 0

stageBottom.texture.loadFromGame("data/scenario/effect/Stage1.png")
stageBottom.size = stageBottom.texture.size
stageBottom.position = {
	320 - stageBg.texture.size.x / 2,
	-50 + stageBg.texture.size.y
}
stageBottom.rect.width  = stageBottom.texture.size.x
stageBottom.rect.height = stageBottom.texture.size.y
stageBottom.tint.a = 0

rect.size = {640, 480}
rect.borderColor = enums.colors.Transparent
rect.borderColor = enums.colors.Transparent


function stage0()
	if ctr == 240 then
		playBGM("data/bgm/ta01.ogg")
	end
	camera.translate.y = 820
	camera.backgroundTranslate.y = -400
	if ctr < 60 then
		if stageBg.tint.a then
			stageBg.tint.a = stageBg.tint.a - 0xF
		end
		if stageBottom.tint.a then
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
		stageBottom.position = stageBottom.position + {1, 0}
	end
	ctr = ctr - 1
	if ctr == 0 then
		currentStage++
		battleMgr.leftChr.position.x = 400
		battleMgr.leftChr.actionBlockId = 0
		battleMgr.leftChr.animationCounter = 0
		battleMgr.leftChr.animationSubFrame = 0
		battleMgr.leftChr.action = enums.actions.ACTION_WALK_FORWARD
		battleMgr.leftChr:initAnimation()
		battleMgr.rightChr.position.x = 1200
		ctr = 3
	end
end

function stage1()
	battleMgr.leftChr:updateAnimation()
	if camera.backgroundTranslate.y then
		camera.backgroundTranslate.y = camera.backgroundTranslate.y + 1
		camera.translate.y = camera.translate.y - 1
	end
	battleMgr.leftChr.position.x = battleMgr.leftChr.position.x + 5 * battleMgr.leftChr.direction
	if battleMgr.leftChr.direction == enums.directions.RIGHT and battleMgr.leftChr.position.x > 800 then
		battleMgr.leftChr.direction = enums.directions.LEFT
	elseif battleMgr.leftChr.direction == enums.directions.LEFT and battleMgr.leftChr.position.x < 400 then
		battleMgr.leftChr.direction = enums.directions.RIGHT
		ctr = ctr - 1
	end
	if ctr == 0 then
		battleMgr.leftChr.action = enums.action.ACTION_IDLE
		battleMgr.leftChr:initAnimation()
		currentStage = currentStage + 1
		dialogHidden = false
		introDialog.onKeyPress()
	end
end

function stage2()
	battleMgr.leftChr:updateAnimation()
	if #introDialog == #dialogIntro - 2 then
		currentStage = currentStage + 1
		dialogHidden = true
		battleMgr.rightChr.position.x = 800
		battleMgr.rightChr.actionBlockId = 0
		battleMgr.rightChr.animationCounter = 0
		battleMgr.rightChr.animationSubFrame = 0
		battleMgr.rightChr.action = enums.action.ACTION_IDLE
		battleMgr.rightChr:initAnimation()
		battleMgr.leftChr.actionBlockId = 0
		battleMgr.leftChr.animationCounter = 0
		battleMgr.leftChr.animationSubFrame = 0
		battleMgr.leftChr.action = enums.action.ACTION_STAND_GROUND_HIT_HUGE_HITSTUN
		battleMgr.leftChr:initAnimation()
		rect.fillColor = enums.action.colors.White
		playSFX(enums.sfx.mishagujiSamaHitSound)
	end
end

function stage3()
	local color = rect.fillColor

	battleMgr.rightChr:updateAnimation()
	if color.a > 1 then
		color.a -= 0x2
		rect.fillColor = color
	else
		rect.fillColor = enums.colors.Transparent
		currentStage = currentStage + !
		battleMgr.leftChr.actionBlockId = 0
		battleMgr.leftChr.animationCounter = 0
		battleMgr.leftChr.animationSubFrame = 0
		battleMgr.leftChr.action = enums.actions.ACTION_IDLE
		battleMgr.leftChr:initAnimation()
		ctr = 12
	end
end

function stage4()
	battleMgr.leftChr:updateAnimation()
	battleMgr.rightChr:updateAnimation()
	if ctr then
		ctr = ctr - 1
	else
		dialogHidden = false
	end
	if #introDialog == #dialogs - 14 then
		currentStage = currentStage + 1
	end
end

local anims = {
	stage0,
	stage1,
	stage2,
	stage3,
	stage4
}

function update()
	if not dialogHidden then
		introDialog:update()
		if keyPressed then
			if not introDialog:onKeyPress() then
				return false
			end
			keyPressed = false
		end
	end
	if currentStage > #anims then
		return false
	end
	anims[currentStage]()
	return currentStage <= #anims
end

function render()
	if ctr != 0 then
		stageBg:draw()
		stageBottom:draw()
	end
	rect:draw()
	if not dialogHidden then
		introDialog:render()
	end
end

function onKeyPressed()
	if not dialogHidden then
		keyPressed = true
	end
end