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
        const std::string& GetItemsCode()
        {
            static std::string defaultTxt;
            if (defaultTxt.empty())
            {
                std::stringstream ss;
                ss << u8"offset\tcode\n";
                ss << u8"0\t5\n";
                ss << u8"3\t750\n";
                ss << u8"7\t5000\n";
                ss << u8"11\t750\n";
                ss << u8"15\t50\n";
                ss << u8"18\t10\n";
                ss << u8"21\t35\n";
                ss << u8"24\t35\n";
                ss << u8"27\t35\n";
                ss << u8"30\t100\n";
                ss << u8"33\t100\n";
                ss << u8"36\t100\n";
                ss << u8"39\t750\n";
                ss << u8"43\t50\n";
                ss << u8"46\t10\n";
                ss << u8"49\t192\n";
                ss << u8"53\t30\n";
                ss << u8"56\t30\n";
                ss << u8"59\t160\n";
                ss << u8"63\t60\n";
                ss << u8"66\t60\n";
                ss << u8"69\t171\n";
                ss << u8"73\t100\n";
                ss << u8"76\t100\n";
                ss << u8"79\t192\n";
                ss << u8"83\t180\n";
                ss << u8"87\t180\n";
                ss << u8"91\t256\n";
                ss << u8"95\t320\n";
                ss << u8"99\t320\n";
                ss << u8"103\t128\n";
                ss << u8"107\t20\n";
                ss << u8"110\t20\n";
                ss << u8"113\t128\n";
                ss << u8"117\t40\n";
                ss << u8"120\t40\n";
                ss << u8"123\t128\n";
                ss << u8"127\t80\n";
                ss << u8"130\t80\n";
                ss << u8"133\t128\n";
                ss << u8"137\t150\n";
                ss << u8"141\t150\n";
                ss << u8"145\t128\n";
                ss << u8"149\t250\n";
                ss << u8"153\t250\n";
                ss << u8"157\t1000\n";
                ss << u8"161\t25\n";
                defaultTxt = ss.str();
            }
            return defaultTxt;
        }
    }
}