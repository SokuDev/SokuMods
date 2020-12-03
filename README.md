# SokuMods [![builds.sr.ht status](https://builds.sr.ht/~delthas/SokuMods.svg)](https://builds.sr.ht/~delthas/SokuMods?)

A repository of all known SWRSToys and SokuEngine mods and their frameworks for Touhou Hisoutensoku (12.3).

This repository was originally created from the original SWRSToys release by *Anonymous Coward* with the following goals:
- make sure the source code for SWRSToys is easy to find
- make a repository of all released modules and their source code, including ones that were not made by Anonymous Coward
- have up-to-date headers of the latest reverse-engineered addresses from the game, updated as new people find more addresses
- set up a CI for easy, reproducible builds and deployment of the latest release of SWRSToys.

The code in this repository comes from various contributors:
- this repository was originally forked off a source archive of SWRSToys, made by *Anonymous Coward*
- *DPhoenix* made [shady-loader, shady-lua](https://github.com/enebe-nb/shady-packer), UPnPNat
- *FireSeal* made SokuRoll
- *fishshapedfish* made DPadFix
- *[Gegel85](https://github.com/Gegel85)* made DiscordIntegraton
- *Ichirin* made CharactersInForeground, GetIP, NetGameScord, PracticeEx, SokuEngine, SWRSokuRoll
- *[RhythmLunatic](https://github.com/RhythmLunatic)* made WindowedFullscreen
- *Shinki* and *PC_volt* made ReplayInputView+
- *[S-len](https://github.com/S-len)* made [SkipIntro](https://github.com/S-len/Soku-SkipIntro)
- delthas made Autopunch, as well as all the plumbing: adding CMake, CI, formatting, ...

If you have made an SWRSToys or SokuEngine module, do contact us either on GitHub or with a PM on Discord to `cc#6439` so we can add it here, with proper credits!

## Using

There are two mod packs:
- the SokuEngine pack includes all mods, a simple in-game UI to enable and disable mods
- the SWRSToys pack includes only SWRSToys mods, and is only configurable from a text file outside of the game

For now **it is recommended you use the SWRSToys pack**.

### SokuEngine

- Download the **[latest SokuEngine release](https://delthas.fr/sokuengine.zip)** and extract it to your game folder.
- Run the game as usual.
- To enable and disable mods, enter the *ModConfig* menu at the bottom of the main menu screen.
- To change the configuration of modules you enabled, modify their `.ini` configuration files (in the `modules/` folder) while the game is turned off, then restart the game.

*Any change made to a .ini file will require a restart.*

### SWRSToys

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

See [here](modules/DiscordIntegration/README.md) for details. (Needs an update.)

### DPadFix

**Map the joystick DPad and trigger buttons to game inputs.**

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

### ReplayDnD

**Drag and drop a replay file to the game executable file to watch it immediately.**

*Dragging a replay to a running game window does not work; you need to drag it to the game exacutable file in Windows Explorer.*

### ReplayInputView *(deprecated)*

*This module is now deprecated, use ReplayInputView+ instead.*

**Display keystrokes in game replays in real time.**

*The replay speed can also be changed with hotkeys: F10 speeds up; F11 slows down; F12 returns to original speed; F8 and F9 cycle through different game inputs display, for the left and right player respectively.*

*This module does not have a configuration file.*

### ReplayInputView+

**Display keystrokes in game replays in real time, as well as hitboxes, ...**

*The replay speed can also be changed with configurable hotkeys: by default F10 speeds up; F9 slows down; F11 pauses and unpauses the game; F12 steps a single frame forward; F4 toggles hitboxes display; F6 displays additional debug information; F7 cycles through different game inputs display.*

### SkipIntro

**Skip the game intro screen, automatically enter a menu or a custom practice preset on start.**

### SWRSokuRoll

**Load SokuRoll automatically.**

*This mod uses a configuration file in its module directory, not the `SokuRoll.ini` in the main game folder. This mod does not require installing SokuRoll (does not require `SokuRoll.dll` and `SokuRoll.ini`).*

### UPnPNat

**Automatically forward your ports every time the game runs.**

*This mod uses the UPnP NAT technology, which lets applications automatically request a port forwarding without user intervention. This technology is known to be disabled by default on most routers and might therefore not work for everyone.*

### WindowedFullscreen

**Make the game window fullscreen, but without stretching, and with fast Alt+Tab.**

*This module does not have a configuration file.*

### WindowResizer

**Make the game window resizable.**

## SokuEngine-exclusive mods

### GetIP

**Automatically copy a host message to your clipboard when hosting.**

### NetGameScore

**Display the current score (in won games) in network play.**

*This module is configured in-game in the ModConfig menu.*

### PracticeEx

**Greatly improve Practice mode by adding several game options and support for macros.**

*This module is configured in-game in Practice mode.*

### shady-loader - shady-lua

**Simply load custom game resources (sprites, images, sounds, music, ...) to the game.**

*This module is configured in-game in the ModConfig menu.*

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
cmake .
```

MinGW and Cygwin are not supported (`__thiscall` is needed in order to be compatible with the base game, which only the Visual C++ Build Tools supports).

After building, run the install target, which will create an `install` folder with all the built files.

## License

- Files in `include/directx/` are licensed according to their license header
- Files named `bin/discord_*`, `lib/discord_*` and those in `DiscordSDK/` are copyrighted by Discord
- Other files in `lib/` are pre-compiled archives copyrighted by Microsoft
- All other files are licensed according to the `LICENSE` file
