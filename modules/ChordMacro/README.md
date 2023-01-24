# ChordMacro

This SWRSToys module allows you to perform sequence inputs ("haiduken"-style special moves that normally require key stroke sequences such as 236B, 621C)
by means of a simpler, more accessible input. This is intended for players who prefer Smash Bros. style gameplay, or for players with disabilities that make sequence inputs difficult.

## Usage

To perform a sequence input, hold the *chord input button* (gamepad: L/R trigger; keyboard: shift or control)
and press either B or C (keyboard default: X or C); include a directional input to select a move, or neutral for `↓↓B`/`↓↓C`.

```
shift X: ↓↓B
shift C: ↓↓C
shift→X: ↓↘→B
shift→C: ↓↘→C
shift←X: ↓↙←B
shift←C: ↓↙←C
shift↑X: →↓↘B
shift↑C: →↓↘C
shift↓X: ←↓↙B
shift↓C: ←↓↙C
```

Please note that not all characters have every chord available to them.

With gamepad, use L/R trigger instead of shift/ctrl.

## Installation

SWRSToys must first be installed.

Copy the module to the Modules/ directory so that it appears in `Modules/ChordMacro/ChordMacro.dll`.
Remember to add `ChordMacro=Modules/ChordMacro/ChordMacro.dll` to `SWRSToys.ini`.

## Limitations

Please note that to use this module on netplay mode requires both devices to have ChordMacro installed. If only one participant needs the mod, the other participant
should also install it but disable it by editing ChordMacro.ini and setting `TriggerEnabled=0` under `[GamepadInput]` and `MacroKey=-1` under `[KeyboardInput]` and `[KeyboardInputP2]`.

On gamepad, only the L/R triggers can be used to perform sequence inputs; this is not configurable.

While holding the sequence input button (default: `shift`; `L/R triggers`), the player will be unable to jump. This is so that they can press up to perform 623B/623C instead.

When used in combination with the D-pad fix module, the D-pad fix module should be loaded first.
Please place the ChordMacro module on a later line in `SWRSToys.ini` than D-pad fix.

Replays that use ChordMacro functionality will not work (unless `VirtualInput=1`, which is not recommended.)

For technical reasons, `VirtualInput=1` is not supported during Netplay.

## Virtual Input mode

By default, "Virtual Input" mode is disabled. When disabled, this module *directly edits* the game's memory in real time in order to trick the game into performing sequence inputs. However, another more obvious approach one might consider is: why not just feed artificial inputs from the keyboard into the game to perform the sequence inputs? Well, this is what `VirtualInput=1` does. However, it has some drawbacks: there is some latency after pressing the inputs, as some time is needed to feed the artificial key sequence in. Furthermore, for technical reasons, this mode doesn't work on netplay mode. It does at least allow replays to record replays correctly!

To enable Virtual Input mode -- which is not recommended, as `VirtualInput=0` provides a smoother experience -- edit `ChordMacro.ini`.