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

#ifndef CharacterConstantsH
#define CharacterConstantsH

const int EXPANSION_CHARACTER = 0x20;
const unsigned char HEADER[] = {0x55, 0xAA, 0x55, 0xAA};
//const char ITEM_MARKER[] = {0x4A, 0x4D};     // alternatively "JM"
const long MIN_START_POS = 48;
const char SKILLS_MARKER[] = {0x69, 0x66};   // alternatively "if"
const char STATS_MARKER[] = {0x69, 0x67};    // alternatively "ig"

#endif

