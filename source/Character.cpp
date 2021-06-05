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

#pragma hdrstop

#include "Character.h"
#include "CharacterConstants.h"
#include "ItemConstants.h"
//---------------------------------------------------------------------------

#pragma package(smart_init)
//---------------------------------------------------------------------------
Character::Character()
{
   initialize();
}
//---------------------------------------------------------------------------
Character::~Character()
{
   if (isFileOpen()) close();

   if (Items)
      delete[] Items;

   Items = NULL;
   temparray = NULL;
}
//---------------------------------------------------------------------------
/*
   Returns true if there are non-perfect gems.
   Returns false if no gems or all gems are perfect.

   This function is not fully portable as the index values for the items
   are based on little-endian CPUs.  The index values for the outer and
   inner switch expressions may need swapping for big-endian CPUs.
*/
bool Character::anyNonPerfectGems() const
{
   for (int i = 0; i < totalitems; ++i)
   {
      if (maskLSB(Version) == 0x40)        // pre-1.07 character file
      {
         if (Items[i].size() == V100_ITEM_LENGTH)  // v1.00 - v1.03
            switch(Items[i][9])
            {
               case 0x13: switch(Items[i][8])
                          {
                             // amethyst
                             case 0xC0:
                             case 0xD0:
                             case 0xE0:
                             case 0xF0: return true;
                          }
                          break;

               case 0x14: switch(Items[i][8])
                          {
                             case 0x10:   // topaz
                             case 0x20:
                             case 0x30:
                             case 0x40:
                             case 0x60:   // sapphire
                             case 0x70:
                             case 0x80:
                             case 0x90:
                             case 0xB0:   // emerald
                             case 0xC0:
                             case 0xD0:
                             case 0xE0: return true;
                          }
                          break;

               case 0x15: switch(Items[i][8])
                          {
                             case 0x00:   // ruby
                             case 0x10:
                             case 0x20:
                             case 0x30:
                             case 0x50:   // diamond
                             case 0x60:
                             case 0x70:
                             case 0x80: return true;
                          }
                          break;

               case 0x16: switch(Items[i][8])
                          {
                             // skull
                             case 0x40:
                             case 0x50:
                             case 0x60:
                             case 0x70: return true;
                          }
                          break;
            }  // end switch
         else              // v1.04 - v1.06
         {
            if (Items[i].size() < 12)
               continue;

            switch(Items[i][11])
            {
               case 0x8D: // chipped
               case 0x99: // flawed
               case 0xCD: // regular
                          switch(Items[i][12])
                          {
                             case 0xD9: // amethyst
                             case 0xE5:
                             case 0x89:
                             case 0x9D:
                             case 0xC9:
                             case 0xDD: // diamond
                                        return true;
                          }
                          break;

               // skull
               case 0xAD: switch(Items[i][12])
                          {
                             case 0x8D: // chipped
                             case 0x99: // flawed
                             case 0xD5: // regular
                             case 0xB1: // flawless
                                        return true;
                          }
                          break;

               // special case, found a flawless amethyst
               case 0xE9: if (Items[i][12] == 0xD9) return true;
                          break;

               case 0xB1: // flawless
                          switch(Items[i][12])
                          {
                             case 0xE5: // topaz
                             case 0x89:
                             case 0x9D:
                             case 0xC9:
                             case 0xDD: // diamond
                                        return true;
                          }
                          break;
            }  // end switch
         }
      }

      // 1.07+ character file including expansion set characters
      else if (maskLSB(Version) == 0x50)
      {
         if (Items[i].size() != V107_GEM_LENGTH)
            continue;

         switch(Items[i][10])
         {
            case 0x36: // chipped or regular
            case 0x66: // flawed
            case 0xA6: // flawless amethyst
            case 0xC6: // flawless
                       switch(Items[i][11])
                       {
                          case 0x66:
                          case 0x67: // amethyst

                          case 0x76: // diamond or emerald, depends on 11th value
                          case 0x77: // 7 = diamond; 6 = emerald

                          case 0x26: // ruby or sapphire, depends on 11th value
                          case 0x27: // 7 = ruby; 6 = sapphire

                          case 0x96:
                          case 0x97: // topaz
                                     return true;
                       }
                       break;

            // skull
            case 0xB7: switch(Items[i][11])
                       {
                          case 0x36: // chipped
                          case 0x56: // regular
                          case 0x66: // flawed
                          case 0xC6: // flawless
                                     return true;
                       }
                       break;
         }  // end switch
      }  // end big if
   }  // end for

   return false;
}
//---------------------------------------------------------------------------
bool Character::areSkillsMaxed() const
{
   for (int i = 0; i < NUM_OF_SKILLS; ++i)
      if (Skills[i] != MAX_SKILL_VALUE) return false;

   return true;
}
//---------------------------------------------------------------------------
/*
   Thanks goes to Stoned2000 for making his checksum calculation source
   available to the public.  The Visual Basic source for his editor can be
   found at http://stoned.d2network.com.
*/
void Character::calculateChecksum()
{
   // make sure we start at the beginning of the file
   std::rewind(charfile);

   Checksum = 0;

   unsigned char data, overflow = 0;

   for (unsigned long i = 0; i < FileSize; ++i)
   {
      // doubles the checksum result by left shifting once
      Checksum <<= 1;

      std::fread(&data, sizeof(data), 1, charfile);

      Checksum += data + overflow;

      if (Checksum < 0)
         overflow = 1;
      else
         overflow = 0;
   }

   // rewind the file in case we need to read/write to it again
   std::rewind(charfile);
}
//---------------------------------------------------------------------------
/*
   Sets the correct misc info value
   This function is only called by writeTempFile()
*/
void Character::checkMiscInfo()
{
   if (Experience == 0 && GoldInBelt == 0 && GoldInStash == 0)
      MiscInfo = Nothing;
   else if (Experience > 0 && GoldInBelt == 0 && GoldInStash == 0)
      MiscInfo = ExpOnly;
   else if (Experience == 0 && GoldInBelt > 0 && GoldInStash == 0)
      MiscInfo = BeltOnly;
   else if (Experience > 0 && GoldInBelt > 0 && GoldInStash == 0)
      MiscInfo = ExpBelt;
   else if (Experience == 0 && GoldInBelt == 0 && GoldInStash > 0)
      MiscInfo = StashOnly;
   else if (Experience > 0 && GoldInBelt == 0 && GoldInStash > 0)
      MiscInfo = ExpStash;
   else if (Experience == 0 && GoldInBelt > 0 && GoldInStash > 0)
      MiscInfo = GoldOnly;
   else if (Experience > 0 && GoldInBelt > 0 && GoldInStash > 0)
      MiscInfo = All;
}
//---------------------------------------------------------------------------
void Character::clearLocations()
{
   items_location = quests_location = skills_location = stats_location =
      waypoints_location = 0;
}
//---------------------------------------------------------------------------
void Character::clearSkillChoices()
{
   SkillChoices = 0;
}
//---------------------------------------------------------------------------
void Character::close()
{
   std::fclose(charfile);
   charfile = NULL;
}
//---------------------------------------------------------------------------
void Character::fillBasicStats(BasicStats& bs)
{
   strcpy(bs.Name, Name);
   bs.Status = getStatus();
   bs.Title = getTitle();
   bs.Class = Class;
   bs.DifficultyLastPlayed = getDifficultyLastPlayed();
   bs.StartingAct = getStartingAct();
}
//---------------------------------------------------------------------------
void Character::fillCharacterStats(CharacterStats& cs)
{
   cs.Strength = Strength;
   cs.Energy = Energy;
   cs.Dexterity = Dexterity;
   cs.Vitality = Vitality;

   cs.CurLife = CurLife >> 8;
   cs.MaxLife = MaxLife >> 8;
   cs.CurMana = CurMana >> 8;
   cs.MaxMana = MaxMana >> 8;
   cs.CurStamina = CurStamina >> 8;
   cs.MaxStamina = MaxStamina >> 8;

   cs.Level = Level;
   cs.Experience = Experience;
   cs.GoldInBelt = GoldInBelt;
   cs.GoldInStash = GoldInStash;
}
//---------------------------------------------------------------------------
/*
   This function should be called only after filling temparray.
*/
void Character::fillItemsArray()
{
   if (Items)
      delete[] Items;

   Items = new Item[totalitems];

   std::fseek(charfile, items_location, SEEK_SET);

   int itemnumber = 0;
   for (int i = 0; i < totalbytes - 1; ++i)
   {
      if (temparray[i] == 0x4A && temparray[i+1] == 0x4D)
      {
         int itemsize = 2;
         int j;
         for (j = i+2; j < totalbytes - 1; ++j)
         {
            if (temparray[j] == 0x4A && temparray[j+1] == 0x4D)
               break;

            ++itemsize;
         }

         if (j == totalbytes - 1)
            ++itemsize;

         Items[itemnumber].size(itemsize);

         for (int k = 0; k < itemsize; ++k)
            Items[itemnumber][k] = temparray[i + k];

         ++itemnumber;
         i += itemsize - 1;
      }  // end if
   }  // end for
}
//---------------------------------------------------------------------------
/*
   Scans the entire character file and marks the location of where quests,
   waypoints, character stats, and skills are located for later use.
   The position of where items are first found in the file is also marked.
*/
void Character::findLocations()
{
   bool foundall = false;
   unsigned char value;
   int cur_pos = std::ftell(charfile);

   std::fseek(charfile, cur_pos, SEEK_SET);
   while(!feof(charfile) && !foundall)
   {
      std::fread(&value, sizeof(value), 1, charfile);
      switch(value)
      {
         // found quests marker "Woo!" (i.e. 57 6F 6F 21)
         case 0x57: std::fread(&value, sizeof(value), 1, charfile);
                    if (value == 0x6f)
                    {
                       std::fread(&value, sizeof(value), 1, charfile);
                       if (value == 0x6f)
                       {
                          std::fread(&value, sizeof(value), 1, charfile);
                          if ((value == 0x21) && (std::ftell(charfile) > MIN_START_POS))
                             quests_location = std::ftell(charfile) + 6;
                       }
                    }

                    // found waypoints marker (i.e. "WS")
                    else if (value == 0x53 && (std::ftell(charfile) > MIN_START_POS))
                       waypoints_location = std::ftell(charfile) + 8;
                    break;

         case 0x67: std::fread(&value, sizeof(value), 1, charfile);
                    // found stats marker (0x6766 or "gf")
                    if (value == 0x66 && (std::ftell(charfile) > MIN_START_POS))
                       stats_location = std::ftell(charfile);
                    break;

         case 0x69: std::fread(&value, sizeof(value), 1, charfile);
                    // found skills marker (0x6966 or "if")
                    if (value == 0x66 && (std::ftell(charfile) > MIN_START_POS))
                    {
                       skills_location = std::ftell(charfile);
                       std::fseek(charfile, NUM_OF_SKILLS+2, SEEK_CUR);

                       std::fread(&NumOfItems, sizeof(NumOfItems), 1, charfile);
                       items_location = std::ftell(charfile);

                       foundall = true;
                    }
                    break;
      }
   }
}
//---------------------------------------------------------------------------
/*
   Returns the total number of items in temparray.
   This function should only be called after filling temparray.
*/
unsigned short Character::findTotalNumberOfItems() const
{
   int totalitems = 0;

   for (int i = 0; i < totalbytes - 1; ++i)
      if (temparray[i] == 0x4A && temparray[i+1] == 0x4D)
      {
         ++totalitems;
         ++i;
      }

   return totalitems;
}
//---------------------------------------------------------------------------
char Character::getClass() const
{
   return Class;
}
//---------------------------------------------------------------------------
/*
   Returns a value indicating the difficulty level the character last played at,
   where 0 = normal, 1 = nightmare, 2 = hell
*/
char Character::getDifficultyLastPlayed()
{
   if (Version < VERSION_109)
      return DifficultyAndAct >> 4;

   if (StartingAct[0] != 0)
      return NORMAL;
   else if (StartingAct[1] != 0)
      return NIGHTMARE;
   else
      return HELL;
}
//---------------------------------------------------------------------------
int Character::getErrorCode() const
{
   return error_code;
}
//---------------------------------------------------------------------------
unsigned long Character::getExperience() const
{
   return Experience;
}
//---------------------------------------------------------------------------
Item *Character::getItems()
{
   return Items;
}
//---------------------------------------------------------------------------
long Character::getLevel() const
{
   return Level;
}
//---------------------------------------------------------------------------
char *Character::getName(bool fromfile)
{
   if (fromfile)
   {
      if (Version < VERSION_109)
         std::fseek(charfile, 8, SEEK_SET);
      else
         std::fseek(charfile, 20, SEEK_SET);

      std::fread(Name, sizeof(Name), 1, charfile);
   }

   return Name;
}
//---------------------------------------------------------------------------
/*
   Returns the number of items in character's inventory.
   Value returned excludes socketed gems/jewels/runes.
*/
unsigned short Character::getNumberOfItems() const
{
   return NumOfItems;
}
//---------------------------------------------------------------------------
ActInfo *Character::getQuests()
{
   return Acts;
}
//---------------------------------------------------------------------------
long Character::getSkillChoices() const
{
   return SkillChoices;
}
//---------------------------------------------------------------------------
char *Character::getSkills()
{
   return Skills;
}
//---------------------------------------------------------------------------
/*
   Returns a value indicating the act the character was saved at, where 0
   indicates the first act
*/
char Character::getStartingAct()
{
   if (Version < VERSION_109)
      return maskMSB(DifficultyAndAct);

   if (StartingAct[0] != 0)
      return maskMSB(StartingAct[0]);
   else if (StartingAct[1] != 0)
      return maskMSB(StartingAct[1]);
   else
      return maskMSB(StartingAct[2]);
}
//---------------------------------------------------------------------------
char Character::getStatus(bool rawvalue) const
{
   if (rawvalue)
      return Status;

   if (maskMSB(Status) < Hardcore)
      return NoDeaths;
   else if (maskMSB(Status) >= Hardcore && maskMSB(Status) < Resurrected)
      return Hardcore;
   else if (maskMSB(Status) >= Resurrected && maskMSB(Status) < Dead)
      return Resurrected;
   else
      return Dead;
}
//---------------------------------------------------------------------------
char Character::getTitle(bool rawvalue) const
{
   if (rawvalue)
      return Title;

   switch(Title)
   {
      case SirDame:
      case Slayer:
      case 0x06:
      case 0x07: if (!isExpansionCharacter())
                    return SirDame;
                 else
                    return Slayer;
      case LordLady:
      case Champion:
      case 0x09:
      case 0x0B: if (!isExpansionCharacter())
                    return LordLady;
                 else
                    return Champion;
      case BaronBaroness:
      case MPatriarch:
      case 0x0D:
      case 0x0E: if (!isExpansionCharacter())
                    return BaronBaroness;
                 else
                    return MPatriarch;
   }

   return None;
}
//---------------------------------------------------------------------------
/*
   Returns the total number of items in character's inventory.
*/
long Character::getTotalNumberOfItems() const
{
   return totalitems;
}
//---------------------------------------------------------------------------
long Character::getVersion() const
{
   return Version;
}
//---------------------------------------------------------------------------
unsigned long Character::getWaypoints(int difficulty, int setnumber) const
{
   return Waypoints[difficulty][setnumber];
}
//---------------------------------------------------------------------------
void Character::initialize()
{
   std::memset(Header, 0, sizeof(Header));
   std::memset(Name, 0, sizeof(Name));
   Status = 0;
   Title = 0;
   Class = 0;

   DifficultyAndAct = 0;
   std::memset(StartingAct, 0, sizeof(StartingAct));
   std::memset(Acts, 0, sizeof(Acts));
   std::memset(Waypoints, 0, sizeof(Waypoints));

   StatsSkillsInfo = 0;
   MiscInfo = 0;
   Dexterity = 0;
   Energy = 0;
   Level = DisplayLevel = 1;
   Strength = 0;
   Vitality = 0;

   CurLife = 0;
   MaxLife = 0;
   CurMana = 0;
   MaxMana = 0;
   CurStamina = 0;
   MaxStamina = 0;

   Experience = 0;
   GoldInBelt = 0;
   GoldInStash = 0;

   std::memset(Skills, 0, sizeof(Skills));
   Items = NULL;

   temparray = NULL;
   totalbytes = 0;

   error_code = 0;

   clearLocations();
}
//---------------------------------------------------------------------------
bool Character::isExpansionCharacter() const
{
   if (maskLSB(Status) == EXPANSION_CHARACTER)
      return true;

   return false;
}
//---------------------------------------------------------------------------
bool Character::isFileOpen() const
{
   if (charfile)
      return true;

   return false;
}
//---------------------------------------------------------------------------
bool Character::isHardcoreCharacter() const
{
   if (maskMSB(Status) == Hardcore)
      return true;

   return false;
}
//---------------------------------------------------------------------------
/*
   Returns true if the file has a valid header
*/
bool Character::isValidHeader() const
{
   if (std::memcmp(Header, HEADER, HEADER_LENGTH) == 0)
      return true;

   return false;
}
//---------------------------------------------------------------------------
char Character::maskLSB(const char value) const
{
   return (value & 0xF0);
}
//---------------------------------------------------------------------------
char Character::maskMSB(const char value) const
{
   return (value & 0x0F);
}
//---------------------------------------------------------------------------
void Character::maxSkills()
{
   std::memset(Skills, MAX_SKILL_VALUE, sizeof(Skills));
}
//---------------------------------------------------------------------------
/*
   Returns false if file was not opened or there was an error.
*/
bool Character::open(const char *filename)
{
   charfile = std::fopen(filename, "rb+");
   this->filename = filename;

   if (charfile == NULL)
   {
      error_code = CANNOT_OPEN_FILE;
      return false;
   }

   readHeader();

   // verify file is a Diablo II character file
   if (!isValidHeader())
   {
      error_code = INVALID_HEADER;
      return false;
   }

   read(true);
   return true;
}
//---------------------------------------------------------------------------
void Character::read(bool update_locations)
{
   if (update_locations)
   {
      initialize();
      findLocations();
   }

   readBasicInfo();
   readQuests();
   readWaypoints();
   readStats();
   readSkills();
   readItems();
}
//---------------------------------------------------------------------------
void Character::readBasicInfo()
{
   std::fseek(charfile, 4, SEEK_SET);
   std::fread(&Version, sizeof(Version), 1, charfile);

   if (Version >= VERSION_109)
   {
      std::fread(&FileSize, sizeof(FileSize), 1, charfile);
      std::fread(&Checksum, sizeof(Checksum), 1, charfile);

      std::fseek(charfile, 20, SEEK_SET);
   }

   std::fread(Name, sizeof(Name), 1, charfile);
   std::fread(&Status, sizeof(Status), 1, charfile);
   std::fread(&Title, sizeof(Title), 1, charfile);

   if (Version < VERSION_109)
      std::fseek(charfile, 34, SEEK_SET);
   else
      std::fseek(charfile, 40, SEEK_SET);

   std::fread(&Class, sizeof(Class), 1, charfile);

   if (Version < VERSION_109)
      std::fseek(charfile, 36, SEEK_SET);
   else
      std::fseek(charfile, 43, SEEK_SET);

   std::fread(&DisplayLevel, sizeof(DisplayLevel), 1, charfile);

   if (Version < VERSION_109)
   {
      std::fseek(charfile, 88, SEEK_SET);
      std::fread(&DifficultyAndAct, sizeof(DifficultyAndAct), 1, charfile);
   }
   else
   {
      std::fseek(charfile, 168, SEEK_SET);
      std::fread(StartingAct, sizeof(StartingAct), 1, charfile);
   }

}
//---------------------------------------------------------------------------
void Character::readHeader()
{
   std::rewind(charfile);
   std::fread(Header, sizeof(Header), 1, charfile);
}
//---------------------------------------------------------------------------
void Character::readItems()
{
   std::fseek(charfile, 0, SEEK_END);
   totalbytes = std::ftell(charfile) - items_location;

   temparray = new unsigned char[totalbytes];

   std::fseek(charfile, items_location, SEEK_SET);
   std::fread(temparray, totalbytes, 1, charfile);

   totalitems = findTotalNumberOfItems();
   fillItemsArray();

   delete[] temparray;

   findGems();
}
//---------------------------------------------------------------------------
void Character::readQuests()
{
   std::fseek(charfile, quests_location, SEEK_SET);
   for (int i = 0; i < NUM_OF_DIFFICULTY; ++i)
   {
      for (int j = 0; j < NUM_OF_ACTS - 1; ++j)
      {
         std::fread(&Acts[i].Intro[j], sizeof(Acts[i].Intro[j]), 1, charfile);
         std::fread(&Acts[i].Quests[j], sizeof(Acts[i].Quests[j]), 1, charfile);

         if (j < 3)
            std::fread(&Acts[i].Completed[j], sizeof(Acts[i].Completed[j]), 1,
                       charfile);
      }

      // make sure we read act 4 completed status correctly
      std::fseek(charfile, -6, SEEK_CUR);
      std::fread(&Acts[i].Completed[3], sizeof(Acts[i].Completed[3]),
                 1, charfile);

      // move on if pre-1.07 version
      if (maskLSB(Version) == 0x40)
         std::fseek(charfile, 28, SEEK_CUR);

      // read act 5 quests if expansion set character
      else if (isExpansionCharacter())
      {
         std::fseek(charfile, 12, SEEK_CUR);

         for (int k = 0; k < NUM_OF_QUESTS; ++k)
            std::fread(&Acts[i].Quests[4][k],
                       sizeof(Acts[i].Quests[4][k]), 1, charfile);

         std::fseek(charfile, 14, SEEK_CUR);
      }
   }
}
//---------------------------------------------------------------------------
void Character::readSkills()
{
   std::fseek(charfile, skills_location, SEEK_SET);
   std::fread(Skills, sizeof(Skills), 1, charfile);
}
//---------------------------------------------------------------------------
void Character::readStats()
{
   // find out if there is any new stats and skill choices remaining
   std::fseek(charfile, stats_location, SEEK_SET);
   std::fread(&StatsSkillsInfo, sizeof(StatsSkillsInfo), 1, charfile);

   // is there any experience and/or gold
   std::fread(&MiscInfo, sizeof(MiscInfo), 1, charfile);

   // skip null byte character version is less than 1.09
   if (Version < VERSION_109)
      std::fseek(charfile, 1, SEEK_CUR);

   // read basic stats
   std::fread(&Strength, sizeof(Strength), 1, charfile);
   std::fread(&Energy, sizeof(Energy), 1, charfile);
   std::fread(&Dexterity, sizeof(Dexterity), 1, charfile);
   std::fread(&Vitality, sizeof(Vitality), 1, charfile);

   switch(StatsSkillsInfo)
   {
      case StatsOnly: std::fread(&StatsLeft, sizeof(StatsLeft), 1, charfile);
                      break;
      case SkillsOnly: std::fread(&SkillChoices, sizeof(SkillChoices), 1, charfile);
                       break;
      case StatsSkills: std::fread(&StatsLeft, sizeof(StatsLeft), 1, charfile);
                        std::fread(&SkillChoices, sizeof(SkillChoices), 1, charfile);
                        break;
   }

   if (StatsSkillsInfo != NoCurLife)
      std::fread(&CurLife, sizeof(CurLife), 1, charfile);

   std::fread(&MaxLife, sizeof(MaxLife), 1, charfile);
   std::fread(&CurMana, sizeof(CurMana), 1, charfile);
   std::fread(&MaxMana, sizeof(MaxMana), 1, charfile);
   std::fread(&CurStamina, sizeof(CurStamina), 1, charfile);
   std::fread(&MaxStamina, sizeof(MaxStamina), 1, charfile);

   std::fread(&Level, sizeof(Level), 1, charfile);

   switch(MiscInfo)
   {
      case ExpOnly: std::fread(&Experience, sizeof(Experience), 1, charfile);
                    break;

      case BeltOnly: std::fread(&GoldInBelt, sizeof(GoldInBelt), 1, charfile);
                     break;

      case ExpBelt: std::fread(&Experience, sizeof(Experience), 1, charfile);
                    std::fread(&GoldInBelt, sizeof(GoldInBelt), 1, charfile);
                    break;

      case StashOnly: std::fread(&GoldInStash, sizeof(GoldInStash), 1, charfile);
                      break;

      case ExpStash: std::fread(&Experience, sizeof(Experience), 1, charfile);
                     std::fread(&GoldInStash, sizeof(GoldInStash), 1, charfile);
                     break;

      case All: std::fread(&Experience, sizeof(Experience), 1, charfile);
      case GoldOnly: std::fread(&GoldInBelt, sizeof(GoldInBelt), 1, charfile);
                std::fread(&GoldInStash, sizeof(GoldInStash), 1, charfile);
                break;
   }
}
//---------------------------------------------------------------------------
void Character::readWaypoints()
{
   std::fseek(charfile, waypoints_location, SEEK_SET);

   for (int i = 0; i < NUM_OF_DIFFICULTY; ++i)
   {
      std::fread(Waypoints[i], sizeof(Waypoints[i]), 1, charfile);
      std::fseek(charfile, 16, SEEK_CUR);
   }
}
//---------------------------------------------------------------------------
void Character::save()
{
   writeBasicStats();
   writeQuests();
   writeWaypoints();
   writeStats();
   writeSkills();
   writeItems();

   writeTempFile();

   // prepare to update the character file
   std::fclose(charfile);
   std::remove(filename.c_str());

   // check to see if the filename needs to be changed
   // to match the character's name
   unsigned int pos = filename.rfind('\\', filename.length());

   if (pos != filename.length())
      ++pos;

   std::string tempname = filename;
   tempname.erase(0, pos);

   // compare filename (w/o extension) to character's name
   if (tempname.compare(0, tempname.length()-4, Name) != 0)
   {
      filename.erase(pos, tempname.length());
      tempname = Name;
      tempname += ".d2s";
      filename.insert(filename.length(), tempname);
   }

   // rename temp file to character file
   std::rename(tempfilename.c_str(), filename.c_str());
   open(filename.c_str());

   // determine if a checksum needs to be calculated and stored
   if (Version >= VERSION_109)
   {
      // store the file's size
      std::fseek(charfile, 0, SEEK_END);
      FileSize = std::ftell(charfile);
      std::fseek(charfile, 8, SEEK_SET);
      std::fwrite(&FileSize, sizeof(FileSize), 1, charfile);

      // make sure the checksum is zero in the file
      Checksum = 0;
      std::fwrite(&Checksum, sizeof(Checksum), 1, charfile);

      calculateChecksum();

      // write the checksum into the file
      std::fseek(charfile, 12, SEEK_SET);
      std::fwrite(&Checksum, sizeof(Checksum), 1, charfile);
   }
}
//---------------------------------------------------------------------------
void Character::setWaypoints(int difficulty, int setnumber, unsigned long newvalue)
{
   Waypoints[difficulty][setnumber] = newvalue;
}
//---------------------------------------------------------------------------
void Character::updateBasicStats(const BasicStats& bs)
{
   strcpy(Name, bs.Name);
   Status = maskLSB(Status) | bs.Status * Hardcore;

   if (!isExpansionCharacter())
   {
      if (bs.Title * SirDame != getTitle())
         Title = bs.Title * SirDame;
   }
   else
   {
      if (bs.Title * Slayer != getTitle())
         Title = bs.Title * Slayer;
   }

   Class = bs.Class;

   if (Version < VERSION_109)
      DifficultyAndAct = 0x10 * bs.DifficultyLastPlayed | bs.StartingAct;
   else
   {
      std::memset(StartingAct, 0, sizeof(StartingAct));
      StartingAct[bs.DifficultyLastPlayed] = 0x80 | bs.StartingAct;
   }
}
//---------------------------------------------------------------------------
void Character::updateCharacterStats(const CharacterStats& cs)
{
   Dexterity = cs.Dexterity;
   Energy = cs.Energy;
   Strength = cs.Strength;
   Vitality = cs.Vitality;

   CurLife = cs.CurLife << 8;
   MaxLife = cs.MaxLife << 8;
   CurMana = cs.CurMana << 8;
   MaxMana = cs.MaxMana << 8;
   CurStamina = cs.CurStamina << 8;
   MaxStamina = cs.MaxStamina << 8;

   Level = cs.Level;
   DisplayLevel = Level;         // updates character's display level
                                 // to match character's actual level
   Experience = cs.Experience;
   GoldInBelt = cs.GoldInBelt;
   GoldInStash = cs.GoldInStash;
}
//---------------------------------------------------------------------------
void Character::updateQuests(const ActInfo *qi)
{
   std::memcpy(Acts, qi, sizeof(Acts));
}
//---------------------------------------------------------------------------
void Character::updateSkills(const char *updated_skills)
{
   std::memcpy(Skills, updated_skills, sizeof(updated_skills));
}
//---------------------------------------------------------------------------
/*
   This function is not fully portable as the index values for the items
   are based on little-endian CPUs.  The index values for the outer and
   inner switch expressions may need swapping for big-endian CPUs.
*/
void Character::upgradeGems()
{
   for (int i = 0; i < totalitems; ++i)
   {
      if (maskLSB(Version) == 0x40)     // pre-1.07 character file
      {
         if (Items[i].size() == V100_ITEM_LENGTH)  // v1.00 - v1.03
            switch(Items[i][9])
            {
               case 0x13: switch(Items[i][8])
                          {
                             // amethyst
                             case 0xC0:
                             case 0xD0:
                             case 0xE0:
                             case 0xF0: Items[i][8] = 0x00;
                                        Items[i][9] = 0x14;
                                        break;
                          }
                          break;

               case 0x14: switch(Items[i][8])
                          {
                             // topaz
                             case 0x10:
                             case 0x20:
                             case 0x30:
                             case 0x40: Items[i][8] = 0x50;
                                        break;
                             // sapphire
                             case 0x60:
                             case 0x70:
                             case 0x80:
                             case 0x90: Items[i][8] = 0xA0;
                                        break;
                             // emerald
                             case 0xB0:
                             case 0xC0:
                             case 0xD0:
                             case 0xE0: Items[i][8] = 0xF0;
                                        break;
                          }
                          break;

               case 0x15: switch(Items[i][8])
                          {
                             // ruby
                             case 0x00:
                             case 0x10:
                             case 0x20:
                             case 0x30: Items[i][8] = 0x40;
                                        break;
                             // diamond
                             case 0x50:
                             case 0x60:
                             case 0x70:
                             case 0x80: Items[i][8] = 0x90;
                                        break;
                          }
                          break;

               case 0x16: switch(Items[i][8])
                          {
                             // skull
                             case 0x40:
                             case 0x50:
                             case 0x60:
                             case 0x70: Items[i][8] = 0x80;
                                        break;
                          }
                          break;
            }  // end switch
         else        // v1.04 - v1.06
         {
            if (Items[i].size() < 12)
               continue;

            switch(Items[i][11])
            {
               case 0x8D: // chipped
               case 0x99: // flawed
               case 0xCD: // regular
                          switch(Items[i][12])
                          {
                             case 0xD9: // amethyst
                             case 0xE5:
                             case 0x89:
                             case 0x9D:
                             case 0xC9:
                             case 0xDD: // diamond
                                        Items[i][11] = 0xC1;
                          }
                          break;

               // skull
               case 0xAD: switch(Items[i][12])
                          {
                             case 0x8D: // chipped
                             case 0x99: // flawed
                             case 0xD5: // regular
                             case 0xB1: // flawless
                                        Items[i][12] = 0xE9;
                          }
                          break;

               // special case, found a flawless amethyst
               case 0xE9: if (Items[i][12] == 0xD9)
                             Items[i][11] = 0xC1;
                          break;

               case 0xB1: // flawless
                          switch(Items[i][12])
                          {
                             case 0xE5: // topaz
                             case 0x89:
                             case 0x9D:
                             case 0xC9:
                             case 0xDD: // diamond
                                        Items[i][11] = 0xC1;
                          }
                          break;
            }  // end switch
         }  // end else
      }

      // 1.07+ character file including expansion set characters
      else if (maskLSB(Version) == 0x50)
      {
         if (Items[i].size() != V107_GEM_LENGTH)
            continue;

         switch(Items[i][10])
         {
            case 0x36: // chipped or regular
            case 0x66: // flawed
            case 0xA6: // flawless amethyst
            case 0xC6: // flawless
                       switch(Items[i][11])
                       {
                          case 0x66:
                          case 0x67: // amethyst

                          case 0x76: // diamond or emerald, depends on 11th value
                          case 0x77: // 7 = diamond; 6 = emerald

                          case 0x26: // ruby or sapphire, depends on 11th value
                          case 0x27: // 7 = ruby; 6 = sapphire

                          case 0x96:
                          case 0x97: // topaz
                                     Items[i][10] = 0x06;
                                     Items[i][11] |= 0x01;
                                     break;
                       }
                       break;

            // skull
            case 0xB7: switch(Items[i][11])
                       {
                          case 0x36: // chipped
                          case 0x56: // regular
                          case 0x66: // flawed
                          case 0xC6: // flawless
                                     Items[i][11] = 0xA6;
                                     Items[i][12] = 0x07;
                                     break;
                       }
                       break;
         }  // end switch
      }
   }  // end for
}
//---------------------------------------------------------------------------
void Character::writeBasicStats()
{
   writeName();
   std::fwrite(&Status, sizeof(Status), 1, charfile);
   std::fwrite(&Title, sizeof(Title), 1, charfile);

   if (Version < VERSION_109)
      std::fseek(charfile, 34, SEEK_SET);
   else
      std::fseek(charfile, 40, SEEK_SET);

   std::fwrite(&Class, sizeof(Class), 1, charfile);

   if (Version < VERSION_109)
      std::fseek(charfile, 36, SEEK_SET);
   else
      std::fseek(charfile, 43, SEEK_SET);

   std::fwrite(&DisplayLevel, sizeof(DisplayLevel), 1, charfile);

   if (Version < VERSION_109)
   {
      std::fseek(charfile, 88, SEEK_SET);
      std::fwrite(&DifficultyAndAct, sizeof(DifficultyAndAct), 1, charfile);
   }
   else
   {
      std::fseek(charfile, 168, SEEK_SET);
      std::fwrite(StartingAct, sizeof(StartingAct), 1, charfile);
   }
}
//---------------------------------------------------------------------------
void Character::writeItems()
{
   std::fseek(charfile, items_location, SEEK_SET);
   for (int i = 0; i < totalitems; ++i)
      std::fwrite(Items[i].getItemArray(), Items[i].size(), 1, charfile);
}
//---------------------------------------------------------------------------
void Character::writeName()
{
   // make sure name is saved correctly
   std::string temp(Name);
   std::memset(Name, 0, sizeof(Name));
   strcpy(Name, temp.c_str());

   if (Version < VERSION_109)
      std::fseek(charfile, 8, SEEK_SET);
   else
      std::fseek(charfile, 20, SEEK_SET);

   std::fwrite(Name, sizeof(Name), 1, charfile);
}
//---------------------------------------------------------------------------
void Character::writeQuests()
{
   std::fseek(charfile, quests_location, SEEK_SET);
   for (int i = 0; i < NUM_OF_DIFFICULTY; ++i)
   {
      // write values of acts 1 through 4
      for (int j = 0; j < NUM_OF_ACTS - 1; ++j)
      {
         std::fwrite(&Acts[i].Intro[j], sizeof(Acts[i].Intro[j]),
                     1, charfile);
         std::fwrite(&Acts[i].Quests[j], sizeof(Acts[i].Quests[j]),
                     1, charfile);

         if (j < 3)
            std::fwrite(&Acts[i].Completed[j],
                        sizeof(Acts[i].Completed[j]), 1, charfile);
      }

      // fill in act completed values for act 4
      std::fseek(charfile, -6, SEEK_CUR);
      std::fwrite(&Acts[i].Completed[3],
                  sizeof(Acts[i].Completed[3]), 1, charfile);


      // move on if pre-1.07 version
      if (maskLSB(Version) == 0x40)
         std::fseek(charfile, 28, SEEK_CUR);

      // write act 5 quests info if expansion set character
      else if (isExpansionCharacter())
      {
         std::fseek(charfile, 12, SEEK_CUR);
         std::fwrite(&Acts[i].Quests[4], sizeof(Acts[i].Quests[4]),
                     1, charfile);
         std::fseek(charfile, 14, SEEK_CUR);
      }
   }
}
//---------------------------------------------------------------------------
void Character::writeSkills()
{
   std::fseek(charfile, skills_location, SEEK_SET);
   std::fwrite(Skills, sizeof(Skills), 1, charfile);
}
//---------------------------------------------------------------------------
void Character::writeStats()
{
   std::fseek(charfile, stats_location, SEEK_SET);

   // make sure correct info is saved if the character is not dead
   if (CurLife > 0)
      StatsSkillsInfo |= 0x4F;

   std::fwrite(&StatsSkillsInfo, sizeof(StatsSkillsInfo), 1, charfile);
   std::fwrite(&MiscInfo, sizeof(MiscInfo), 1, charfile);

   // skip null byte if character version is less than 1.09
   if (Version < VERSION_109)
      std::fseek(charfile, 1, SEEK_CUR);

   std::fwrite(&Strength, sizeof(Strength), 1, charfile);
   std::fwrite(&Energy, sizeof(Energy), 1, charfile);
   std::fwrite(&Dexterity, sizeof(Dexterity), 1, charfile);
   std::fwrite(&Vitality, sizeof(Vitality), 1, charfile);

   switch(StatsSkillsInfo)
   {
      case StatsOnly: std::fseek(charfile, 4, SEEK_CUR);
                      break;

      case StatsSkills: std::fseek(charfile, 4, SEEK_CUR);
      case SkillsOnly: std::fwrite(&SkillChoices, sizeof(SkillChoices), 1, charfile);
                       break;
   }

   if (StatsSkillsInfo != NoCurLife)
      std::fwrite(&CurLife, sizeof(CurLife), 1, charfile);

   std::fwrite(&MaxLife, sizeof(MaxLife), 1, charfile);
   std::fwrite(&CurMana, sizeof(CurMana), 1, charfile);
   std::fwrite(&MaxMana, sizeof(MaxMana), 1, charfile);
   std::fwrite(&CurStamina, sizeof(CurStamina), 1, charfile);
   std::fwrite(&MaxStamina, sizeof(MaxStamina), 1, charfile);

   std::fwrite(&Level, sizeof(Level), 1, charfile);

   switch(MiscInfo)
   {
      case ExpOnly: std::fwrite(&Experience, sizeof(Experience), 1, charfile);
                    break;

      case BeltOnly: std::fwrite(&GoldInBelt, sizeof(GoldInBelt), 1, charfile);
                     break;

      case ExpBelt: std::fwrite(&Experience, sizeof(Experience), 1, charfile);
                    std::fwrite(&GoldInBelt, sizeof(GoldInBelt), 1, charfile);
                    break;

      case StashOnly: std::fwrite(&GoldInStash, sizeof(GoldInStash), 1, charfile);
                      break;

      case ExpStash: std::fwrite(&Experience, sizeof(Experience), 1, charfile);
                     std::fwrite(&GoldInStash, sizeof(GoldInStash), 1, charfile);
                     break;

      case GoldOnly: std::fwrite(&GoldInBelt, sizeof(GoldInBelt), 1, charfile);
                     std::fwrite(&GoldInStash, sizeof(GoldInStash), 1, charfile);
                     break;

      case All: std::fwrite(&Experience, sizeof(Experience), 1, charfile);
                std::fwrite(&GoldInBelt, sizeof(GoldInBelt), 1, charfile);
                std::fwrite(&GoldInStash, sizeof(GoldInStash), 1, charfile);
                break;
   }

   std::fwrite(SKILLS_MARKER, sizeof(SKILLS_MARKER), 1, charfile);
   std::fwrite(Skills, sizeof(Skills), 1, charfile);
}
//---------------------------------------------------------------------------
/*
   This function makes sure that any changes to the character's
   experience and gold values are correctly stored in the file.
*/
void Character::writeTempFile()
{
   tempfilename = std::tmpnam(NULL);
   std::FILE *tempfile = std::fopen(tempfilename.c_str(), "wb");

   std::rewind(charfile);

   int startpos = stats_location+1;
   unsigned char filedata;

   while(std::ftell(charfile) < startpos)
   {
      std::fread(&filedata, sizeof(filedata), 1, charfile);
      std::fwrite(&filedata, sizeof(filedata), 1, tempfile);
   }

   unsigned char prevMiscInfo = MiscInfo;

   checkMiscInfo();

   std::fwrite(&MiscInfo, sizeof(MiscInfo), 1, tempfile);
   std::fseek(charfile, 1, SEEK_CUR);

   int numOfBytes = 16+ 1+ 22+1+ 4;

   if (Version < VERSION_109)
      numOfBytes += 1;

   // calculate exact # of bytes to read
   switch(StatsSkillsInfo)
   {
      case StatsOnly:
      case SkillsOnly: numOfBytes += 4;
                       break;
      case StatsSkills: numOfBytes += 4+4;
                        break;
   }

   // read and write # of bytes calculated above
   for (int i = 0; i < numOfBytes; ++i)
   {
      std::fread(&filedata, sizeof(filedata), 1, charfile);
      std::fwrite(&filedata, sizeof(filedata), 1, tempfile);
   }

   // write any experience and/or gold
   switch(MiscInfo)
   {
      case ExpOnly: std::fwrite(&Experience, sizeof(Experience), 1, tempfile);
                    break;

      case BeltOnly: std::fwrite(&GoldInBelt, sizeof(GoldInBelt), 1, tempfile);
                     break;

      case ExpBelt: std::fwrite(&Experience, sizeof(Experience), 1, tempfile);
                    std::fwrite(&GoldInBelt, sizeof(GoldInBelt), 1, tempfile);
                    break;

      case StashOnly: std::fwrite(&GoldInStash, sizeof(GoldInStash), 1, tempfile);
                      break;

      case ExpStash: std::fwrite(&Experience, sizeof(Experience), 1, tempfile);
                     std::fwrite(&GoldInStash, sizeof(GoldInStash), 1, tempfile);
                     break;

      case GoldOnly: std::fwrite(&GoldInBelt, sizeof(GoldInBelt), 1, tempfile);
                     std::fwrite(&GoldInStash, sizeof(GoldInStash), 1, tempfile);
                     break;

      case All: std::fwrite(&Experience, sizeof(Experience), 1, tempfile);
                std::fwrite(&GoldInBelt, sizeof(GoldInBelt), 1, tempfile);
                std::fwrite(&GoldInStash, sizeof(GoldInStash), 1, tempfile);
                break;
   }

   // skip over correct # of bytes in original file
   switch(prevMiscInfo)
   {
      case ExpOnly:
      case BeltOnly:
      case StashOnly: std::fseek(charfile, 4, SEEK_CUR);
                      break;

      case ExpBelt:
      case ExpStash:
      case GoldOnly: std::fseek(charfile, 8, SEEK_CUR);
                     break;

      case All: std::fseek(charfile, 12, SEEK_CUR);
                break;
   }

   std::fwrite(SKILLS_MARKER, sizeof(SKILLS_MARKER), 1, tempfile);
   std::fwrite(Skills, sizeof(Skills), 1, tempfile);
   std::fwrite(ITEM_MARKER, sizeof(ITEM_MARKER), 1, tempfile);
   std::fwrite(&NumOfItems, sizeof(NumOfItems), 1, tempfile);

   // write items
   for (int i = 0; i < totalitems; ++i)
      std::fwrite(Items[i].getItemArray(), Items[i].size(), 1, tempfile);

   std::fclose(tempfile);
}
//---------------------------------------------------------------------------
void Character::writeWaypoints()
{
   std::fseek(charfile, waypoints_location, SEEK_SET);

   for (int i = 0; i < NUM_OF_DIFFICULTY; ++i)
   {
      std::fwrite(Waypoints[i], sizeof(Waypoints[i]), 1, charfile);
      std::fseek(charfile, 16, SEEK_CUR);
   }
}
//---------------------------------------------------------------------------
void Character::findGems()
{
   Gems.clear();

//   if (Items[i].itemVersion() == V100_ITEM)  // v1.00 - v1.03
//   if (maskLSB(Version) == 0x40)        // pre-1.07 character file
//   {
      for (int i = 0; i < totalitems; ++i)
      {
         // v1.00 - v1.03
         if (Version < 0x50 && Items[i].itemVersion() == V100_ITEM)
         {
/*
            switch(Items[i][9])
            {
               case 0x11: switch (Items[i][8])
                          {
                             case 0x00: // stamina potion
                             case 0x10: // antidote potion
                             case 0x20: // rejuvenation potion
                             case 0x30: // full rejuvenation potion
                             case 0x40: // thawing potion
                                        Gems.push_back(i);
                                        break;
                          }
                          break;

               // strangling gas potion
//               case 0x12: if (Items[i][8] == 0x10)
//                             Gems.push_back(i);
//                          break;

               case 0x13: switch(Items[i][8])
                          {
                             // amethyst
                             case 0xC0:
                             case 0xD0:
                             case 0xE0:
                             case 0xF0: Gems.push_back(i);
                                        break;
                          }
                          break;

               case 0x14: switch(Items[i][8])
                          {
                             // perfect amethyst
                             case 0x00:

                             // topaz
                             case 0x10:
                             case 0x20:
                             case 0x30:
                             case 0x40:
                             case 0x50:

                             // sapphire
                             case 0x60:
                             case 0x70:
                             case 0x80:
                             case 0x90:
                             case 0xA0:

                             // emerald
                             case 0xB0:
                             case 0xC0:
                             case 0xD0:
                             case 0xE0:
                             case 0xF0: Gems.push_back(i);
                                        break;
                          }
                          break;

               case 0x15: switch(Items[i][8])
                          {
                             // ruby
                             case 0x00:
                             case 0x10:
                             case 0x20:
                             case 0x30:
                             case 0x40:

                             // diamond
                             case 0x50:
                             case 0x60:
                             case 0x70:
                             case 0x80:
                             case 0x90:

//                             case 0xC0: // healing potion

                             // minor mana potion
                             case 0xF0: Gems.push_back(i);
                                        break;
                          }
                          break;

               case 0x16: switch(Items[i][8])
                          {
                             case 0x00: // light mana potion
                             
                             // skull
                             case 0x40:
                             case 0x50:
                             case 0x60:
                             case 0x70:
                             case 0x80: Gems.push_back(i);
                                        break;
                          }
                          break;

               // healing potion
               case 0x95: switch(Items[i][8])
                          {
                             case 0xA0: // minor healing potion
                             case 0xB0: // light healing potion
                             case 0xC0: // healing potion
                             case 0xD0: // strong healing potion
                             case 0xE0: // greater healing potion
                                        Gems.push_back(i);
                                        break;
                          }
                          break;
            }
*/
//            char currentgem = (Items[i][8] >> 4 & 0x0F) | (Items[i][9] << 4);
            short currentgem = Items[i][9] & 0x1F;
            currentgem <<= 8;
            currentgem += Items[i][8];
            if (currentgem >= 0x1100 && currentgem <= 0x1140 ||
                currentgem >= 0x13C0 && currentgem <= 0x1680)
               Gems.push_back(i);
         }  // end if
//      }  // end for
//   }
   else              // v1.04 - v1.06, v1.07+
   {
      std::string item;
//      for (int i = 0; i < totalitems; ++i)
//      {
         if (!Items[i].isSimpleItem())
            continue;
/*
         long gem = calculateGemCode(i);
         char colour = gem >> 16 & 0x000000FF;
         char condition = gem >> 8 & 0x000000FF;
         char confirmcode = gem & 0x000000FF;

         switch(colour)
         {
            case 'v': // amethyst
                      switch(condition)
                      {
                         case 'c': // chipped
                         case 'f': // flawed
                         case 's': // standard
                         case 'z': // flawless -- the only special case
                         case 'p': // perfect
                                   if (confirmcode == 'g')
                                      Gems.push_back(i);
                                   break;
                      }
                      break;

            case 'b': // sapphire
            case 'g': // emerald
            case 'r': // ruby
            case 'w': // diamond
            case 'y': // topaz
                      switch(condition)
                      {
                         case 'c': // chipped
                         case 'f': // flawed
                         case 's': // standard
                         case 'l': // flawless
                         case 'p': // perfect
                                   if (confirmcode == 'g')
                                      Gems.push_back(i);
                                   break;
                      }
                      break;

            // skulls
            case 'c': // chipped
            case 'f': // flawed
            case 'l': // flawless
            case 'u': // regular
            case 'z': // perfect
                      if (condition == 'k' && confirmcode == 's')
                         Gems.push_back(i);
                      break;
         }

         // is this a rune or rejuvenation potion?
         if (confirmcode == 'r')
         {
            switch(condition)
            {
               case '0':
               case '1':
               case '2': if (colour >= '0' && colour <= '9')
                            Gems.push_back(i);
                         break;

               case '3': if (colour >= '0' && colour <= '3')
                            Gems.push_back(i);
                         break;

               case 'v': if (colour == 'l' || colour == 's')
                            Gems.push_back(i);
                         break;
            }
         }
         // is this a potion?
         else if ((confirmcode == 'h' || confirmcode == 'm') &&
                  condition == 'p')
         {
            if (colour >= '0' && colour <= '5')
               Gems.push_back(i);
         }
*/
//         std::string item = calculateGemCode(i);
         calculateGemCode(i, item);
         char gem = item[0];
         char gemcondition = item[1];
         char gemcolour = item[2];
//         long item = calculateGemCode(i);
//         char gem = item >> 24;
//         char gemcondition = item >> 16;
//         char gemcolour = item >> 8;

         switch(gem)
         {
            case 'g': switch(gemcondition)
                      {
                         case 'c': // chipped
                         case 'f': // flawed
                         case 's': // regular
                         case 'l': // flawless
                         case 'p': // perfect
                                   switch(gemcolour)
                                   {
                                      case 'v': // amethyst
                                                if (gemcondition != 'l')
                                                   Gems.push_back(i);
                                                break;

                                      case 'w': // diamond
                                      case 'g': // emerald
                                      case 'r': // ruby
                                      case 'b': // sapphire
                                      case 'y': // topaz
                                                Gems.push_back(i);
                                                break;
                                   }
                                   break;

                         case 'z': // flawless amethyst
                                   if (gemcolour == 'v')
                                      Gems.push_back(i);
                                   break;
                      }
                      break;

            case 's': // skulls
                      if (gemcondition == 'k')
                         switch(gemcolour)
                         {
                            case 'c': // chipped
                            case 'f': // flawed
                            case 'u': // regular
                            case 'l': // flawless
                            case 'z': // perfect
                                      Gems.push_back(i);
                                      break;
                         }
                      break;

            case 'r': // rejuvenation potions
                      if (gemcondition == 'v' &&
                          (gemcolour == 'l' || gemcolour == 's'))
                         Gems.push_back(i);
                      break;

            case 'h': // healing potions
            case 'm': // mana potions
                      if (gemcondition == 'p' &&
                          gemcolour >= '1' && gemcolour <= '5')
                         Gems.push_back(i);
                      break;

            case 'v': // stamina potions
            case 'y': // antidote potions
                      if (gemcondition == 'p' && gemcolour == 's')
                         Gems.push_back(i);
                      break;

            case 'w': // thawing potions
                      if (gemcondition == 'm' && gemcolour == 's')
                         Gems.push_back(i);
                      break;

         }
      }  // end for
   }  // end if
}
//---------------------------------------------------------------------------
int Character::getNumberOfGems()
{
   return Gems.size();
}
//---------------------------------------------------------------------------
void Character::calculateGemCode(int itemnumber, std::string& strcode)
{
   if (Items[itemnumber].itemVersion() == V104_ITEM)
   {
      unsigned long code;

      code = Items[itemnumber][13];
      code <<= 8;
      code += Items[itemnumber][12];
      code <<= 8;
      code += Items[itemnumber][11];
      code <<= 8;
      code += Items[itemnumber][10];
      code >>= 2;

      strcode[0] = code & 0x000000FF;
      strcode[1] = code >> 8;
      strcode[2] = code >> 16;
      strcode[3] = code >> 24;
   }
   else if (maskLSB(Version) == 0x50)
   {
      strcode[0] = (Items[itemnumber][9] >> 4 & 0x0F) | (Items[itemnumber][10] << 4);
      strcode[1] = (Items[itemnumber][10] >> 4 & 0x0F) | (Items[itemnumber][11] << 4);
      strcode[2] = (Items[itemnumber][11] >> 4 & 0x0F) | (Items[itemnumber][12] << 4);
      strcode[3] = (Items[itemnumber][12] >> 4 & 0x0F) | (Items[itemnumber][13] << 4);
   }
}
//---------------------------------------------------------------------------
/*
   Converts the specified original gems to the specified final gems.
   Returns the number of gems converted.

   This code is really messy.  I may clean it up in the future.
*/
int Character::convertGems(const GemTypes& gt)
{
   unsigned char oldgem, oldgemcondition, oldgemcolour,
                 gem, gemcondition, gemcolour;

//   if (maskLSB(Version) == 0x40)        // pre-1.07 character file
//   {
      // v1.00 - v1.03
//      if (Items[Gems[0]].size() == V100_ITEM_LENGTH)
      if (Version < 0x50 && Items[Gems[0]].itemVersion() == V100_ITEM)
      {
         if (gt.oldgem > -1)
         {
            switch((gt.oldgem & 0x0F) % 7)
            {
               // amethyst
               case 0: oldgem = 0x30;
                       switch(gt.oldgem >> 4)
                       {
                          case 0: oldgem |= 0x0C; break;
                          case 1: oldgem |= 0x0D; break;
                          case 2: oldgem |= 0x0E; break;
                          case 3: oldgem |= 0x0F; break;
                          case 4: oldgem = 0x40; break;
                       }
                       break;
               // diamond
               case 1: oldgem = 0x50;
                       switch(gt.oldgem >> 4)
                       {
                          case 0: oldgem |= 0x05; break;
                          case 1: oldgem |= 0x06; break;
                          case 2: oldgem |= 0x07; break;
                          case 3: oldgem |= 0x08; break;
                          case 4: oldgem |= 0x09; break;
                       }
                       break;

               // ruby
               case 3: oldgem = 0x50;
                       switch(gt.oldgem >> 4)
                       {
                          case 0: oldgem |= 0x00; break;
                          case 1: oldgem |= 0x01; break;
                          case 2: oldgem |= 0x02; break;
                          case 3: oldgem |= 0x03; break;
                          case 4: oldgem |= 0x04; break;
                       }
                       break;

               // emerald
               case 2: oldgem = 0x40;
                       switch(gt.oldgem >> 4)
                       {
                          case 0: oldgem |= 0x0B; break;
                          case 1: oldgem |= 0x0C; break;
                          case 2: oldgem |= 0x0D; break;
                          case 3: oldgem |= 0x0E; break;
                          case 4: oldgem |= 0x0F; break;
                       }
                       break;

               // sapphire
               case 4: oldgem = 0x40;
                       switch(gt.oldgem >> 4)
                       {
                          case 0: oldgem |= 0x06; break;
                          case 1: oldgem |= 0x07; break;
                          case 2: oldgem |= 0x08; break;
                          case 3: oldgem |= 0x09; break;
                          case 4: oldgem |= 0x0A; break;
                       }
                       break;

               // topaz
               case 6: oldgem = 0x40;
                       switch(gt.oldgem >> 4)
                       {
                          case 0: oldgem |= 0x01; break;
                          case 1: oldgem |= 0x02; break;
                          case 2: oldgem |= 0x03; break;
                          case 3: oldgem |= 0x04; break;
                          case 4: oldgem |= 0x05; break;
                       }
                       break;

               // skull
               case 5: oldgem = 0x60;
                       switch(gt.oldgem >> 4)
                       {
                          case 0: oldgem |= 0x04; break;
                          case 1: oldgem |= 0x05; break;
                          case 2: oldgem |= 0x06; break;
                          case 3: oldgem |= 0x07; break;
                          case 4: oldgem |= 0x08; break;
                       }
                       break;

            }  // end switch
         }
         else
         {
            if (gt.oldname.find("Rejuvenation") != std::string::npos)
            {
               if (gt.oldname.find("Full") != std::string::npos)
                  oldgem = 0x13;
               else
                  oldgem = 0x12;
            }
            else if (gt.oldname.find("Healing") != std::string::npos)
            {
               if (gt.oldname.find("Minor") != std::string::npos)
                  oldgem = 0x5A;
               else if (gt.oldname.find("Light") != std::string::npos)
                  oldgem = 0x5B;
               else if (gt.oldname.find("Greater") != std::string::npos)
                  oldgem = 0x5D;
               else if (gt.oldname.find("Super") != std::string::npos)
                  oldgem = 0x5E;
               else
                  oldgem = 0x5C;
            }
            else if (gt.oldname.find("Mana") != std::string::npos)
            {
               if (gt.oldname.find("Minor") != std::string::npos)
                  oldgem = 0x5F;
               else if (gt.oldname.find("Light") != std::string::npos)
                  oldgem = 0x60;
               else if (gt.oldname.find("Greater") != std::string::npos)
                  oldgem = 0x62;
               else if (gt.oldname.find("Super") != std::string::npos)
                  oldgem = 0x63;
               else
                  oldgem = 0x61;
            }
            else if (gt.oldname.find("Antidote") != std::string::npos)
            {
               oldgem = 0x11;
            }
            else if (gt.oldname.find("Stamina") != std::string::npos)
            {
               oldgem = 0x10;
            }
            else if (gt.oldname.find("Thawing") != std::string::npos)
            {
               oldgem = 0x14;
            }
         }

         if (gt.newgem > -1)
         {
            switch((gt.newgem & 0x0F) % 7)
            {
               // amethyst
               case 0: gem = 0x30;
                       switch(gt.newgem >> 4)
                       {
                          case 0: gem |= 0x0C; break;
                          case 1: gem |= 0x0D; break;
                          case 2: gem |= 0x0E; break;
                          case 3: gem |= 0x0F; break;
                          case 4: gem = 0x40; break;
                       }
                       break;
               // diamond
               case 1: gem = 0x50;
                       switch(gt.newgem >> 4)
                       {
                          case 0: gem |= 0x05; break;
                          case 1: gem |= 0x06; break;
                          case 2: gem |= 0x07; break;
                          case 3: gem |= 0x08; break;
                          case 4: gem |= 0x09; break;
                       }
                       break;

               // ruby
               case 3: gem = 0x50;
                       switch(gt.newgem >> 4)
                       {
                          case 0: gem |= 0x00; break;
                          case 1: gem |= 0x01; break;
                          case 2: gem |= 0x02; break;
                          case 3: gem |= 0x03; break;
                          case 4: gem |= 0x04; break;
                       }
                       break;

               // emerald
               case 2: gem = 0x40;
                       switch(gt.newgem >> 4)
                       {
                          case 0: gem |= 0x0B; break;
                          case 1: gem |= 0x0C; break;
                          case 2: gem |= 0x0D; break;
                          case 3: gem |= 0x0E; break;
                          case 4: gem |= 0x0F; break;
                       }
                       break;

               // sapphire
               case 4: gem = 0x40;
                       switch(gt.newgem >> 4)
                       {
                          case 0: gem |= 0x06; break;
                          case 1: gem |= 0x07; break;
                          case 2: gem |= 0x08; break;
                          case 3: gem |= 0x09; break;
                          case 4: gem |= 0x0A; break;
                       }
                       break;

               // topaz
               case 6: gem = 0x40;
                       switch(gt.newgem >> 4)
                       {
                          case 0: gem |= 0x01; break;
                          case 1: gem |= 0x02; break;
                          case 2: gem |= 0x03; break;
                          case 3: gem |= 0x04; break;
                          case 4: gem |= 0x05; break;
                       }
                       break;

               // skull
               case 5: gem = 0x60;
                       switch(gt.newgem >> 4)
                       {
                          case 0: gem |= 0x04; break;
                          case 1: gem |= 0x05; break;
                          case 2: gem |= 0x06; break;
                          case 3: gem |= 0x07; break;
                          case 4: gem |= 0x08; break;
                       }
                       break;

            }  // end switch
         }
         else
         {
            if (gt.newname.find("Rejuvenation") != std::string::npos)
            {
               if (gt.newname.find("Full") != std::string::npos)
                  gem = 0x13;
               else
                  gem = 0x12;
            }
            else if (gt.newname.find("Healing") != std::string::npos)
            {
               if (gt.newname.find("Minor") != std::string::npos)
                  gem = 0x5A;
               else if (gt.newname.find("Light") != std::string::npos)
                  gem = 0x5B;
               else if (gt.newname.find("Greater") != std::string::npos)
                  gem = 0x5D;
               else if (gt.newname.find("Super") != std::string::npos)
                  gem = 0x5E;
               else
                  gem = 0x5C;
            }
            else if (gt.newname.find("Mana") != std::string::npos)
            {
               if (gt.newname.find("Minor") != std::string::npos)
                  gem = 0x5F;
               else if (gt.newname.find("Light") != std::string::npos)
                  gem = 0x60;
               else if (gt.newname.find("Greater") != std::string::npos)
                  gem = 0x62;
               else if (gt.newname.find("Super") != std::string::npos)
                  gem = 0x63;
               else
                  gem = 0x61;
            }
            else if (gt.newname.find("Antidote") != std::string::npos)
            {
               gem = 0x11;
            }
            else if (gt.newname.find("Stamina") != std::string::npos)
            {
               gem = 0x10;
            }
            else if (gt.newname.find("Thawing") != std::string::npos)
            {
               gem = 0x14;
            }
         }
      }  // end if size() test
//   }

   // v1.04 - v1.06, v1.07+
   else
   {
      // see if we're dealing with gems or skulls
      if (gt.oldgem > -1)
      {
         oldgem = 'g';

         switch(gt.oldgem >> 4)
         {
            case 0: oldgemcondition = 'c'; break; // chipped
            case 1: oldgemcondition = 'f'; break; // flawed
            case 2: oldgemcondition = 's'; break; // regular
            case 3: oldgemcondition = 'l'; break; // flawless
            case 4: oldgemcondition = 'p'; break; // perfect
         }

         switch((gt.oldgem & 0x0F) % 7)
         {
            case 0: oldgemcolour = 'v';        // amethyst
                    // take into account flawless amethyst
                    if (oldgemcondition == 'l')
                       oldgemcondition = 'z';
                    break;
            case 1: oldgemcolour = 'w'; break; // diamond
            case 2: oldgemcolour = 'g'; break; // emerald
            case 3: oldgemcolour = 'r'; break; // ruby
            case 4: oldgemcolour = 'b'; break; // sapphire
            case 5: oldgem = 's';               // skulls
                    if (oldgemcondition == 'p')
                       oldgemcondition = 'z';
                    else if (oldgemcondition == 's')
                       oldgemcondition = 'u';
                    break;
            case 6: oldgemcolour = 'y'; break; // topaz
         }
      }
      else  // runes or potions selected
      {
         if (gt.oldname.find("Rejuvenation") != std::string::npos)
         {
            oldgem = 'r';
            if (gt.oldname.find("Full") != std::string::npos)
               oldgemcondition = 'l';
            else
               oldgemcondition = 's';
         }
         else if (gt.oldname.find("Healing") != std::string::npos)
         {
            oldgem = 'h';
            if (gt.oldname.find("Minor") != std::string::npos)
               oldgemcondition = '1';
            else if (gt.oldname.find("Light") != std::string::npos)
               oldgemcondition = '2';
            else if (gt.oldname.find("Greater") != std::string::npos)
               oldgemcondition = '4';
            else if (gt.oldname.find("Super") != std::string::npos)
               oldgemcondition = '5';
            else
               oldgemcondition = '3';
         }
         else if (gt.oldname.find("Mana") != std::string::npos)
         {
            oldgem = 'm';
            if (gt.oldname.find("Minor") != std::string::npos)
               oldgemcondition = '1';
            else if (gt.oldname.find("Light") != std::string::npos)
               oldgemcondition = '2';
            else if (gt.oldname.find("Greater") != std::string::npos)
               oldgemcondition = '4';
            else if (gt.oldname.find("Super") != std::string::npos)
               oldgemcondition = '5';
            else
               oldgemcondition = '3';
         }
         else if (gt.oldname.find("Antidote") != std::string::npos)
         {
            oldgem = 'y';
            oldgemcondition = 'p';
            oldgemcolour = 's';
         }
         else if (gt.oldname.find("Stamina") != std::string::npos)
         {
            oldgem = 'v';
            oldgemcondition = 'p';
            oldgemcolour = 's';
         }
         else if (gt.oldname.find("Thawing") != std::string::npos)
         {
            oldgem = 'w';
            oldgemcondition = 'm';
            oldgemcolour = 's';
         }
      }

      // see if we're dealing with gems or skulls
      if (gt.newgem > -1)
      {
         gem = 'g';

         switch(gt.newgem >> 4)
         {
            case 0: gemcondition = 'c'; break; // chipped
            case 1: gemcondition = 'f'; break; // flawed
            case 2: gemcondition = 's'; break; // regular
            case 3: gemcondition = 'l'; break; // flawless
            case 4: gemcondition = 'p'; break; // perfect
         }

         switch((gt.newgem & 0x0F) % 7)
         {
            case 0: gemcolour = 'v';
                    if (gemcondition == 'l')
                       gemcondition = 'z';
                    break;
            case 1: gemcolour = 'w'; break;
            case 2: gemcolour = 'g'; break;
            case 3: gemcolour = 'r'; break;
            case 4: gemcolour = 'b'; break;
            case 5: gem = 's';                  // skulls
                    if (gemcondition == 'p')
                       gemcondition = 'z';
                    else if (gemcondition == 's')
                       gemcondition = 'u';
                    break;
            case 6: gemcolour = 'y'; break;
         }
      }
      else  // runes or potions selected
      {
         if (gt.newname.find("Rejuvenation") != std::string::npos)
         {
            gem = 'r';
            if (gt.newname.find("Full") != std::string::npos)
               gemcondition = 'l';
            else
               gemcondition = 's';
         }
         else if (gt.newname.find("Healing") != std::string::npos)
         {
            gem = 'h';
            if (gt.newname.find("Minor") != std::string::npos)
               gemcondition = '1';
            else if (gt.newname.find("Light") != std::string::npos)
               gemcondition = '2';
            else if (gt.newname.find("Greater") != std::string::npos)
               gemcondition = '4';
            else if (gt.newname.find("Super") != std::string::npos)
               gemcondition = '5';
            else
               gemcondition = '3';
         }
         else if (gt.newname.find("Mana") != std::string::npos)
         {
            gem = 'm';
            if (gt.newname.find("Minor") != std::string::npos)
               gemcondition = '1';
            else if (gt.newname.find("Light") != std::string::npos)
               gemcondition = '2';
            else if (gt.newname.find("Greater") != std::string::npos)
               gemcondition = '4';
            else if (gt.newname.find("Super") != std::string::npos)
               gemcondition = '5';
            else
               gemcondition = '3';
         }
         else if (gt.newname.find("Antidote") != std::string::npos)
         {
            gem = 'y';
            gemcondition = 'p';
            gemcolour = 's';
         }
         else if (gt.newname.find("Stamina") != std::string::npos)
         {
            gem = 'v';
            gemcondition = 'p';
            gemcolour = 's';
         }
         else if (gt.newname.find("Thawing") != std::string::npos)
         {
            gem = 'w';
            gemcondition = 'm';
            gemcolour = 's';
         }
      }
   }

   char currentgem, currentgemcondition, currentgemcolour;
   int gemsconverted = 0;
   std::string temp;
   std::string desireditem;

   if (maskLSB(Version) == 0x40)        // pre-1.07 character file
   {
      for (unsigned int i = 0; i < Gems.size(); ++i)
      {
         // v1.00 - v1.03
         if (Items[Gems[i]].itemVersion() == V100_ITEM)
         {
            currentgem = (Items[Gems[i]][8] >> 4 & 0x0F) | (Items[Gems[i]][9] << 4);
            if (currentgem == oldgem)
            {
               // want to convert to gem/skull
               if (gem >= 0x3C && gem <= 0x59 || gem >= 0x64 && gem <= 0x68)
               {
                  // see if original item was a potion
                  if (oldgem >= 0x10 && oldgem <= 0x14 ||
                      oldgem >= 0x5A && oldgem <= 0x63)
                  {
                     // only convert if potion is not in belt
                     if ((Items[Gems[i]][9] & 0x80) == 0x80 &&
                         (Items[Gems[i]][10] & 0x01) == 0x00)
                        continue;
                  }
               }
               // want to convert to potions
               else
               {
                  // see if original item was a gem/skull
                  if (oldgem >= 0x3C && oldgem <= 0x59 ||
                      oldgem >= 0x64 && oldgem <= 0x68)
                  {
                     // only convert if gem/skull not in item
                     if ((Items[Gems[i]][9] & 0x80) == 0x80 &&
                         (Items[Gems[i]][10] & 0x01) == 0x01)
                        continue;
                  }
               }

               Items[Gems[i]][8] = (gem << 4);
               Items[Gems[i]][9] = (gem >> 4 & 0x0F) | 0x10;

               ++gemsconverted;
            }  // end if (found gem to change)
         }

         // v1.04 - v1.06
         else
         {
            calculateGemCode(Gems[i], temp);
            currentgem = temp[0];
            currentgemcondition = temp[1];
            currentgemcolour = temp[2];

            long code = 0x20;
            code <<= 8;

            // want to change skulls
            if (oldgem == 's' && currentgem == 's' && currentgemcondition == 'k' &&
                currentgemcolour == oldgemcondition)
            {
               // just changing skull condition
               if (gem == 's')
               {
                  code += gemcondition;
                  code <<= 8;
                  code += 'k';
                  code <<= 8;
                  code += gem;
               }
               // want rejuvenation potion
               else if (gem == 'r')
               {
                  short location = Items[Gems[i]][8];
                  location <<= 8;
                  location += Items[Gems[i]][9];

                  // only change if potion is not glued in item
                  if ((location & 0x03FC)== 0x03FC)
                     continue;

                  code += gemcondition;
                  code <<= 8;
                  code += 'v';
                  code <<= 8;
                  code += gem;
               }
               // want healing/mana potion
               else if (gem == 'h' || gem == 'm')
               {
                  short location = Items[Gems[i]][8];
                  location <<= 8;
                  location += Items[Gems[i]][9];

                  // only change if potion is not glued in item
                  if ((location & 0x03FC)== 0x03FC)
                     continue;

                  code += gemcondition;
                  code <<= 8;
                  code += 'p';
                  code <<= 8;
                  code += gem;
               }
               else if (gem == 'v' || gem == 'y' || gem == 'w')
               {
                  short location = Items[Gems[i]][8];
                  location <<= 8;
                  location += Items[Gems[i]][9];

                  // only change if potion is not glued in item
                  if ((location & 0x03FC)== 0x03FC)
                     continue;

                  code += gemcolour;
                  code <<= 8;
                  code += gemcondition;
                  code <<= 8;
                  code += gem;
               }
               else
               {
                  code += gemcolour;
                  code <<= 8;
                  code += gemcondition;
                  code <<= 8;
                  code += 'g';
               }

               code <<= 2;

               desireditem[0] = code & 0x000000FF;
               desireditem[1] = code >> 8;
               desireditem[2] = code >> 16;
               desireditem[3] = code >> 24;

               updateGem(Gems[i], desireditem);

               ++gemsconverted;
            }

            else if (oldgem == 'r' && currentgem == 'r' &&
                     currentgemcondition == 'v' && currentgemcolour == oldgemcondition)
            {
               // want to convert to skulls
               if (gem == 's')
               {
                  short location = Items[Gems[i]][8];
                  location <<= 8;
                  location += Items[Gems[i]][9];

                  // only change if potion is not in belt
                  if ((location & 0x03FC) == 0x01FC)
                     continue;

                  code += gemcondition;
                  code <<= 8;
                  code += 'k';
                  code <<= 8;
                  code += gem;
               }
               // just changing rejuvenation potion quality
               else if (gem == 'r')
               {
                  code += gemcondition;
                  code <<= 8;
                  code += 'v';
                  code <<= 8;
                  code += gem;
               }
               // healing or mana potion desired
               else if (gem == 'h' || gem == 'm')
               {
                  code += gemcondition;
                  code <<= 8;
                  code += 'p';
                  code <<= 8;
                  code += gem;
               }
               else if (gem == 'v' || gem == 'y' || gem == 'w')
               {
                  code += 's';
                  code <<= 8;
                  code += gemcondition;
                  code <<= 8;
                  code += gem;
               }
               else
               {
                  short location = Items[Gems[i]][8];
                  location <<= 8;
                  location += Items[Gems[i]][9];

                  // only change if potion is not in belt
                  if ((location & 0x03FC) == 0x01FC)
                     continue;

                  code += gemcolour;
                  code <<= 8;
                  code += gemcondition;
                  code <<= 8;
                  code += gem;
               }

               code <<= 2;

               desireditem[0] = code & 0x000000FF;
               desireditem[1] = code >> 8;
               desireditem[2] = code >> 16;
               desireditem[3] = code >> 24;

               updateGem(Gems[i], desireditem);

               ++gemsconverted;
            }

            else if ((oldgem == 'h' || oldgem == 'm') &&
                     (currentgem == 'h' || currentgem == 'm') &&
                     currentgemcondition == 'p' && currentgemcolour == oldgemcondition)
            {
               // want to convert to skulls
               if (gem == 's')
               {
                  short location = Items[Gems[i]][8];
                  location <<= 8;
                  location += Items[Gems[i]][9];

                  // only change if potion is not in belt
                  if ((location & 0x03FC) == 0x01FC)
                     continue;

                  code += gemcondition;
                  code <<= 8;
                  code += 'k';
                  code <<= 8;
                  code += gem;
               }
               // just changing rejuvenation potion quality
               else if (gem == 'r')
               {
                  code += gemcondition;
                  code <<= 8;
                  code += 'v';
                  code <<= 8;
                  code += gem;
               }
               // healing or mana potion desired
               else if (gem == 'h' || gem == 'm')
               {
                  code += gemcondition;
                  code <<= 8;
                  code += 'p';
                  code <<= 8;
                  code += gem;
               }
               else if (gem == 'v' || gem == 'y' || gem == 'w')
               {
                  code += 's';
                  code <<= 8;
                  code += gemcondition;
                  code <<= 8;
                  code += gem;
               }
               else
               {
                  short location = Items[Gems[i]][8];
                  location <<= 8;
                  location += Items[Gems[i]][9];

                  // only change if potion is not in belt
                  if ((location & 0x03FC) == 0x01FC)
                     continue;

                  code += gemcolour;
                  code <<= 8;
                  code += gemcondition;
                  code <<= 8;
                  code += gem;
               }

               code <<= 2;

               desireditem[0] = code & 0x000000FF;
               desireditem[1] = code >> 8;
               desireditem[2] = code >> 16;
               desireditem[3] = code >> 24;

               updateGem(Gems[i], desireditem);

               ++gemsconverted;
            }

            else if ((oldgem == 'v' || oldgem == 'y' || oldgem == 'w') &&
                     (currentgem == 'v' || currentgem == 'y' == currentgem == 'w') &&
                     currentgemcondition == oldgemcondition && currentgemcolour == 's')
            {
               // want to convert to skulls
               if (gem == 's')
               {
                  short location = Items[Gems[i]][8];
                  location <<= 8;
                  location += Items[Gems[i]][9];

                  // only change if potion is not in belt
                  if ((location & 0x03FC) == 0x01FC)
                     continue;

                  code += gemcondition;
                  code <<= 8;
                  code += 'k';
                  code <<= 8;
                  code += gem;
               }
               // just changing rejuvenation potion quality
               else if (gem == 'r')
               {
                  code += gemcondition;
                  code <<= 8;
                  code += 'v';
                  code <<= 8;
                  code += gem;
               }
               // healing or mana potion desired
               else if (gem == 'h' || gem == 'm')
               {
                  code += gemcondition;
                  code <<= 8;
                  code += 'p';
                  code <<= 8;
                  code += gem;
               }
               else if (gem == 'v' || gem == 'y' || gem == 'w')
               {
                  code += 's';
                  code <<= 8;
                  code += gemcondition;
                  code <<= 8;
                  code += gem;
               }
               else
               {
                  short location = Items[Gems[i]][8];
                  location <<= 8;
                  location += Items[Gems[i]][9];

                  // only change if potion is not in belt
                  if ((location & 0x03FC) == 0x01FC)
                     continue;

                  code += gemcolour;
                  code <<= 8;
                  code += gemcondition;
                  code <<= 8;
                  code += gem;
               }

               code <<= 2;

               desireditem[0] = code & 0x000000FF;
               desireditem[1] = code >> 8;
               desireditem[2] = code >> 16;
               desireditem[3] = code >> 24;

               updateGem(Gems[i], desireditem);

               ++gemsconverted;
            }

            // want to change other gems
            else if (currentgem == oldgem &&
                     currentgemcondition == oldgemcondition &&
                     currentgemcolour == oldgemcolour)
            {
               // want to convert to skulls
               if (gem == 's')
               {
                  code += gemcondition;
                  code <<= 8;
                  code += 'k';
                  code <<= 8;
                  code += gem;
               }
               else if (gem == 'r')
               {
                  short location = Items[Gems[i]][8];
                  location <<= 8;
                  location += Items[Gems[i]][9];

                  // only change if potion is not glued in item
                  if ((location & 0x03FC)== 0x03FC)
                     continue;

                  code += gemcondition;
                  code <<= 8;
                  code += 'v';
                  code <<= 8;
                  code += gem;
               }
               // healing or mana potion desired
               else if (gem == 'h' || gem == 'm')
               {
                  short location = Items[Gems[i]][8];
                  location <<= 8;
                  location += Items[Gems[i]][9];
                  // only change if potion is not glued in item
                  if ((location & 0x03FC)== 0x03FC)
                     continue;

                  code += gemcondition;
                  code <<= 8;
                  code += 'p';
                  code <<= 8;
                  code += gem;
               }
               else if (gem == 'v' || gem == 'y' || gem == 'w')
               {
                  short location = Items[Gems[i]][8];
                  location <<= 8;
                  location += Items[Gems[i]][9];

                  // only change if potion is not glued in item
                  if ((location & 0x03FC)== 0x03FC)
                     continue;

                  code += 's';
                  code <<= 8;
                  code += gemcondition;
                  code <<= 8;
                  code += gem;
               }
               else
               {
                  code += gemcolour;
                  code <<= 8;
                  code += gemcondition;
                  code <<= 8;
                  code += 'g';
               }

               code <<= 2;

               desireditem[0] = code & 0x000000FF;
               desireditem[1] = code >> 8;
               desireditem[2] = code >> 16;
               desireditem[3] = code >> 24;

               updateGem(Gems[i], desireditem);

               ++gemsconverted;
            }
         }
      }  // end for
   }
      // 1.07+ character file including expansion set characters
   else if (maskLSB(Version) == 0x50)
   {
//      std::string temp;
//      std::string desireditem;

      for (unsigned int i = 0; i < Gems.size(); ++i)
      {
         calculateGemCode(Gems[i], temp);
         currentgem = temp[0];
         currentgemcondition = temp[1];
         currentgemcolour = temp[2];

         // want to change skulls
         if (oldgem == 's' && currentgem == 's' &&
             currentgemcondition == 'k' && currentgemcolour == oldgemcondition)
         {
            desireditem[0] = gem;

            // just changing skull condition
            if (gem == 's')
            {
               desireditem[1] = 'k';
               desireditem[2] = gemcondition;
            }
            // rejuvenation potion desired
            else if (gem == 'r')
            {
               char location = Items[Gems[i]][7] >> 2 & 0x00000007;
               // only change if item is not glued in a socket
               if (location == GLUED_IN_SOCKET)
                  continue;

               desireditem[1] = 'v';
               desireditem[2] = gemcondition;
            }
            // healing or mana potion desired
            else if (gem == 'h' || gem == 'm')
            {
               char location = Items[Gems[i]][7] >> 2 & 0x00000007;
               // only change if item is not glued in a socket
               if (location == GLUED_IN_SOCKET)
                  continue;

               desireditem[1] = 'p';
               desireditem[2] = gemcondition;
            }
            else if (gem == 'v' || gem == 'y' || gem == 'w')
            {
               char location = Items[Gems[i]][7] >> 2 & 0x00000007;
               // only change if item is not glued in a socket
               if (location == GLUED_IN_SOCKET)
                  continue;

//               if (gem == 'w')
//                  desireditem[1] == 'm';
//               else
//                  desireditem[1] = 'p';

//               desireditem[2] = 's';
               desireditem[1] = gemcondition;
               desireditem[2] = gemcolour;
            }
            else
            {
               desireditem[1] = gemcondition;
               desireditem[2] = gemcolour;
            }

            desireditem[3] = 0x20;
            updateGem(Gems[i], desireditem);

            ++gemsconverted;
         }

         // want to change rejuvenation potions
         else if (oldgem == 'r' && currentgem == 'r' &&
                  currentgemcondition == 'v' && currentgemcolour == oldgemcondition)
         {
            desireditem[0] = gem;

            // want to convert to skulls
            if (gem == 's')
            {
               char location = Items[Gems[i]][7] >> 2 & 0x00000007;
               // only change if potion is not in belt
               if (location == BELT)
                  continue;

               desireditem[1] = 'k';
               desireditem[2] = gemcondition;
            }
            // just changing rejuvenation potion quality
            else if (gem == 'r')
            {
               desireditem[1] = 'v';
               desireditem[2] = gemcondition;
            }
            // healing or mana potion desired
            else if (gem == 'h' || gem == 'm')
            {
               desireditem[1] = 'p';
               desireditem[2] = gemcondition;
            }
            else if (gem == 'v' || gem == 'y' || gem == 'w')
            {
               desireditem[1] = gemcondition;
               desireditem[2] = gemcolour;
            }
            else
            {
               char location = Items[Gems[i]][7] >> 2 & 0x00000007;
               // only change if potion is not in belt
               if (location == BELT)
                  continue;

               desireditem[1] = gemcondition;
               desireditem[2] = gemcolour;
            }

            desireditem[3] = 0x20;
            updateGem(Gems[i], desireditem);

            ++gemsconverted;
         }

         // want to change healing or mana potions
         else if ((oldgem == 'h' || oldgem == 'm') &&
                  (currentgem == 'h' || currentgem == 'm') &&
                  currentgemcondition == 'p' && currentgemcolour == oldgemcondition)
         {
            desireditem[0] = gem;

            // want to convert to skulls
            if (gem == 's')
            {
               char location = Items[Gems[i]][7] >> 2 & 0x00000007;
               // only change if potion is not in belt
               if (location == BELT)
                  continue;

               desireditem[1] = 'k';
               desireditem[2] = gemcondition;
            }
            // want rejuvenation potion
            else if (gem == 'r')
            {
               desireditem[1] = 'v';
               desireditem[2] = gemcondition;
            }
            // just changing healing or mana potion quality
            else if (gem == 'h' || gem == 'm')
            {
               desireditem[1] = 'p';
               desireditem[2] = gemcondition;
            }
            else if (gem == 'v' || gem == 'y' || gem == 'w')
            {
               desireditem[1] = gemcondition;
               desireditem[2] = gemcolour;
            }
            else
            {
               char location = Items[Gems[i]][7] >> 2 & 0x00000007;
               // only change if potion is not in belt
               if (location == BELT)
                  continue;

               desireditem[1] = gemcondition;
               desireditem[2] = gemcolour;
            }

            desireditem[3] = 0x20;
            updateGem(Gems[i], desireditem);

            ++gemsconverted;
         }

         // antidote, stamina, or thawing potion
         else if ((oldgem == 'y' || oldgem == 'v' || oldgem == 'w') &&
                  (currentgem == 'y' || currentgem == 'v' || currentgem == 'w') &&
                  currentgemcondition == oldgemcondition && currentgemcolour == 's')
         {
            desireditem[0] = gem;

            if (gem == 's')
            {
               char location = Items[Gems[i]][7] >> 2 & 0x00000007;
               // only change if potion is not in belt
               if (location == BELT)
                  continue;

               desireditem[1] = 'k';
               desireditem[2] = gemcondition;
            }
            else if (gem == 'r')
            {
               desireditem[1] = 'v';
               desireditem[2] = gemcondition;
            }
            else if (gem == 'h' || gem == 'm')
            {
               desireditem[1] = 'p';
               desireditem[2] = gemcondition;
            }
            else if (gem == 'w' || gem == 'v' || gem == 'y')
            {
//               if (gem == 'w')
//                  desireditem[1] = 'p';
//               else
//                  desireditem[1] = 'm';

//               desireditem[2] = 's';
               desireditem[1] = gemcondition;
               desireditem[2] = gemcolour;
            }
            else
            {
               char location = Items[Gems[i]][7] >> 2 & 0x00000007;
               // only change if potion is not in belt
               if (location == BELT)
                  continue;

               desireditem[1] = gemcondition;
               desireditem[2] = gemcolour;
            }

            desireditem[3] = 0x20;
            updateGem(Gems[i], desireditem);

            ++gemsconverted;
         }

         // want to change other gems (i.e. non-skulls)
//         else if (currentgem == oldgem)
         else if (currentgem == oldgem && currentgemcondition == oldgemcondition &&
                  currentgemcolour == oldgemcolour)
         {
            desireditem[0] = gem;

//            if (currentgemcondition == oldgemcondition)
//            {
               // want to convert to skulls
               if (gem == 's')
               {
                  desireditem[1] = 'k';
                  desireditem[2] = gemcondition;
               }
               // rejuvenation potion desired
               else if (gem == 'r')
               {
                  char location = Items[Gems[i]][7] >> 2 & 0x00000007;
                  // only change if gem is not socketed
                  if (location == GLUED_IN_SOCKET)
                     continue;

                  desireditem[1] = 'v';
                  desireditem[2] = gemcondition;
               }
               // healing or mana potion desired
               else if (gem == 'h' || gem == 'm')
               {
                  char location = Items[Gems[i]][7] >> 2 & 0x00000007;
                  // only change if gem is not socketed
                  if (location == GLUED_IN_SOCKET)
                     continue;

                  desireditem[1] = 'p';
                  desireditem[2] = gemcondition;
               }
               else if (gem == 'v' || gem == 'y' || gem == 'w')
               {
                  char location = Items[Gems[i]][7] >> 2 & 0x00000007;
                  // only change if gem is not socketed
                  if (location == GLUED_IN_SOCKET)
                     continue;

                  desireditem[1] = gemcondition;
                  desireditem[2] = gemcolour;
               }
               else  // some other gem desired
               {
                  desireditem[1] = gemcondition;
                  desireditem[2] = gemcolour;
               }

               desireditem[3] = 0x20;
               updateGem(Gems[i], desireditem);

               ++gemsconverted;
//            }
         }
      }  // end for
   }  // end if

   return gemsconverted;
}
//---------------------------------------------------------------------------
void Character::updateGem(const itemnumber, const std::string& newgem)
{
   if (Items[itemnumber].itemVersion() == V104_ITEM)
   {
      Items[itemnumber][10] = newgem[0];
      Items[itemnumber][11] = newgem[1];
      Items[itemnumber][12] = newgem[2];
      Items[itemnumber][13] = newgem[3];
   }
   else if (maskLSB(Version) == 0x50)
   {
      Items[itemnumber][9] = (Items[itemnumber][9] & 0x0F) | (newgem[0] << 4);
      Items[itemnumber][10] = (newgem[0] >> 4 & 0x0F) | (newgem[1] << 4);
      Items[itemnumber][11] = (newgem[1] >> 4 & 0x0F) | (newgem[2] << 4);
      Items[itemnumber][12] = (newgem[2] >> 4 & 0x0F) | (newgem[3] << 4);
      Items[itemnumber][13] |= (newgem[3] >> 4 & 0x0F);
   }
}
//---------------------------------------------------------------------------

