# Diablo II Character Editor
Copyright (c) 2000-2003 By Burton Tsang<br>
Copyright (c) 2021 By Walter Couto<br>

Diablo is a trademark or registered trademark of Blizzard Entertainment, Inc., in the U.S. and/or other countries.<br>
This utility is in no way affiliated with Blizzard Entertainment.<br>
______________________________________________

### How To Use

1. Run the D2Editor.exe file<br>
2. Open a Diablo II character file (press CTRL-O or click on File, Open) which is located in the "save" directory of where you installed Diablo II.<br>
3. Make changes (type in a value if editing level, gold, experience, etc. or use the mouse if changing title, class, or state)<br>
4. Save changes (press CTRL-S or click on File, Save)<br>
5. Play Diablo II.<br>

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
- Skills<br>
- Quests<br>
- Waypoints<br>
- Gold in belt and stash<br>
- Experience<br>
- Upgrade Gems to perfect state<br>
- Upgrade Potions to their highest level<br>
- Upgrade Potions to Full Rejuvenation<br>
- Upgrade Durablity of all weopons and armor to the max value<br>
- Upgrade Quantity of all stackable items to the max value<br>
- Convert a particular type and quality of Gem, Potion or Skull to another type or quality of Gem, Potion or Skull<br>

### Known Issues

- Tested with Diablo II V1.14d (It has the logic for V1.00 through V1.09 including the Lord of Destruction expansion set so it should work the same as version 1.92 on those files)**
- Starting with Diablo II V1.13c the maximum gold in your stash no longer depends on yoru character's level, and is now a flat cap of 2,500,000 instead.  The editor will now use this value when editing files marked as v1.10 or higher when determining the limit for your gold in your stash.
- You must close the character file before playing Diablo II. This is because there are no options to enable file sharing for the read and write functions I'm using in ANSI C++.

### Distribution

Please read the LICENSE file for more information regarding the license.<br>
<br>
If you have comments, suggestions, bugs reports, report them at https://github.com/WalterCouto/D2CE/issues<br>
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

### Revision History
**Version 2.00 (June 18, 2021)**<br>
- Updated: Converted from a C++ Builder 5 project to a Visual Studio 2019 project<br>
- Updated: Changed Main Dialog to resemble the in-game Character Stats screen.<br>
- Updated: Changed Waypoint and Quest Dialogs to remove tabs and display all in initial screen<br>
- Updated: In the Quest dialog, changed to tri-state checkboxes so either a quest is not started, started or completed fully<br>
- Updated: Changed Skills dialog to show all the skill under their category as displayed in the game in one screen.<br>
- Updated: The logic to upgrade gems to the perfect quality to use same logic as the GPS converter code added in the 1.93 version for consistent handling of gem/potion conversions.<br>
- Updated: The editor now works with v1.10 of the character file. It now reads/writes all sections properly and also handles Corpse, Merc and Golem sections. The maximum character stats have also been adjusted to match the limits due to the file structure.<br>
- Updated: use std::error_code to hold error codes from file open or save operations.<br>
- Updated: The original Diablo II icon with transparant backgroud is used.<br>
- Updated: Level Requirements will show requirments using the version of from the character file. V1.10 is assumed to have the latest Level Requirements from Diable II 1.14b, while versions 1.07-1.09 will level requirements for those versions and any version below 1.07 will show the level requirements tha existed since 1.00. When loading the Level Requirements dialog with no character file opened, it will assume the latest file version.<br>
- Updated: The GPS Converter Dialog is now fuly called Gems, Potions & Skulls Converter as is the menu item. The logic around converting GPS has been merged across supported character file versions and the upgrade potions and gems methods. Support added for Runes.<br>
- Updated: The Adobe Acrobat PDF help file has been updated.<br>
- Updated: Modfied item reading logic to be able to read v1.15 (D2R) character files.<br>
- Updated: File reading will now use read/write share mode again but cpp file has _MSC_VER guard around thus allowing the file to still be portable in theory. This means you can once again open a file already opened by the game or something else but it is still recommended you do not have it open by anything else.<br>
- Updated: namespace d2ce is now part of all non-MFC classes, i.e. the portable code. This quickly identifies what is part of UI logic and what is part of the core.<br>
- Updated: Max Everything menu item to also make all potions the best quality, all gems and skulls perfect, all stackable items fully filled and all weapons and armor have max durability.<br>
- Updated: Max Everything will not make all stats their maximum bit value and all Skill have the max valu eof 20. Instead it will change the stats points and skill point yet to distribute value to be what the maximum earned points would be in a game. The user can still make all skill points be 20 in the Skills dialog and can edit the stat values to be maximum if they wish to have values beyond of what is possible in a game.<br>
- Updated: Check Character now checks for more possible issues, like one of the stats having points lower then minimum expected value.<br>
<br>

- Added: Upgrades from potions of any quality to potions of highest quality as well as potions of any kind to Full Rejuvenation potions. These operations are available under the Upgrade submenu of the Options main menu.<br>
- Added: Hotkey to the View, Reload menu item (can use F5 to reload the currently opened file).<br>
- Added: Check for Dead Hardcore character added during opeing a Character file allowing user mark a dead hardcore character as alive again. The "Check character" option also performs this check.<br>
- Added: MRU support so you can easily open previously opened files.<br>
- Added: New ActInfo class seperates the logic of handling Quest and Act information to simplify and make less error prone other parts of the code.<br>
- Added: Max Quanity For All Stackables menu item that fully fills all stackables.<br>
- Added: Max Durability For All Items which makes all Armour and Weapons have maximum durablity without being indestructable.<br>
- Added: Reset Stats menu item under Options which will do exactly what Akara does after completing The Den of Evil.<br>
<br>

- Removed: Upgrade Gems button from main form. There is now a Upgrade submenu under options which contains menu items to upgrade Gems and Potions.<br>
- Removed: "Remove New Skills" menu item was remove as that functionality is now part of the "Check Character" menu item.<br>
<br>

**Version 1.94 (April 17, 2003)**<br>
- Added: You can now convert potions to gems/skulls and vice versa. (Note: The Gem converter is now called the GPS converter. GPS stands for<br> Gems, Potions, Skulls.<br>
<br>

- Fixed: Gems could not be converted to regular skulls.<br>
<br>

**Version 1.93 (March 30, 2003)**<br>
- Added: Option to convert gems from one type and condition to another.<br>
<br>

- Updated: The Adobe Acrobat PDF help file has been updated and renamed to UserGuide.pdf.<br>
<br>

**Version 1.92 (March 15, 2003)**<br>
- Added: Titles for hardcore and expansion characters.  They were not in prior versions.<br>
<br>

**Version 1.91 (January 10, 2003)**<br>
- Added: Some functions to the Character and Item classes that may be useful for other programmers.<br>
<br>

- Removed: Non-portable code that was added in v1.90<br>
<br>

**Version 1.90 (January 9, 2003)**<br>
- Fixed: Web address in About box and README.TXT was incorrect<br>
<br>

- Removed: Refresh action has been removed since it can't be used with the ANSI C++ code as mentioned in the "Known Issues" section above.<br>
<br>

**Version 1.89 (January 3, 2003)**<br>
- Fixed: Quests not reading properly.<br>
<br>

- Added: Editor will notify you if it cannot open a character file. (Make sure the file isn't in use by another application.)<br>
<br>

**Version 1.88 (April 3, 2002)**<br>
- Fixed: Sometimes non-perfect gems were not detected correctly.<br>
<br>

- Removed: Second URL link in the About box.<br>
<br>

**Version 1.87 (February 12, 2002)**<br>
- Fixed: A minor bug which caused character stats to not read correctly.<br>
<br>

**Version 1.861 (November 21, 2001)**<br>
- Fixed: A rare bug where certain character names may cause the editor to not display the correct stats when the character is first loaded.<br>
<br>

**Version 1.86 (November 7, 2001)**<br>
- Updated: Checksum calculation code has been reworked to be more portable and is also easier to understand than before.<br>
<br>

- Removed: Redundant code that was not used in the earlier 1.86 version.<br>
<br>

**Version 1.86 (November 5, 2001)**<br>
- Updated: The editor now works with Diablo II v1.09.  Thanks goes to Stoned2000 for making his checksum calculation source available to the public.<br>
<br>

**Version 1.85 (October 13, 2001)**<br>
- New: Editor is now open source and this is the last release. The main portion of the code has been converted to ANSI C++ (with the binary file read/write code in ANSI C) for portability. The front end (GUI) was created with Borland's C++ Builder 5.<br>
<br>

**Version 1.84 (August 17, 2001)**<br>
- Fixed: Flawless amethyst not upgraded by Upgrade Gems feature.<br>
<br>

**Version 1.83 (July 20, 2001)**<br>
- Fixed: Upgrading gems sometimes messed up the characters.<br>
<br>

**Version 1.82 (July 17, 2001)**<br>
- Fixed: Quest values were not properly updated when using the Quest form for the first time, changing quest values and then switching between acts or closing the form.<br>
<br>

**Version 1.81 (July 15, 2001)**<br>
- Fixed: Quests weren't updating correctly on the form.<br>
<br>

- Updated: Two more reward options added to Act 5.<br>
- Updated: Two more character titles were added to account for the Nightmare and Hell difficulty levels.<br>
<br>

**Version 1.8 (July 14, 2001)**<br>
- Updated: The editor has been updated to fully support the new Lord of Destruction (LOD) expansion set.  You will only see the corresponding LOD options (i.e. Act 5 classes, waypoints, etc.) if the character being edited is an expansion set character.<br>
<br>

- Added: Batch option for the various forms in the editor.
<br>

- Changed: Level Requirements form can now be viewed at the same time you edit your character.<br>
<br>

**Version 1.7 (June 30, 2001)**<br>
- Updated: The editor now works with Diablo II v1.07+ including the Lord of Destruction expansion pack.  The editor has not been updated to support the new classes and their skills or Act 5 as of yet but those will be in the next version.<br>
<br>

- Fixed: A bug where decreasing your character's level (don't know why you would want to) did not reduce the experience accordingly.<br>
<br>

**Version 1.6f (May 14, 2001)**<br>
- Changed: Quests dialog code have been reworked again.  Hopefully, it should work better now.<br>
<br>

- Fixed: A bug where changing the name of your character did not rename the corresponding character files.<br>
<br>

**Version 1.6e (April 27, 2001)**<br>
- Fixed: A bug where if you changed any quest settings, the program didn't let you save.<br>
<br>

**Version 1.6d (April 13, 2001)**<br>
- Added: Option to enable/disable an act manually.  Before, the next act would be enabled based on whether you completed the last quest for the previous act.<br>
<br>

- Changed: Quests code has been reworked which results in a smaller executable than the previous version.<br>
<br>

- Fixed: A bug where if you had an edited character and opened a new file without first saving the current one, and selected Yes, the stats would overwrite the newly opened file.<br>
<br>

**Version 1.6c (March 8, 2001)**<br>
- Fixed: The experience would always reset to experience requirement value based on your characters level.  Current life, mana, and stamina values would be changed to match their maximum values, respectively.<br>
<br>

- Changed: Updated the PDF help file a bit.<br>
<br>

**Version 1.6b (January 10, 2001)**<br>
- Fixed: Current life, mana, or stamina values did not change when their respective maximum values changed.<br>
<br>

**Version 1.6a (January 8, 2001)**<br>
- Changed: Upgrade Gems option now works with Diablo II v1.04.<br>
<br>

- Fixed: Could not tab to the current life, mana, and stamina edit boxes.<br>
<br>

**Version 1.6 (January 7, 2001)**<br>
- Updated: The editor now works with the latest version -- v1.04.<br>
<br>

- Changed: Backup option is now default to on.<br>
- Changed: Character file you are editing is also displayed on the title bar.<br>
- Changed: Can now edit current life, mana, stamina.  If current life, mana, or stamina is less than maximum life, mana, or stamina, they will be upgraded to match their respective maximum values.<br>
- Changed: Maximum values for life, mana, and stamina have been increased to 5000.<br>
- Changed: When using v1.04, Upgrade Gems is not available.  I will fix this as soon as possible.<br>
- Changed: A few changes have been made to the About box.<br>
<br>

**Version 1.5f (October 16, 2000)**<br>
- Changed: Updated the web link in the About box to reflect the new home page<br>
<br>

**Version 1.5e (October 9, 2000)**<br>
- Changed: Fixed a bug where save option was not activated when you edited your character's waypoints twice.<br>
- Changed: Updated the D2Help.pdf file.<br>
<br>

**Version 1.5d (September 4, 2000)**<br>
- Changed: Fixed a display problem for the Imbue option.  Act 1, Quest 5 would not display "Imbue" even though you can imbue in the game.<br>
<br>

**Version 1.5c (September 1, 2000)**<br>
- Changed: Item bug has been fixed.  All items are now recorded correctly.<br>
- Changed: Improved quest status detection (hopefully).<br>
- Changed: Made web page and email addresses clickable links in the About box and increased the font size for readability.<br>
<br>

**Version 1.5b (August 26, 2000)**<br>
- Changed: You can now edit experience and gold without having to acquire some before editing.<br>
- Changed: The editor will now start in your Diablo II save directory if Diablo II is installed correctly (i.e. game information is stored in the registry).  If Diablo II is not installed or there is no registry entry for Diablo II then the editor will default to the current directory it is located in.<br>
- Changed: Changed Act 1, Quest 5 status information to show as completed after you have used your imbue but did not press Q twice.<br>
- Changed: Renamed the "Edit Skill Trees" button to "Skill Trees".<br>
- Changed: More thorough error checking.<br>
- Changed: Updated the screenshots in the D2Help.pdf file<br>
<br>

**Version 1.5a (August 15, 2000)**<br>
- Changed: Fixed act statuses not being saved if you select to complete all quests for an act.  Town waypoints will also be activated for any of the acts in which a quest is set to "Started/In Progress" or "Completed".<br>
- Changed: Upgrade Gems button will only be enabled if your character has any non-perfect gems.  If your character has no gems or all gems are perfect then this button will not be enabled.<br>
- Changed: Removed some redundant code and improved memory usage.<br>
<br>

**Version 1.5 (August 12, 2000)**<br>
- Added: Option to change quest status.  You can turn on the imbue option by going to Act 1, Quest 5, and selecting the Imbue status. **Note:**  If quest status is not correctly displayed, please email me your character file and state what quests you have completed so I may write down the values.<br>
- Added: Upgrade Gems option.  Upgrades all gems to their perfect state.<br>
<br>

- Changed: Fixed incorrect values for Level Information form and improved the layout.<br>
- Changed: Moved the Refresh option to the View menu.<br>
- Changed: Character status has been updated more accurately.<br>
- Changed: D2Help.pdf has been updated.<br>
<br>

**Version 1.4 (July 31, 2000)**<br>
- Added: Option to activate/deactivate waypoints.  You will only be able to access waypoints for later acts if you have finished the required previous acts.<br>
- Added: Level Information Option under View menu.  Displays experience required and max gold limits for given level.<br>
- Added: Restore character option.  This will allow you to restore a character from a previous backup (i.e. character file with extension .bak must exist).<br>
<br>

- Changed: Renamed "Fix Skill Choices Remaining" button to "Remove New Skills Button".<br>
- Changed: Fixed some of the skills names which did not match the names in the game.<br>
- Changed: Values for various stats will no longer be changed, when saving, if you edited a character manually (i.e. hex editor) and you have not changed any of the values.<br>
- Changed: Updated D2Help.pdf file<br>
<br>

**Version 1.3b (July 26, 2000)**<br>
- Added: Ability to change the Starting Act (i.e. Act 1 through 4) though you will not be able to complete any quests for later acts if any previous act(s) have not been completed.<br>
- Added: Ability to change the difficulty level the character was last played at.<br>
<br>

- Changed: Fixed a bug with the level-experience check.<br>
- Changed: Gold In Belt max limit fixed (only affected characters with level less than 16)<br>
- Changed: Can now change character's name but only one '\_' character is allowed.  If you add more than one, the character's name will be reset to the previous name and the character files will not be renamed.<br>
<br>

**Version 1.3a (July 19, 2000)**<br>
- Added: "Check character" option.  This is for people who edited their own character and finding it doesn't work in Diablo II.  This option checks for any limit violations and corrects them.<br>
<br>

- Changed: Improved checking for character title status and character state<br>
- Changed: Improved level-experience checking and fixed a bug where changing the level would change the experience even if you had more than the minimum required for any level<br>
- Changed: Updated the PDF help file a bit.<br>
<br>

**Version 1.3 (July 17, 2000)**
- Added: You can now edit character title<br>
- Added: Hotkey to the File, Close menu item (can use CTRL-C to close the currently opened file)<br>
- Added: A Refresh option (under the Edit menu or press F5) to refresh the stats.<br>
- Added: Option to backup character files located under the Options menu<br>
- Added: Level-Experience check.  When you edit your character's level and you currently do not have enough experience to meet that level, your experience points will automatically be updated to meet the minimum required.<br>
- Added: A basic help file is included in Adobe Acrobat (PDF) format.  The file name is D2Help.pdf<br>
<br>

- Changed: The character level is now the same on the character selection screen as in the game.<br>
- Changed: You can now edit skills individually with two view options.<br>
- Changed: You can now have your character file opened at the same time you play Diablo II though the stats are only updated when you select "Save and Exit Game". **NOTE:** Character stats are not updated if you do not select "Save and Exit Game"<br>
- Changed: stamina limit remains the same as before but the life and mana limits have been increased<br>
<br>

- Removed: max level limit.  Max level is now 99 (game limit)**<br>
- Removed: "Max All Skills" button<br>
<br>

**Version 1.2b (July 9, 2000)**<br>
- added: an Undo option (under the Edit menu) which undoes all changes<br>
- added: Character State to select whether your character is normal (singleplayer and multiplayer), hardcore or dead :)<br>
<br>

- changed: you can now edit character level but there is a max limit set<br>
- changed: "Fix Skills Choices Remaining" button now only active if you have skill choices left<br>
- changed: "Max All Skills" button is only active if your skills aren't already at max<br>
<br>

- fixed bug: if your character had new stats (from a level up) and you saved the character without using them, then when you go edit the file with the editor the stats would be incorrect<br>
- fixed bug: if your character is dead (from playing with hardcore setting), then stats would be shifted<br>
<br>

**Version 1.2a (July 3, 2000)**<br>
- changed max limits for max life, mana, and stamina so stamina bar would not constantly change (when you are playing as the barbarian class) and you have maxed all skills<br>
<br>

- added a "Fix Skill Choices Remaining" button; whenever you attain a new level, you also get more skill choices and a New Skills button would remain onscreen unless this value in your character file was cleared<br>
<br>

**Version 1.2 (July 3, 2000)**<br>
- set maximum limits for character stats<br>
- can now change character class<br>
- can now enter four digits for max life, max mana, and max stamina<br>
- added "Max Everything" button so you don't have to do edit all stats manually; maxes all stats including all skills<br>
<br>

**Version 1.1 (June 30, 2000)**<br>
- minor bug fixes<br>
- no longer requires VCL50.BPL runtime library<br>
<br>

**Version 1.0 (June 29, 2000)**<br>
- written in C++ Builder 5 in two days<br>
- you can edit character strength, energy, dexterity, vitality, max life/mana/stamina, max all skill trees, gold, and experience<br>
