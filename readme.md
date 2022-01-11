# Diablo II Character Editor
Copyright (c) 2000-2003 By Burton Tsang<br>
Copyright (c) 2021-2022 By Walter Couto<br>

Diablo is a trademark or registered trademark of Blizzard Entertainment, Inc., in the U.S. and/or other countries.<br>
This utility is in no way affiliated with Blizzard Entertainment.<br>
______________________________________________

### How To Use

1. Run the D2Editor.exe file<br>
2. Open a Diablo II character file (press CTRL-O or click on File, Open) which is located in the "save" directory of where you installed Diablo II.<br>
3. Make changes (type in a value if editing level, gold, experience, etc. or use the mouse if changing title, class, or state)<br>
4. Save changes (press CTRL-S or click on File, Save)<br>
5. Play Diablo II.<br>

![alt text](https://github.com/WalterCouto/D2CE/blob/main/screenshot.png)

### Notes

You can edit the following stats:<br>
- Character class<br>
- Character name<br>
- Character state<br>
- Character title<br>
- Starting act<br>
- Level<br>
- Strength<br>
- Energy<br>
- Dexterity<br>
- Vitality<br>
- Maximum Life<br>
- Maximum Mana<br>
- Maximum Stamina<br>
- Mercenary state, class, name, Level and Experience<br>
- Skills<br>
- Quests<br>
- Waypoints<br>
- Gold in belt and stash<br>
- Experience<br>
- Upgrade Gems to perfect state<br>
- Upgrade Potions to their highest level<br>
- Upgrade Potions to Full Rejuvenation<br>
- Upgrade Durability of all weapons and armor to the max value<br>
- Upgrade Quantity of all stackable items to the max value<br>
- Convert a particular type and quality of Gem, Potion or Skull to another type or quality of Gem, Potion or Skull<br>

### Known Issues

- Tested with Diablo II Resurrected v1.1.67554 and Diablo II Classic v1.14d (It has the logic for v1.00 through v1.09 including the Lord of Destruction expansion set so it should work the same as version 1.92 of the orginal code on those files)**
- Starting with Diablo II Classic V1.13c the maximum gold in your stash no longer depends on yoru character's level, and is now a flat cap of 2,500,000 instead.  The editor will now use this value when editing Ressurrected files or Classic files marked as v1.10 or higher when determining the limit for your gold in your stash.
- You must close the character file before playing Diablo II. This is because there are no options to enable file sharing for the read and write functions I'm using in ANSI C++.

### Distribution

Please read the LICENSE file for more information regarding the license.<br>
<br>
If you have comments, suggestions, bug report, report them at https://github.com/WalterCouto/D2CE/issues<br>
<br>
If you want to Contribute, please read the [Contribution guidelines for this project](CONTRIBUTING.md)<br>
<br>
Check the following site for updates at https://github.com/WalterCouto/D2CE<br>

### Useful Links

* https://github.com/dschu012/D2SLib<br>
* https://github.com/squeek502/d2itemreader<br>
* https://github.com/nokka/d2s<br>
* https://github.com/krisives/d2s-format<br>
* https://tristram-archives.github.io/diablo2_infodump//2013/just%20hosting%20these,%20Downloaded%20from%20Internet/documentation/d2s_save_file_format_1.13d.html<br>
* https://diablo.antikrist.org/item-codes/<br>
* [d2s Binary File Format](d2s_File_Format.md)<br>

### Revision History
**Version 2.09 (Jan 8, 2022)**<br>
- Updated: Fix up logic around Act V being marked complete when the truth is the game does not mark that act complete when completing that difficulty.<br>
- Updated: Fix up Quest dialog logic to properly handle cases when quests state are modified.<br>
<br>

**Version 2.08 (Jan 6, 2022)**<br>
- Updated: logic for handling Character progressions properly. Now characer Title, Last Difficulty Played, Last Act Played, or Quest completed will be properly validated.<br>
- Updated: Quest and Waypoint dialogs now properly take into account Character progression.<br>
<br>

**Version 2.07 (Dec 6, 2021)**<br>
- Updated: json export to use jsoncpp for easier export/import logic<br>
- Updated: fix bug where Hardcore characters would be detected as dead<br>
- Updated: get proper level requirement information for Set and Unique items<br>
<br>

- Added: tests for the serialized Json export/import functionality<br>
- Added: ability to directly load and edit Json files via the File Open and File Save commands.<br>
- Added: ability to convert a JSON character file to a D2S character  file.<br>
<br>

**Version 2.06 (Nov 22, 2021)**<br>
- Added: Context menu for Gem, Potion or Skull items in Items Form<br>
- Added: Reading/Writing and Json export of the menu appearance section<br>
- Added: New Serialized Json Export compatable with D2SLib format. This format contains only the information available in the d2s file without the detailed interpretation given by the existing Json export logic. The previous JSON export is now called "Detailed" while the new JSON export is called "Serialized" in the menu.<br>
<br>

**Version 2.05 (Nov 14, 2021)**<br>
- Updated: Fix up handling of reading files with a corpse<br>
- Updated: Fix up setting/clearing the is potion bit<br>
<br>

- Added: Items Dialog to allow viewing of all items in inventory<br>
- Added: Added unit tests to cover file with corpse data<br>

**Version 2.04 (Nov 5, 2021)**<br>
- Updated: Fix up handling of reading pre 1.09 file items<br>
- Updated: Application Title will show "Diablo II: Resurrected Character Editor" unless an older Diablo II version file is loaded<br>
- Updated: Added current changes to user guide<br>
<br>

- Added: Mercenary class to encapsulate Mercenary information and items<br>
- Added: Mercenary Dialog to allow editing of Mercenary Info<br>
- Added: more unit tests to cover older file versions<br>

**Version 2.03 (Oct 2, 2021)**<br>
- Updated: Fix up handling of Simple Quest Items<br>
- Updated: Properly identify "Scroll of Inifuss" and "Key to the Cairn Stones" items<br>
- Updated: Update json unit test to ignore newline differences<br>

**Version 2.02 (Oct 1, 2021)**<br>
- Updated: Logic to account for Diablo II Ressurected Saved Games folder<br>

**Version 2.01 (Sept 25, 2021)**<br>
- Updated: Moved logic in UI down to Character class for consistency. Refactored and cleaned up code to simplify logic<br>
- Updated: Fixed bug in About window to properly show version as 3 digits.<br>
- Updated: Fixed bug in renaming character files.<br>
- Updated: Max Durability For All Items now also updates Mercenary Items.<br>
<br>

- Added: Unit test project and tests.<br>
- Added: Reading of Merc and NPC sections
- Added: logic to fetch magical properties for an item
- Added: JSON export option in File menu that produces output compatible with https://github.com/nokka/d2s
<br>

- Removed: remove long revision history prior to 2.00 as it is not relevant any longer and you can get that detail from the 1.9.6 branch
<br>

**Version 2.00 (June 18, 2021)**<br>
- Updated: Converted from a C++ Builder 5 project to a Visual Studio 2019 project<br>
- Updated: Changed Main Dialog to resemble the in-game Character Stats screen.<br>
- Updated: Changed Waypoint and Quest Dialogs to remove tabs and display all in initial screen<br>
- Updated: In the Quest dialog, changed to tri-state checkboxes so either a quest is not started, started or completed fully<br>
- Updated: Changed Skills dialog to show all the skill under their category as displayed in the game in one screen.<br>
- Updated: The logic to upgrade gems to the perfect quality to use same logic as the GPS converter code added in the 1.93 version for consistent handling of gem/potion conversions.<br>
- Updated: The editor now works with v1.10 of the character file. It now reads/writes all sections properly and also handles Corpse, Merc and Golem sections. The maximum character stats have also been adjusted to match the limits due to the file structure.<br>
- Updated: use std::error_code to hold error codes from file open or save operations.<br>
- Updated: The original Diablo II icon with transparent background is used.<br>
- Updated: Level Requirements will show requirements using the version of from the character file. V1.10 is assumed to have the latest Level Requirements from Diablo II 1.14b, while versions 1.07-1.09 will level requirements for those versions and any version below 1.07 will show the level requirements that existed since 1.00. When loading the Level Requirements dialog with no character file opened, it will assume the latest file version.<br>
- Updated: The GPS Converter Dialog is now fully called Gems, Potions & Skulls Converter as is the menu item. The logic around converting GPS has been merged across supported character file versions and the upgrade potions and gems methods. Support added for Runes.<br>
- Updated: The Adobe Acrobat PDF help file has been updated.<br>
- Updated: Modified item reading logic to be able to read v1.15 (D2R) character files.<br>
- Updated: File reading will now use read/write share mode again but cpp file has _MSC_VER guard around thus allowing the file to still be portable in theory. This means you can once again open a file already opened by the game or something else but it is still recommended you do not have it open by anything else.<br>
- Updated: namespace d2ce is now part of all non-MFC classes, i.e. the portable code. This quickly identifies what is part of UI logic and what is part of the core.<br>
- Updated: Max Everything menu item to also make all potions the best quality, all gems and skulls perfect, all stackable items fully filled and all weapons and armor have max durability.<br>
- Updated: Max Everything will not make all stats their maximum bit value and all Skill have the max value of 20. Instead it will change the stats points and skill point yet to distribute value to be what the maximum earned points would be in a game. The user can still make all skill points be 20 in the Skills dialog and can edit the stat values to be maximum if they wish to have values beyond of what is possible in a game.<br>
- Updated: Check Character now checks for more possible issues, like one of the stats having points lower than minimum expected value.<br>
<br>

- Added: Upgrades from potions of any quality to potions of highest quality as well as potions of any kind to Full Rejuvenation potions. These operations are available under the Upgrade submenu of the Options main menu.<br>
- Added: Hotkey to the View, Reload menu item (can use F5 to reload the currently opened file).<br>
- Added: Check for Dead Hardcore character added during opening a Character file allowing user mark a dead hardcore character as alive again. The "Check character" option also performs this check.<br>
- Added: MRU support so you can easily open previously opened files.<br>
- Added: New ActInfo class separates the logic of handling Quest and Act information to simplify and make less error prone other parts of the code.<br>
- Added: Max Quantity For All Stackables menu item that fully fills all stackables.<br>
- Added: Max Durability For All Items which makes all Armour and Weapons have maximum durability without being indestructible.<br>
- Added: Reset Stats menu item under Options which will do exactly what Akara does after completing The Den of Evil.<br>
<br>

- Removed: Upgrade Gems button from main form. There is now a Upgrade submenu under options which contains menu items to upgrade Gems and Potions.<br>
- Removed: "Remove New Skills" menu item was remove as that functionality is now part of the "Check Character" menu item.<br>
<br>
