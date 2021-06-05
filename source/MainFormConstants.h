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

#ifndef MainFormConstantsH
#define MainFormConstantsH

TColor EDITED_COLOUR = clAqua;
TColor NORMAL_COLOUR = clWindow;

AnsiString HINTTEXT = "Maximum value is ";
AnsiString MAXEXP_TEXT = "3,600,000,000";

// character class
enum {Amazon, Sorceress, Necromancer, Paladin, Barbarian, Druid, Assassin};

const int MAX_BASICSTATS = 50000;            // strength, dexterity, vitality,
                                             // energy max value
const unsigned long MAX_EXPERIENCE = 3600000000;       // experience max value
const int MAX_LMS = 5000;                    // life, mana, stamina max value

// titles for regular non-expansion set characters
static const char *FemaleTitle[NUM_OF_TITLES] = {"None", "Dame", "Lady", "Baroness"},
                  *MaleTitle[NUM_OF_TITLES] = {"None", "Sir", "Lord", "Baron"};

// titles for regular expansion set characters
static const char *FemaleExpansionTitle[NUM_OF_TITLES] = {"None", "Slayer", "Champion", "Matriarch"},
                  *MaleExpansionTitle[NUM_OF_TITLES] = {"None", "Slayer", "Champion", "Patriarch"};

// titles for hardcore non-expansion set characters
static const char *FemaleHardcoreTitle[NUM_OF_TITLES] = {"None", "Countess", "Duchess", "Queen"},
                  *MaleHardcoreTitle[NUM_OF_TITLES] = {"None", "Count", "Duke", "King"};

// titles for hardcore expansion set characters
static const char *HardcoreExpansionTitle[NUM_OF_TITLES] = {"None", "Destroyer", "Conqueror", "Guardian"};

#endif

