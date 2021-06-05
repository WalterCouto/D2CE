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
#ifndef ConstantsH
#define ConstantsH

// character status
enum {NoDeaths, Hardcore = 4, Resurrected = 8, Dead = 0x0C};

// character title
enum {None, SirDame = 4, Slayer = 5, LordLady = 8, Champion = 0x0A,
      BaronBaroness = 0x0C, MPatriarch = 0x0F};

const int HEADER_LENGTH = 4;
const int MAX_SKILL_VALUE = 20;
const int NAME_LENGTH = 16;       // character name including terminating NULL

// difficulty levels
const int NORMAL = 0;
const int NIGHTMARE = 1;
const int HELL = 2;

const int NUM_OF_ACTS = 5;
const int NUM_OF_CLASSES = 7;
const int NUM_OF_DIFFICULTY = 3;
const int NUM_OF_QUESTS = 6;
const int NUM_OF_SKILLS = 30;
const int NUM_OF_TITLES = 4;
const int NUM_OF_WP_SETS = 2;                // # of waypoints sets

const int VERSION_109 = 0x5C;

// error codes
const int INVALID_HEADER = 576;
const int CANNOT_OPEN_FILE = 584;

#endif

