/*
    Diablo 2 Character Editor
    Copyright (C) 2000-2003  Burton Tsang

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/
//---------------------------------------------------------------------------

#ifndef CharacterH
#define CharacterH
//---------------------------------------------------------------------------
#include <cstdio>
#include <string>
#include <vector>
#include "Constants.h"
#include "DataTypes.h"
#include "Item.h"
//---------------------------------------------------------------------------

class Character
{
private:
   // new stats and new skills status
   enum {NoCurLife = 0x8F, NoStatsSkills = 0xCF, StatsOnly = 0xDF,
         SkillsOnly = 0xEF, StatsSkills = 0xFF};

   // experience and gold status
   enum {Nothing = 0x1F, ExpOnly = 0x3F, BeltOnly = 0x5F, ExpBelt = 0x7F,
         StashOnly = 0x9F, ExpStash = 0xBF, GoldOnly = 0xDF, All = 0xFF};

   // the following variables are what would be found in the character file format
   // variables that are commented out means that no functions have been
   // declared that use them
   unsigned char Header[HEADER_LENGTH];
   long Version;                    // pos 4 in file, character file version
   unsigned long FileSize;          // pos 8 (1.09+ only), file's size
   long Checksum;                   // pos 12 (1.09+ only), stores (possible) checksum
   char Name[NAME_LENGTH];          // pos 8 (pre-1.09, otherwise pos 20),
                                    // name includes terminating NULL
   char Status;                     // pos 24 (pos 36, 1.09+ only), determines character status
   char Title;                      // pos 25 (pos 37, 1.09+ only), character's title
   char Class;                      // pos 34 (pre-1.09, otherwise pos 40),
                                    // character class
   char DisplayLevel;               // pos 36 (pre-1.09, otherwise pos 43),
                                    // level displayed at character selection
                                    // screen
//   time_t FileDateTime;             // pos 48 (1.09+ only), file date and time
                                    // using the Standard C time() function
   unsigned char DifficultyAndAct;
         // pos 88 (used in pre-1.09 only)
         // four most significant bits = difficulty level last played,
         // four least significant bits = which act is character saved at
   unsigned char StartingAct[NUM_OF_DIFFICULTY];
         // pos 168 (normal, nightmare, hell; used in 1.09+ only)
         // four MSBs value always 8 (hex, i.e. 0x80)
         // four least significant bits = which act is character saved at
//   unsigned long HirelingExperience;   // pos 187 (1.09+ only), hireling's experience

   ActInfo Acts[NUM_OF_DIFFICULTY];
      // normal act info starts at pos 345 (1.09+ only)
      // nightmare act info starts at pos 441 (1.09+ only)
      // hell act info starts at pos 537 (1.09+ only)

// pos 438 (pre-1.09, otherwise pos 643), waypoints for normal level
// pos 462 (pre-1.09, otherwise pos 667), waypoints for nightmare level
// pos 486 (pre-1.09, otherwise pos 691), waypoints for hell level
   unsigned long Waypoints[NUM_OF_DIFFICULTY][NUM_OF_WP_SETS];

/*
   The next two char fields (i.e. StatsSkillsInfo, MiscInfo) can be represented
   as one short field with the bits represented as follows:

   LSB:  0 Strength
         1 Energy
         2 Dexterity
         3 Vitality
         4 Stat Points Remaining
         5 Skill Choices Remaining
         6 Life (current)
         7 Life (max)
         8 Mana (current)
         9 Mana (max)
         10 Stamina (current)
         11 Stamina (max)
         12 Level
         13 Experience
         14 Gold in Inventory
   MSB:  15 Gold in Stash

   More info about the .d2s file structure can be found at Trevin Beattie's
   web site http://www.xmission.com/~trevin/DiabloIIv1.09_File_Format.html
*/
   unsigned char StatsSkillsInfo;   // pos 562 (pre-1.09), stats and skills notification
                                    // byte
   unsigned char MiscInfo;          // pos 563 (pre-1.09), determines if there is experience
                                    // and/or gold in belt and/or stash
   long Strength;                   // pos 565 (pre-1.09)
   long Energy;                     // pos 569 (pre-1.09)
   long Dexterity;                  // pos 573 (pre-1.09)
   long Vitality;                   // pos 577 (pre-1.09)
   long StatsLeft;                  // value of stats left
   long SkillChoices;               // value of skill choices remaining
   long CurLife;                    // shift right 8 bits for actual value
   long MaxLife;                    // shift right 8 bits for actual value
   long CurMana;                    // shift right 8 bits for actual value
   long MaxMana;                    // shift right 8 bits for actual value
   long CurStamina;                 // shift right 8 bits for actual value
   long MaxStamina;                 // shift right 8 bits for actual value
   long Level;
   unsigned long Experience;
   long GoldInBelt;
   long GoldInStash;
   char Skills[NUM_OF_SKILLS];
   unsigned short NumOfItems;       // # of items in inventory excluding gems
                                    // in socketed items
   Item *Items;                     // items in inventory
   std::vector<int> Gems;           // inventory of all gems

   // the following variables are not part of the character file format
   std::FILE *charfile;
   std::string filename, tempfilename;
   int error_code;
   long items_location,
        quests_location,
        skills_location,
        stats_location,
        waypoints_location,
        totalbytes,                 // total # of bytes used by items
        totalitems;                 // total # of items in character file
   unsigned char *temparray;

   void calculateChecksum();
   void calculateGemCode(int itemnumber, std::string& strcode);
   void checkMiscInfo();
   void clearLocations();
   void findGems();
   void fillItemsArray();
   void findLocations();
   unsigned short findTotalNumberOfItems() const;
   void initialize();
   void readBasicInfo();
   void readHeader();
   void readItems();
   void readQuests();
   void readSkills();
   void readStats();
   void readWaypoints();
   void writeBasicStats();
   void writeItems();
   void writeName();
   void writeQuests();
   void writeSkills();
   void writeStats();
   void writeWaypoints();
   void writeTempFile();
   void updateGem(const itemnumber, const std::string& newgem);

public:
   Character();
   ~Character();

   bool anyNonPerfectGems() const;
   bool areSkillsMaxed() const;
   void clearSkillChoices();
   void close();
   int convertGems(const GemTypes& gt);
   void fillBasicStats(BasicStats& bs);
   void fillCharacterStats(CharacterStats& cs);
   char getClass() const;
   char getDifficultyLastPlayed();
   int getErrorCode() const;
   unsigned long getExperience() const;
   int getNumberOfGems();
   Item *getItems();
   long getLevel() const;
   char *getName(bool fromfile = false);
   unsigned short getNumberOfItems() const;
   ActInfo *getQuests();
   long getSkillChoices() const;
   char *getSkills();
   char getStartingAct();
   char getStatus(bool rawvalue = false) const;
   char getTitle(bool rawvalue = false) const;
   long getTotalNumberOfItems() const;
   long getVersion() const;
   unsigned long getWaypoints(int difficulty, int setnumber) const;
   bool isExpansionCharacter() const;
   bool isFileOpen() const;
   bool isHardcoreCharacter() const;
   bool isValidHeader() const;
   char maskLSB(const char value) const;
   char maskMSB(const char value) const;
   void maxSkills();
   bool open(const char *filename);
   void read(bool update_locations = false);
   void save();
   void setWaypoints(int difficulty, int setnumber, unsigned long newvalue);
   void updateBasicStats(const BasicStats& bs);
   void updateCharacterStats(const CharacterStats& cs);
   void updateQuests(const ActInfo *qi);
   void updateSkills(const char *updated_skills);
   void upgradeGems();
};

#endif

