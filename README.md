# SokuMods [![builds.sr.ht status](https://builds.sr.ht/~delthas/SokuMods.svg)](https://builds.sr.ht/~delthas/SokuMods?)

A repository of all known SWRSToys and SokuEngine mods and their frameworks for Touhou Hisoutensoku (12.3).

This repository was originally created from the original SWRSToys release by *Anonymous Coward* with the following goals:
- make sure the source code for SWRSToys is easy to find
- make a repository of all released modules and their source code, including ones that were not made by Anonymous Coward
- have up-to-date headers of the latest reverse-engineered addresses from the game, updated as new people find more addresses
- set up a CI for easy, reproducible builds and deployment of the latest release of SWRSToys.

The code in this repository comes from various contributors:
- this repository was originally forked off a source archive of SWRSToys, made by *Anonymous Coward*
- *[DPhoenix](https://github.com/enebe-nb)* made [shady-loader](https://github.com/enebe-nb/shady-packer), UPnPNat
- *FireSeal* made SokuRoll
- *[fishshapedfish](https://github.com/fishshapedfish)* made DPadFix, ReplayReSync
- *[PinkySmile](https://github.com/Gegel85)* made DiscordIntegraton, SokuStreaming, InfiniteDecks, TrialMode, AdvancedPraticeMode, HostInBackground
- *Ichirin* made CharactersInForeground, NetGameScore, PracticeEx, SokuEngine, SWRSokuRoll, EasyDecks
- *[RhythmLunatic](https://github.com/RhythmLunatic)* made WindowedFullscreen
- *Shinki* and *[PC_volt](https://github.com/PCvolt)* made ReplayInputView+
- *[PC_volt](https://github.com/PCvolt)* made LabTool
- *[S-len](https://github.com/S-len)* made [SkipIntro](https://github.com/S-len/Soku-SkipIntro) and InGameHostlist
- *Fear Nagae*, *[PC_volt](https://github.com/PCvolt)*, *[PinkySmile](https://github.com/Gegel85)*, *[S-len](https://github.com/S-len)* made FixMeiling236
- *[PinkySmile](https://github.com/Gegel85)* made ReDelay, based on an original version by *Nepoke*
- *mauve* made th123e (and its loader, in `th123eloader/`)
- *[delthas](https://github.com/delthas)* made Autopunch, GetIP, SokuReplays, as well as all the plumbing: adding CMake, CI, formatting, ...

If you have made an SWRSToys or SokuEngine module, do contact us either on GitHub or with a PM on Discord to `cc#6439` so we can add it here, with proper credits!

## Using

There are two mod packs:
- the SokuEngine pack includes all mods, a simple in-game UI to enable and disable mods
- the SWRSToys pack includes only SWRSToys mods, and is only configurable from a text file outside of the game

For now **it is recommended you use the SWRSToys pack**.

### SokuEngine

- Download and install the [Visual C++ 2019 x86 redistribuable](https://aka.ms/vs/16/release/vc_redist.x86.exe)
- Download the **[latest SokuEngine release](https://delthas.fr/sokuengine.zip)** and extract it to your game folder.
- Run the game as usual.
- To enable and disable mods, enter the *ModConfig* menu at the bottom of the main menu screen.
- To change the configuration of modules you enabled, modify their `.ini` configuration files (in the `modules/` folder) while the game is turned off, then restart the game.

*Any change made to a .ini file will require a restart.*

### SWRSToys (recommended)

- Download the **[latest SWRSToys release](https://delthas.fr/swrstoys.zip)** and extract it to your game folder.
- Open SWRSToys.ini in Notepad and enable some modules by deleting their `;`, then save the file.
- Modify the `.ini` configuration files of modules you enabled (in the `modules/` folder) while the game is turned off.
- Run the game as usual.

*Any change made to a .ini file will require a restart.*

## SWRSToys/Common modules

### Autopunch

**Automatically host without forwarding your ports, with any other Autopunch user.**

*This module does not have a configuration file.*

### BGMChanger

**Replace some of the game BGMs.**

*The only supported format is OGG Vorbis.*

### CharactersInForeground

**Draw characters over HP bars, for better clarity.**

*This module does not have a configuration file.*

### DiscordIntegration

**Automatically show your game status in Discord, send Soku invites and join games with one click on Discord.**

*The mod merely sends and receives IP addresses from Discord, you'll still need to forward your ports or use Autopunch.*

See [here](modules/DiscordIntegration/README.md) for details.

### DPadFix

**Map the joystick DPad and trigger buttons to game inputs.**

### EasyDecks

**Edit your decks freely during a netplay session.**

### FixMeiling236

**Fix a serious game bug regarding an underministic Meiling 236 recovery duration which could cause game desyncs.**

*This module is very lightweight; it is advised & good netplay etiquette to always enable it.*

### GetIP

**Automatically copy your IP and an optional host message to your clipboard when starting to host.**

*This replaces an old deprecated module of the same name that was SokuEngine-exclusive and buggy.*

### InGameHostlist

**Overhauls the netplay menu by adding an in-game graphical lobby/hostlist connected to the Discord host list bot.**

*The mod is connected to Konni only, not Parvati. Hopefully your server will move to Konni soon.*

### LabTool

**Enhance practice mode by adding fast character position save/reset, character state colors, and frame data info.**

<details>
  <summary>Details</summary>

#### Characters state colors

- Display of **grazing** in *transparent*, **CH** in *red*, **invulnerability** in *blue* (do not trust it for DP moves, as they are strangely hardcoded), **guard unavailable** in *purple*.
- **Position management**: 5 hardcoded positions + 1 position customizable with the save key, press 4, 1, 2, 3, 6 or 5 with the reset key. You can set your own keys in the .ini file.

#### Frame data

- **Gap time** in a blockstring displays the number of idle frames the defender comes back to between the first and second hit. If nothing is displayed, the string is tight, else, it is safely mashable by the amount displayed on the console. "Xf gap gets beaten by Xf move, and trades with X+1f move" is how you should interpret it.
- **Frame advantage**: both characters must be on ground for the result to be displayed.
- **Highjump cancel advantage**: p2 remains on the ground while p1 must hjc for the result to be displayed.
- **Skills reset** with a key customizable in the .ini, reset the levels and the skills you used to default.

</details>

### MemoryPatch

**Do some misceallenous patches to the game, such as disallowing spectating by default.**

Edit MemoryPatch.ini to enable/disable options.

Available patches:
- 16bitsColor: reduce rendering color depth from 32 bits to 16 bits
- InputFreedom: accept gamepad input even when the device is inactive
- DefaultDenyWatch: disallow spectating by default
- DefaultDenyBattle: spectate by default
- AllowMultiInstance: allow running multiple instances of the game at once
- DisableWeatherEffect: disable all weather effects
- AlwaysRandomMusic: always play random stage music regardless of in-game choice
- NativeDPadFix: use the gamepad DPad rather than the circle pad (use only if DPadFix does not work)

### NetBattleCounter

**Display the number of consecutive online matches you play, and optionally play specific sounds on consecutive games played.**

### NetBellChanger

**Change the game start bell sound for online matches.**

### NetProfileView

**Display profile name for players in a game with a specific formatting (color, font, ...) (spectating and/or playing).**

### ReDelay

**Simulate game input delay in Practice mode.**

*Make sure to read the configuration file and set your game control keys properly for the mod to work.*

### ReplayDnD

**Run replay files easily from Windows.**

*This associates .rep with Soku for easy watching in one click. You can also drag replays or folders containing replays to the th123.exe file.*

### ReplayInputView+

**Display keystrokes in game replays in real time, as well as hitboxes, ...**

*The replay speed can also be changed with configurable hotkeys: by default F10 speeds up; F9 slows down; F11 pauses and unpauses the game; F12 steps a single frame forward; F4 toggles hitboxes display; F6 displays additional debug information; F7 cycles through different game inputs display.*

*ReplayInputView+ replaces the dated ReplayInputView mod.*

### ReplayReSync

**Work around a game bug causing desyncs by forcing the game state to Moutain Vapor or Normal.**

*By default, the game is in normal state. When ending a game/spectate/replay during mountain vapor, the game goes in Mountain Vapor state and stays this way until the next game starts. When joining, spectating, or watching a replay that does not correspond to your state, you will be desynced.*

*The mod allows you to automatically resync some replays (currently, those downloaded from sokureplays) and manually set your weather state if needed.*

### shady-loader

**Simply load custom game resources (sprites, images, sounds, music, ...) to the game.**

*To add and remove mods, use the shady-manage program in `modules/shady-loader/` or use the in-game ModConfig menu if you're using SokuEngine.*

*To create mods, refer to the [official project documentation](https://github.com/enebe-nb/shady-packer/wiki/tools-loader).*

### SkipIntro

**Skip the game intro screen, automatically enter a menu or a custom practice preset on start.**

### SokuReplays

**Helps [SokuReplays](https://sokureplays.delthas.fr/) track your games, even accross IP changes.**

### SokuStreaming

**Automatically create an overlay with useful information for streaming, usable from a browser or from OBS Studio.**

*To use the mod, start playing or spectating, and in OBS Studio, add a Browser Capture at URL `http://localhost` with resolution 1920 x 1080.*

### SWRSokuRoll

**Load SokuRoll automatically.**

*This mod uses a configuration file in its module directory, not the `SokuRoll.ini` in the main game folder. This mod does not require installing SokuRoll (does not require `SokuRoll.dll` and `SokuRoll.ini`).*

### th123e

**Translate the game to English.**

*This mod translates the game from Japanese to English. It is a replacement for the legacy `th123e.exe` loader file with identical functionality.*

*This module does not have a configuration file.*

### UPnPNat

**Automatically forward your ports every time the game runs.**

*This mod uses the UPnP NAT technology, which lets applications automatically request a port forwarding without user intervention. This technology is known to be disabled by default on most routers and might therefore not work for everyone.*

### WindowedFullscreen

**Make the game window fullscreen, but without stretching, and with fast Alt+Tab.**

*This module does not have a configuration file.*

### WindowResizer

**Make the game window resizable.**

## SokuEngine-exclusive mods

### NetGameScore

**Display the current score (in won games) in network play.**

*This module is configured in-game in the ModConfig menu.*

### PracticeEx

**Greatly improve Practice mode by adding several game options and support for macros.**

*This module is configured in-game in Practice mode.*

## Making a module

- Make a new folder in `modules/`. You can copy an existing module and adapt it.
- Edit the root `CMakeLists.txt` file accordingly.
- Your module will automatically be injected.

## Building

Clone the project, including its submodules:
```
git clone --recursive https://github.com/SokuDev/SokuMods.git
```
Or, if you have already cloned without pulling the submodules, download them:
```
git submodule update --init --recursive
```

Install Visual Studio (or CMake and the Visual C++ Build Tools).

If you're running Visual Studio 2017 or newer: import the directory in Visual Studio, the project will be recognized automatically.

If you're running Visual Studio 2015 or older: you will need to install CMake and run:
```
cmake -A Win32 .
```

MinGW and Cygwin are not supported (`__thiscall` is needed in order to be compatible with the base game, which only the Visual C++ Build Tools supports).

After building, run the install target, which will create an `install` folder with all the built files.

## License

- Files in `include/directx/` are licensed according to their license header
- Files named `bin/discord_*`, `lib/discord_*` and those in `DiscordSDK/` are copyrighted by Discord
- Other files in `lib/` are pre-compiled archives copyrighted by Microsoft
- All other files are licensed according to the `LICENSE` file
