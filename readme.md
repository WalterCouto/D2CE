# Diablo II Character Editor
Copyright (c) 2000-2003 By Burton Tsang<br>
Copyright (c) 2021-2023 By Walter Couto<br>

Diablo is a trademark or registered trademark of Blizzard Entertainment, Inc., in the U.S. and/or other countries.<br>
This utility is in no way affiliated with Blizzard Entertainment.<br>
______________________________________________

### How To Use

1. Run the D2Editor.exe file<br>
2. Open a Diablo II character file (press CTRL-O or click on File, Open) which is located in the "save" directory of where you installed Diablo II: Resurrected.<br>
3. Make changes (type in a value if editing level, gold, experience, etc. or use the mouse if changing title, class, or state)<br>
4. Save changes (press CTRL-S or click on File, Save)<br>
5. Play Diablo II.<br>

![alt text](https://github.com/WalterCouto/D2CE/blob/main/screenshot.png)

### Notes

You can do the following:<br>
- Modify basic stats (Class, Name, Experience, Strength, Level, Gold in belt, etc.)
- Modify Skills<br>
- Modify Quests<br>
- Modify Waypoints<br>
- Upgrade Gems to perfect state<br>
- Upgrade Potions to their highest level<br>
- Upgrade Potions to Full Rejuvenation<br>
- Upgrade Durability of all weapons and armor to the max value<br>
- Upgrade Quantity of all stackable items to the max value<br>
- Convert a particular type and quality of Gem, Potion or Skull to another type or quality of Gem, Potion or Skull<br>
- Modify number of Sockets or what is socketed<br>
- Create, Import and Export of items<br>

### Known Issues

- Tested with Diablo II: Resurrected v1.5.73090 (PTR 2.6) and Diablo II Classic v1.14d **(It has unit tests confirming it supports all versions of the d2s file format, but the files used in testing, other than v1.14d and D2R files, have not been tested in a real game)**
- Starting with Diablo II Classic V1.13c the maximum gold in your stash no longer depends on your character's level, and is now a flat cap of 2,500,000 instead.  The editor will now use this value when editing Resurrected files or Classic files marked as v1.10 or higher when determining the limit for your gold in your stash.
- For changes to take effect, you must not have Diablo II running and you should close the character file before playing Diablo II.
- To open a d2s from a D2R Mod, the D2R Mod must be properly installed on the system and you must open the file from it's mod location i.e. \Saved Games\Diablo II Resurrected\mods\btdiablo.
- A json export file that contains the D2R Mod name can be opened from any location and will work properly if the D2R Mod is properly installed on the system.
- An item that has an unknown image, will display with a default image.

### Distribution

Please read the LICENSE file for more information regarding the license.<br>
<br>
If you have comments, suggestions, bug report, report them at https://github.com/WalterCouto/D2CE/issues<br>
<br>
If you want to Contribute, please read the [Contribution guidelines for this project](CONTRIBUTING.md)<br>
<br>
Check the following site for updates at https://github.com/WalterCouto/D2CE<br>

### Useful Links

* https://github.com/zhaoleimxd/JamellaD2E<br>
* https://github.com/daidodo/diablo_edit<br>
* https://github.com/dschu012/d2s<br>
* [d2s Binary File Format](d2s_File_Format.md)<br>

### Revision History
**Version 2.21 (April 4, 2023)**
- Updated: Fix error message from bad character file<br>
- Updated: Support added PTR 2.6<br>
- Updated: Layout of Items Form to take into account possible sizes  of inventories in D2R mods.<br>
- Updated: Json export now adds the D2R mod name if applicable.<br>
<br>

- Added: Support to read D2R standard mod character files, for example BTDiablo and Rebirth.<br>
<br>

**Version 2.20 (January 5, 2023)**
- Updated: Item tooltip to display red for unmet requirements and blue for magically enhanced attributes<br>
- Updated: Character progression logic to adjust character level to at least match the recommended level.<br>
- Updated: Updated logic for level change to reduce outstanding Stat/Skill points when level is reduced.<br>
- Updated: If file is corrupt on open and a backup exists, ask if user would like to replace the file with the backup.<br>
- Updated: Some character files may incorrectly mark Act IV complete and Act V introduced causing program to incorrectly mark all of the last quest of Act IV as completed. The program now corrects the data by checking if the town waypoint is active.<br>
<br>

- Added: Set Items now display all the possible Bonus attribues in the tooltip<br>
- Added: Add Create Item Form to allow creation of any item available in the game.<br>
- Added: Add ability to make items Ethereal or not in item forms.<br>
<br>

**Version 2.19 (October 3, 2022)**
- Updated: fix up bugs for unique items that incorrectly showed up in some cases, like the new 2.5 Charms<br>
<br>

**Version 2.18 (October 2, 2022)**
- Updated: allow poison and cold max attribute to be specified on it's own<br>
- Updated: Fix up tooltip display when min and max damage are both present<br>
<br>

**Version 2.17 (September 30, 2022)**
- Updated: fix up reading of character files for PTR 2.5<br>
- Updated: fix issue with renaming character missing one file<br>
- Updated: fix up some edge cases for adding runeword to item<br>
- Updated: fix logic to make sure unique items report the highest required level<br>
- Updated: "Max Everything" now will make all applicable items have superior quality and upgrade the item tier to the highest possible tier<br>
<br>

- Added: add support for making an item Superior<br>
- Added: add support for upgrading item tier to be exceptional or elite depending on character level<br>
<br>

**Version 2.16 (June 5, 2022)**
- Updated: fix up mercenary for PTR 2.4 changes to Barbarian<br>
- Updated: reorganized item context menu<br>
- Updated: fixed "reload" issue with mercenary data that would not read the items.<br>
- Updated: modified json export to contain more information to allow it to recreate a d2s file more accurately.<br>
<br>

- Added: add support for dragging and dropping items.<br>
- Added: add d2i item files and support for import/export of items. The application supports the standard d2i files that exists today but if the item being exported is a D2R PTR 2.4 ear or personalized item that contains utf-8 characters outside the ASCII range, it will export the d2i file as a D2R v1.2.x item  which is not the same as the format of existing d2i files.<br>
- Added: add ability to socket and unsocket items<br>
- Added: add ability to convert character file to a different version via the "Change Version" menu item.<br>
- Added: add ability to apply runewords to item via the "Apply Runeword" context menu item.<br>
<br>

**Version 2.15 (April 26, 2022)**
- Updated: Reorganize resources and add txt file to allow for future localization and customizations.<br>
- Updated: Create GPS Dialog now chooses a beltable item by default when launched from the belt inventory.<br>
- Updated: Fixed some bugs related to item tooltip strings<br>
- Updated: Fixed bugs retrieving skill names for magical attributes for non-character specific skills<br>
- Updated: Fixed D2R items with realm GUID. D2R now stores 128 bits instead of the 96 bits for realm GUID<br>
- Updated: Properly calculate the displayed requirements for items. (level, dexterity and Strength)<br>
- Updated: Fixed player name reading for PTR2.4 and allow use of UTF-8 characters in player name<br>
<br>

**Version 2.14 (Mar 11, 2022)**
- Updated: Updated jewel alternate images.<br>
- Updated: Updated item context menus across forms showing items.<br>
- Updated: Fixed hiring of new mercenary to properly save items.<br>
<br>

- Added: Shared Stash support. Users can now view and edit items in the shared stash if the file is present in the same location as their character.<br>
<br>

**Version 2.13 (Feb 28, 2022)**
- Updated: Fix detection of ID for set items.<br>
- Updated: Fix up logic for reading/writing 1.00 - 1.06 files<br>
- Updated: Add alternate ring/amulet images<br>
- Updated: Fix issue with upgrading unsupported potion<br>
- Updated: Fix dexterity "+" button to work properly<br>
- Updated: Display items in sockets in all locations.<br>
<br>

- Added: Ability to personalize items or remove the personalization from items.<br>
- Added: Ability to make weapons or armor indestructible.<br>
- Added: Gems, Potions & Skulls Creator dialog and menu items. This allows you to add Gems, Potions, Skulls or Runes to empty slots in our inventory.<br>
<br>

**Version 2.12 (Feb 14, 2022)**
- Updated: Fix detection of change in Character Form.<br>
- Updated: Fix logic for handling character title/progression<br>
- Updated: Fix logic handling skill choices left that could cause duplication of skill points<br>
<br>

- Added: Ability to add sockets to all items or in the Item form. Number of sockets that can be added it limited by the type of item and the item level.<br>
<br>

**Version 2.11 (Feb 6, 2022)**<br>
- Updated: Fix bug in Gem convertor in Resurrected when new huffman code is of a different size then the original, the result caused game to crash on startup.<br>
<br>

**Version 2.10 (Jan 18, 2022)**<br>
- Updated: Fix bug with validation of Act V on Hell difficulty if game has been complete<br>
- Updated: Fix bug with identifying Zakarum and Vortex shields<br>
- Updated: Fix bug with combining multiple instances of a single magical attribute<br>
- Updated: Fix bug with determining if an item is indestructible when the max durability is zero.<br>
- Updated: Fix logic to keep reading magical properties even if a non-supported time based property exists.<br>
<br>

- Added: Helper methods for character progression changes along with unit tests.<br>
<br>
