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

#ifndef ItemConstantsH
#define ItemConstantsH

// zero-indexed byte positions of various fields
const int ID_POS = 2;               // ID field
const int SIMPLE_POS = 4;           // simple item field
const int SOCKETED_POS = 3;         // socketed field
const int SOCKETED_COUNT_POS = 13;  // # of socketed items

// values that are found in the various item fields
const char BELT = 0x02;
const char GLUED_IN_SOCKET = 0x06;

const char ITEM_MARKER[] = {0x4A, 0x4D};  // alternatively "JM"
const int SIMPLE_ITEM_LENGTH = 14;        // # of bytes for simple items (v1.09+ only)
const int V100_ITEM_LENGTH = 27;
const int V104_GEM_LENGTH = 15;
const int V107_GEM_LENGTH = 14;           // gem length for v1.07+ characters

const int V100_ITEM = 100;          // v1.00 - v1.03 item
const int V104_ITEM = 104;          // v1.04 - v1.06 item

#endif

