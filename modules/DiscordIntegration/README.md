# HisoutensokuDiscordIntegration
Discord integration for Touhou 12.3: Hisoutensoku

## Download and installation
### Requirement
To install this module, you will need Touhou 12.3 and SWRSToys.
(If you don't have SWRSToys.ini in your hisoutensoku folder, you probably don't have it)

### Installing
- Download the module [here](https://github.com/Gegel85/HisoutensokuDiscordIntegration/releases/latest).
- Make a Modules folder inside your hisoutensoku directory (If you don't already have one).
- Create a DiscordIntegration folder inside the Modules folder.
- Copy DiscordIntegration.ini and DiscordIntegration.dll inside the DiscordIntegration you created. (If you can't find DiscordIntegration.ini inside the zip, copy the file called DiscordIntegration with the dll)
- In SWRSToys.ini (next to your th123.exe file), add the line DiscordIntegration=Modules/DiscordIntegration/DiscordIntegration.dll
- Copy discord_game_sdk.dll in your soku folder (Right next to th123.exe)
- Launch the game and share your activity on Discord !

## Using with wine
If you are running your game through wine, the game will crash on startup if you enable the mod.
To get it to work, you will need [this](https://github.com/koukuno/wine-discord-ipc-bridge). 
It will enable any program that uses the discord-rpc to work with wine and should fix the mod crashing.

## Features
- Full support for the rich presence (So the screenshots sections for examples)
- Invite and join games. You will however still need to portforward or use autopunch.
If you don't, invited people will fail to connect.

## Screenshots
- ![](https://imgur.com/y699oVg.png)
- ![](https://imgur.com/B7SBwp6.png)
- ![](https://imgur.com/N96PPMU.png)
- ![](https://imgur.com/rclVAgP.png)
- ![](https://imgur.com/CuIetGj.png)
- ![](https://imgur.com/VvXcKYq.png)
- ![](https://imgur.com/f0ZLnJO.png)
- ![](https://imgur.com/v1tk4pP.png)
- ![](https://imgur.com/PzxfOVd.png)

## INI fields
- Enabled: 0 -> Disabled, 1 -> Enabled
- ClientID: This is the client ID used to identify the game on discord.
  Changing this value will break invites and probably more. 
- InviteIp: Forces the IP to use for discord invites.
  By default the mod takes your public IP but if you use hamachi or use the invite through LAN,
  it might not work. This is made for these cases.
- RefreshTime: The time in millisecond between each presence refresh.
  The higher, the more accuracy but higher might cause performance issues.
- StringFile: The file to load the strings from. By default, it takes the JSON from the system local (if found),
  or the english one.

## Customizing the presence
The string files are JSON files containing all the rich presence properties for each part of the game.
All the menu attributes are as follow:
```JSON
"screen_name": {
    "title": "...",
    "description": "...",
    "image": "...",
    "small_image": "...",
    "image_text": "...",
    "small_image_text": "...",
    "has_timer": false,
    "has_set_length_timer": false
}
```
besides for submenus, stages and characters that are special values.
- title: Middle part of the presence. (Discord calls it state)
- description: Top part of the presence. (Discord calls it details)
- image: The image key to use. (Discord calls it large_image_key)
- small_image: The small image key to use. (Discord calls it small_image_key)
- image_text: The tooltip for the large image. (Discord calls it large_image_text)
- small_image_text: The tooltip for the small image. (Discord calls it small_image_text)
- has_timer: Show a timer showing how long you have been in this particular scene.
- has_set_length_timer: Show the time since the first game started (the start of the set).
  The timer is reset if you are neither in a character select, nor loading nor in battle.

Valid image keys are as follow:
-  reimu, marisa, sakuya, alice, patchouli, youmu, remilia, yuyuko, yukari, suika, reisen, aya, komachi, iku, tenshi, sanae, cirno, meiling, okuu, suwako, random_select
-  cover (Icon of the game)
-  stage_1 to stage_20 (In the order from character select)

In the string, you can specify parts that will be replaced with some part of the game state.
These needs to be placed to be placed between double curly brackets (For example: {{lChr}}).
This is the list of all the valid entries and what they do.
- submenu: The name of the title screen submenu, picked from the submenus array. (Only valid in title screen)
- lChrImg: Left character image key. (reimu, marisa, okuu, random_select, ....)
- lChrFullName: Left character full name, picked from the characters array.
- lChrShortName: Left character short name, picked from the characters array.
- lProfileName: Left profile name.
- lWins: The number of matchs won by the left player during this set.
- rChrImg: Right character image key. (reimu, marisa, okuu, random_select, ....)
- rChrFullName: Right character full name, picked from the characters array.
- rChrShortName: Right character short name, picked from the characters array.
- rProfileName: Right profile name.
- rWins: The number of matchs won by the Right player during this set.
- myChrImg: The player character image key (Can be right side if not hosting in netplay). (reimu, marisa, okuu, random_select, ....)
- myChrFullName: The player character full name, picked from the characters array (Can be right side if not hosting in netplay).
- myChrShortName: The player character short name, picked from the characters array (Can be right side if not hosting in netplay).
- myProfileName: The player profile name (Can be right side if not hosting in netplay).
- myWins: The number of matchs won by the player during this set (Can be right side if not hosting in netplay).
- myWr: The win/loss ratio of the player in percent.
- opChrImg: The opponent character image key (Can be left side if not hosting in netplay). (reimu, marisa, okuu, random_select, ....)
- opChrFullName: The opponent character full name, picked from the characters array (Can be left side if not hosting in netplay).
- opChrShortName: The opponent character short name, picked from the characters array (Can be left side if not hosting in netplay).
- opProfileName: The opponent profile name (Can be left side if not hosting in netplay).
- opWins: The number of matchs won by the opponent during this set (Can be left side if not hosting in netplay).
- opWr: The win/loss ratio of the opponent in percent.
- stageImg: The currently selected stage image key (stage_1, stage_2, ....)
- stageName: The currently selected stage name, picked from the stages array.

It is advised to modify an already existing file and force this file to be loaded in the INI.
It will leave less room for error and any loading error will be reported by the mod on startup if forced in the INI.

If you wish to translate the mod in your language, you can create a new file following the this convention: <localcode>.json
You can then either do a pull request on this repository or send it to PinkySmile#3506 on Discord.

## Known bugs
- Using a discord invite while the game is on the logo screen will teleport successfully to the connect screen and, on successful connection, will crash the game.
- Using a discord invite while the game before the title screen button appears (So just before and and during the introduction) will teleport successfully to the connect screen and, on successful connection, will crash the game.  

If you are annoyed by the above 2 issues, you can install the IntroSkip mod.

- A rare bug with DPadFix can cause the controllers to because unresponsive with both characters doing the same input in netplay, requiring a restart of the game to fix (Under investigation).
- A bug can occur causing the profiles in the network screen to because invisible and crash the game if selected (Under investigation).

## Build
To build the project you will need, the MSVC compiler, [discord_game_sdk](https://discord.com/developers/docs/game-sdk/sdk-starter-guide#step-1-get-the-thing) library and cmake.
- Clone the repository
- Make a build folder inside
- Open a powershell window inside the build folder
- `cmake .. -DCMAKE_BUILD_TYPE=Release -DDiscordSDK_DIR=C:/path_to_the_game_sdk/lib/x86`
- `cmake --build . --target DiscordIntegration`   
Warning: If you plan to use the dll you built yourself,
take the discord_game_sdk.dll provided in the game sdk library you downloaded (C:/path_to_the_game_sdk/lib/x86/discord_game_sdk.dll)

## Special thanks
Thanks to [Slen](https://github.com/S-len), [PC_volt](https://github.com/PCvolt) and [cc](https://github.com/delthas) for their help with this project.
