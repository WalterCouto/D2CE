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
        const std::string& GetBodyLocs()
        {
            static std::string defaultTxt;
            if (defaultTxt.empty())
            {
                std::stringstream ss;
                ss << u8"Body Location\tCode\n";
                ss << u8"None\t\n";
                ss << u8"Head\thead\n";
                ss << u8"Neck\tneck\n";
                ss << u8"Torso\ttors\n";
                ss << u8"Right Arm\trarm\n";
                ss << u8"Left Arm\tlarm\n";
                ss << u8"Right Ring\trrin\n";
                ss << u8"Left Ring\tlrin\n";
                ss << u8"Belt\tbelt\n";
                ss << u8"Feet\tfeet\n";
                ss << u8"Gloves\tglov\n";
                defaultTxt = ss.str();
            }
            return defaultTxt;
        }
    }
}