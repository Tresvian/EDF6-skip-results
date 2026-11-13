# ResultsSkip

Press **Esc** on EARTH DEFENSE FORCE 6's post-mission results screen to skip the remaining item and upgrade presentation. Loot and upgrades are applied before the skip. Enter still works normally.

One DLL. No settings, logs, telemetry, or persistent data. Game scripts and saves are not modified by the plugin.

## Install

Requires [EDFModLoader](https://github.com/BlueAmulet/EDFModLoader) for EDF6.

Close EDF6 and extract the mod ZIP into the game directory. The plugin goes at `Mods/Plugins/ResultsSkip.dll`.

Start the game and press Esc during the results presentation. Hold briefly if the screen is still fading in.

To uninstall, close the game and remove `Mods/Plugins/ResultsSkip.dll`.

## Compatibility

Windows x64, Steam build **17055427**. Required `EDF.dll` SHA-256:

```
0D5092393A1B9D8F1B75B0C93D8A8F65615A36A92C46E40349C2B66C733D0916
```

The plugin silently disables itself on an unsupported build or conflicting hook site. It only skips when EDF6 is focused and its active script function is `Result_Common`. Mods that replace that routine are unsupported. Do not hot-unload the DLL.

## Build

Install Microsoft C++ Build Tools with the Windows SDK and run `build.bat`. It runs the policy tests, builds `dist/Mods/Plugins/ResultsSkip.dll`, and checks its EDF6 loader export. No third-party libraries are required.

## License

MIT. Provided as-is, without warranty; see [LICENSE](LICENSE).

Unofficial mod. EDFModLoader is by BlueAmulet; EARTH DEFENSE FORCE belongs to its respective rights holders.