/*
    Diablo II Character Editor
    Copyright (C) 2021-2023 Walter Couto

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
        const std::string& GetGridDimensionsTxt()
        {
            static std::string defaultTxt;
            if (defaultTxt.empty())
            {
                std::stringstream ss;
                ss << u8"name\tversion\tX\tY\n";
                ss << u8"Stash\t0\t6\t4\n";
                ss << u8"Stash\t100\t6\t8\n";
                ss << u8"Stash\t5\t10\t10\n";
                ss << u8"Inventory\t0\t10\t4\n";
                ss << u8"Inventory\t100\t10\t4\n";
                ss << u8"Inventory\t5\t10\t4\n";
                ss << u8"Horadric Cube\t0\t3\t4\n";
                ss << u8"Horadric Cube\t100\t3\t4\n";
                ss << u8"Horadric Cube\t5\t3\t4\n";
                defaultTxt = ss.str();
            }
            return defaultTxt;
        }
    }
}