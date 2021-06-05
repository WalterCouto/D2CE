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

#include <cstdio>
#include "Item.h"
#include "ItemConstants.h"
//---------------------------------------------------------------------------

#pragma package(smart_init)
//---------------------------------------------------------------------------
Item::Item(): num_of_bytes(-1), data(NULL)
{
}
//---------------------------------------------------------------------------
Item::Item(int itemsize): num_of_bytes(itemsize), data(new unsigned char(itemsize))
{
}
//---------------------------------------------------------------------------
Item::~Item()
{
   num_of_bytes = -1;
   delete[] data;
   data = NULL;
}
//---------------------------------------------------------------------------
unsigned char& Item::operator [](int position)
{
   return data[position];
}
//---------------------------------------------------------------------------
const unsigned char& Item::operator [](int position) const
{
   return data[position];
}
//---------------------------------------------------------------------------
unsigned char *Item::getItemArray()
{
   return data;
}
//---------------------------------------------------------------------------
bool Item::isIdentified() const
{
   if ((data[ID_POS] & 0x10) == 0x10)
      return true;

   return false;
}
//---------------------------------------------------------------------------
/*
   Returns true if this item is a simple item (i.e. 14 byte item)
*/
bool Item::isSimpleItem() const
{
   if ((data[SIMPLE_POS] & 0x20) == 0x20)
      return true;

   return false;
}
//---------------------------------------------------------------------------
bool Item::isSocketed() const
{
   if ((data[SOCKETED_POS] & 0x08) == 0x08)
      return true;

   return false;
}
//---------------------------------------------------------------------------
int Item::size() const
{
   return num_of_bytes;
}
//---------------------------------------------------------------------------
/*
   Allocates memory for the item's data deleting any previous memory
   allocated for the data array
*/
void Item::size(int itemsize)
{
   num_of_bytes = itemsize;

   if (data)
      delete[] data;

   data = new unsigned char[itemsize];
}
//---------------------------------------------------------------------------
char Item::socketedItemCount() const
{
   char value = data[SOCKETED_COUNT_POS] & 0x0E;
   value >>= 1;
   return value;
}
//---------------------------------------------------------------------------
int Item::itemVersion() const
{
   switch(data[SIMPLE_POS] & 0x38)
   {
      case 0x00: return V100_ITEM;

      case 0x38: return V104_ITEM;
   }
}
