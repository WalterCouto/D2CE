/*
    Diablo II Character Editor
    Copyright (C) 2021-2022 Walter Couto

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/
//---------------------------------------------------------------------------

#include "pch.h"
#include <sstream>

namespace d2ce
{
    namespace TxtReaderHelper
    {
        const std::string& GetPlayerClass()
        {
            static std::string defaultTxt;
            if (defaultTxt.empty())
            {
                std::stringstream ss;
                ss << u8"Player Class\tCode\n";
                ss << u8"Amazon\tama\n";
                ss << u8"Sorceress\tsor\n";
                ss << u8"Necromancer\tnec\n";
                ss << u8"Paladin\tpal\n";
                ss << u8"Barbarian\tbar\n";
                ss << u8"Expansion\t\n";
                ss << u8"Druid\tdru\n";
                ss << u8"Assassin\tass\n";
                defaultTxt = ss.str();
            }
            return defaultTxt;
        }
    }
}