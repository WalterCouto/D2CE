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
        const std::string& GetSuperiorModsTxt()
        {
            static std::string defaultTxt;
            if (defaultTxt.empty())
            {
                std::stringstream ss;
                ss << u8"mod1code\tmod1param\tmod1min\tmod1max\tmod2code\tmod2param\tmod2min\tmod2max\tarmor\tweapon\tshield\tscepter\twand\tstaff\tbow\tboots\tgloves\tbelt\n";
                ss << u8"att\t0\t1\t3\t\t0\t\t\t0\t1\t0\t1\t1\t1\t1\t0\t0\t0\n";
                ss << u8"dmg%\t0\t5\t15\t\t0\t\t\t0\t1\t0\t1\t1\t1\t1\t0\t0\t0\n";
                ss << u8"ac%\t0\t5\t15\t\t0\t\t\t1\t0\t1\t0\t0\t0\t0\t1\t1\t1\n";
                ss << u8"att\t0\t1\t3\tdmg%\t0\t5\t15\t0\t1\t0\t1\t1\t1\t1\t0\t0\t0\n";
                ss << u8"dur%\t0\t10\t15\t\t\t\t\t1\t1\t1\t1\t1\t1\t1\t1\t1\t1\n";
                ss << u8"att\t0\t1\t3\tdur%\t0\t10\t15\t0\t1\t0\t1\t1\t1\t1\t0\t0\t0\n";
                ss << u8"dmg%\t0\t5\t15\tdur%\t0\t10\t15\t0\t1\t0\t1\t1\t1\t1\t0\t0\t0\n";
                ss << u8"ac%\t0\t5\t15\tdur%\t0\t10\t15\t1\t0\t1\t0\t0\t0\t0\t1\t1\t1\n";
                defaultTxt = ss.str();
            }
            return defaultTxt;
        }
    }
}