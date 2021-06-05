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

#ifndef DataTypesH
#define DataTypesH

#include <string>

struct BasicStats
{
   char Name[NAME_LENGTH];
   char Status;
   char Title;
   char Class;
   char DifficultyLastPlayed;
   char StartingAct;
};

struct CharacterStats
{
   long Strength;
   long Energy;
   long Dexterity;
   long Vitality;
   long CurLife;
   long MaxLife;
   long CurMana;
   long MaxMana;
   long CurStamina;
   long MaxStamina;
   long Level;
   unsigned long Experience;
   long GoldInBelt;
   long GoldInStash;
};

struct ActInfo
{
   unsigned short Intro[NUM_OF_ACTS];              // has player read/heard intro
   unsigned short Quests[NUM_OF_ACTS][NUM_OF_QUESTS]; // actual quests
   unsigned short Completed[NUM_OF_ACTS - 1];
};

struct GemTypes
{
   // runes fields
   char original, desired;

   // gems and skulls fields
   // set oldgem and/or newgem to -1 to indicate runes
   char oldgem, oldgemcondition;
   char newgem, newgemcondition;
   std::string oldname, newname;

   GemTypes()
   {
      original = desired = 
      oldgem = oldgemcondition =
      newgem = newgemcondition = -1;
   }
};

#endif

