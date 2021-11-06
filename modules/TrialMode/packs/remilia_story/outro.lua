local sum = 0
local score = getPackScores()

for i = 1, 8 do
	sum = sum + score[i]
end
sum = sum / 8
if sum > 3 then
	playBGM("data/bgm/st00.ogg")
	loadBackground(packPath.."/outro2A.png")
	loadBackground(packPath.."/outro2B.png")
else
	playBGM("data/bgm/st01.ogg")
	loadBackground(packPath.."/outro1A.png")
	loadBackground(packPath.."/outro1B.png")
	addCommand("text This is outro one")
	addCommand("color FF0000 FFFFFF")
	addCommand("text This is outro one")
	addCommand("text This is outro one")
	addCommand("text This is outro one")
	addCommand("text This is outro one")
end