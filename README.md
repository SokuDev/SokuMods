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
