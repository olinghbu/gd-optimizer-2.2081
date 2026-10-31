# GD Optimizer

Lowers the priority of background programs (Discord, Steam, browsers...) while you are inside a level, and optionally raises Geometry Dash to High priority. Everything is restored when you leave the level.

Use the button in the bottom menu of the main menu to turn it on or off, and the mod settings to change the list of programs.

Port to Geode v5 / GD 2.2081. Uses the Windows API directly instead of `wmic`, so it no longer causes lag when entering a level and works on Windows 11 24H2+ (where `wmic` was removed).
